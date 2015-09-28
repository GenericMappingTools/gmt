#! /bin/bash
# $Id $
#
# Computes the gravity anomaly over a sphere with radius r = 15 m and center at -15 m.
# The sphere is aproximated by two hemi-spheres
# Compare with analytical expressions

ps=grdgravmag3D_grav_sph.ps
r=10
z0=-15
rho=1000

# Compute two half spheres with radius = 10 and center at -15
gmt grdmath -R-10/10/-10/10 -I0.5 X Y HYPOT $r DIV ACOS SIN $r MUL $z0 ADD     = top_half.grd
gmt grdmath -R-10/10/-10/10 -I0.5 X Y HYPOT $r DIV ACOS SIN $r MUL NEG $z0 ADD = bot_half.grd

echo -50 0 > li
echo  50 0 >> li
gmt sample1d li -Fl -I2 > li1.dat

# Compute the effect of the two hemi-spheres and add them
gmt grdgravmag3d top_half.grd -C$rho -Zb -Gtop_g.grd -V -x
gmt grdgravmag3d bot_half.grd -C$rho -Zt -Gbot_g.grd -V -x
gmt grdmath bot_g.grd top_g.grd ADD = sphere_g.grd


# Compute the effect of the two hemi-spheres alog XX axis and add them
grdgravmag3d top_half.grd -C$rho -Zb -Fli1.dat > tt.dat 
grdgravmag3d bot_half.grd -C$rho -Zt -Fli1.dat > tb.dat 
gmtmath tt.dat tb.dat ADD = t.dat


# Profile of analytic anomaly
gmt gmtmath -T-50/50/1 T $z0 HYPOT 3 POW INV 6.674e-6 MUL 4 MUL 3 DIV PI MUL $r 3 POW MUL $rho MUL $z0 ABS MUL = ztmp.dat
gmt psxy ztmp.dat -R-50/50/0/0.125 -JX14c/8c -Bx10f5 -By.01 -BWSne+t"Anomaly (mGal)" -W1p,200/0/0 -P -K > $ps

gmt psxy t.dat -i0,2 -R -JX -Sc.15c -Gblue -O -K >> $ps

echo -49 0 > li
echo  49 0 >> li
gmt sample1d li -Fl -I2 -S-49/49 > li1.dat

# Compute the anomaly using the two hemi-sphere grids at once
gmt grdgravmag3d top_half.grd bot_half.grd -C1000 -Fli1.dat > t2.dat
gmt psxy t2.dat -i0,2 -R -JX -Sc.15c -Ggreen -O -K >> $ps

gmt makecpt -T0.047/0.125/0.001 > t.cpt
gmt grdimage sphere_g.grd -Ct.cpt -JX12c -B2f1 -BWSen+t"Gravity anomaly of a sphere" -Y10.5c -O >> $ps

rm -f t*.dat t.cpt li*.dat top_*.grd bot_*.grd ztmp.dat
