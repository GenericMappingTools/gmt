#!/usr/bin/env bash
#
#	This script makes the documentation in Appendix E.
#
#	Paul Wessel, v 1.1
#
xwidth=0.45	# Width of each box (all units are in inches)
ywidth=0.45	# Height of each box
w=0.9		# Width of two adjacent boxes
dx=0.50		# Amont to translate to the right
dy=0.50		# Amount to translate up
y=0.05		# Initial offset in x
x=0.05		# Initial offset in y
back=-5.20	# Amount to translate to left after 1 row

cat << END > tt.App_E.d
0	0
$xwidth	0
$xwidth	$ywidth
0	$ywidth
END

gmt begin GMT_App_E
gmt basemap -R0/5.75/0/7.55 -Jx1i -B0
gmt set MAP_FRAME_PEN thinner
for iy in 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
do
	for ix in 1 2 3 4 5 6
	do
		let p=iy*6+ix
		gmt plot -R0/$xwidth/0/$ywidth -Jx1i -Gp$p+r300 tt.App_E.d -X${x}i -Y${y}i -B0
		gmt plot -GP$p+r300 tt.App_E.d -X${xwidth}i -B0
		echo "0 0.225" | gmt plot -R0/$w/0/$ywidth -N -Sc0.17i -Wthinnest -Gwhite
		echo "0 0.225 $p" | gmt text -R0/$w/0/$ywidth -N -F+f9p,Helvetica-Bold
		y=0.0
		x=$dx
	done
	y=$dy
	x=$back
done
gmt end show
