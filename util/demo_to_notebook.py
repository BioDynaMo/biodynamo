import re
import os
import sys
import json
import time
import doctest
import textwrap
import subprocess
from nbformat import v3, v4
from datetime import datetime, date
import argparse

# Hacky solution to avoid picking up ParaView's python packages (i.e. pygments)
# that causes the notebooks to not be generated
try:
	os.environ['PYTHONPATH'] = os.environ['ROOTSYS'] + "/lib"
except:
	print("Error: ROOTSYS was not set. Please source thisbdm.sh.")
	exit(1)

# List of types that will be considered when looking for a C++ function. If a macro returns a
# type not included on the list, the regular expression will not match it, and thus the function
# will not be properly defined. Thus, any other type returned by function  must be added to this list
# for the script to work correctly.
gTypesList = ["inline int", "void", "int",
			  "string", "bool", "double", "float", "char"]


def Indent(string, spaces=2):
	new_string = ''
	lines = string.splitlines()
	skip = True
	for line in lines:
		if line == "" and skip:
			continue
		new_string += line + "\n"
		skip = False
	return new_string


def Unindent(string, spaces=2):
	"""
	Returns string with each line unindented by 2 spaces. If line isn't indented, it stays the same.
	>>> Unindent("   foobar")
	'foobar\\n'
	>>> Unindent("foobar")
	'foobar\\n'
	>>> Unindent('''foobar
	...    foobar
	... foobar''')
	'foobar\\nfoobar\\nfoobar\\n'
	"""
	newstring = ''
	lines = string.splitlines()
	for line in lines:
		if line.startswith(spaces*' '):
			newstring += (line[spaces:] + "\n")
		else:
			newstring += (line + "\n")

	return newstring


def ReadHeader(text):
	"""
	Extracts the description from the header, and removes the copyright notice
	"""
	lines = text.splitlines()

	# Skip copyright notice
	lines = lines[13:]

	newTitle = ""
	visualize = False

	description = ''
	for i, line in enumerate(lines):
		if line.startswith("// \\title "):
			newTitle = line[9:]
		elif line.startswith("// \\visualize"):
			visualize = True
		elif line.startswith("//"):
			if line == "//" or not line.startswith("// --"):
				description += ('# ' + line[3:] + '\n')
		else:
			break
	newtext = ''
	for line in lines[i:]:
		newtext += (line + "\n")
	description = description.replace("\\f$", "$")
	description = description.replace("\\f[", "$$")
	description = description.replace("\\f]", "$$")
	return newtext, description, newTitle, visualize


def ExtractMainFunction(text):
	"""
	Extracts the contents of the Simulate(argc, argv) function.
	"""
	functionContentRe = re.compile(
		r'{((\n|.)*)}', flags=re.DOTALL | re.MULTILINE)

	match = functionContentRe.search(text)

	text = match.group()
	text = text[1:-1]  # remove "{" and "}"

	new_text = ''
	for line in text.splitlines():
		if "argc, argv" in line:
			new_text += line.replace("argc, argv",
									 '"{}"'.format(tutName)) + "\n"
			continue
		if "return 0;" in line:
			new_text += '\n'
			continue
		new_text += line + "\n"
	return new_text


def Comments(text):
	"""
	Converts comments delimited by // and on a new line into a markdown cell.
	>>> Comments('''// This is a
	... // multiline comment
	... void function(){}''')
	'# <markdowncell>\\n# This is a\\n#  multiline comment\\n# <codecell>\\nvoid function(){}\\n'
	>>> Comments('''void function(){
	...    int variable = 5 // Comment not in cell
	...    // Comment also not in cell
	... }''')
	'void function(){\\n   int variable = 5 // Comment not in cell\\n   // Comment also not in cell\\n}\\n'
	"""
	text = text.splitlines()
	newtext = ''
	inComment = False

	for line in text:
		if line.startswith("//") and not inComment:  # True if first line of comment
			inComment = True
			newtext += "# <markdowncell>\n"
			# Don't use .capitalize() if line starts with hash, ie it is a header
			if line[2:].lstrip().startswith("#"):
				newtext += ("# " + line[2:]+"\n")
			else:
				newtext += ("# " + line[2:].lstrip().capitalize()+"\n")
		# True if first line after comment
		elif inComment and not line.startswith("//"):
			inComment = False
			newtext += "# <codecell>\n"
			newtext += (line+"\n")
		# True if in the middle of a comment block
		elif inComment and line.startswith("//"):
			newtext += ("# " + line[2:] + "\n")
		else:
			newtext += (line+"\n")

	return newtext


