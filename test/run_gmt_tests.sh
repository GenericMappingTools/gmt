#!/bin/sh
#	$Id: run_gmt_tests.sh,v 1.2 2006-11-28 16:07:58 pwessel Exp $
#
#	test script for GMT/test directory
#
# Will find all the subdirectories in GMT/test and run all the
# Bourne or csh scripts found in each directory

# Get all directories below GMT/test and filter out the CVS directories
find . -type d -print -mindepth 1 | grep -v CVS > /tmp/$$.dirs

# Set here and use it to reset the dir in case a script fails to finish properly

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
	# Look for C shell scripts
	ls *.csh > /tmp/$$.lis
	if [ -s /tmp/$$.lis ]; then	# Found C shell scripts
		for script in *.csh; do
			cd $here/$dir
			csh $script
		done
	fi
done < /tmp/$$.dirs
cd $here
rm -f /tmp/$$.*
