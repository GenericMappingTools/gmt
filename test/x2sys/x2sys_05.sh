#!/bin/bash
#

ln -fs "${GMT_SRCDIR:-.}"/../mgd77/01010221.mgd77 .
OLDX=$X2SYS_HOME
export X2SYS_HOME=.
gmt x2sys_init TEST -Dmgd77 -Emgd77 -F -Gd -Rd -I15m
echo "$X2SYS_HOME" >> $X2SYS_HOME/TEST/TEST_paths.txt
gmt x2sys_binlist -TTEST 01010221.mgd77 -D --FORMAT_FLOAT_OUT=%.2f > x2sys_05.txt
rm -rf TEST
diff --strip-trailing-cr x2sys_05.txt "${GMT_SRCDIR:-.}"/x2sys_05.txt > fail

if [ ! "X$OLDX" = "X" ]; then # Reset prior setting
	export X2SYS_HOME=$OLDX
fi
