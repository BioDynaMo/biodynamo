#!/bin/bash
# script to find the first difference in two very large files (java and cpp)
# paginates the file and compares them page per page
# after the first page shows a difference it stops and outputs the page number
# as well as the content of the pages
#
#
# ./find_first_diff.sh FILE1 FILE2 [STARTING_PAGE LINES]
# PARAMS:
#   FILE1 FILE2    two large files that should be inspected for differences
#   STARTING_PAGE  page number that should be extracted (default 0)
#   LINES          specifies the number of lines per page (default 100000)
#                  if LINES paramater is used also STARTING_PAGE must be specified
# OUTPUT:
#   outputs the number of the page that contains the first difference as well
#   as the number of differences in that page to stdout
#   furthermore it creates two files "FILE1_page" and "FILE2_page" which contain
#   the lines with the first differences.
#
# usage example:
# ./find_first_diff.sh java cpp 1 100000

FILE1=$1
FILE2=$2
I=${3:-0}
LINES=${4:-100000}

DIFFS=0
while [ $DIFFS -eq 0 ]; do
  echo "processing page: ${I}"  
  ./paginate.sh $FILE1 $I $LINES >${FILE1}_page
  ./paginate.sh $FILE2 $I $LINES >${FILE2}_page
  DIFFS=$(diff ${FILE1}_page ${FILE2}_page | wc -l)

  I=$(($I+1))
done
I=$(($I-1))
echo "first diff found on page ${I}"
echo "#diffs: ${DIFFS}"
