#!/bin/sh
#
#	$Id: time_testing_1.sh,v 1.1 2007-05-28 19:40:30 pwessel Exp $
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 1:
# Get the epochs (which now decodes to a rata die number and a day fraction
# which is 0.0 unless the epoch occurs during a day) from gmt_time_system.h
# and convert to relative time using the new TIME_SYSTEM rata.  The values
# should match the new rata die + day fraction for each epoch.

echo -n "GMT: Test time conversions, part 1 (rata die):		"

sed -e 's/"//g' ../../src/gmt_time_systems.h | awk -F, '{if (NR > 1 && NR < 9) print $2, 1, $4, $5 }' | gmtconvert --TIME_SYSTEM=rata -fi0T -fo0t --D_FORMAT=%.12g | awk '{if ($1 != ($3+$4)) print $0}' > log
if [ -s log ]; then
        echo "[FAILED]"
else
        echo "[OK"]
        rm -f log
fi
