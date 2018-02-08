#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import StringIO
import os.path
import datetime

def remove_old_kernel(content, kernel_name):
    kernel_start_pos = content.rfind("// kernel " + kernel_name + " start \n")

    if kernel_start_pos != - 1:
        match = "// kernel " + kernel_name + " end \n"
        kernel_end_pos = content.rfind(match)
        content = content[:kernel_start_pos] + content[kernel_end_pos + len(match):]

    return content

def stringify_kernel(kernel_file, header_file):
    if os.path.exists(kernel_file) == False:
        print "The kernel file does not exist!"
        return

    kernel = open(kernel_file, "r")
    header = open(header_file, "w+")

    header_content = header.read()
    kernel_content = kernel.read()

    header.seek(0)
    header.truncate()

    kernel_name = os.path.splitext(os.path.basename(kernel_file))[0]
    header_content = remove_old_kernel(header_content, kernel_name)

    pos = header_content.rfind("#endif")

    modified_content = header_content[:pos]

    modified_content += "// kernel " + kernel_name + " start \n" \
                     +  "// kernel stringified at " \
                     +  datetime.datetime.now().strftime("%Y-%m-%d %H:%M") + "\n\n"

    modified_content += "#ifndef " + kernel_name.upper() + "_H_\n"
    modified_content += "#define " + kernel_name.upper() + "_H_\n\n"

    modified_content += "const char* const " + kernel_name + " = "

    for line in StringIO.StringIO(kernel_content):
        modified_content += ('"' + line.rstrip() + '\n"'.encode('string_escape')
 + "\n")

    modified_content += ";\n" \
                     + "// kernel " + kernel_name + " end \n\n" \
                     + header_content[pos:]

    modified_content += "#endif  // " + kernel_name.upper() + "_H_\n"

    header.write(modified_content)

    kernel.close()
    header.close()


if __name__ == "__main__":
    if len(sys.argv) == 3:
        stringify_kernel(sys.argv[1], sys.argv[2])