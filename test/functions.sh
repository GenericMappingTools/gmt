#
#	$Id: functions.sh,v 1.2 2007-11-15 04:20:41 remko Exp $
#
# Functions to be used with test scripts

# Print the shell script name and purpose and fill out to 72 characters
header () {
	printf "%-72s" "$0: $1"
}

# Compare the ps file with its original. Check $1.ps (if $1 given) or $ps
pscmp () {
	file=${1:-`basename $ps .ps`}
	compare -density 100 -metric PSNR $file.ps orig/$file.ps $file.png 2>&1 | grep -v inf > fail
	passfail $file
}

passfail () {
	if [ -s fail ]; then
        	echo "[FAIL]"
		echo $1 >> ../fail_count.d
		mv -f fail $1.log
	else
        	echo "[PASS]"
        	rm -f fail $1.log $1.png
	fi
}
