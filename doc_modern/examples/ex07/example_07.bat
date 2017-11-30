REM		GMT EXAMPLE 07
REM
REM		$Id$
REM
REM Purpose:	Make a basemap with earthquakes and isochrons etc
REM GMT progs:	pscoast, pslegend, pstext, psxy
REM DOS calls:	del, echo
REM
echo GMT EXAMPLE 07
set ps=example_07.ps
gmt pscoast -R-50/0/-10/20 -JM9i -K -Slightblue -GP26+r300+ftan+bdarkbrown -Dl -Wthinnest -B10 --FORMAT_GEO_MAP=dddF > %ps%
gmt psxy -R -J -O -K @fz_07.txt -Wthinner,- >> %ps%
gmt psxy @quakes_07.txt -R -J -O -K -h1 -Sci -i,1,2s0.01 -Gred -Wthinnest >> %ps%
gmt psxy -R -J -O -K @isochron_07.txt -Wthin,blue >> %ps%
gmt psxy -R -J -O -K @ridge_07.txt -Wthicker,orange >> %ps%
echo S 0.1i c 0.08i red thinnest 0.3i ISC Earthquakes > tmp
gmt pslegend -R -J -O -K -DjTR+w2.2i+o0.2i -F+pthick+ithinner+gwhite --FONT_ANNOT_PRIMARY=18p,Times-Italic tmp >> %ps%
echo -43 -5 SOUTH > tmp
echo -43 -8 AMERICA >> tmp
echo -7 11 AFRICA >> tmp
gmt pstext -R -J -O -F+f30,Helvetica-Bold,white=thin tmp >> %ps%
del .gmt*
del tmp
