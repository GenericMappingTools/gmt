#!/bin/sh
#	$Id: run_doc_tests.sh,v 1.1 2007-09-13 17:39:08 remko Exp $
#
#	Test newly created plots for documentation against archive
#
# Specify archived images to check against on command line, or otherwise checks all.

echo "File                          : STATUS"
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
        f=`basename $o`
        echo $f | awk '{printf "%-30s: ", $1}'
        compare -density 100 -metric PSNR $f $o $f.diff.png 2>&1 | grep inf > fail
        if [ ! -s fail ]; then
                echo "[FAIL]"
                echo failed >> fail_count.d
        else
                echo "[PASS]"
                rm -f $f.diff.png
        fi
done
wc -l fail_count.d | awk '{printf "GMT Documentation EPS file failures: %d\n", $1}'
rm -f fail_count.d fail
