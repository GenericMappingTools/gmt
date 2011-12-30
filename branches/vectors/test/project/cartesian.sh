#!/bin/bash
#	$Id$
#
# Tests project in Cartesian manipulations

. ../functions.sh
header "Test project for rotating Cartesian data"

cat << EOF > pts.$$
1.7	1
-1	1.5
-1.5	-1.3
1.5	-0.5
EOF
cat << EOF >> axes.$$
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
echo "$2 $3" | project -N -C$cx/$cy -A$1 -Fpqrs
}

makeaxis () {
# Expects azimuth file
project -N -C$cx/$cy -A-$1 -Fpq $2
}

gmtset PS_CHAR_ENCODING ISOLatin1+

ps=cartesian.ps
pstext -R0/8.5/0/11 -Jx1i -F+jCB -P -Xa0 -Ya0 -K > $ps \
<<< "4.25 10.25 [x,y] (black dot). 2nd pair is (p,q) and third is (r,s) [plotted as red dot]"
ypos=1.25
By=Sn
for az in 30 135 200 290 ; do
	xpos=0.75
	az90=`gmtmath -Q $az 90 ADD =`
	Bx=We
	while read x y; do
		makeaxis $az axes.$$ > $$.a
		makeproj $az $x $y > $$.d
		psbasemap -R-2/2/-2/2 -JX1.5i -B2g1${Bx}${By} -O -K -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy 0 $az" | psxy -R -J -O -K -SW0.2 -W0.25p -Xa$xpos -Ya$ypos >> $ps
		psxy -R -J -O -K -W1p,red -Xa$xpos -Ya$ypos $$.a >> $ps
		echo "$cx $cy $az 0.75" | psxy -R -J -O -K -SV0.01/0.15/0.1 -Gred -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy $az90 0.75" | psxy -R -J -O -K -SV0.01/0.15/0.1 -Gred -Xa$xpos -Ya$ypos >> $ps
		makeproj -$az 1.75 0 > $$.x
		makeproj -$az 0 1.75 > $$.y
		awk '{printf "%s %s P", $1, $2}' $$.x | pstext -R -J -F+f7p+a$az -O -K -A -Xa$xpos -Ya$ypos >> $ps
		awk '{printf "%s %s Q", $1, $2}' $$.y | pstext -R -J -F+f7p+a$az90 -O -K -A -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy 0 0.75" | psxy -R -J -O -K -SV0.01/0.15/0.1 -Gblack -Xa$xpos -Ya$ypos >> $ps
		echo "$cx $cy 90 0.75" | psxy -R -J -O -K -SV0.01/0.15/0.1 -Gblack -Xa$xpos -Ya$ypos >> $ps
		pstext -R -J -O -K -F+f7p,white -Xa$xpos -Ya$ypos >> $ps <<< "1.75 0 x"
		pstext -R -J -O -K -F+f7p,white -Xa$xpos -Ya$ypos >> $ps <<< "0 1.8 y"
		(echo "$x $y"; cut -f3,4 $$.d) | psxy -R -J -O -K -W0.5p,- -Xa$xpos -Ya$ypos >> $ps
		echo "$x $y" | psxy -R -J -O -K -Xa$xpos -Ya$ypos -Sc0.075i -Gblack >> $ps
		cut -f3,4 $$.d | psxy -R -J -O -K -Xa$xpos -Ya$ypos -Sc0.075i -Gred >> $ps
		printf "0 2 [%s,%s] (%.2f,%.2f) (%.2f,%.2f)\n" $x $y `cat $$.d` | \
			pstext -R -J -F+f8p+jCB -O -K -Xa$xpos -Ya$ypos -N -D0/0.2i >> $ps
		pstext -R -J -O -K -Xa$xpos -Ya$ypos -N -D-0.05i/0.05i -Gwhite -W -F+f8p+jBR >> $ps <<< "2 -2 @~a@~ = $az\\232"
		xpos=`gmtmath -Q $xpos 1.9 ADD =`
		Bx=we
	done < pts.$$
	ypos=`gmtmath -Q $ypos 2.25 ADD =`
	By=sn
done
psxy -R -J -O -T >> $ps

rm -f $$.* *.$$

pscmp
