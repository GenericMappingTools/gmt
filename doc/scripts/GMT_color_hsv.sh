#!/usr/bin/env bash
#
# Show a slice through RGB cube for h = 60 (yellow) and illustrate effect of intensity changes
# Cheating and rotating the slice to the horizontal
ps=GMT_color_hsv.ps
x=4
r=217
b=54
smin=1.0
smax=0.1
vmin=0.3
vmax=1.0
y=`gmt math -Q $x 2 SQRT MUL =`
gmt grdmath -I1 -R0/255/0/255 Y 256 MUL X ADD = rgb_cube.grd
gmt grdmath -I1 -R0/255/0/255 Y 255 DIV = v.grd
gmt grdmath -I1 -R0/255/0/255 1 X Y DIV SUB X Y LE MUL = s.grd
gmt math -T0/255/1 -N3/0  T 256 MUL 0.5 SUB -C2 256 ADD  = | awk '{printf "%s\t%d/%d/0\t%s\t%d/%d/255\n", $2, $1, $1, $3, $1, $1}' > rgb_cube.cpt
gmt psxy -R0/8/0/8 -Jx1i -P -K -X0.75i -Y2i -T > $ps
cat << EOF >> $ps
gsave
-54.735611 rotate
EOF
cat << EOF > path.txt
0	0
255	255
0	255
0	0
EOF
gmt psclip -R0/255/0/255 -JX${x}i/${y}i path.txt -O -K >> $ps
gmt grdimage rgb_cube.grd -Crgb_cube.cpt -JX -O -K >> $ps
cat << EOF > t.txt
0.1	C
0.3	C
0.5	C
0.7	C
0.9	C
EOF
gmt grdcontour v.grd -C0.1 -Gd1 -W0.5p -JX -O -K >> $ps
gmt grdcontour s.grd -C0.1 -Gd1 -W0.5p,- -JX -O -K >> $ps
gmt psclip -O -K -C >> $ps
gmt pstext -R -JX -O -K -N -Dj0.06i << EOF >> $ps
0	-7	18	54.735611	1	LT	B
252	245	18	54.735611	1	LT	W
5	270	18	54.735611	1	CB	Y
# Values
0	260	12	0	0	RT	1.0
0	209	12	0	0	RT	0.8
0	158	12	0	0	RT	0.6
0	107	12	0	0	RT	0.4
-2	82	12	0	6	RT	(s@-min@-, v@-min@-)
0	56	12	0	0	RT	0.2
0	5	12	0	0	RT	0.0
# Saturations
235	255	12	54.735611	6	LB	(s@-max@-, v@-max@-)
260	253	12	54.735611	0	LB	0.0
209	253	12	54.735611	0	LB	0.2
158	253	12	54.735611	0	LB	0.4
107	253	12	54.735611	0	LB	0.6
56	253	12	54.735611	0	LB	0.8
5	253	12	54.735611	0	LB	1.0
-20	127.5	14	90		1	CB	Value
127.5	270	14	0		1	BC	Saturation
146	125	14	54.7356		1	BC	Gray axis
-80	150	12	54.735611	0	RM	(h, s, v) = (60\232, 0.75, 0.85)
-100	151	12	54.735611	0	RM	(r, b, b) = (217, 217, 54)
EOF
gmt psxy -R -JX -O -K -W1p path.txt >> $ps
echo "$b $r" | gmt psxy -R -JX -O -K -Sc9p -Gred -W0.5p >> $ps
# Make the RGB path (R=G, B) path that is returned by GMT_illuminate
# First make s(i)
gmt math -T-1/0/0.01 0.75 $smin SUB T MUL 0.75 ADD = saturation.txt
gmt math -T0/1/0.01 $smax 0.75 SUB T MUL 0.75 ADD = >> saturation.txt
# Then make v(i)
gmt math -T-1/0/0.01 0.85 $vmin SUB T MUL 0.85 ADD = value.txt
gmt math -T0/1/0.01 $vmax 0.85 SUB T MUL 0.85 ADD = >> value.txt
# Create hsv
cat << EOF | gmt math STDIN HSV2RGB = | awk '{print $3, $1}' | gmt psxy -R -JX -O -K -W1p -Sc0.1i -Gwhite -N >> $ps
60	$smax	$vmax
60	$smin	$vmin
EOF
paste saturation.txt value.txt | awk '{print 60, $2, $4}' | gmt math STDIN HSV2RGB = | awk '{print $3, $1}' > curve.txt
gmt  psxy -R -JX -O -K -W2p curve.txt >> $ps
gmt  psxy -R -JX -O -K -W0.25p,white curve.txt >> $ps
gmt  psxy -R -JX -O -K -Sv14p+e+jc+h0.5 -G0 -W2p << EOF >> $ps
145 230 17 2i
35	150 -107 2i
EOF
gmt pstext -R -JX -O -K -N -Dj0.05/0.05 -Gwhite << EOF >> $ps
145	230	12	17	0	MC	Intensity @~\256@~ +1
35	150	12	73	0	MC	\0551 @~\254@~ Intensity 
EOF
echo "grestore" >> $ps
gmt psxy -R -JX -O -T >> $ps
rm -f t.txt s.grd v.grd rgb_cube.cpt rgb_cube.grd curve.txt path.txt saturation.txt value.txt
