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

import json
import urllib.request
import urllib.error
import base64
import getpass

import subprocess as sp
from print_command import Print

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
