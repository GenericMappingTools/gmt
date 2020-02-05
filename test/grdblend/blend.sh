#!/usr/bin/env bash
#

ps=blend.ps

# Make 4 constant grids
gmt grdmath -R0/6/0/6 -I0.1 0 = a.nc
gmt grdmath -R4/10/0/5 -I0.1 2 = b.nc
gmt grdmath -R0/6/4/10 -I0.1 8 = c.nc
gmt grdmath -R4/10/4/10 -I0.1 4 = d.nc
gmt makecpt -Crainbow -T0/8 > t.cpt
# Just add them up
gmt grdblend ?.nc -R0/10/0/10 -I0.1 -Gblend.nc
gmt grdimage blend.nc -Ct.cpt -JX3i -P -B2 -BWSne -K -X0.75i -Y0.75i > $ps
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
0	4
EOF
gmt psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
cat << EOF > info.txt
a.nc	-R1/5/1/5	1
b.nc	-R5/10/1/4	1
c.nc	-R5/9/5/9	1
d.nc	-R1/5/5/9	1
EOF
echo 10 10 average | gmt pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f14p >> $ps
# Blend the overlapping grids
gmt grdblend info.txt -R0/10/0/10 -I0.1 -Gblend.nc
gmt grdimage blend.nc -Ct.cpt -J -O -X3.5i -B2 -BWSne -K >> $ps
gmt psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
# Draw the inside regions
gmt psxy -R -J -O -K -W0.5p,- -L << EOF >> $ps
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
echo 10 10 blend | gmt pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f14p >> $ps
gmt psscale -Ct.cpt -Dx3.25i/1.5i+w2.8i/0.15i+jML -O -K -B1 >> $ps
gmt grdblend ?.nc -Co -R0/10/0/10 -I0.1 -Gblend.nc
gmt grdimage blend.nc -Ct.cpt -J -O -X-3.5i -Y3.3i -B2 -BWsne -K >> $ps
gmt psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
echo 10 10 last | gmt pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f14p >> $ps
gmt grdblend ?.nc -Cf -R0/10/0/10 -I0.1 -Gblend.nc
gmt grdimage blend.nc -Ct.cpt -J -O -X3.5i -B2 -BWsne -K >> $ps
gmt psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
echo 10 10 first | gmt pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f14p >> $ps
gmt psscale -Ct.cpt -Dx3.25i/1.5i+w2.8i/0.15i+jML -O -K -B1 >> $ps
gmt grdblend ?.nc -Cl -R0/10/0/10 -I0.1 -Gblend.nc
gmt grdimage blend.nc -Ct.cpt -J -O -X-3.5i -Y3.3i -B2 -BWsne -K >> $ps
echo 10 10 low | gmt pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f14p >> $ps
gmt psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
gmt grdblend ?.nc -Cu -R0/10/0/10 -I0.1 -Gblend.nc
gmt grdimage blend.nc -Ct.cpt -J -O -X3.5i -B2 -BWsne -K >> $ps
gmt psxy -Rblend.nc lines.txt -J -O -K -W1p >> $ps
echo 10 10 high | gmt pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f14p >> $ps
gmt psscale -Ct.cpt -Dx3.25i/1.5i+w2.8i/0.15i+jML -O -B1 >> $ps

