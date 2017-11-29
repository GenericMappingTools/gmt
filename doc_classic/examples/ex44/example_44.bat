REM             GMT EXAMPLE 44
REM             $Id$
REM
REM Purpose:      Illustrate use of map inserts
REM GMT progs:    pscoast, psbasemap, mapproject
REM DOS calls:	  Would need equivalent of UNIX read
REM

echo GMT EXAMPLE 44
set ps=example_44.ps

REM Bottom map of Australia
gmt pscoast -R110E/170E/44S/9S -JM6i -P -Baf -BWSne -Wfaint -N2/1p  -EAU+gbisque -Gbrown -Sazure1 -Da -K -Xc --FORMAT_GEO_MAP=dddF > %ps%
gmt psbasemap -R -J -O -K -DjTR+w1.5i+o0.15i/0.1i+stmp -F+gwhite+p1p+c0.1c+s >> %ps%
REM read x0 y0 w h < tmp
gmt pscoast -Rg -JG120/30S/3.81c -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque -O -K -X11.049c -Y6.05864582401c >> %ps%
gmt psxy -R -J -O -K -T  -X-11.049c -Y-6.05864582401c >> %ps%
REM Determine size of insert map of Europe
gmt mapproject -R15W/35E/30N/48N -JM2i -W > tmp
REM read w h < tmp
gmt pscoast -R10W/5E/35N/44N -JM6i -Baf -BWSne -EES+gbisque -Gbrown -Wfaint -N1/1p -Sazure1 -Df -O -K -Y4.5i --FORMAT_GEO_MAP=dddF >> %ps%
gmt psbasemap -R -J -O -K -DjTR+w5.08c/2.36650592089c+o0.15i/0.1i+stmp -F+gwhite+p1p+c0.1c+s >> %ps%
REM read x0 y0 w h < tmp
gmt pscoast -R15W/35E/30N/48N -JM$w -Da -Gbrown -B0 -EES+gbisque -O -K -X9.779c -Y9.21137679749c --MAP_FRAME_TYPE=plain >> %ps%
gmt psxy -R -J -O -T -X-9.779c -Y-9.21137679749c >> %ps%
del tmp
