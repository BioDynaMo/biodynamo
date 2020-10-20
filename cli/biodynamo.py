#!/usr/bin/env python3
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


import argparse
import sys
from assist_command import AssistCommand
from build_command import BuildCommand
from demo_command import DemoCommand
from new_command import NewCommand
from run_command import RunCommand
from version import Version

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='biodynamo',
        description='This is the BioDynaMo command line interface. It guides '
                    'you during the whole simulation workflow. From starting '
                    'a new project, to compiling and executing, all the way '
                    'to requesting assistance from the BioDynaMo developers.',
        epilog='')

    sp = parser.add_subparsers(dest='cmd')
    parser.add_argument('-v', '--version',
                        action='store_true',
                        help='Display BioDynaMo version')
    parser.add_argument('--shortversion',
                        action='store_true',
                        help='Display BioDynaMo short version')


    assist_sp = sp.add_parser('assist', help='Use this command if you need help from the '
                                 'BiodynaMo developers. This command helps you '
                                 'to gather information which is required to '
                                 'reproduce and debug your issue. First, '
                                 'it will ask you to commit all uncommited changes. '
                                 'Afterwards, it will attempt to build and run your '
                                 'simulation. During this process it will collect '
                                 'logs which will be commited and uploaded in a '
                                 'seperate git branch. In the end it will output '
                                 'a link that you should add to your e-mail or '
                                 'slack message when you describe your issue.' )

    build_sp = sp.add_parser('build', help='Builds the simulation binary')

    clean_sp = sp.add_parser('clean', help='Removes all build files')

    demo_sp = sp.add_parser('demo', help='Creates pre-built demos.')

    new_sp = sp.add_parser('new', help='Creates a new simulation project. Creates '
    'a template project, renames it to the given simulation name, '
    'configures git.')
    new_sp.add_argument('SIMULATION_NAME', type=str, help='simulation name help')
    new_sp.add_argument('--github', action='store_true', help='Create a Github repository.'    )

    run_sp = sp.add_parser('run', help='Executes the simulation')

    args, unknown = parser.parse_known_args()

    if args.cmd == 'new':
        if len(unknown) != 0:
            new_sp.print_help()
            sys.exit()
        NewCommand(args.SIMULATION_NAME, args.github)
    elif args.cmd == 'build':
        if len(unknown) != 0:
            build_sp.print_help()
            sys.exit()
        BuildCommand()
    elif args.cmd == 'clean':
        if len(unknown) != 0:
            clean_sp.print_help()
            sys.exit()
        BuildCommand(clean=True, build=False)
    elif args.cmd == 'demo':
        demo_name = None
        destination = None
        if len(unknown) >= 1:
            demo_name = unknown[0]
        if len(unknown) >= 2:
            destination = unknown[1]
        DemoCommand(demo_name, destination)
    elif args.cmd == 'run':
        RunCommand(args=unknown)
    elif args.cmd == 'assist':
        if len(unknown) != 0:
            assist_sp.print_help()
            sys.exit()
        AssistCommand()
    elif args.version:
        print(Version.string())
        sys.exit()
    elif args.shortversion:
        print(Version.shortstring())
        sys.exit()
    else:
        parser.print_help()
