#!/usr/bin/env python3

import argparse
from new_command import NewCommand
from build_command import BuildCommand
from run_command import RunCommand
from assist_command import AssistCommand

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='biodynamo',
        description='This is the BioDynaMo command line interface. It guides '
                    'you during the whole simulation workflow. From starting '
                    'a new project, to compiling and executing, all the way '
                    'to requesting assistance from the BioDynaMo developers.',
        epilog='')

    sp = parser.add_subparsers(dest='cmd')

    sp.add_parser('assist', help='Use this command if you need help from the '
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

    sp.add_parser('build', help='Builds the simulation binary')

    sp.add_parser('clean', help='Removes all build files')

    spp = sp.add_parser('new', help='Creates a new simulation project. Downloads '
    'a template project from BioDynaMo, renames it to the given simulation name, '
    'creates a new Github repository and configures git.')
    spp.add_argument('SIMULATION_NAME', type=str, help='simulation name help')
    spp.add_argument('--no-github', action='store_true', help='Do not create a Github repository.'    )

    sp.add_parser('run', help='Executes the simulation')

    args = parser.parse_args()

    if args.cmd == 'new':
    	NewCommand(args.SIMULATION_NAME, args.no_github)
    elif args.cmd == 'build':
    	BuildCommand()
    elif args.cmd == 'clean':
    	BuildCommand(clean=True, build=False)
    elif args.cmd == 'run':
    	RunCommand()
    elif args.cmd == 'assist':
    	AssistCommand()
    else:
        parser.print_help()
