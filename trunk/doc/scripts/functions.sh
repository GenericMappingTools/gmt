#
#	$Id$
#
# Functions to be used with test scripts

getbox () {
# Expects -Joption and distance in km from map center
range=`(echo -$2 -$2; echo $2 $2) | mapproject $1 -R0/360/-90/90 -I -Fk -C`
printf " -R%f/%f/%f/%fr\n" $range
}

getrect () {
# Expects xmin xmax ymin ymax in km relative to map center
# -R and -J are set by preceding GMT commands
(echo -$1 -$1; echo -$1 $1; echo $1 $1; echo $1 -$1) | mapproject -R -J -I -Fk -C
}

# Determine if awk is buggy
result=`echo 1 | awk '{print sin($1)}'`
if [ $result = 1 ]; then        # awk is rotten
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
srcdir=`cd ../../src;pwd`
export PATH=$srcdir:$PATH
export LD_LIBRARY_PATH=$srcdir:${LD_LIBRARY_PATH:-/usr/lib}

# Make sure to cleanup at end
trap "\rm -f .gmt* gmt.conf $$.*" EXIT

# Start with proper GMT defaults
gmtset -Du PS_CHAR_ENCODING ISOLatin1+
