#!/usr/bin/env bash
# Updated version to handle direct writing of sliced grids

ps=gspline_4.ps

# Figure 5 in Wessel, P. (2009), A general-purpose Green's function-based
#	interpolator, Computers & Geosciences, 35, 1247-1254.

R2D=12/32/0/6
Z=5/10
R3D=$R2D/$Z
dz=0.25
view=200/25
method=r
tens=0.85
# Write a single 3-D data cube
gmt greenspline -R$R3D -I$dz -G3D_cube.grd @Table_5_23.txt -S${method}${tens} -Z5 -D+x"x-distance [km]"+y"y-distance [km]"+z"z-distance [km]"+d"Uranium Oxide [%]"+v"uoxide"
# Make a list of the cube z-levels
gmt math -T${Z}/${dz} -o0 T = t.lis
gmt psbasemap -R$R2D/$Z -JX6i/3i -JZ2.5i -p$view -Bx5f1g1 -By1g1 -Bz2f1 -BWSneZ+b -P -K > $ps
# Plot small cube symbols at the data points
gmt psxyz -R -JX -JZ -p$view -O -K @Table_5_23.txt -Su0.05i -Gblack -Wfaint >> $ps
# Loop over the z-levels and draw the 10 % contour only
while read z; do
	gmt grdcontour -JX "3D_cube.grd?($z)" -C10 -L9/11 -S8 -D | gmt psxyz -R$R2D/$Z -JX -JZ -p$view -O -K -Gp39+r300+fgray+b- -Wthin -i0,1,2+s0+o${z} >> $ps
done < t.lis
echo "12 6 Volume exceeding 10% UO@-2@- concentration" | gmt pstext -R$R2D/$Z -JX -JZ -p$view -F+jLT+f16p -O -K -Z10 -Dj0.1i >> $ps
gmt psxyz -R -JX -JZ -p$view -O -Wthin << EOF >> $ps
>
12 0 5
12 0 10
>
12 0 10
12 6 10
>
12 0 10
32 0 10
EOF
