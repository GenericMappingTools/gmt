#
#	$Id$
#
# Functions to be used with test scripts

# Print the shell script name and purpose and fill out to 72 characters
# and make sure to use US system defaults
header () {
	printf "%-72s" "$0: $1"
}

# Compare the ps file with its original. Check $1.ps (if $1 given) or $ps
pscmp () {
	f=${1:-`basename $ps .ps`}
	d=`basename $PWD`
	rms=`compare -density 100 -metric RMSE $f.ps orig/$f.ps $f.png 2>&1`
	if test $? -ne 0; then
        	echo "[FAIL]"
		echo $d/$f: $rms >> ../fail_count.d
	elif test `echo 200 \> $rms|cut -d' ' -f-3|bc` -eq 1; then
        	echo "[PASS]"
        	rm -f $f.png $f.ps
	else
        	echo "[FAIL]"
		echo $d/$f: RMS Error = $rms >> ../fail_count.d
	fi
}

passfail () {
	if [ -s fail ]; then
        	echo "[FAIL]"
		echo $d/$1: `wc -l fail`ed lines >> ../fail_count.d
		mv -f fail $1.log
	else
        	echo "[PASS]"
        	rm -f fail $1.log $1.png
	fi
}

# Temporary change LANG to C
LANG=C

# Extend executable and library path to use the current version
srcdir=`cd ../../src;pwd`
export PATH=$srcdir:$srcdir/meca:$srcdir/mgd77:$PATH
export LD_LIBRARY_PATH=$srcdir:$srcdir/meca:$srcdir/mgd77:${LD_LIBRARY_PATH:-/usr/lib}

# Make sure to cleanup at end
trap "\rm -f .gmt* gmt.conf" EXIT

# Start with proper GMT defaults
gmtset -Du
