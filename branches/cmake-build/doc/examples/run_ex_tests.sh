#!/bin/bash
#	$Id$
#
#	Test newly created plots for documentation against archive
#
# Specify archived images to check against on command line, or otherwise checks all.

echo "Test GMT Example PS files against archive"
echo "--------------------------------------"
echo "File                            STATUS"
echo "--------------------------------------"

# Now do the comparison and tally the fails in fail_count.d

rm -f fail_count.d

for o in $* ; do
        f=`basename $o .ps`
	printf "%-32s" $f.ps
	rms=`gm compare -density 200 -maximum-error 0.001 -highlight-color magenta -highlight-style assign -metric rmse -file $f.png $f.ps ../fig/$f.ps 2>&1`
	if test $? -ne 0; then
        	echo "[FAIL]"
		rms=`(sed -nE '/Total:/s/ +Total: ([0-9.]+) .+/\1/p'|cut -c-5) <<< "$rms"`
		echo $f: RMS Error = $rms >> fail_count.d
	else
        	echo "[PASS]"
        	rm -f $f.png $f.ps
	fi
done

echo "--------------------------------------"
if test -s fail_count.d ; then
	wc -l fail_count.d | awk '{printf "GMT Example PS file failures: %d\n", $1}'
	cat fail_count.d
	echo "--------------------------------------"
else
	rm -f fail_count.d
fi
