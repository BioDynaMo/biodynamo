# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

import os
import getpass
import re
import json
import urllib.request
import urllib.error
import http.client
import base64
import subprocess as sp
import sys
import time
from print_command import Print

def ValidateSimName(sim_name):
    pattern = re.compile("^[a-zA-Z]+[a-zA-Z0-9\-_]+$")
    if not pattern.match(sim_name):
        Print.error("Error: simulation name '{0}' is not valid.".format(sim_name))
        Print.error("       Allowed characters are a-z A-Z 0-9 - and _")
        Print.error("       Must start with a-z or A-Z")
        sys.exit(1)

## Removes any created files during NewCommand and exits the program
def CleanupOnError(sim_name):
    try:
        sp.check_output(["rm", "-rf", sim_name])
    except:
        Print.error("Error: Failed to remove folder {0}".format(sim_name))
    sys.exit(1)

def CopyTemplate(sim_name):
    Print.new_step("Copy simulation template")
    try:
        src_path = "{0}/biodynamo/simulation-template".format(os.environ['BDM_INSTALL_DIR'])
        sp.check_output(["cp", "-R", src_path, "."])
        sp.check_output(["mv", "simulation-template", sim_name])
    except sp.CalledProcessError as err:
        Print.error("Error while copying the template project.")
        # Do not use CleanupOnError here
        # One failure could be an already existing directory
        # we must not remove it
        sys.exit(1)

def ModifyFileContent(filename, fn):
	with open(filename) as f:
		content = f.read()

	content = fn(content)

	with open(filename, "w") as f:
		f.write(content)

def CustomizeFiles(sim_name):
    Print.new_step("Customize files")
    try:
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
    except:
        Print.error("Error: File customizations failed")
        CleanupOnError(sim_name)

def InitializeNewGitRepo(sim_name):
    Print.new_step("Initialize new git repository")
    sp.check_output(["git", "init"], cwd=sim_name)

    # check if user name and email are set
    try:
        out = sp.check_output(["git", "config", "user.name"], cwd=sim_name).decode("utf-8")
    except sp.CalledProcessError as err:
        # User name not set
        print("Your git user name is not set.")
        response = input("Please enter your name (e.g. Mona Lisa): ")
        out = sp.check_output(["git", "config", "user.name", response], cwd=sim_name)

    try:
        out = sp.check_output(["git", "config", "user.email"], cwd=sim_name).decode("utf-8")
    except sp.CalledProcessError as err:
        # User name not set
        print("Your git user e-mail is not set.")
        response = input("Please enter your e-mail address: ")
        out = sp.check_output(["git", "config", "user.email", response], cwd=sim_name)

    sp.check_output(["git", "add", "."], cwd=sim_name)
    sp.check_output(["git", "commit", "-m", "\"Initial commit\""], cwd=sim_name)

def CreateNewGithubRepository(sim_name):
    Print.new_step("Create Github repository")
    gh_user = input("Please enter your Github username: ")
    gh_pass = getpass.getpass("Please enter your Github password: ")

    # create new github repo
    try:
        data = {"name": sim_name , "description": "Simulation powered by BioDynaMo"}
        headers = {'Content-Type': 'application/json'}
        bytes = json.dumps(data).encode('utf-8')
        url = "https://api.github.com/user/repos"

        request = urllib.request.Request(url, data=bytes, headers=headers)

        credentials = ('%s:%s' % (gh_user, gh_pass))
        encoded_credentials = base64.b64encode(credentials.encode('ascii'))
        request.add_header('Authorization', 'Basic %s' % encoded_credentials.decode("ascii"))
        result = urllib.request.urlopen(request)
    except urllib.error.HTTPError as err:
        Print.error("Github repository creation failed.")
        Print.error(err)
        CleanupOnError(sim_name)

    # Connect github repository with local
    try:
        repo_url = "https://github.com/" + gh_user + "/" + sim_name + ".git"
        sp.check_output(["git", "remote", "add", "origin", repo_url], cwd=sim_name)
    except sp.CalledProcessError as err:
        Print.error("Error: Setting remote github url ({0}) failed.".format(repo_url))
        CleanupOnError(sim_name)

def NewCommand(sim_name, github):
    if github:
        print("Info: This command requires a Github.com account.")
        print("      Please have your account details ready, or ")
        print("      go over to https://github.com/join to sign up.")

    ValidateSimName(sim_name)
    CopyTemplate(sim_name)
    CustomizeFiles(sim_name)
    InitializeNewGitRepo(sim_name)
    if github:
        CreateNewGithubRepository(sim_name)

    Print.success(sim_name + " has been created successfully!")
    print('To compile and run this simulation, change the directory by calling '
            '"cd %s"' % (sim_name))
