#! /bin/sh -xe
#
#       Examples of how to use grdbarb
#

prefix=grdbarb_

gmt gmtset MAP_FRAME_TYPE   = plain \
           MAP_FRAME_AXES   = WeSn  \
           FONT_TITLE       = 20p,Helvetica,black \
           MAP_TITLE_OFFSET = 0p

gmt makecpt -T0/45/5 -Z > wind.cpt

gmt grdmath -Rg -f0f,1y -I10 X           = dir.grd  # -f0f is needed for poles
gmt grdmath -Rg -f0f,1y -I10 Y ABS 2 DIV = spd.grd
gmt grdmath dir.grd 180 SUB SIND spd.grd MUL = u.grd
gmt grdmath dir.grd 180 SUB COSD spd.grd MUL = v.grd

#
# Example 1, Linear x-y plot -JX
#
n=1
title="$prefix$n Linear x-y plot -JX"
gmt grdbarb u.grd v.grd -W -JX24/12 -R -Ba30 -B+t"$title" -N > $prefix$n.ps

#
# Example 2, Equidistant Cylindrical Projection -JQ
#
n=`expr $n + 1`
title="$prefix$n Equidistant Cylindrical Projection -JQ"
gmt grdbarb u.grd v.grd -W -JQ24    -R -Ba30 -B+t"$title" -N > $prefix$n.ps

#
# Example 3, Stereographic Projection -JS
#
n=`expr $n + 1`
title="$prefix$n Stereographic Projection -JS"
gmt grdbarb u.grd v.grd -W -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 4, Lambert Projection -JL
#
n=`expr $n + 1`
title="$prefix$n Lambert Projection -JL"
ropt=`
( echo -10000 -5000 ; echo 10000 7500 ) \
| gmt mapproject -Jl180/30/30/60/1 -R180/181/30/31 -Fk -I \
| awk '{ lo = $1; la = $2 ; getline ; printf "%f/%f/%f/%f", lo,la,$1,$2 }'`
gmt grdbarb u.grd v.grd -W -JL180/30/60/30/24 -R${ropt}+r -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 5, Set color palette with -C
#
n=`expr $n + 1`
title="$prefix$n Set color palette with -C"
gmt grdbarb u.grd v.grd -C./wind.cpt -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 6, Set color palette and pen with -C and -W
#
n=`expr $n + 1`
title="$prefix$n Set color and pen with -C and -W"
gmt grdbarb u.grd v.grd -W -C./wind.cpt -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 7, test for -T (no difference can not be seen comparing with Ex2)
#
n=`expr $n + 1`
title="$prefix$n Test for -T"
gmt grdbarb u.grd v.grd -W -JQ24 -T -R0/360/-90/90 -Ba30 -B+t"$title" -N > $prefix$n.ps

#
# Example 8, pseudo 3-D plot with -JZ -p
#
n=`expr $n + 1`
title="$prefix$n 3-D plot with -JZ -p"
gmt grdbarb u.grd v.grd -C./wind.cpt -Wdefault,black+c -JQ20 -R0/360/-90/90/-90/90 -Ba30g30 -BWeSnZ1234+t"$title" -JZ5 -Bza30 -p150/45 > $prefix$n.ps

#
# Example 9, Justify wind barb position with -Q+jb
#
n=`expr $n + 1`
title="$prefix$n Justify wind barb position with -Q+jb"
gmt grdbarb u.grd v.grd -Q+jb -W -C./wind.cpt -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 10, Justify wind barb position with -Q+jc
#
n=`expr $n + 1`
title="$prefix$n Justify wind barb position with -Q+jc"
gmt grdbarb u.grd v.grd -Q+jc -W -C./wind.cpt -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps

#
# Example 11, Justify wind barb position with -Q+je
#
n=`expr $n + 1`
title="$prefix$n Justify wind barb position with -Q+je"
gmt grdbarb u.grd v.grd -Q+je -W -C./wind.cpt -JS180/90/17 -R0/360/-20/90 -Ba30g30 -B+t"$title" > $prefix$n.ps
