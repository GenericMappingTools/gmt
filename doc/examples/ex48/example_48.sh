#!/bin/bash
ps=global.ps
pdf=global.pdf
cat << EOF > spots.txt
157.8583W	21.3069N	61	300	HNL	BC	0.6i
149.5585W	17.5516S	-120	120	PPT	TC	0.6i
139.6917E	35.6895N	56	170	HND	RB	0.3i
70.6693W	33.4489S	215	322	SCL	TL	0.25i
151.2070E	33.8675S	-10	145	SYD	TR	0.85i
118.2437W	34.0522N	142	306	LAX	BL	0.80i
EOF
gmt makecpt -Ccategorical -T0/9/1 > t.cpt
echo "-12000	lightgray	12000 lightgray" > g.cpt
gmt sphtriangulate spots.txt -Qv > patches.txt
gmt grdgradient etopo10m.nc -Nt2 -A45 -Gint.nc
gmt grdimage etopo10m.nc -Iint.nc -Cg.cpt -Rg -JG205/-10/7i -P -K -Xc > $ps
gmt psxy -R -J -O -K patches.txt -L -Ct.cpt -t65 >> $ps
gmt psxy -R -J -O -K -SW2000k -Gwhite@40 spots.txt >> $ps
gmt pscoast -R -J -Gblack -A500 -O -K -Bafg >> $ps
# Make a 15 degrees by 250 km gridlines around each airport
gmt math -T500/2000/250 -o0 T = tmp
while read radius; do
	gmt psxy -R -J -O -K -SW${radius}k+a -W0.5p spots.txt >> $ps
done < tmp
daz=15
while read lon lat az1 az2 label just off; do
	az1=`gmt math -Q $az1 $daz DIV CEIL  $daz MUL =`
	az2=`gmt math -Q $az2 $daz DIV FLOOR $daz MUL =`
	gmt math -T${az1}/${az2}/$daz -N4/2 -fg -C0 0 $lon ADD -C1 $lat ADD -C3 2000 ADD = | gmt psxy -R -J -O -K -S=0.1 -W0.5p >> $ps
	echo $lon $lat $label | gmt pstext -R -J -O -K -DJ${off}v0.5p,white -F+f16p+j${just} -N >> $ps
	echo $lon $lat $label | gmt pstext -R -J -O -K -DJ${off}v0.25p -F+f16p+j${just} -N -Gwhite -W0.25p -TO >> $ps
done < spots.txt
gmt psxy spots.txt -R -J -O -K -Fn -W1.5p+o250k+v0.2i+gred --MAP_VECTOR_SHAPE=0.5 >> $ps
gmt psxy spots.txt -R -J -O -K -SE-500 -Gorange -W0.25p >> $ps
# Make an arc or radius 4.5 inches from 45 to 135 degrees and use it to place text
gmtmath -T45/135/1 T -C0 COSD -C1 SIND -Ca 4.5 MUL = path.txt
gmt psxy -R-3.5/3.5/0/6 -Jx1i -O -Y3.5i path.txt -Sqn1:+l"IMPORTANT PACIFIC AIRPORTS"+v+f32p -Wfaint,white -N >> $ps
#gmt psxy -R -J -O -T >> $ps
gmt psconvert -Tf -P $ps
open $pdf
gmt set PS_COMMENTS false
rm -f spots.txt g.cpt t.cpt int.nc patches.txt path.txt tmp
