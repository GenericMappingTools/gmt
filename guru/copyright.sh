#!/bin/sh
#	$Id: copyright.sh,v 1.4 2005-07-12 04:13:24 pwessel Exp $

# Tool that replaces the 1991-$1 Copyright string with 1991-$2 where
# $1 and $2 are the two year arguments passed to the script.
# Also does the documentation which started in 2000.

if [ $# -ne 2 ]; then
	echo "usage: copyright.sh oldyear newyear"
	exit
fi

if [ ! -d guru ]; then
	echo "copyright.sh must run from top GMT directory"
	exit
fi

last=$1
this=$2
cat guru/GMT_all_files.lis guru/GMT_special.lis > $$
ls doc/*.tex >> $$

n=`cat $$ | wc -l`

i=0
while [ $i -lt $n ]; do
	i=`expr $i + 1`
	f=`sed -n ${i}p $$`
	sed -e "s/1991-$last/1991-$this/g" -e "s/1991 -- $last/1991 -- $this/g" -e "s/2000-$last/2000-$this/g" $f > $f.new
	mv -f $f.new $f
done
rm -f $$
