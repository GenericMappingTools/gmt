#!/usr/bin/env bash
# Determine which remote data sets are used in our doc and test scripts
# These should ideally not change and be placed on the static server
# Note: We do not keep track of cache files, just the data sets

USERDIR=$(gmt --show-userdir)	# Name or users .gmt dir
SERVERDIR=${USERDIR}/server	# Name of top dir with remote data sets locally
TESTDIR=test			# Name of dir with all tests
DOCDIR=doc/scripts		# Name of dir with all documentation scripts
RESDIR=~			# Name of dir where we place the resulting long

if [ ! -d admin ]; then
	echo "remote-files-used.sh: Must be run from top-level gmt directory" >&2
	exit 1
fi

# 1. Make a list of all remote files; remote trailing slash if a tiled directory, and chop off any .grd and _g and _p (we add them below)
grep earth_ ${SERVERDIR}/gmt_data_server.txt | awk '{print $2}' | awk -F/ '{print $1}' | awk -F. '{printf "@%s\n", $1}' | awk -F_ '{printf "%s_%s_%s\n", $1, $2, $3}' | sort -u > /tmp/earth.lis

# 2. Make sure we start with nothing
rm -f /tmp/data

while read data; do	# For each unique remote data set, e.g., @earth_day_10m
	for res in "" "_g" "_p" ; do	# Try the two resolutions and no resolution specified
		find ${TESTDIR} ${DOCDIR} -name '*.sh' -exec grep -H "${data}${res}" {} \; > /tmp/result
		if [ -s /tmp/result ]; then
			echo ${data}${res} >> /tmp/data
		else
			echo "Not used: ${data}${res}"
		fi
	done 
done < /tmp/earth.lis

#3. Sort and only keep unique entries

sort -u /tmp/data > ${RESDIR}/remote_files_used.txt
echo "List of remote data used in scripts: ${RESDIR}/remote_files_used.txt"
