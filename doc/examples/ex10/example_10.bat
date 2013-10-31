REM		GMT EXAMPLE 10
REM
REM		$Id$
REM
REM Purpose:	Make 3-D bar graph on top of perspective map
REM GMT progs:	pscoast, pstext, psxyz
REM DOS calls:	echo, del, gawk
REM
echo GMT EXAMPLE 10
set ps=example_10.ps
gmt pscoast -Rd -JX8id/5id -Dc -Sazure2 -Gwheat -Wfaint -A5000 -p200/40 -K > %ps%
echo {print $1, $2, $3} > awk.txt
gawk -f awk.txt languages.txt | gmt pstext -R -J -O -K -p -D-0.2i/0 -F+f30p,Helvetica-Bold,firebrick=thinner+jRM >> %ps%
gmt psxyz languages.txt -R-180/180/-90/90/0/2500 -J -JZ2.5il -So0.3i -Gdarkgreen -Wthinner -Bx60 -By30 -Bza500+lLanguages -BWSneZ+t"World Languages By Continent" -O -p --FORMAT_GEO_MAP=dddF >> %ps%
del awk.txt .gmt*
