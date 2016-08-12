REM             GMT EXAMPLE 39
REM             $Id$
REM
REM Purpose:      Illustrate evaluation of spherical harmonic coefficients
REM GMT progs:    psscale, pstext, makecpt, grdimage, grdgradient. sph2grd
REM DOS calls:  del
REM

echo GMT EXAMPLE 39
set ps=example_39.ps

REM Evaluate the first 180, 90, and 30 order/degrees of Venus spherical
REM harmonics topography model, skipping the L = 0 term (radial mean).
REM File truncated from http://www.ipgp.fr/~wieczor/SH/VenusTopo180.txt.zip
REM Wieczorek, M. A., Gravity and topography of the terrestrial planets,
REM   Treatise on Geophysics, 10, 165-205, doi:10.1016/B978-044452748-6/00156-5, 2007

gmt sph2grd VenusTopo180.txt -I1 -Rg -Ng -Gv1.nc -F1/1/25/30
gmt sph2grd VenusTopo180.txt -I1 -Rg -Ng -Gv2.nc -F1/1/85/90
gmt sph2grd VenusTopo180.txt -I1 -Rg -Ng -Gv3.nc -F1/1/170/180
gmt grd2cpt v3.nc -Crainbow -E > t.cpt
gmt grdgradient v1.nc -Nt0.75 -A45 -Gvint.nc
gmt grdimage v1.nc -Ivint.nc -JG90/30/5i -P -K -Bg -Ct.cpt -X3i -Y1.1i > %ps%
echo 4 4.5 L = 30 | gmt pstext -R0/6/0/6 -Jx1i -O -K -Dj0.2i -F+f16p+jLM -N >> %ps%
gmt psscale --FORMAT_FLOAT_MAP="%'g" -Ct.cpt -O -K -Dx1.25i/-0.2i+jTC+w5.5i/0.1i+h -Bxaf -By+lm >> %ps%
gmt grdgradient v2.nc -Nt0.75 -A45 -Gvint.nc
gmt grdimage v2.nc -Ivint.nc -JG -O -K -Bg -Ct.cpt -X-1.25i -Y1.9i >> %ps%
echo 4 4.5 L = 90 | gmt pstext -R0/6/0/6 -Jx1i -O -K -Dj0.2i -F+f16p+jLM -N >> %ps%
gmt grdgradient v3.nc -Nt0.75 -A45 -Gvint.nc
gmt grdimage v3.nc -Ivint.nc -JG -O -K -Bg -Ct.cpt -X-1.25i -Y1.9i >> %ps%
echo 4 4.5 L = 180 | gmt pstext -R0/6/0/6 -Jx1i -O -K -Dj0.2i -F+f16p+jLM -N >> %ps%
echo 3.75 5.4 Venus Spherical Harmonic Model | gmt pstext -R0/6/0/6 -Jx1i -O -F+f24p+jCM -N >> %ps%
REM Clean up
del v*.nc t.cpt gmt.history
