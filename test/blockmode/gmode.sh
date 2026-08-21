#!/usr/bin/env bash
# Test that the grid option in blockmode matches the table output to ~EPS
# If max fractional difference is < 1e-7 we assume it is good and write 1
# else it is 0.  If any of the 4 outputs fail then the test fails, and
# the file fail will indicate which one(s) caused the problem.

gmt blockmode @ship_15.txt -I1 -R-115/-105/20/30 -fg -E -o2:5 > dump.txt
gmt blockmode @ship_15.txt -I1 -R-115/-105/20/30 -fg -E -Gfield_%s.grd -Az,s,l,h
# Mode z:
gmt grd2xyz field_z.grd -s -o2 > tmp
z=$(gmt convert -A dump.txt tmp -o0,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =)
# L1 :
gmt grd2xyz field_s.grd -s -o2 > tmp
s=$(gmt convert -A dump.txt tmp -o1,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =)
# l:
gmt grd2xyz field_l.grd -s -o2 > tmp
l=$(gmt convert -A dump.txt tmp -o2,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =)
# h:
gmt grd2xyz field_h.grd -s -o2 > tmp
h=$(gmt convert -A dump.txt tmp -o3,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =)
echo $z $s $l $h
if [ ! "$z $s $l $h" = "1 1 1 1" ] ; then
 	echo "$z $s $l $h" > fail
fi
