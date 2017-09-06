#!/bin/bash
#		GMT EXAMPLE 19
#
# Purpose:	Illustrates various color pattern effects for maps
# GMT modules:	grdimage, grdmath, makecpt, pscoast, pstext, psimage
# Unix progs:	echo, rm
# PW: Want no annotation or ticks but are getting them.
export GMT_PPID=$$
gmt begin ex19 ps
  gmt grdmath -Rd -I1 -r Y COSD 2 POW = lat.nc
  gmt grdmath X = lon.nc
  gmt makecpt -Cwhite,blue -T0,1 -Z -N > lat.cpt
  gmt makecpt -Crainbow -T-180/180 > lon.cpt
  gmt subplot begin 3x1 -Fs6.5i/3.25i -M0 -Lbltr
#   First make a worldmap with graded blue oceans and rainbow continents
    gmt grdimage lat.nc -JI0 -Clat.cpt -nl -c1,1
    gmt pscoast -Dc -A5000 -Gc
    gmt grdimage lon.nc -Clon.cpt -nl
    gmt pscoast -Q
    gmt pscoast -Dc -A5000 -Wthinnest
    echo "0 20 15TH INTERNATIONAL" | gmt pstext -F+f32p,Helvetica-Bold,red=thinner
    echo "0 -10 GMT CONFERENCE" | gmt pstext -F+f32p,Helvetica-Bold,red=thinner
    echo "0 -30 Honolulu, Hawaii, April 1, 2018" | gmt pstext -F+f18p,Helvetica-Bold,green=thinnest
#   Then show example of color patterns and placing a PostScript image
    gmt pscoast -Dc -A5000 -Gp86+fred+byellow+r100 -Sp@circuit.ras+r100 -c2,1
    echo "0 30 SILLY USES OF" | gmt pstext -F+f32p,Helvetica-Bold,lightgreen=thinner
    echo "0 -30 COLOR PATTERNS" | gmt pstext -F+f32p,Helvetica-Bold,magenta=thinner
    gmt psimage -DjCM+w3i @GMT_covertext.eps
#   Finally repeat 1st plot but exchange the colors
    gmt grdimage lon.nc -Clon.cpt -nl -c3,1
    gmt pscoast -Dc -A5000 -Gc
    gmt grdimage lat.nc -Clat.cpt -nl
    gmt pscoast -Q
    gmt pscoast -Dc -A5000 -Wthinnest
    echo "0 20 15TH INTERNATIONAL" | gmt pstext -F+f32p,Helvetica-Bold,red=thinner
    echo "0 -10 GMT CONFERENCE" | gmt pstext -F+f32p,Helvetica-Bold,red=thinner
    echo "0 -30 Honolulu, Hawaii, April 1, 2018" | gmt pstext -F+f18p,Helvetica-Bold,green=thinnest
  gmt subplot end
gmt end
rm -f lat.nc lon.nc lat.cpt lon.cpt
