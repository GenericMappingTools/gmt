#!/usr/bin/env bash
#
# Copyright (c) 2012-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script determines which remote data files are used across
# all the GMT example, doc, and test scripts and writes unique list to stdout.
#
#NOTE: Currently, we exclude anim01-15.  As new animations are added, please make adjustments

if [ ! -d cmake ]; then
	echo "remote_data_use.sh: Must be run from top-level gmt directory" >&2
	exit 1
fi

# 1. Find all occurrences of remote grids in all scripts except anim01-anim15 but skip mention in comments
find doc test -name '*.sh' -exec egrep -H '@earth_relief|@earth_mask|@earth_day|@earth_night|@earth_age' {} \; | grep -v '\.sh:#' | grep -v 'anim1[0-5].sh' | grep -v 'anim[1-9].sh' > /tmp/t1.lis
# 2. Find the individual words starting with "@" but skip anything that has a variable name.
awk '{for (k = 1; k <= NF; k++) if (substr ($k, 1, 1) == "@") print $k}' /tmp/t1.lis | egrep -v '\$|cpt' > /tmp/t2.lis
awk '{for (k = 1; k <= NF; k++) if (substr ($k, 3, 1) == "@") print substr ($k,3)}' /tmp/t1.lis | egrep -v '\$|cpt' >> /tmp/t2.lis
sort -u /tmp/t2.lis
rm -f /tmp/t[12].lis
