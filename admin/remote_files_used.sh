#!/usr/bin/env bash
# Determine which remote data sets are used in our doc and test scripts
# These should ideally not change and be placed on the static server
# Note: We do not keep track of cache files, just the earth data sets

TESTDIR=test		# Name of dir with all tests
DOCDIR=doc/scripts	# Name of dir with all documentation scripts
RESDIR=~		# Name of dir where we place the resulting long

if [ ! -d admin ]; then
	echo "remote-files-used.sh: Must be run from top-level gmt directory" >&2
	exit 1
fi

grep '@earth_' ${TESTDIR}/**/*.sh ${DOCDIR}/*.sh | awk -F'@earth_' '{print $2}' | awk '{printf("@earth_%s\n", $1)}' | grep -v _holes | sort -u > ${RESDIR}/remote_files_used.txt
echo "List of remote data sets used: ${RESDIR}/remote_files_used.txt"
cat ${RESDIR}/remote_files_used.txt
