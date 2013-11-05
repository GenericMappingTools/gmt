#!/bin/sh
# $Id$
# Script to help with editing coastline files
FID=$1
grep "Id = $FID N" GSHHS+WDBII/GSHHS/res_f/GSHHS_*.txt | awk -F: '{print $2}' > this
mate this
awk '{if ($1 == '"$FID"') printf "%s\th\n%d\ti\n%d\tl\n%d\tc\n", $2, $3, $4, $5}' fix_in_1.10.txt > t.lis
while read ID r; do
	if [ ! $ID = "X" ]; then
		file=`grep "Id = $ID N" GSHHS+WDBII/GSHHS/res_$r/*.txt | awk -F: '{print $1}'`
#		echo $file
		mate $file
	else
		echo "No version of $FID at resolution $r"
	fi
done < t.lis
rm -f t.lis this
