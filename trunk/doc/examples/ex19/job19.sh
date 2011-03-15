#!/bin/bash
#		GMT EXAMPLE 19
#		$Id: job19.sh,v 1.20 2011-03-15 02:06:31 guru Exp $
#
# Purpose:	Illustrates various color pattern effects for maps
# GMT progs:	gmtset, grdimage, grdmath, makecpt, pscoast, pstext, psimage
# Unix progs:	rm
#
. ../functions.sh
ps=../example_19.ps

# First make a worldmap with graded blue oceans and rainbow continents

grdmath -Rd -I1 Y COSD 2 POW = lat.nc
grdmath -Rd -I1 X Y ABS 90 NEQ MUL = lon.nc
echo "0 white 1 blue" > lat.cpt
makecpt -Crainbow -T-180/180/360 -Z > lon.cpt
grdimage lat.nc -Sl -JI0/6.5i -Clat.cpt -P -K -Y7.5i -B0 > $ps
pscoast -R -J -O -K -Dc -A5000 -Gc >> $ps
grdimage lon.nc -Sl -J -Clon.cpt -O -K >> $ps
pscoast -R -J -O -K -Q >> $ps
pscoast -R -J -O -K -Dc -A5000 -Wthinnest >> $ps
echo "0 20 9TH INTERNATIONAL" | pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -10 GMT CONFERENCE" | pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -30 Honolulu, Hawaii, April 1, 2011" | pstext -R -J -O -K \
	-F+f18p,Helvetica-Bold,green=thinnest >> $ps

# Then show example of color patterns and placing a PostScript image

pscoast -R -J -O -K -Dc -A5000 -Gp100/86:FredByellow -Sp100/circuit.ras -B0 -Y-3.25i >> $ps
echo "0 30 SILLY USES OF" | pstext -R -J -O -K -F+f32p,Helvetica-Bold,lightgreen=thinner >> $ps
echo "0 -30 COLOR PATTERNS" | pstext -R -J -O -K -F+f32p,Helvetica-Bold,magenta=thinner >> $ps
psimage -C3.25i/1.625i/CM -W3i GMT_covertext.eps -O -K >> $ps

# Finally repeat 1st plot but exchange the patterns

grdimage lon.nc -Sl -J -Clon.cpt -O -K -Y-3.25i -B0 -U"Example 19 in Cookbook" >> $ps
pscoast -R -J -O -K -Dc -A5000 -Gc >> $ps
grdimage lat.nc -Sl -J -Clat.cpt -O -K >> $ps
pscoast -R -J -O -K -Q >> $ps
pscoast -R -J -O -K -Dc -A5000 -Wthinnest >> $ps
echo "0 20 9TH INTERNATIONAL" | pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -10 GMT CONFERENCE" | pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -30 Honolulu, Hawaii, April 1, 2011" | pstext -R -J -O \
	-F+f18p,Helvetica-Bold,green=thinnest >> $ps

rm -f l*.nc l*.cpt