def split(text):
	"""
	Splits the text string into main, helpers, and rest. main is the main function,
	i.e. the function tha thas the same name as the macro file. Helpers is a list of
	strings, each a helper function, i.e. any other function that is not the main function.
	Finally, rest is a string containing any top-level code outside of any function.
	Comments immediately prior to a helper cell are converted into markdown cell,
	added to the helper, and removed from rest.
	Intended for C++ files only.
	>>> split('''void tutorial(){
	...    content of tutorial
	... }''')
	('void tutorial(){\\n   content of tutorial\\n}', [], '')
	>>> split('''void tutorial(){
	...    content of tutorial
	... }
	... void helper(arguments = values){
	...    helper function
	...    content spans lines
	... }''')
	('void tutorial(){\\n   content of tutorial\\n}', ['\\n# <markdowncell>\\n A helper function is created: \\n# <codecell>\\n%%cpp -d\\nvoid helper(arguments = values){\\n   helper function\\n   content spans lines\\n}'], '')
	>>> split('''#include <header.h>
	... using namespace NAMESPACE
	... void tutorial(){
	...    content of tutorial
	... }
	... void helper(arguments = values){
	...    helper function
	...    content spans lines
	... }''')
	('void tutorial(){\\n   content of tutorial\\n}', ['\\n# <markdowncell>\\n A helper function is created: \\n# <codecell>\\n%%cpp -d\\nvoid helper(arguments = values){\\n   helper function\\n   content spans lines\\n}'], '#include <header.h>\\nusing namespace NAMESPACE')
	>>> split('''void tutorial(){
	...    content of tutorial
	... }
	... // This is a multiline
	... // description of the
	... // helper function
	... void helper(arguments = values){
	...    helper function
	...    content spans lines
	... }''')
	('void tutorial(){\\n   content of tutorial\\n}', ['\\n# <markdowncell>\\n  This is a multiline\\n description of the\\n helper function\\n \\n# <codecell>\\n%%cpp -d\\nvoid helper(arguments = values){\\n   helper function\\n   content spans lines\\n}'], '')
	"""
	functionReString = "("
	for cpptype in gTypesList:
		functionReString += ("^%s|") % cpptype

	functionReString = functionReString[:-1] + \
		r")\s?\*?&?\s?[\w:]*?\s?\([^\)]*\)\s*\{.*?^\}"

	functionRe = re.compile(functionReString, flags=re.DOTALL | re.MULTILINE)
	#functionre = re.compile(r'(^void|^int|^Int_t|^TF1|^string|^bool|^double|^float|^char|^TCanvas|^TTree|^TString|^TSeqCollection|^Double_t|^TFile|^Long64_t|^Bool_t)\s?\*?\s?[\w:]*?\s?\([^\)]*\)\s*\{.*?^\}', flags = re.DOTALL | re.MULTILINE)
	functionMatches = functionRe.finditer(text)
	helpers = []
	main = ""
	for matchString in [match.group() for match in functionMatches]:
		if findFunctionName(matchString) == "Simulate":  # the main simulation function
			main = matchString
		else:
			helpers.append(matchString)

	# Create rest by replacing the main and helper functions with blank strings
	rest = text.replace(main, "")

	for helper in helpers:
		rest = rest.replace(helper, "")

	newHelpers = []
	lines = text.splitlines()
	for helper in helpers:      # For each helper function
		# Look through the lines until the
		for i, line in enumerate(lines):
			# first line of the helper is found
			if line.startswith(helper[:helper.find("\n")]):
				j = 1
				commentList = []
				# Add comment lines immediately prior to list
				while lines[i-j].startswith("//"):
					commentList.append(lines[i-j])
					j += 1
				if commentList:                  # Convert list to string
					commentList.reverse()
					helperDescription = ''
					for comment in commentList:
						if comment in ("//", "// "):
							helperDescription += "\n\n"  # Two newlines to create hard break in Markdown
						else:
							helperDescription += (comment[2:] + "\n")
							rest = rest.replace(comment, "")
					break
				else:   # If no comments are found create generic description
					helperDescription = "A helper function is created:"
					break

		if findFunctionName(helper) != "main":  # remove void main function
			newHelpers.append("\n# <markdowncell>\n " +
							  helperDescription + " \n# <codecell>\n%%cpp -d\n" + helper)

	headers = ''
	for line in rest.splitlines():
		if line.startswith("#include"):
			headers += line + "\n"
			rest = rest.replace(line, "")

	# remove newlines and empty comments at the end of string
	rest = rest.rstrip("\n /")

	return main, newHelpers, headers, rest


