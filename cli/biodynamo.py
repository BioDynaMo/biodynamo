#!/usr/bin/env python3

# from optparse import OptionParser
import argparse
# import new_command
from new_command import NewCommand
from build_command import BuildCommand
from run_command import RunCommand
from assist_command import AssistCommand

if __name__ == '__main__':
	parser = argparse.ArgumentParser(prog='biodynamo')
	sp = parser.add_subparsers(dest='cmd')
	for cmd in ['build', 'clean', 'run', 'assist']:
		sp.add_parser(cmd)
	for cmd in ['new']:
		spp = sp.add_parser(cmd)
		spp.add_argument('SIMULATION_NAME', type=str)

	args = parser.parse_args()

	if args.cmd == 'new':
		NewCommand(args.SIMULATION_NAME)
	if args.cmd == 'build':
		BuildCommand()
	if args.cmd == 'clean':
		BuildCommand(clean=True, build=False)
	if args.cmd == 'run':
		RunCommand()
	if args.cmd == 'assist':
		AssistCommand()