#!/bin/sh
#	$Id: run_gmt_tests.sh,v 1.4 2007-05-31 02:51:31 pwessel Exp $
#
#	test script for GMT/test directory
#
# Will find all the subdirectories in GMT/test and run all the
# Bourne scripts found in each directory

# Get all directories below GMT/test and filter out the CVS directories
find . -type d -print -mindepth 1 | grep -v CVS > /tmp/$$.dirs

# Set here and use it to reset the dir in case a script fails to finish properly

# Each script that fails will write a line to fail_count.d so we can tally errors

rm -f fail_count.d
touch fail_count.d
here=`pwd`
while read dir; do
	cd $here/$dir
	# Look for Bourne shell scripts
	ls *.sh > /tmp/$$.lis
	if [ -s /tmp/$$.lis ]; then	# Found Bourne shell scripts
		for script in *.sh; do
			cd $here/$dir
			sh $script
		done
	fi
done < /tmp/$$.dirs
cd $here
wc -l fail_count.d | awk '{printf "GMT test script failures: %d\n", $1}'
rm -f /tmp/$$.* # fail_count.d
