#!/bin/bash
# this bash script takes a large text input file and divides them into a number
# of pages. One page specified by a parameter is written to stdout.
#
# ./paginate.sh INPUT_FILE [PAGE LINE_NUMBERS]
# PARAMS:
#   INPUT_FILE path to the file that should be processed
#   PAGE       page number that should be extracted (default 0)
#   LINES      specifies the number of lines per page (default 10000)
#              if LINES paramater is used also page must be specified
# usage example:
# ./paginate.sh large_file 1 100000 >page_1

PAGE=${2:-0}
LINES=${3:-10000}

FROM=$(($PAGE*$LINES))

cat $1 | tail -n +$FROM | head -n $LINES
