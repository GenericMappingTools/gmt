REM		GMT EXAMPLE 09
REM
REM
REM Purpose:	Make wiggle plot along track from geoid deflections
REM GMT progs:	gmtconvert, pswiggle, pstext, psxy
REM DOS calls:	del
REM
echo GMT EXAMPLE 09
set ps=example_09.ps
gmt pswiggle @tracks_09.txt -R185/250/-68/-42 -K -Jm0.13i -Ba10f5 -BWSne+g240/255/240 -G+red -G-blue -Z2000 -Wthinnest -DjBR+w500+l@~m@~rad+o0.2i > %ps%
gmt psxy -R -J -O -K @ridge_09.txt -Wthicker >> %ps%
gmt psxy -R -J -O -K @fz_09.txt -Wthinner,- >> %ps%
REM Take label from segment header and plot near coordinates of last record of each track
gmt convert -El @tracks_09.txt | gmt pstext -R -J -F+f10p,Helvetica-Bold+a50+jRM+h -D-0.05i -O >> %ps%
del .gmt*
