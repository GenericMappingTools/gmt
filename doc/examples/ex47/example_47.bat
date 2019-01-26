REM             GMT EXAMPLE 47
REM
REM Purpose:      Illustrate use of gmtregress with different norms and types
REM GMT progs:    gmtregress, psxy

echo GMT EXAMPLE 47
set ps=example_47.ps

gmt which -Gl @hertzsprung-russell.txt
REM Make a sub-function for plotting one panel
echo "gmt regress data -Fxm %1 %2 -T2.85/5.25/0.1 > tmp" > plot_one.bat
echo "gmt psxy -R -J -Bxafg -Byafg -B%3+gbisque -O -K %4 %5 data -Sc0.05i -Gblue" >> plot_one.bat
echo "gmt psxy -R -J -O -K %4 %5 giants -Sc0.1i -W0.25p -N" >> plot_one.bat
echo "gmt psxy -R -W1p,red -J -O -K %4 %5 tmp -W1p" >> plot_one.bat

REM Allow outliers (commented out by #) to be included in the analysis:
sed -e s/#//g hertzsprung-russell.txt > data
REM Identify the red giants (outliers)
grep '#' hertzsprung-russell.txt | sed -e s/#//g > giants

gmt psxy -R2.85/5.25/3.9/6.3 -JX-2i/2i -T -P -K -Xa1i -Ya1i > %ps%
REM L1
CALL plot_one -Er -N1 WSne -Xa1.2i -Ya01i >> %ps%
CALL plot_one -Eo -N1 Wsne -Xa1.2i -Ya3.25i >> %ps%
CALL plot_one -Ex -N1 Wsne -Xa1.2i -Ya5.5i >> %ps%
CALL plot_one -Ey -N1 WsNe+tL1 -Xa1.2i -Ya7.75i >> %ps%
REM L2
CALL plot_one -Er -N2 wSne -Xa3.3i -Ya1i >> %ps%
CALL plot_one -Eo -N2 wsne -Xa3.3i -Ya3.25i >> %ps%
CALL plot_one -Ex -N2 wsne -Xa3.3i -Ya5.5i >> %ps%
CALL plot_one -Ey -N2 wsNe+tL2 -Xa3.3i -Ya7.75i >> %ps%
REM LMS
CALL plot_one -Er -Nr weSn -Xa5.4i -Ya1i >>%ps%
CALL plot_one -Eo -Nr wesn -Xa5.4i -Ya3.25i >> %ps%
CALL plot_one -Ex -Nr wesn -Xa5.4i -Ya5.5i >> %ps%
CALL plot_one -Ey -Nr wesN+tLMS -Xa5.4i -Ya7.75i >> %ps%
REM Labels
echo REDUCED MAJOR AXIS | gmt pstext -R -J -O -K -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya1i >> %ps%
echo ORTHOGONAL | gmt pstext -R -J -O -K -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya3.25i >> %ps%
echo X ON Y | gmt pstext -R -J -O -K -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya5.5i >> %ps%
echo Y ON X | gmt pstext -R -J -O -K -F+cRM+jTC+a90 -N -Dj0.2i -Xa5.4i -Ya7.75i >> %ps%
gmt psxy -R -J -O -T >> %ps%
DEL tmp
DEL data
DEL giants
