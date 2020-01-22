#!/usr/bin/env bash
# Testing gmt grdfft power spectrum for known signal

ps=cylundulation.ps

# Make data set of two sinusoids and some noise:
# 1) Propagating in 20 degrees direction:  J = 3,  A = 6, phi = 1.0 radian
# 2) Propagating in -60 degrees direction: I = 18, A = 4, phi = 0.5 radian
# 3) Random noise with mean = 2 and std = 1
# Note: Commands creating surf.nc are commented out since they include adding noise and we
# don't want the noise to change each time, giving PS failures.
#gmt grdmath -R0/127/0/127 -I1 X 18 COSD MUL Y 18 SIND MUL ADD 128 DIV 3 MUL 2 MUL PI MUL 1 ADD COS 6 MUL = A.nc
#gmt grdmath -R0/127/0/127 -I1 X -60 COSD MUL Y -60 SIND MUL ADD 128 DIV 18 MUL 2 MUL PI MUL 0.5 ADD COS 4 MUL = B.nc
#gmt grdmath -R0/127/0/127 -I1 2 1 NRAND = C.nc
#gmt grdmath A.nc B.nc ADD C.nc ADD = surf.nc
gmt makecpt -Cpolar -T-14/14 > t.cpt
gmt grdimage @surf.nc -JX4.5i -Ct.cpt -B64f32g64+u" m" -BWSne -P -K -Xc -Y0.5i > $ps
gmt psxy -R@surf.nc -J -O -K -Sv0.21i+e -Wthick -Gblack << EOF >> $ps
64 64 18 2i
64 64 -60 2i
EOF
gmt psscale -DJRM+w4.5i/0.1i -Ct.cpt -Baf -R -J -O -K >> $ps
gmt grdfft @surf.nc -Ex+w -N+zp --GMT_FFT=brenner > /dev/null
gmt makecpt -Cwhite,gray -T0/0.5 -N > t.cpt
gmt makecpt -N -T0.5/3.5/1 -Crainbow >> t.cpt
gmt grdimage surf_mag.nc -R-1/1/-1/1 -J -O -K -Ct.cpt -Bafg1+u" m@+-1@+" -BWSne+t"Two cylindrical undulations and noise" -Y5i >> $ps
y=`gmt math -Q 18 TAND =`
x=`gmt math -Q 90 -60 ADD TAND =`
gmt psxy -R-1/1/-1/1 -J -O -K -Wthin << EOF >> $ps
>
-1	-$y
1	$y
>
-$x	1
$x	-1
EOF
gmt psxy -R -J -O -K -Sm0.1i+e+b -W0.25p -Gblack << EOF >> $ps
0 0 1i 0 18
0 0 1.5i -60 0
EOF
gmt pstext -R -J -O -K -F+f12p << EOF >> $ps
0.65 -0.35 60@.
0.52 0.08 18@.
EOF
gmt psscale -DJRM+w4.5i/0.1i -Ct.cpt -R -J -O -K >> $ps
gmt psxy -R -J -O -T >> $ps
