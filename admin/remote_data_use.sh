#!/usr/bin/env bash
#
# Copyright (c) 2012-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script determines which remote data files are used across
# all the GMT example, doc, and test scripts and writes unique list to stdout.

if [ ! -d cmake ]; then
	echo "remote_data_use.sh: Must be run from top-level gmt directory" >&2
	exit 1
fi

# 1. Find all occurrences of remote grids but skip comments
find doc test -name '*.sh' -exec egrep '@earth_relief|@earth_mask|@earth_day|@earth_night|@earth_age' {} \; | grep -v '^#' > /tmp/t.lis
# 2. Find the individual words starting with "@" but skip anything that has a variable name.
awk '{for (k = 1; k <= NF; k++) if (substr ($k, 1, 1) == "@") print $k}' /tmp/t.lis | grep -v '\$' | sort -u
rm -f /tmp/t.lis
