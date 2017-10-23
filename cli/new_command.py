import os
import getpass
import re
import requests
import subprocess as sp
import sys

def ValidateSimName(sim_name):
    pattern = re.compile("^[a-zA-Z]+[a-zA-Z0-9\-_]+$")
    if not pattern.match(sim_name):
        print("Error: simulation name '{0}' is not valid.".format(sim_name))
        print("       Allowed characters are a-z A-Z 0-9 - and _")
        print("       Must start with a-z or A-Z")
        sys.exit(1)

def CloneTemplateRepository(sim_name):
    try:
        sp.check_output(["git", "clone", "https://github.com/BioDynaMo/simulation-templates.git", sim_name])
    except sp.CalledProcessError as err:
        print("Error while checkout out the template project from BioDynaMo git repository")
        sys.exit(1)

def RemoveGitFolder(sim_name):
    sp.check_output(["rm", "-rf", sim_name+"/.git"])


def ModifyFileContent(filename, fn):
    with open(filename) as f:
        content = f.read()

    content = fn(content)

    with open(filename, "w") as f:
        f.write(content)

def CustomizeFiles(sim_name):
    # README.md
    ModifyFileContent(sim_name + "/README.md", lambda content: "# " + sim_name + "\n")

    # CMakelists.txt
    ModifyFileContent(sim_name + "/CMakeLists.txt", lambda content: content.replace("my-simulation", sim_name))

    # source files
    include_guard = sim_name.upper().replace("-", "_") + "_H_"
    ModifyFileContent(sim_name + "/src/my-simulation.h", lambda c: c.replace("MY_SIMULATION_H_", include_guard))
    ModifyFileContent(sim_name + "/src/my-simulation.cc", lambda c: c.replace("my-simulation", sim_name))
    #   rename
    os.rename(sim_name + "/src/my-simulation.h", sim_name + "/src/" + sim_name + ".h")
    os.rename(sim_name + "/src/my-simulation.cc", sim_name + "/src/" + sim_name + ".cc")

def InitializeNewGitRepo(sim_name):
    sp.check_output(["git", "init"], cwd=sim_name)

    # check if user name and email are set
    try:
        out = sp.check_output(["git", "config", "user.name"], cwd=sim_name).decode("utf-8")
    except sp.CalledProcessError as err:
        # User name not set
        print("Git user name is not set.")
        response = input("Please enter your name: ")
        out = sp.check_output(["git", "config", "user.name", response], cwd=sim_name)

    try:
        out = sp.check_output(["git", "config", "user.email"], cwd=sim_name).decode("utf-8")
    except sp.CalledProcessError as err:
        # User name not set
        print("Git user e-mail is not set.")
        response = input("Please enter your e-mail: ")
        out = sp.check_output(["git", "config", "user.email", response], cwd=sim_name)

    sp.check_output(["git", "add", "."], cwd=sim_name)
    sp.check_output(["git", "commit", "-m", "\"Initial commit\""], cwd=sim_name)

def CreateNewGithubRepository(sim_name):
    gh_user = input("Username for 'https://github.com': ")
    repo_url = "https://github.com/" + gh_user + "/" + sim_name + ".git"
    try:
        gh_pass = getpass.getpass("Password for 'https://" + gh_user + "@github.com':")
        url     = 'https://api.github.com/user/repos'
        r = requests.post(url, auth=(gh_user, gh_pass), data='{"name":"' + sim_name + '", "description": "Simulation powered by BioDynaMo"}')
        gh_pass = "123456789abcdefgh"
        if r.status_code != 201:
            print("Github repository creation failed.")
            print(r.status_code)
            print(r.text)
            sys.exit(1)
    except sp.CalledProcessError as err:
        print("Github repository creation failed.")
        print(err)
        sys.exit(1)

    try:
        sp.check_output(["git", "remote", "add", "origin", repo_url], cwd=sim_name)
    except sp.CalledProcessError as err:
        print("Setting remote to github url ({0}) failed.".format(repo_url))
        sys.exit(1)

def PushChanges(sim_name):
    sp.check_output(["git", "push", "origin", "master"], cwd=sim_name)

def NewCommand(sim_name):
    print("new " + sim_name)
    # TODO Message that user needs an github account

    ValidateSimName(sim_name)
    CloneTemplateRepository(sim_name)
    RemoveGitFolder(sim_name)
    CustomizeFiles(sim_name)
    InitializeNewGitRepo(sim_name)
    CreateNewGithubRepository(sim_name)
    PushChanges(sim_name)

    print(sim_name + " has been created successfully!")
