#!/bin/bash
#
#	$Id: blend.sh,v 1.1 2011-05-25 20:36:19 guru Exp $

. ../functions.sh
header "Test grdblend for blending overlapping grids"

ps=blend.ps
# Make 4 constant grids
grdmath -R0/6/0/6 -I0.1 0 = a.nc
grdmath -R4/10/0/5 -I0.1 2 = b.nc
grdmath -R4/10/4/10 -I0.1 4 = c.nc
grdmath -R0/6/4/10 -I0.1 8 = d.nc
makecpt -Crainbow -T0/8/1 -Z > t.cpt
# Just add them up
grdblend ?.nc -R0/10/0/10 -I0.1 -Gblend.nc
grdimage blend.nc -Ct.cpt -JX4i -P -B2WSne -K -X2i > $ps
# Draw grid outlines
cat << EOF > lines.txt
> a
6	0
6	6
0	6
> b
4	0
4	5
10	5
> c
4	10
4	4
10	4
> d
6	10
6	4
10	4
EOF
psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
psscale -Ct.cpt -D4.25i/2i/4i/0.15i -O -K -B1:"stacking": >> $ps
cat << EOF > info.txt
a.nc	-R1/5/1/5	1
b.nc	-R5/10/1/4	1
c.nc	-R5/9/5/9	1
d.nc	-R1/5/5/9	1
EOF
# Blend the overlapping grids
grdblend info.txt -R0/10/0/10 -I0.1 -Gblend.nc
grdimage blend.nc -Ct.cpt -J -O -Y4.5i -B2WSne -K >> $ps
psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
# Draw the inside regions
psxy -R -J -O -K -W0.5p,- -L << EOF >> $ps
> a
1	1
5	1
5	5
1	5
> b
5	1
10	1
10	4
5	4
> c
5	5
9	5
9	9
5	9
> d
1	5
5	5
5	9
1	9
EOF
psscale -Ct.cpt -D4.25i/2i/4i/0.15i -O -B1:"blending": >> $ps
rm -f *.nc t.cpt *.txt

pscmp
