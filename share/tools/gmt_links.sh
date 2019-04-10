#!/usr/bin/env bash
#
# Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates, removes, or lists symbolic links for each GMT
# module to the main GMT executable that allow users to run modules
# directly.
#
# Run this script on the command line with:
#   $(gmt --show-datadir)/tools/gmt_links.sh create|remove
#
# With no arguments we simply check for the links.
#
# It expects the GMT executable to be in the search path and that
# you have permission to perform the changes in the bin directory.

# check for bash
[ -z "$BASH_VERSION" ] && return

if [ "X$1" = "Xdelete" ]; then
	mode=1
elif [ "X$1" = "Xcreate" ]; then
	mode=2
else
	mode=0
fi
bin=`gmt --show-bindir`
cwd=`pwd`

gmt_modules=`gmt --show-modules`
compat_modules="minmax gmt2rgb gmtstitch gmtdp grdreformat ps2raster"

cd $bin
for module in ${gmt_modules} ${compat_modules}; do
	if [ $mode -eq 1 ]; then	# Delete links
		rm -f $module
	elif [ $mode -eq 2 ]; then	# Create new links (remove old if present)
		ln -sf gmt $module
	else				# List what we find
		if [ -h $module ]; then
			printf "Link for module %16s: %s\n" $module "Present"
		else
			printf "Link for module %16s: %s\n" $module "Absent"
		fi
	fi
done
cd $cwd
