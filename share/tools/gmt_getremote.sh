#!/usr/bin/env bash
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script downloads any missing cache or data sets from the remote server.
#
# Run this script on the command line with:
#
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
SERVER=$(gmt get GMT_DATA_SERVER)

# The "gmt which" command
gmtwhich="gmt which -Gu"
#Uncomment the line below for a dry-run
# gmtwhich="printf %s\n"

# Download the data hash table
curl -sk ${SERVER}/gmt_hash_server.txt > /tmp/gmt_hash_server.txt
if [ $? -ne 0 ]; then
	echo "Error: curl failed with error $?" >&2
	exit 1
fi
# Download the server data list
curl -sk ${SERVER}/gmt_data_server.txt > /tmp/gmt_data_server.txt
if [ $? -ne 0 ]; then
	echo "Error: curl failed with error $?" >&2
	exit 1
fi

# Download cache files
if [ ${cache} -eq 1 ]; then
	echo "Download all cache files from ${SERVER} not in ~/.gmt/cache (be patient)" >&2
	# Get list of data files
	awk 'NR>=2 && !/^ *#/ {print $2}' /tmp/gmt_data_server.txt | sort -u > /tmp/gmt_sorted_server.txt
	# Get list of all files (data + cache)
	awk 'NR>=2 {print $1}' /tmp/gmt_hash_server.txt | sort -u > /tmp/gmt_sorted_all.txt
	# Get list of cache files (i.e., files in gmt_hash_server.txt but not in gmt_data_server.txt)
	comm -23 /tmp/gmt_sorted_all.txt /tmp/gmt_sorted_server.txt | awk '{print "@"$1}' | xargs ${gmtwhich}
fi

# Download data files
if [ ${data} -eq 1 ]; then
	echo "Download all remote data files from ${SERVER} not in ~/.gmt/server (be very patient)" >&2
	# match all lines starting with '/server/'
	awk '/^\/server\// {print "@"$2}' /tmp/gmt_data_server.txt | xargs ${gmtwhich}
fi

rm -f /tmp/gmt_hash_server.txt /tmp/gmt_data_server.txt
