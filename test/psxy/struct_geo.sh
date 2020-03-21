#!/usr/bin/env bash
# Test all structural geology symbols from Jose
ps=struct_geo.ps
reg=-R0/10/0/10
# Must temporarily change GMT_USERDIR to the gallery documentation dir
export GMT_USERDIR=`gmt --show-sharedir`/../doc/rst/source/users-contrib-symbols/geology

gmt psxy $reg -JM12c -T -K -P > $ps

echo 1 9 60 40 | gmt psxy -R -J -Skgeo-plane/24p -Wthin -O -K >> $ps
echo 1 9 1 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 2 9 60 40 | gmt psxy -R -J -Skgeo-plane_hor/24p -Wthin -O -K >> $ps
echo 2 9 2 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 3 9 60 40 | gmt psxy -R -J -Skgeo-plane_vert/24p -Wthin -O -K >> $ps
echo 3 9 3 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 4 9 60 40 | gmt psxy -R -J -Skgeo-plane_inv/24p -Wthin -O -K >> $ps
echo 4 9 4 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 5 9 60 40 -50 | gmt psxy -R -J -Skgeo-plane_rake/24p -Wthin -Gblack -O -K >> $ps
echo 5 9 5 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 6 9 60 40 | gmt psxy -R -J -Skgeo-plane_gentle/24p -Wthin -Gblack -O -K >> $ps
echo 6 9 6 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 7 9 60 40 | gmt psxy -R -J -Skgeo-plane_medium/24p -Wthin -Gblack -O -K >> $ps
echo 7 9 7 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 8 9 60 40 | gmt psxy -R -J -Skgeo-plane_steep/24p -Wthin -Gblack -O -K >> $ps
echo 8 9 8 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 9 9 60 40 | gmt psxy -R -J -Skgeo-plane_und/24p -Wthin -Gblack -O -K >> $ps
echo 9 9 9 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps

echo 1 7 60 40 | gmt psxy -R -J -Skgeo-foliation/24p -Wthin -Gblack -O -K >> $ps
echo 1 7 10 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 2 7 60 40 | gmt psxy -R -J -Skgeo-foliation_hor/24p -Wthin -Gblack -O -K >> $ps
echo 2 7 11 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 3 7 60 40 | gmt psxy -R -J -Skgeo-foliation_vert/24p -Wthin -Gblack -O -K >> $ps
echo 3 7 12 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 4 7 60 40 | gmt psxy -R -J -Skgeo-cleavage/24p -Wthin -Gblack -O -K >> $ps
echo 4 7 13 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 5 7 60 40 | gmt psxy -R -J -Skgeo-cleavage_hor/24p -Wthin -Gblack -O -K >> $ps
echo 5 7 14 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 6 7 60 40 | gmt psxy -R -J -Skgeo-cleavage_vert/24p -Wthin -Gblack -O -K >> $ps
echo 6 7 15 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 7 7 60 40 | gmt psxy -R -J -Skgeo-foliation-2/24p -Wthin -Gblack -O -K >> $ps
echo 7 7 16 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 8 7 60 40 | gmt psxy -R -J -Skgeo-foliation-3/24p -Wthin -Gblack -O -K >> $ps
echo 8 7 17 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps

echo 1 5 60 40 | gmt psxy -R -J -Skgeo-joint/24p -Wthin -Gblack -O -K >> $ps
echo 1 5 18 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 2 5 60 40 | gmt psxy -R -J -Skgeo-joint_hor/24p -Wthin -Gblack -O -K >> $ps
echo 2 5 19 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 3 5 60 40 | gmt psxy -R -J -Skgeo-joint_vert/24p -Wthin -Gblack -O -K >> $ps
echo 3 5 20 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 4 5 150 40 | gmt psxy -R -J -Skgeo-lineation/24p -Wthin -Gblack -O -K >> $ps
echo 4 5 21 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 5 5 | gmt psxy -R -J -Skgeo-lineation_vert/24p -Wthin -Gblack -O -K >> $ps
echo 5 5 22 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 6 5 60 | gmt psxy -R -J -Skgeo-lineation_hor/24p -Wthin -Gblack -O -K >> $ps
echo 6 5 23 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 7 5 150 40 | gmt psxy -R -J -Skgeo-lineation-2/24p -Wthin -Gblack -O -K >> $ps
echo 7 5 24 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps
echo 8 5 150 40 | gmt psxy -R -J -Skgeo-lineation-3/24p -Wthin -Gblack -O -K >> $ps
echo 8 5 25 | gmt pstext -R -J -F+f9p,29+jBC -D0/0.5c -O -K >> $ps

echo 0.7 9.9 Bedding | gmt pstext -R -J -F+f10p,29+jBL -N -O -K >> $ps
echo 0.7 7.9 Foliation, cleavage | gmt pstext -R -J -F+f10p,29+jBL -N -O -K >> $ps
echo 0.7 5.9 Joint | gmt pstext -R -J -F+f10p,29+jBL -N -O -K >> $ps
echo 3.7 5.9 Lineation | gmt pstext -R -J -F+f10p,29+jBL -N -O -K >> $ps
echo 0.5 10.4 Geology Symbols in GMT | gmt pstext -R -J -F+f14p,29+jBL -N -O -K >> $ps
gmt pstext -R -J -F+f7p,29+jLT -M -O -N >> $ps << END
> 0.5 4.3 6p 10.6c j
1 - Strike and dip of beds. 2 - Horizontal beds. 3 - Strike of vertical beds. 4 - Strike and dip of overturned beds. 5 - Strike and dip of bed with rake of lineation. 6 - Strike and dip direction of gently dipping beds. 7 - Strike and dip direction of moderatly dipping beds. 8 - Strike and dip direction of steeply dipping beds. 9 - Strike and dip of crenulated or undulated beds. 10 - Strike and dip of foliation. 11 - Horizontal foliation. 12 - Strike of vertical foliation. 13. Strike and dip of cleavage. 14 - Horizontal cleavage. 15 - Strike of vertical cleavage. 16 - Strike and dip of foliation 2. 17 - Strike and dip of foliation 3. 18 - Strike and dip of joints. 19 - Horizontal joints. 20 - Strike of vertical joints. 21 - Trend and plunge of lineation. 22 - Vertical lineation. 23 - Horizontal lineation. 24 - Trend and plunge of lineation 2. 25 - Trend and plunge of lineation 3.
END