def findFunctionName(text):
	"""
	Takes a string representation of a C++ function as an input,
	finds and returns the name of the function
	>>> findFunctionName('void functionName(arguments = values){}')
	'functionName'
	>>> findFunctionName('void functionName (arguments = values){}')
	'functionName'
	>>> findFunctionName('void *functionName(arguments = values){}')
	'functionName'
	>>> findFunctionName('void* functionName(arguments = values){}')
	'functionName'
	>>> findFunctionName('void * functionName(arguments = values){}')
	'functionName'
	>>> findFunctionName('void class::functionName(arguments = values){}')
	'class::functionName'
	"""
	functionNameReString = "(?<="
	for cpptype in gTypesList:
		functionNameReString += ("(?<=%s)|") % cpptype

	functionNameReString = functionNameReString[:-
												1] + r")\s?\*?\s?[^\s]*?(?=\s?\()"

	functionNameRe = re.compile(
		functionNameReString, flags=re.DOTALL | re.MULTILINE)

	#functionnamere = re.compile(r'(?<=(?<=int)|(?<=void)|(?<=TF1)|(?<=Int_t)|(?<=string)|(?<=double)|(?<=Double_t)|(?<=float)|(?<=char)|(?<=TString)|(?<=bool)|(?<=TSeqCollection)|(?<=TCanvas)|(?<=TTree)|(?<=TFile)|(?<=Long64_t)|(?<=Bool_t))\s?\*?\s?[^\s]*?(?=\s?\()', flags = re.DOTALL | re.MULTILINE)
	match = functionNameRe.search(text)
	functionname = match.group().strip(" *\n")
	return functionname


def processmain(text):
	argumentsCell = ''

	if text:
		argumentsre = re.compile(
			r'(?<=\().*?(?=\))', flags=re.DOTALL | re.MULTILINE)
		arguments = argumentsre.search(text)

		if len(arguments.group()) > 3:
			# argumentsCell = "# <markdowncell> \n Arguments are defined. \n# <codecell>\n"
			# , flags = re.DOTALL) #| re.MULTILINE)
			individualArgumentre = re.compile(r'[^/\n,]*?=[^/\n,]*')
			argumentList = individualArgumentre.findall(arguments.group())
			for argument in argumentList:
				argumentsCell += argument.strip("\n ") + ";\n"
			# argumentsCell += "# <codecell>\n"

	return text, argumentsCell


def changeMarkdown(code):
	code = code.replace("~~~", "```")
	code = code.replace("{.cpp}", "cpp")
	code = code.replace("{.bash}", "bash")
	return code


def RemoveIncludeGuardsAndNamespace(text):
	lines = text.splitlines()
	new_text = ''
	for line in lines:
		if line.startswith("#ifndef DEMO_") or line.startswith("#define DEMO_") or line.startswith("#endif  // DEMO_"):
			continue
		elif line.startswith("namespace") or line.startswith("}  // namespace"):
			continue
		else:
			new_text += line + "\n"

	return new_text

# Creates the macro function declaration, such that `root -x function_name.C`
# can be used
def CreateMainFunction(content):
	signature = "void {}()".format(tutName) + " {\n"
	return signature + Indent(content) + "\n}\n"

