#!/bin/sh
#	$Id: GMT_App_E.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#
#	This script makes the documentation in Appendix E.
#
#	Paul Wessel, v 1.1
#

xwidth=0.45	# Width of each box
ywidth=0.45	# Height of each box
w=0.9		# Width of two adjacent boxes
dx=0.55		# Amont to translate to the right
dy=0.50		# Amount to translate up
y=0.05		# Initial offset in x
x=0.05		# Initial offset in y
back=-5.45	# Amount to translate to left after 1 row

cat << END > App_E.d
0	0
$xwidth	0
$xwidth	$ywidth
0	$ywidth
END

psbasemap -R0/6.0/0/7.55 -Jx1 -P -B0 -K > GMT_App_E.ps
for iy in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14
do
	for ix in 1 2 3 4 5 6
	do
		p=`echo "$iy * 6 + $ix" | bc`
		psxy -R0/$xwidth/0/$ywidth -Jx1 -Gp0/$p -O -K App_E.d -X$x -Y$y >> GMT_App_E.ps
		psxy -R -Jx -W2 -L -O -K App_E.d >> GMT_App_E.ps
		psxy -R -Jx -GP0/$p -O -K App_E.d -X$xwidth >> GMT_App_E.ps
		psxy -R -Jx -W2 -L -O -K App_E.d >> GMT_App_E.ps
		echo "0 0.225" | psxy -R0/$w/0/$ywidth -Jx -O -K -N -Sc0.17 -W1 -G255 >> GMT_App_E.ps
		echo "0 0.225 9 0 1 CM $p" | pstext -R0/$w/0/$ywidth -Jx -O -K -N >> GMT_App_E.ps
		y=0.0
		x=$dx
	done
	y=$dy
	x=$back
done

\rm -f App_E.d
