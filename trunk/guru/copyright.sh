#!/bin/sh 
# $Id$

# Tool that updates the 199?-???? Copyright string to the current year.

if [ ! -d guru ]; then
	echo "copyright.sh must run from top GMT directory"
	exit
fi

newyear=`date '+%Y'`
# 1. Find all GMT files with "Copyright" in it; exclude files in /include and this script
find . -name '*.[ch]' -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >| $$.tmp.lis
find . -name '*.in'   -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >> $$.tmp.lis
find . -name '*.tex'  -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >> $$.tmp.lis
find . -name '*.txt'  -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >> $$.tmp.lis
find . -name '*.sh'   -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >> $$.tmp.lis
find . -name '*.mk'   -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >> $$.tmp.lis
find . -name 'README*' -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >> $$.tmp.lis
find . -name '*akefile' -exec grep -H Copyright {} \; | grep -v ${newyear} | awk -F: '{print $1}' >> $$.tmp.lis
sort -u $$.tmp.lis | egrep -v '/include/|copyright.sh|triangle.c|triangle.h|README.TRIANGLE' > $$.progs.lis

# 2. Make sed substitution script
let lastyear=newyear-1
thisyear=1991
rm -f $$.sed.lis
while [ $thisyear -lt $lastyear ]; do
	echo "s/Copyright (c) ${thisyear}-${lastyear}/Copyright (c) ${thisyear}-${newyear}/g" >> $$.sed.lis
	let thisyear=thisyear+1
done

# 3. Update the files
while read f; do
	sed -f $$.sed.lis $f > $f.new
	mv -f $f.new $f
done < $$.progs.lis

# 4. Clean up
rm -f $$.sed.lis $$.tmp.lis $$.progs.lis
