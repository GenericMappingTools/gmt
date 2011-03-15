# Bash script inclusion to test all GMT examples.
# It sets the environment such that the examples are created as in the manual.

# Determine if awk is buggy
result=`echo 1 | awk '{print sin($1)}'`
if [ $result = 1 ]; then	# awk is rotten
	if [ `type nawk | grep "not found" | wc -l` -eq 1 ]; then
		export AWK=gawk
	else
		export AWK=nawk
	fi
else
	export AWK=awk
fi

# Temporary change LANG to C
LANG=C

# Extend executable and library path to use the current version
srcdir=`cd ../../../../../src;pwd`
export PATH=$srcdir:$PATH
export LD_LIBRARY_PATH=$srcdir:${LD_LIBRARY_PATH:-/usr/lib}

# Make sure to cleanup at end
trap "\rm -f .gmt* gmt.conf" EXIT

# Start with proper GMT defaults
gmtset -Du PS_EPS false FORMAT_TIME_LOGO "Version 5"
