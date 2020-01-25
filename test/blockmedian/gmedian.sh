#!/usr/bin/env bash
# Test that the grid option in blockmedian matches the table output to ~EPS
# If max fractional difference is < 1e-7 we assume it is good and write 1
# else it is 0.  If any of the 6 outputs fail then the test fails, and
# the file fail will indicate which one(s) caused the problem.

gmt blockmedian @ship_15.txt -I1 -R-115/-105/20/30 -fg -E -o2:5 > dump.txt
gmt blockmedian @ship_15.txt -I1 -R-115/-105/20/30 -fg -E -Gfield_%s.grd -Az,s,l,h
gmt blockmedian @ship_15.txt -I1 -R-115/-105/20/30 -fg -Eb -o4,5 > qdump.txt
gmt blockmedian @ship_15.txt -I1 -R-115/-105/20/30 -fg -Eb -Gfield_%s.grd -Aq25,q75

# Median z:
gmt grd2xyz field_z.grd -s -o2 > tmp
z=`gmt convert -A dump.txt tmp -o0,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =`
# L1 s:
gmt grd2xyz field_s.grd -s -o2 > tmp
s=`gmt convert -A dump.txt tmp -o1,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =`
# l:
gmt grd2xyz field_l.grd -s -o2 > tmp
l=`gmt convert -A dump.txt tmp -o2,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =`
# h:
gmt grd2xyz field_h.grd -s -o2 > tmp
h=`gmt convert -A dump.txt tmp -o3,4 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =`
# 25% quartile:
gmt grd2xyz field_q25.grd -s -o2 > tmp
q25=`gmt convert -A qdump.txt tmp -o0,2 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =`
# 75% quartile:
gmt grd2xyz field_q75.grd -s -o2 > tmp
q75=`gmt convert -A qdump.txt tmp -o1,2 | awk '{print ($1-$2)/$1}' | gmt math STDIN  -T -Sf ABS UPPER 1e-7 LT =`
echo $z $s $l $q25 $q75 $h
if [ ! "$z $s $l $q25 $q75 $h" = "1 1 1 1 1 1" ] ; then
 	echo "$z $s $l $q25 $q75 $h" > fail
fi
