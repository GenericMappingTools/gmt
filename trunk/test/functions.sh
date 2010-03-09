#
#	$Id: functions.sh,v 1.12 2010-03-09 18:55:53 remko Exp $
#
# Functions to be used with test scripts

# Print the shell script name and purpose and fill out to 72 characters
# and make sure to use US system defaults
header () {
	printf "%-72s" "$0: $1"
	gmtdefaults -Du > .gmtdefaults4
	gmtset PAPER_MEDIA letter
}

# Compare the ps file with its original. Check $1.ps (if $1 given) or $ps
pscmp () {
	f=${1:-`basename $ps .ps`}
	rms=`compare -density 100 -metric RMSE $f.ps orig/$f.ps $f.png 2>&1`
	if test $? -ne 0; then
        	echo "[FAIL]"
		echo $f: $rms >> ../fail_count.d
	elif test `echo 200 \> $rms|cut -d' ' -f-3|bc` -eq 1; then
        	echo "[PASS]"
        	rm -f $f.png $f.ps
	else
        	echo "[FAIL]"
		echo $f: RMS Error = $rms >> ../fail_count.d
	fi
	rm -f .gmtdefaults4 .gmtcommands4
}

passfail () {
	if [ -s fail ]; then
        	echo "[FAIL]"
		echo $1: `wc -l fail`ed lines >> ../fail_count.d
		mv -f fail $1.log
	else
        	echo "[PASS]"
        	rm -f fail $1.log $1.png
	fi
	rm -f .gmtdefaults4 .gmtcommands4
}
