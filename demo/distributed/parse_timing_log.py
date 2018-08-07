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
import collections
import re
import sys


def parse_timing_values(lines):
    """Returns a dictionary of lists of timing values.

    There is no implied ordering among the elements in the same list, or
    across lists.
    """

    pattern = re.compile(r'(.*?) time (\d+)\s?ms')
    ret = collections.defaultdict(list)
    for line in lines:
        matches = pattern.match(line)
        if matches is not None:
            kind = matches.group(1)
            value = float(matches.group(2))
            ret[kind].append(value)
    return ret


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('log_path', help='Path to the log file.')
    args = parser.parse_args()
    with open(args.log_path, 'r') as lines:
        timing_values = parse_timing_values(lines)
    kinds = timing_values.keys()
    kinds.sort()
    values = [timing_values[k] for k in kinds]
    print(','.join(kinds))
    for i in range(len(values[0])):
        print(','.join(str(vs[i]) if len(vs) > i else '' for vs in values))


if __name__ == '__main__':
    main()
