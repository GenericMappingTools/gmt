#!/bin/sh
#	$Id: run_doc_tests.sh,v 1.3 2008-03-28 01:55:15 remko Exp $
#
#	Test newly created plots for documentation against archive
#
# Specify archived images to check against on command line, or otherwise checks all.

echo "File                            STATUS"
echo "--------------------------------------"

# Get the file names of all archived images

if [ $# -eq 0 ] ; then
	origs=orig/*.eps
else
	origs=$*
fi

# Now do the comparison and tally the fails in fail_count.d

rm -f fail_count.d
touch fail_count.d

for o in $origs ; do
        f=`basename $o .eps`
	printf "%-32s" $f.eps
	ae=`compare -density 100 -fuzz 1% -metric AE $f.eps orig/$f.eps $f.png 2>&1`
	if [ "$ae" == "0" ]; then
        	echo "[PASS]"
        	rm -f $f.png
	else
        	echo "[FAIL]"
		echo $f: $ae failed pixels >> fail_count.d
	fi
done

echo "--------------------------------------"
wc -l fail_count.d | awk '{printf "GMT Documentation EPS file failures: %d\n", $1}'
cat fail_count.d
rm -f fail_count.d
