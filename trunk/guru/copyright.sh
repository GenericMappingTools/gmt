#!/bin/sh
#	$Id: copyright.sh,v 1.1 2004-01-02 20:10:01 pwessel Exp $

# Tool that replaces the 1991-$1 Copyright string with 1991-$2 where
# $1 and $2 are the two year arguments passed to the script

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
n=`cat guru/GMT_all_files.lis | wc -l`

i=0
while [ $i -lt $n ]; do
	i=`expr $i + 1`
	f=`sed -n ${i}p guru/GMT_all_files.lis`
	sed -e "s/1991-$last/1991-$this/g" $f > $f.new
	mv -f $f.new $f
done
