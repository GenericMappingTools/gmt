#!/bin/bash
#
#	$Id $
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 7:
# Deal with parsing of date strings with 2-digit years and month names.

cat <<% > tt7.txt
2012-01-24T00:00:00
2012-02-24T00:00:00
2012-03-24T00:00:00
2012-04-24T00:00:00
2012-05-24T00:00:00
2012-06-24T00:00:00
2012-07-24T00:00:00
2012-08-24T00:00:00
2012-09-24T00:00:00
2012-10-24T00:00:00
2012-11-24T00:00:00
2012-12-24T00:00:00
%
gmt gmtconvert --TIME_SYSTEM=rata -f0T --FORMAT_DATE_IN=yy-o-dd <<% > tt7.d
12-JAN-24
12-FEB-24
12-MAR-24
12-APR-24
12-MAY-24
12-JUN-24
12-JUL-24
12-AUG-24
12-SEP-24
12-OCT-24
12-NOV-24
12-DEC-24
%
paste tt7.txt tt7.d | $AWK '{if ($1 != $2) print $0}' > fail
