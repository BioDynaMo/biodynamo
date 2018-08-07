#!/usr/bin/env python
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

"""A simple utility to parse timing log from Ray driver script.

The log could be produced by `script` command, e.g.::

    script timing.log -c "python driver.py -l $PWD/build/libdistributed-ray.so"

The parsed values will be written out to stdout in CSV format so that they can
be analysed right away, e.g.::

    python parse_timing_log.py timing.log > timing.csv
"""

import argparse
import sys


def parse_timing_values(lines):
    """Returns two lists, one for reassemble, another for disassemble times.

    There is no implied ordering among the elements in the same list, or
    across both lists.
    """

    reassemble = []
    disassemble = []
    for line in lines:
        if line.startswith('Reassemble time '):
            reassemble.append(float(line[line.rindex(' ') + 1 :]))
        if line.startswith('Disassemble time '):
            disassemble.append(float(line[line.rindex(' ') + 1 :]))
    return reassemble, disassemble


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('log_path', help='Path to the log file.')
    args = parser.parse_args()
    with open(args.log_path, 'r') as lines:
        reassemble, disassemble = parse_timing_values(lines)
    assert(len(reassemble) == len(disassemble))
    print('Reassemble,Disassemble')
    for r, d in zip(reassemble, disassemble):
        print('{},{}'.format(r, d))


if __name__ == '__main__':
    main()
