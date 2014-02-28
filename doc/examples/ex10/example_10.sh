#!/bin/bash
#		GMT EXAMPLE 10
#		$Id$
#
# Purpose:	Make 3-D bar graph on top of perspective map
# GMT progs:	pscoast, pstext, psxyz, pslegend
# Unix progs:	$AWK
#
ps=example_10.ps
gmt pscoast -Rd -JX8id/5id -Dc -Sazure2 -Gwheat -Wfaint -A5000 -p200/40 -K > $ps
$AWK '{print $1, $2, $3+$4+$5+$6+$7}' languages.txt \
	| gmt pstext -R -J -O -K -p -Gwhite@30 -D-0.25i/0 \
	-F+f30p,Helvetica-Bold,firebrick=thinner+jRM >> $ps
gmt psxyz languages.txt -R-180/180/-90/90/0/2500 -J -JZ2.5i -So0.3i -Gpurple -Wthinner \
	--FONT_TITLE=30p,Times-Bold --MAP_TITLE_OFFSET=-0.7i -O -K -p --FORMAT_GEO_MAP=dddF \
	-Bx60 -By30 -Bza500+lLanguages -BWSneZ+t"World Languages By Continent" >> $ps
$AWK '{print $1, $2, $3+$4, $3}' languages.txt \
	| gmt psxyz -R -J -JZ -So0.3ib -Gblue -Wthinner -O -K -p >> $ps
$AWK '{print $1, $2, $3+$4+$5, $3+$4}' languages.txt \
	| gmt psxyz -R -J -JZ -So0.3ib -Gdarkgreen -Wthinner -O -K -p >> $ps
$AWK '{print $1, $2, $3+$4+$5+$6, $3+$4+$5}' languages.txt \
	| gmt psxyz -R -J -JZ -So0.3ib -Gyellow -Wthinner -O -K -p >> $ps
$AWK '{print $1, $2, $3+$4+$5+$6+$7, $3+$4+$5+$6}' languages.txt \
	| gmt psxyz -R -J -JZ -So0.3ib -Gred -Wthinner -O -K -p >> $ps
gmt pslegend -R -J -JZ -D-170/-80/1.35i/0/BL -O --FONT=Helvetica-Bold \
	-F+glightgrey+pthinner+s-4p/-6p/grey20@40 -p legend.txt >> $ps
