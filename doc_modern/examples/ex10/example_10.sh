#!/bin/bash
#		GMT EXAMPLE 10
#		$Id$
#
# Purpose:	Make 3-D bar graph on top of perspective map
# GMT modules:	pscoast, pstext, psxyz, pslegend
# Unix progs:
#
ps=example_10.ps
gmt pscoast -Rd -JQ0/37.5/8i -Dc -Sazure2 -Gwheat -Wfaint -A5000 -p200/40 -K > $ps
gmt makecpt -Cpurple,blue,darkgreen,yellow,red -T0,1,2,3,4,5 > t.cpt
gmt math -T @languages_10.txt -o0-2 -C2 3 COL ADD 4 COL ADD 5 COL ADD 6 COL ADD = \
	| gmt pstext -R -J -O -K -p -Gwhite@30 -D-0.25i/0 \
	-F+f30p,Helvetica-Bold,firebrick=thinner+jRM+z >> $ps
gmt psxyz @languages_10.txt -R-180/180/-90/90/0/2500 -J -JZ2.5i -So0.3i+Z5 -Ct.cpt -Wthinner \
	--FONT_TITLE=30p,Times-Bold --MAP_TITLE_OFFSET=-0.7i -O -K -p --FORMAT_GEO_MAP=dddF \
	-Baf -Bza500+lLanguages -BWSneZ+t"World Languages By Continent" >> $ps
gmt pslegend -R -J -JZ -DjLB+o0.2i+w1.35i/0+jBL -O --FONT=Helvetica-Bold \
	-F+glightgrey+pthinner+s-4p/-6p/grey20@40 -p @legend_10.txt >> $ps
