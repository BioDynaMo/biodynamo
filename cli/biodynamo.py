#!/usr/bin/env python3

# from optparse import OptionParser
import argparse
# import new_command
from new_command import New

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='biodynamo')
    sp = parser.add_subparsers(dest='cmd')
    for cmd in ['assist', 'build', 'run', 'star']:
        sp.add_parser(cmd)
    for cmd in ['new']:
        spp = sp.add_parser(cmd)
        spp.add_argument('SIMULATION_NAME', type=str)

    args = parser.parse_args()

    if args.cmd == 'new':
        NewCommand(args.SIMULATION_NAME)
