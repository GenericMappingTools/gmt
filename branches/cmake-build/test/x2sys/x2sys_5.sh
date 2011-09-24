#!/bin/bash
# $Id$
#

. ../functions.sh
header "Test x2sys_binlist on cruise C2308"

cp ../mgd77/01010221.mgd77 .
OLDX=$X2SYS_HOME
export X2SYS_HOME=`pwd`
x2sys_init TEST -D../../share/x2sys/mgd77 -Emgd77 -F -Gd -Rd -I15m
echo "$X2SYS_HOME" >> $X2SYS_HOME/TEST/TEST_paths.txt
x2sys_binlist -TTEST 01010221.mgd77 -D --FORMAT_FLOAT_OUT=%.3f > bin.txt
rm -rf TEST
diff -q --strip-trailing-cr bin.txt orig/bin.txt >> fail
passfail x2sys_5

if [ ! "X$OLDX" = "X" ]; then	# Reset prior setting
	export X2SYS_HOME=$OLDX
fi
rm -f 01010221.mgd77 bin.txt