# -------------------------------------
# ------------ Main Program------------
# -------------------------------------
def mainfunction(text, visualize):
	"""
	Main function. Calls all other functions. Also, it adds a cell that draws the result. The working text is
	then converted to a version 3 jupyter notebook, subsequently updated to a version 4. Then, metadata
	associated with the language the macro is written in is attatched to he notebook. Finally the
	notebook is executed and output as a Jupyter notebook.
	"""
	# Modify text from macros to suit a notebook
	main, helpers, headers, rest = split(text)
	main_macro = CreateMainFunction(Indent(ExtractMainFunction(main)))
	main,  argumentsCell = processmain(main)
	# Remove function, Unindent, and convert comments to Markdown cells
	main = Comments(Unindent(ExtractMainFunction(main)))
	rest = RemoveIncludeGuardsAndNamespace(rest)

	# Command for loading rootlogon.C
	libloading_macro = '%jsroot on\ngROOT->LoadMacro("${BDMSYS}/etc/rootlogon.C");\n\n'

	c_macro = headers + rest + main_macro
	with open(outPathNameMacro, 'w') as fout:
		fout.write(c_macro)

	if argumentsCell:
		main = argumentsCell + main

	if visualize:
		visComment = "# <markdowncell>\n Let's visualize the output!"
		main += '\n%s\n# <codecell>\nVisualizeInNotebook();\n' % visComment

	# Convert top level code comments to Markdown cells
	rest = Comments(rest)

	# Construct text by starting with top level code, then the helper functions, and finally the main function.
	# Also add cells for headerfile, or keepfunction
	text = "# <codecell>\n" + rest

	for helper in helpers:
		text += helper

	text += ("\n# <codecell>\n" + main)

	# Change to standard Markdown
	newDescription = changeMarkdown(description)

  # Horizontal title line
	hline = '<hr style="border-top-width: 4px; border-top-color: #34609b;">'

	# Add the title and header of the notebook
	text = "# <markdowncell> \n# # %s\n%s\n%s# \n# \n# <codecell>\n%s\n# <codecell>\n%s\n# <codecell>\n%s" % (
		tutTitle, hline, newDescription, libloading_macro, headers, text)

	# Create a notebook from the working text
	nbook = v3.reads_py(text)
	nbook = v4.upgrade(nbook)  # Upgrade v3 to v4

	# Load notebook string into json format, essentially creating a dictionary
	json_data = json.loads(v4.writes(nbook))

	# add the corresponding metadata
	json_data['metadata'] = {
		"kernelspec": {
			"display_name": "ROOT C++",
			"language": "c++",
			"name": "root"
		},
		"language_info": {
			"codemirror_mode": "text/x-c++src",
			"file_extension": ".C",
			"mimetype": " text/x-c++src",
			"name": "c++"
		}
	}

	# write the json file with the metadata
	with open(outPathName, 'w') as fout:
		json.dump(json_data, fout, indent=1, sort_keys=True)

	timeout = 60

	execute = "--execute"
	if args.skip:
		execute = ""

	# Call commmand that executes the notebook and creates a new notebook with the output
	nbconvert_cmd = "jupyter nbconvert --to=html --ExecutePreprocessor.timeout=%d %s %s" % (timeout, execute, outPathName)
	r = subprocess.call(["jupyter", "nbconvert", "--to=html", "--ExecutePreprocessor.timeout=%d" %
						 timeout,  execute,  outPathName])

	if r != 0:
		sys.stderr.write(
			"NOTEBOOK_CONVERSION_ERROR: nbconvert failed for notebook %s with return code %s\n" % (outname, r))
		sys.stderr.write("FAILED COMMAND: %s\n" % nbconvert_cmd)
		exit(1)


if __name__ == "__main__":

	parser = argparse.ArgumentParser()
	parser.add_argument('--tutpath', type=str)
	parser.add_argument('--outdir', type=str)
	parser.add_argument('--skip', action='store_true')

	args = parser.parse_args()

	tutPathName = args.tutpath
	tutPath = os.path.dirname(tutPathName)
	tutFileName = os.path.basename(tutPathName)
	tutName, extension = tutFileName.split(".")
	tutTitle = re.sub(r"([A-Z\d])", r" \1", tutName).title()
	tutTitle = tutTitle.replace("_", " ")
	outname = tutName + ".ipynb"
	outnameMacro = tutName + ".C"
	outnameconverted = tutName + ".html"

	# Extract output directory
	if args.outdir:
		outdir = args.outdir
	else:
		outdir = tutPath
	outPathName = os.path.join(outdir, outname)
	outPathNameMacro = os.path.join(outdir, outnameMacro)
	date = datetime.now().strftime("%A, %B %d, %Y at %I:%M %p")

	# Open the file to be converted
	with open(tutPathName) as fin:
		text = fin.read()

	text, description, newTitle, visualize = ReadHeader(text)

	if newTitle != "":
		tutTitle = newTitle

	starttime = time.time()
	mainfunction(text, visualize)
