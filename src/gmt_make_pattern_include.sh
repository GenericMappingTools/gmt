#!/usr/bin/env bash
#
# Copyright (c) 2012-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script just makes the include file PSL_patterns.h
# used by postscriptlight.c.  We only ran this once but save for
# posterity in case we need to do something similar. To avoid
# it being run I have added the next two lines
echo "gmt_make_pattern_include.sh.sh - Generate PSL_patterns.h"
echo "Not use, exiting here"
exit
#
COPY_YEAR=$(date +%Y)
cat << EOF > PSL_patterns.h
/*--------------------------------------------------------------------
 *
 *      Copyright (c) 2009-$COPY_YEAR by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *--------------------------------------------------------------------*/

/* The 90 former Sun rasterfile patterns are now stored here as static char arrays.
 * They are all 64x64 1-bit patterns, so need 64*64/8 = 512 bytes each.
 */

static unsigned char PSL_pattern[90][512] = {
EOF
let k=1
while [ $k -le 90 ]; do
	name=`echo $k | awk '{printf "PSL_pattern_%2.2d.ras\n", $1}'`
	printf "\t{\t/* $name */\n" >> PSL_patterns.h
	od -j32 -t uC -v -An $name | sed '/^$/d' | awk '{printf "\t\t%3d", $1}; {for (i=2; i<=NF;i++) printf ", %3d", $i}; {printf ",\n"}' >> PSL_patterns.h
	printf "\t},\n" >> PSL_patterns.h
	let k=k+1
done
printf "};\n" >> PSL_patterns.h
