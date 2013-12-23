REM             GMT EXAMPLE 38
REM             $Id
REM
REM Purpose:    Illustrate histogram equalization on topography grids
REM GMT progs:  psscale, pstext, makecpt, grdhisteq, grdimage, grdinfo, grdgradient
REM DOS calls:  del
REM

echo GMT EXAMPLE 38
set ps=example_38.ps

gmt makecpt -Crainbow -T0/1700/100 -Z > t.cpt
gmt makecpt -Crainbow -T0/15/1 > c.cpt
gmt grdgradient topo.nc -Nt1 -fg -A45 -Gitopo.nc
gmt grdhisteq topo.nc -Gout.nc -C16
gmt grdimage topo.nc -Iitopo.nc -Ct.cpt -JM3i -Y5i -K -P -B5 -BWSne > %ps%
echo 315 -10 Original | gmt pstext -Rtopo.nc -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> %ps%
gmt grdimage out.nc -Cc.cpt -J -X3.5i -K -O -B5 -BWSne >> %ps%
echo 315 -10 Equalized | gmt pstext -R -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> %ps%
gmt psscale -D0i/-0.4i/5i/0.15ih -O -K -Ct.cpt -Ba500 -By+lm -E+n >> %ps%
gmt grdhisteq topo.nc -Gout.nc -N
gmt makecpt -Crainbow -T-3/3/0.1 -Z > c.cpt
gmt grdimage out.nc -Cc.cpt -J -X-3.5i -Y-3.3i -K -O -B5 -BWSne >> %ps%
echo 315 -10 Normalized | gmt pstext -R -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> %ps%
gmt grdhisteq topo.nc -Gout.nc -N
gmt grdimage out.nc -Cc.cpt -J -X3.5i -K -O -B5 -BWSne >> %ps%
echo 315 -10 Quadratic | gmt pstext -R -J -O -K -F+jTR+f14p -T -Gwhite -W1p -Dj0.1i >> %ps%
gmt psscale -D0i/-0.4i/5i/0.15ih -O -Cc.cpt -Bx1 -By+lz -E+n >> %ps%
REM Clean up
del itopo.nc out.nc *.cpt
