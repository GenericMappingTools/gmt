#
#	$Id: functions.sh,v 1.4 2008-03-28 18:53:16 remko Exp $
#
# Functions to be used with test scripts

# Print the shell script name and purpose and fill out to 72 characters
header () {
	printf "%-72s" "$0: $1"
}

# Compare the ps file with its original. Check $1.ps (if $1 given) or $ps
pscmp () {
	f=${1:-`basename $ps .ps`}
	rms=(`compare -density 100 -metric RMSE $f.ps orig/$f.ps $f.png 2>&1`)
	if [ ${rms[0]//.*/} -lt 10 ]; then
        	echo "[PASS]"
        	rm -f $f.png
	else
        	echo "[FAIL]"
		echo $f: RMS Error = ${rms[*]} >> ../fail_count.d
	fi
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
}
