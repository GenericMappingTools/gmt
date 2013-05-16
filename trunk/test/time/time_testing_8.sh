#!/bin/bash
#
#	$Id $
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 8:
# Deal with parsing of date strings with 2-digit years and month names and no delimiters.

cat <<% > tt8.txt
2010-01-22T00:00:00
2010-02-22T00:00:00
2010-03-22T00:00:00
2010-04-22T00:00:00
2010-05-22T00:00:00
2010-06-22T00:00:00
2010-07-22T00:00:00
2010-08-22T00:00:00
2010-09-22T00:00:00
2010-10-22T00:00:00
2010-11-22T00:00:00
2010-12-22T00:00:00
%
gmt gmtconvert --TIME_SYSTEM=rata -f0T --FORMAT_DATE_IN=yyodd <<% > tt8.d
10jan22
10feb22
10mar22
10apr22
10may22
10jun22
10jul22
10aug22
10sep22
10oct22
10nov22
10dec22
%
paste tt8.txt tt8.d | $AWK '{if ($1 != $2) print $0}' > fail
