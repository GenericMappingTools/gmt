#!/usr/bin/env bash
#
# Tests gmt project in Cartesian manipulations

ps=cartesian.ps

cat << EOF > pts.tt
1.7	1
-1	1.5
-1.5	-1.3
1.5	-0.5
EOF
cat << EOF >> axes.tt
> x-axis
-3	0
3	0
> y-axis
0	-3
0	3
EOF
cx=0
cy=0

makeproj () {
# Expects azimuth x y
echo "$2 $3" | gmt project -N -C$cx/$cy -A$1 -Fpqrs
}

makeaxis () {
# Expects azimuth file
gmt project -N -C$cx/$cy -A-$1 -Fpq $2
}

gmt set PS_CHAR_ENCODING ISOLatin1+

gmt pstext -R0/8.5/0/11 -Jx1i -F+jCB -P -Xa0 -Ya0 -K > $ps \
<<< "4.25 10.25 [x,y] (black dot). 2nd pair is (p,q) and third is (r,s) [plotted as red dot]"
ypos=1.25
By=Sn
for az in 30 135 200 290 ; do
	xpos=0.75
	az90=`gmt math -Q $az 90 ADD =`
	Bx=We
	while read x y; do
		makeaxis $az axes.tt > tt.a
		makeproj $az $x $y > tt.d
		gmt psbasemap -R-2/2/-2/2 -JX1.5i -B2g1 -B${Bx}${By} -O -K -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy 0 $az" | gmt psxy -R -J -O -K -SW0.2i -W0.25p -Xa$xpos -Ya$ypos >> $ps
		gmt psxy -R -J -O -K -W1p,red -Xa$xpos -Ya$ypos tt.a >> $ps
		echo "$cx $cy $az 0.75" | gmt psxy -R -J -O -K -SV0.15i+e+a60 -W0.5p -Gred -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy $az90 0.75" | gmt psxy -R -J -O -K -SV0.15i+e+a60 -W0.5p -Gred -Xa$xpos -Ya$ypos >> $ps
		makeproj -$az 1.75 0 > tt.x
		makeproj -$az 0 1.75 > tt.y
		$AWK '{printf "%s %s P", $1, $2}' tt.x | gmt pstext -R -J -F+f7p+a$az -O -K -A -Xa$xpos -Ya$ypos >> $ps
		$AWK '{printf "%s %s Q", $1, $2}' tt.y | gmt pstext -R -J -F+f7p+a$az90 -O -K -A -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy 0 0.75" | gmt psxy -R -J -O -K -SV0.15i+e+a60 -W0.5p -Gblack -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy 90 0.75" | gmt psxy -R -J -O -K -SV0.15i+e+a60 -W0.5p -Gblack -Xa$xpos -Ya$ypos >> $ps
		gmt pstext -R -J -O -K -F+f7p,white -Xa$xpos -Ya$ypos >> $ps <<< "1.75 0 x"
		gmt pstext -R -J -O -K -F+f7p,white -Xa$xpos -Ya$ypos >> $ps <<< "0 1.8 y"
		(echo "$x $y"; cut -f3,4 tt.d) | gmt psxy -R -J -O -K -W0.5p,- -Xa$xpos -Ya$ypos >> $ps
		echo "$x $y" | gmt psxy -R -J -O -K -Xa$xpos -Ya$ypos -Sc0.075i -Gblack >> $ps
		cut -f3,4 tt.d | gmt psxy -R -J -O -K -Xa$xpos -Ya$ypos -Sc0.075i -Gred >> $ps
		printf "0 2 [%s,%s] (%.2f,%.2f) (%.2f,%.2f)\n" $x $y `cat tt.d` | \
			gmt pstext -R -J -F+f8p+jCB -O -K -Xa$xpos -Ya$ypos -N -D0/0.2i >> $ps
		gmt pstext -R -J -O -K -Xa$xpos -Ya$ypos -N -D-0.05i/0.05i -Gwhite -W -F+f8p+jBR >> $ps <<< "2 -2 @~a@~ = $az@."
		xpos=`gmt math -Q $xpos 1.9 ADD =`
		Bx=we
	done < pts.tt
	ypos=`gmt math -Q $ypos 2.25 ADD =`
	By=sn
done
gmt psxy -R -J -O -T >> $ps

