#!/usr/bin/env bash -ex
#
#       Examples of how to use psbarb
#

prefix=psbarb_

gmt gmtset MAP_FRAME_TYPE   = plain \
           MAP_FRAME_AXES   = WeSn  \
           FONT_TITLE       = 20p,Helvetica,black \
           MAP_TITLE_OFFSET = 0p

gmt makecpt -T0/45/5 -Z > wind.cpt

gmt grdmath -Rg -f0f,1y -I10 X           = dir.grd  # -f0f is needed for poles
gmt grdmath -Rg -f0f,1y -I10 Y ABS 2 DIV = spd.grd
gmt grd2xyz dir.grd > .$$.dir.txt
gmt grd2xyz spd.grd | paste .$$.dir.txt - | awk '{print $1,$2,$3,$6}' > wind.txt
rm -f .$$.dir.txt
awk '{print $1,$2,$4,$3,$4}' wind.txt > wind_C.txt  # add 3rd col for -C

#
# Example 1, Linear x-y plot -JX
#
n=1
title="$prefix$n Linear x-y plot -JX"
gmt psbarb wind.txt -Q0.2i -JX24/12 -R -Ba30 -B+t"$title" -N > $prefix$n.ps

#
# Example 2, Equidistant Cylindrical Projection -JQ
#
n=`expr $n + 1`
title="$prefix$n Equidistant Cylindrical Projection -JQ"
gmt psbarb wind.txt -Q0.2i -JQ24    -R -Ba30 -B+t"$title" -N > $prefix$n.ps

#
# Example 3, Stereographic Projection -JS
#
n=`expr $n + 1`
title="$prefix$n Stereographic Projection -JS"
gmt psbarb wind.txt -Q0.2i -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 4, Lambert Projection -JL
#
n=`expr $n + 1`
title="$prefix$n Lambert Projection -JL"
ropt=`
( echo -10000 -5000 ; echo 10000 7500 ) \
| gmt mapproject -Jl180/30/30/60/1 -R180/181/30/31 -Fk -I \
| awk '{ lo = $1; la = $2 ; getline ; printf "%f/%f/%f/%f", lo,la,$1,$2 }'`
gmt psbarb wind.txt -Q0.2i -JL180/30/60/30/24 -R${ropt}+r -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 5, Set color palette with -C
#
n=`expr $n + 1`
title="$prefix$n Set color palette with -C"
gmt psbarb wind_C.txt -Q0.2i -C./wind.cpt -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 6, Set color palette and pen with -C and -W
#
n=`expr $n + 1`
title="$prefix$n Set color and pen with -C and -W"
gmt psbarb wind_C.txt -Q0.2i -C./wind.cpt -W -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 7, Set color palette and color pen with -C and -W+c
#
n=`expr $n + 1`
title="$prefix$n Set color with -C and -W+c"
gmt psbarb wind_C.txt -Q0.2i -C./wind.cpt -Wdefault,black+c -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 8, 3-D plot with -JZ -p
#
n=`expr $n + 1`
title="$prefix$n 3-D plot with -JZ -p"
awk '{print $1,$2,$2,$4,$3,$4}' wind.txt > wind_C_3d.txt  # add z-value
gmt psbarb wind_C_3d.txt -Q0.2i -C./wind.cpt -Wdefault,black+c -JQ20 -R0/360/-90/90/-90/90 -Ba30g30 -BWeSnZ1234+t"$title" -JZ5 -Bza30 -p150/45 > $prefix$n.ps

#
# Example 9, Set wind barb size in data file by not specifying -Q
#            3rd column = length, 4th column = width
#
n=`expr $n + 1`
title="$prefix$n Set wind barb size in data file"
awk '{size=$1/360; print $1,$2, size, size/2 ,$3,$4}' wind.txt | \
gmt psbarb -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 10, Justify wind barb position with -Q+jb
#
n=`expr $n + 1`
title="$prefix$n Justify wind barb position with -Q+jb"
gmt psbarb wind_C.txt -Q0.2i+jb -C./wind.cpt -Wdefault,black+c -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 11, Justify wind barb position with -Q+jc
#
n=`expr $n + 1`
title="$prefix$n Justify wind barb position with -Q+jc"
gmt psbarb wind_C.txt -Q0.2i+jc -C./wind.cpt -Wdefault,black+c -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 12, Justify wind barb position with -Q+je
#
n=`expr $n + 1`
title="$prefix$n Justify wind barb position with -Q+je"
gmt psbarb wind_C.txt -Q0.2i+je -C./wind.cpt -Wdefault,black+c -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps
