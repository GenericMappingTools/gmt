REM             GMT EXAMPLE 42
REM             $Id$
REM
REM Purpose:      Illustrate Antarctica and stereographic projection
REM GMT progs:    gmtset, makecpt, grdimage, pscoast, pslegend, psscale, pstext, psxy [grdconvert]
REM DOS calls:	  [curl]
REM

echo GMT EXAMPLE 42
set ps=example_42.ps

gmt set FONT_ANNOT_PRIMARY 12p FONT_LABEL 12p PROJ_ELLIPSOID WGS-84 FORMAT_GEO_MAP dddF
REM Data obtained via website and converted to netCDF thus:
REM curl http://www.antarctica.ac.uk//bas_research/data/access/bedmap/download/bedelev.asc.gz
REM gunzip bedelev.asc.gz
REM grdconvert bedelev.asc BEDMAP_elevation.nc=ns -V

gmt makecpt -Cearth -T-7000/4000 > z.cpt
gmt grdimage -Cz.cpt BEDMAP_elevation.nc -Jx1:60000000 -Q -P -K > %ps%
gmt pscoast -R-180/180/-90/-60 -Js0/-90/-71/1:60000000 -Bafg -Di -W0.25p -O -K >> %ps%
gmt psscale -Cz.cpt -DjRM+w2.5i/0.2i+o0.5i/0+jLM+mc -R -J -O -K -F+p+i -Bxa1000+lELEVATION -By+lm >> %ps%
REM GSHHG
gmt pscoast -R -J -Di -Glightblue -Sroyalblue2 -O -K -X2i -Y4.75i >> %ps%
gmt pscoast -R -J -Di -Glightbrown -O -K -A+ag -Bafg >> %ps%
echo H 18 Times-Roman Legend > legend.txt
echo D 0.1i 1p >> legend.txt
echo S 0.15i s 0.2i blue  0.25p 0.3i Ocean >> legend.txt
echo S 0.15i s 0.2i lightblue  0.25p 0.3i Ice front >> legend.txt
echo S 0.15i s 0.2i lightbrown  0.25p 0.3i Grounding line >> legend.txt
gmt pslegend -DjLM+w1.7i+jRM+o0.5i/0 -R -J -O -K -F+p+i legend.txt >> %ps%
REM Fancy line
echo 0 5.55 > line
echo 2.5 5.55 >> line
echo 5.0 4.55 >> line
echo 7.5 4.55 >> line
gmt psxy -R0/7.5/0/10 -Jx1i -O -K -B0 -W2p -X-2.5i -Y-5.25i line >> %ps%
echo 0 5.2 BEDMAP > labels
echo 0 9.65 GSHHG >> labels
gmt pstext -R -J -O -F+f18p+jBL -Dj0.1i/0 labels >> %ps%
