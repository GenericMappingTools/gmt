#!/bin/bash
#	$Id$
#
#	Test newly created plots for documentation against archive
#
# Specify archived images to check against on command line, or otherwise checks all.

echo "Test GMT Documentation PS files against archive"
echo "--------------------------------------"
echo "File                            STATUS"
echo "--------------------------------------"

# Now do the comparison and tally the fails in fail_count.d

rm -f fail_count.d

for o in $* ; do
        f=`basename $o .ps`
	printf "%-32s" $f.ps
	rms=`compare -density 100 -metric RMSE $f.ps ../fig/$f.ps $f.png 2>&1`
	if test $? -ne 0; then
        	echo "[FAIL]"
		echo $f: $rms >> fail_count.d
	elif test `echo 40 \> $rms|cut -d' ' -f-3|bc` -eq 1; then
        	echo "[PASS]"
        	rm -f $f.png
	else
        	echo "[FAIL]"
		echo $f: RMS Error = $rms >> fail_count.d
	fi
done

echo "--------------------------------------"
if test -s fail_count.d; then
	wc -l fail_count.d | awk '{printf "GMT Documentation PS file failures: %d\n", $1}'
	cat fail_count.d
	echo "--------------------------------------"
else
	rm -f fail_count.d
fi
