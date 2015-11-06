#! /bin/bash
# $Id$
#
# Computes the gravity anomaly of a sphere both analytical and descrete triangles

ps=spheres.ps

r=10; z0=-15; ro=1000;

echo -50 0 > li
echo  50 0 >> li
gmt sample1d li -Fl -I1 > li1.dat

gmt gmtgravmag3d -Tr"${src:-.}"/sphere.raw -C$ro -Fli1.dat > ptodos_g.dat

# xyzokb solution
gmt psxy ptodos_g.dat -i0,2 -R-50/50/0/0.125 -JX14c/10c -Bx10f5 -By.01 -BWSne+t"Anomaly (mGal)" -W1p -P -K > $ps
gmt psxy ptodos_g.dat -i0,2 -R -JX -Sc.1c -G0 -O -K >> $ps


# Profile of analytic anomaly
gmt gmtmath -T-50/50/1 T $z0 HYPOT 3 POW INV 6.674e-6 MUL 4 MUL 3 DIV PI MUL $r 3 POW MUL $ro MUL $z0 ABS MUL = ztmp.dat
gmt psxy ztmp.dat -R -JX -W1p,200/0/0 -O >> $ps
