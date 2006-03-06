#!/bin/sh
#	$Id: run_gmt_tests.sh,v 1.1 2006-03-06 09:43:48 pwessel Exp $
#
#	test script for GMT/test directory
#
# Will find all the subdirectories in GMT/test and run all the
# Bourne or csh scripts found in each directory

# Get all directories below GMT/test and filter out the CVS directories
find . -type d -print -mindepth 1 | grep -v CVS > $$.dirs

# Set here and use it to reset the dir in case a script fails to finish properly

here=`pwd`
while read dir; do
	cd $here/$dir
	for script in *.*sh; do	# Looks for both *.sh and *.csh
		cd $here/$dir
		sh $script
	done
done < $$.dirs
cd $here
rm -f $$.dirs
