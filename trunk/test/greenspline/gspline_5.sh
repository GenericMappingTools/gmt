#!/bin/bash
#
#       $Id: gspline_5.sh,v 1.1 2011-06-17 03:23:30 guru Exp $

. ../functions.sh
header "greenspline: Testing Spherical 3-D interpolation"

ps=gspline_5.ps

# Figure 6 in Wessel, P. (2009), A general-purpose Green's function-based
#	interpolator, Computers & Geosciences, 35, 1247–1254.

D=3
Limit=10
tension=0.9999
gmtset FONT_ANNOT_PRIMARY 12p
#extract_epic -D1957/1/1/2007/12/31 -M5/12 > quakes.xyz
awk '{print $1, $2, 1}' quakes.xyz | blockmean -Rg -I$D -fg -Sw > quakes_count.d
greenspline -Rg -I1 quakes_count.d -D4 -SQ${tension} -Gquake_count.nc
# Calculate number of quakes per 10^-6km^2 per year
grdmath quake_count.nc PI 6371.007181 2 POW MUL 180 DIV $D DUP 2 DIV SIND MUL MUL 2 MUL DIV 50 DIV 1000 MUL 1000 MUL DUP $Limit GT 0 NAN MUL = density.nc
makecpt -Cgray -T0/160/40 -I > $$.cpt
pstext -R0/8/0/11 -Jx1i -K -P -N -D-0.25i/0.2i -F+jLB+f16p -X1.25i << EOF > $ps
0	2.5	c)
0	5.3	b)
0	8.55	a)
EOF
pscoast -R60/-35/220/30r -JH180/5i -B40f20/20WSne -Gwhite -Wthinnest -A1000 -K -O --MAP_ANNOT_OBLIQUE=46 --FORMAT_GEO_MAP=ddd:mm:ssF -X0.25i >> $ps
grdimage density.nc -C$$.cpt -R -J -O -K -Q >> $ps
grdcontour density.nc -C20 -R -J -O -K -Wcthinnest >> $ps
psxy -R -J -O -K trench.d ridge.d transform.d -W0.5p,- >> $ps
H=`echo 220 30 | mapproject -R -J | cut -f2`
H2=`gmtmath -Q $H 0.5 MUL =`
psscale -C$$.cpt -D5.4i/${H2}i/2i/0.15i -O -K -E -B/:"10@+-6@+ km@+-2@+yr@+-1@+": >> $ps
#
pscoast -Rg -JH180/6i -B0 -Gwhite -Wthinnest -A1000 -K -O -Y2.8i -X-0.25i >> $ps
grdimage density.nc -C$$.cpt -J -O -K -Q >> $ps
grdcontour density.nc -C20 -J -O -K -Wcthinnest >> $ps
psxy -R -J -O -K trench.d ridge.d transform.d -W0.5p,- >> $ps
mapproject -R60/-35/220/30r -JH180/5i -I << EOF | psxy -Rg -JH180/6i -O -K -A -W1p -L >> $ps
0	0
5	0
5	$H
0	$H
EOF
#
pscoast -Rg -JH180/6i -B0 -Gwhite -Wthinnest -A1000 -K -O -Y3.25i  >> $ps
psxy -R -J -O -K -Sc0.02i -Gblack quakes.xyz >> $ps
psxy -R -J -O -K trench.d ridge.d transform.d -W0.5p,- >> $ps
psxy -R$R -J -O -T >> $ps
rm $$.cpt quakes_count.d quake_count.nc density.nc

pscmp
