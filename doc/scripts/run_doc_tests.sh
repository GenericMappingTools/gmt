#!/bin/sh
#	$Id: run_doc_tests.sh,v 1.2 2007-11-15 04:15:48 remko Exp $
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
	compare -density 100 -metric PSNR $f.eps orig/$f.eps $f.png 2>&1 | grep -v inf > fail
	if [ -s fail ]; then
        	echo "[FAIL]"
		echo $f >> fail_count.d
		mv -f fail $f.log
	else
        	echo "[PASS]"
        	rm -f fail $f.log $f.png
	fi
done

echo "--------------------------------------"
wc -l fail_count.d | awk '{printf "GMT Documentation EPS file failures: %d\n", $1}'
cat fail_count.d
rm -f fail_count.d fail
