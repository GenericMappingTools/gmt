#!/bin/bash
#	$Id$
#
#	test script for GMT/test directory
#
# Specify the scripts to be run and its result compared to the archive.

echo "Run test scripts"
echo "------------------------------------------------------------------------------"
echo "Script:                      Purpose                                    STATUS"

# Now do the comparison and tally the fails in fail_count.d

rm -f fail_count.d

here=`pwd`
olddir=

for o in $* ; do
	dir=`dirname $o`
	script=`basename $o .sh`.sh
	if test "$dir" != "$olddir"; then
		cd $here/$dir
		echo "------------------------------------------------------------------------------"
		echo "Directory: $dir"
		echo "------------------------------------------------------------------------------"
		olddir=$dir
	fi
	bash $script
done
cd $here

echo "------------------------------------------------------------------------------"
if test -s fail_count.d ; then
	wc -l fail_count.d | awk '{printf "GMT test script failures: %d\n", $1}'
	cat fail_count.d
	echo "------------------------------------------------------------------------------"
else
	rm -f fail_count.d
fi
