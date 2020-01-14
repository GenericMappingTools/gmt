#!/usr/bin/env bash
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script downloads any missing cache or data sets from the remote server.
#
# Run this script on the command line with:
#   $(gmt --show-sharedir)/tools/gmt_getremote.sh [cache|data]
#
# It expects the GMT executable to be in the search path and that
# you have the curl executable installed.

# check for bash
[ -z "$BASH_VERSION" ] && return

if ! [ -x "$(command -v gmt)" ]; then
	echo 'Error: gmt is not found in your search PATH.' >&2
	exit 1
fi
if ! [ -x "$(command -v curl)" ]; then
	echo 'Error: curl is not found in your search PATH.' >&2
	exit 1
fi

# By default download both cache (=1) and data (=1)
data=1
cache=1
if [ "X$1" = "Xcache" ]; then	# Not do data
	data=0
elif [ "X$1" = "Xdata" ]; then	# Not do cache
	cache=0
fi

# Get current remote data server
SERVER=`gmt get GMT_DATA_SERVER`
curl -sk ${SERVER}/gmt_hash_server.txt > /tmp/gmt_hash_server.txt
if [ $? -ne 0 ]; then
	echo "Error: curl failed with error $?" >&2
	exit 1
fi

if [ ${cache} -eq 1 ]; then
	echo "Download all cache files from ${SERVER} not in ~/.gmt/cache (be patient)" >&2
	awk 'NF==3 && $1!~/^earth_relief/ {print "@"$1}' /tmp/gmt_hash_server.txt | xargs gmt which -Gc
fi
if [ ${data} -eq 1 ]; then
	echo "Download all remote DEM grids from ${SERVER} not in ~/.gmt/server (be very patient)" >&2
	awk 'NF==3 && $1~/earth_relief/ {print "@"$1}' /tmp/gmt_hash_server.txt | xargs gmt which -Gu
fi
rm -f /tmp/gmt_hash_server.txt
