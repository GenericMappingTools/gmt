#!/bin/bash
#		GMT EXAMPLE 10
#		$Id$
#
# Purpose:	Make 3-D bar graph on top of perspective map
# GMT progs:	pscoast, pstext, psxyz
# Unix progs:	$AWK, rm
#
ps=example_10.ps
gmt pscoast -Rd -JX8id/5id -Dc -Sazure2 -Gwheat -Wfaint -A5000 -p200/40 -K > $ps
$AWK '{print $1, $2, $3}' languages.txt \
	| gmt pstext -R -J -O -K -p -D-0.2i/0 -F+f30p,Helvetica-Bold,firebrick=thinner+jRM >> $ps
gmt psxyz languages.txt -R-180/180/-90/90/0/2500 -J -JZ2.5i -So0.3i -Gdarkgreen -Wthinner \
	--FONT_TITLE=30p,Times-Bold --MAP_TITLE_OFFSET=-0.7i -O -p --FORMAT_GEO_MAP=dddF \
	-Bx60 -By30 -Bza500+lLanguages -BWSneZ+t"World Languages By Continent" >> $ps
