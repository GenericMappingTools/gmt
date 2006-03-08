#!/bin/sh
#	$Id: cartesian.sh,v 1.1 2006-03-08 22:27:56 pwessel Exp $
#
# Tests project in Cartesian manipulations

cat << EOF > azim.$$
30
135
200
290
EOF
cat << EOF > pts.$$
1.75	1
-1	1.5
-1.5	-1.25
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
#
makeproj () {
# Expects azimuth cx cy x y
echo "$4 $5" | project -N -C$2/$3 -A$1 -Fpqrs
}
makeaxis () {
# Expects azimuth cx cy file
project -N -C$2/$3 -A$1 -Fpq $4 -M
}

psxy -R0/8.5/0/11 -Jx1i -P -Xa0 -Ya0 -K /dev/null -U/0.75i/0.5i/"(p,q) (r,s)" > cartesian.ps
ypos=1.25
By=Sn
while read az; do
	xpos=0.75
	az90=`gmtmath -Q $az 90 SUB =`
	Bx=We
	while read x y; do
		makeproj $az $cx $cy $x $y > $$.d
		psbasemap -R-2/2/-2/2 -JX1.5i -B2g1${Bx}${By} -O -K -Xa$xpos -Ya$ypos >> cartesian.ps
		makeaxis $az $cx $cy axes.$$ | psxy -R -J -O -K -M -W1p -Xa$xpos -Ya$ypos >> cartesian.ps
		echo "$cx $cy $az 1" | psxy -R -J -O -K -SV0.01/0.12/0.05 -Ggreen -Xa$xpos -Ya$ypos >> cartesian.ps
		echo "$cx $cy $az90 1" | psxy -R -J -O -K -SV0.01/0.12/0.05 -Ggreen -Xa$xpos -Ya$ypos >> cartesian.ps
		echo $x $y | psxy -R -J -O -K -Xa$xpos -Ya$ypos -Sc0.075i -Gblack >> cartesian.ps
		cut -f3,4 $$.d | psxy -R -J -O -K -Xa$xpos -Ya$ypos -Sc0.075i -Gred >> cartesian.ps
		awk '{printf "0 2 9 0 0 CB (%.2f, %.2f) (%.2f, %.2f)\n", $1, $2, $3, $4}' $$.d | pstext -R -J -O -K -Xa$xpos -Ya$ypos -N -D0/0.2i >> cartesian.ps
		xpos=`gmtmath -Q $xpos 2 ADD =`
		Bx=we
	done < pts.$$
	ypos=`gmtmath -Q $ypos 2.25 ADD =`
	By=sn
done < azim.$$
psxy -R -J -O /dev/null >> cartesian.ps
gv cartesian.ps &
rm -f $$.*
