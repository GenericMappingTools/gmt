#!/bin/sh
# $Id: prog_use.sh,v 1.2 2011-06-17 18:59:45 guru Exp $
# Returns a count of how many time a GMT program is used in our test scripts
# including doc and examples.  Give -l to see which scripts [Default returns the count]
#
. gmt_shell_functions.sh
# Determines in which test and example scripts each GMT module (incl supplement) is used
find . -name '*_func.c' -print | tr '/' ' ' | awk '{print $NF}' | sed -e s/_func.c//g > /tmp/t.lis
while read prog; do
	find . -name '*.sh' -exec grep -H $prog {} \; | sed -e 'sB/share/doc/gmtB/docBg' | awk -F: '{print $1}' | sort -u > /tmp/list
	n=`gmt_nrecords /tmp/list`
	if [ $# -gt 0 ]; then
		echo " "
		echo "-------------------"
	fi
	echo "===> $prog [$n]"
	if [ $# -gt 0 ]; then
		echo "-------------------"
		cat /tmp/list
	fi
done < /tmp/t.lis

