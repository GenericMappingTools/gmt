#!/bin/bash
#	$Id: run_gmt_tests.sh,v 1.15 2011-03-15 02:06:38 guru Exp $
#
#	test script for GMT/test directory
#
# Will find all the subdirectories in GMT/test and run all the
# Bourne scripts found in each directory.
# If directory/ies are specified as arguments: will only run scripts in those.

echo "Run test scripts"
echo "------------------------------------------------------------------------------"
echo "Script:                      Purpose                                    STATUS"

# Get all directories below GMT/test and filter out the CVS directories
# PW: For now exclude supplements since they are not ported yet
if [ $# -eq 0 ] ; then
   dirs=`find . -mindepth 1 -maxdepth 1 -type d -print | egrep -v 'CVS|meca|mgd77'`
else
   dirs=$*
fi

# Set here and use it to reset the dir in case a script fails to finish properly
# Each script that fails will write a line to fail_count.d so we can tally errors

rm -f fail_count.d
touch fail_count.d
here=`pwd`
for dir in $dirs; do
	cd $here/$dir
	echo "------------------------------------------------------------------------------"
	echo "Directory: $dir"
	echo "------------------------------------------------------------------------------"
	# Look for Bourne shell scripts
	list=`ls *.sh 2> /dev/null`
	for script in $list; do
		bash $script
	done
done
cd $here
echo "------------------------------------------------------------------------------"
wc -l fail_count.d | awk '{printf "GMT test script failures: %d\n", $1}'
cat fail_count.d
rm -f fail_count.d
echo "------------------------------------------------------------------------------"
