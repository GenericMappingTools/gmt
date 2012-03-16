#!/bin/bash
#
#	$Id$
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 1:
# Get the epochs (which now decodes to a rata die number and a day fraction
# which is 0.0 unless the epoch occurs during a day) from gmt_time_system.h
# and convert to relative time using the new TIME_SYSTEM rata.  The values
# should match the new rata die + day fraction for each epoch.

header "Test time conversions (rata die)"

( gmtconvert --TIME_SYSTEM=rata -fi0T -fo0t --FORMAT_FLOAT_OUT=%.12g | grep -v ">" | awk '{if ($1 != $2) print $0}' > fail ) <<%
 2000-01-01T12:00:00    730120.5
-4713-11-25T12:00:00  -1721423.5
 1858-11-17T00:00:00    678576.0
 1985-01-01T00:00:00    724642.0
 1970-01-01T00:00:00    719163.0
 0001-01-01T00:00:00         1.0
%

passfail time_testing_1
