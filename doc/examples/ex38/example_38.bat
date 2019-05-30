REM             GMT EXAMPLE 38
REM
REM Purpose:    Illustrate histogram equalization on topography grids
REM GMT progs:  psscale, pstext, makecpt, grdhisteq, grdimage
REM DOS calls:  del
REM

echo GMT EXAMPLE 38
set ps=example_38.ps

gmt makecpt -Crainbow -T0/1700 > t.cpt
gmt makecpt -Crainbow -T0/15/1 > c.cpt
gmt grdhisteq @topo_38.nc -Gout.nc -C16
gmt grdimage @topo_38.nc -I+a45+nt1 -Ct.cpt -JM3i -Y6i -K -P -B5 -BWSne > %ps%
echo 315 -10 Original | gmt pstext -R@topo_38.nc -J -O -K -F+jTR+f14p -Gwhite -W1p -Dj0.1i >> %ps%
gmt grdimage out.nc -Cc.cpt -J -X3.8i -K -O -B5 -BWSne >> %ps%
echo 315 -10 Equalized | gmt pstext -R -J -O -K -F+jTR+f14p -Gwhite -W1p -Dj0.1i >> %ps%
gmt psscale -Dx-0.2i/-0.4i+jTC+w5i/0.15i+h+e+n -O -K -Ct.cpt -Ba500 -By+lm >> %ps%
gmt grdhisteq @topo_38.nc -Gout.nc -N
gmt makecpt -Crainbow -T-3/3 > c.cpt
gmt grdimage out.nc -Cc.cpt -J -X-3.8i -Y-3.3i -K -O -B5 -BWSne >> %ps%
echo 315 -10 Normalized | gmt pstext -R -J -O -K -F+jTR+f14p -Gwhite -W1p -Dj0.1i >> %ps%
gmt grdhisteq @topo_38.nc -Gout.nc -Q
gmt makecpt -Crainbow -T0/15 > q.cpt
gmt grdimage out.nc -Cq.cpt -J -X3.8i -K -O -B5 -BWSne >> %ps%
echo 315 -10 Quadratic | gmt pstext -R -J -O -K -F+jTR+f14p -Gwhite -W1p -Dj0.1i >> %ps%
gmt psscale -Dx-0.2i/-0.4i+w5i/0.15i+h+jTC+e+n -O -K -Cc.cpt -Bx1 -By+l"z@-n@-" >> %ps%
gmt psscale -Dx-0.2i/-1.0i+w5i/0.15i+h+jTC+e+n -O -Cq.cpt -Bx1 -By+l"z@-q@-" >> %ps%
REM Clean up
del out.nc *.cpt
