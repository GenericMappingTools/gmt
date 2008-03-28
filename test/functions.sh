#
#	$Id: functions.sh,v 1.3 2008-03-28 01:54:28 remko Exp $
#
# Functions to be used with test scripts

# Print the shell script name and purpose and fill out to 72 characters
header () {
	printf "%-72s" "$0: $1"
}

# Compare the ps file with its original. Check $1.ps (if $1 given) or $ps
pscmp () {
	file=${1:-`basename $ps .ps`}
	ae=`compare -density 100 -fuzz 1% -metric AE $file.ps orig/$file.ps $file.png 2>&1`
	if [ "$ae" == "0" ]; then
        	echo "[PASS]"
        	rm -f $file.png
	else
        	echo "[FAIL]"
		echo $file: $ae failed pixels >> ../fail_count.d
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
