#!/bin/bash
#		GMT EXAMPLE 19
#		$Id$
#
# Purpose:	Illustrates various color pattern effects for maps
# GMT progs:	gmtset, grdimage, grdmath, makecpt, pscoast, pstext, psimage
# Unix progs:	rm
#
ps=example_19.ps

# First make a worldmap with graded blue oceans and rainbow continents

gmt grdmath -Rd -I1 -r Y COSD 2 POW = lat.nc
gmt grdmath -Rd -I1 -r X = lon.nc
echo "0 white 1 blue" > lat.cpt
gmt makecpt -Crainbow -T-180/180/360 -Z > lon.cpt
gmt grdimage lat.nc -JI0/6.5i -Clat.cpt -P -K -Y7.5i -B0 -nl > $ps
gmt pscoast -R -J -O -K -Dc -A5000 -Gc >> $ps
gmt grdimage lon.nc -J -Clon.cpt -O -K -nl >> $ps
gmt pscoast -R -J -O -K -Q >> $ps
gmt pscoast -R -J -O -K -Dc -A5000 -Wthinnest >> $ps
echo "0 20 12TH INTERNATIONAL" | gmt pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -10 GMT CONFERENCE" | gmt pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -30 Honolulu, Hawaii, April 1, 2015" | gmt pstext -R -J -O -K \
	-F+f18p,Helvetica-Bold,green=thinnest >> $ps

# Then show example of color patterns and placing a PostScript image

gmt pscoast -R -J -O -K -Dc -A5000 -Gp100/86:FredByellow -Sp100/circuit.ras -B0 -Y-3.25i >> $ps
echo "0 30 SILLY USES OF" | gmt pstext -R -J -O -K -F+f32p,Helvetica-Bold,lightgreen=thinner >> $ps
echo "0 -30 COLOR PATTERNS" | gmt pstext -R -J -O -K -F+f32p,Helvetica-Bold,magenta=thinner >> $ps
gmt psimage -C3.25i/1.625i/CM -W3i GMT_covertext.eps -O -K >> $ps

# Finally repeat 1st plot but exchange the patterns

gmt grdimage lon.nc -J -Clon.cpt -O -K -Y-3.25i -B0 -nl >> $ps
gmt pscoast -R -J -O -K -Dc -A5000 -Gc >> $ps
gmt grdimage lat.nc -J -Clat.cpt -O -K -nl >> $ps
gmt pscoast -R -J -O -K -Q >> $ps
gmt pscoast -R -J -O -K -Dc -A5000 -Wthinnest >> $ps
echo "0 20 12TH INTERNATIONAL" | gmt pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -10 GMT CONFERENCE" | gmt pstext -R -J -O -K -F+f32p,Helvetica-Bold,red=thinner >> $ps
echo "0 -30 Honolulu, Hawaii, April 1, 2015" | gmt pstext -R -J -O \
	-F+f18p,Helvetica-Bold,green=thinnest >> $ps

rm -f l*.nc l*.cpt gmt.conf
