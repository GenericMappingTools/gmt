REM               GMT EXAMPLE 39
REM
REM Purpose:      Illustrate evaluation of spherical harmonic coefficients
REM GMT modules:  colorbar, text, makecpt, grdimage, sph2grd
REM
gmt begin ex39
	REM Evaluate the first 180, 90, and 30 order/degrees of Venus spherical
	REM harmonics topography model, skipping the L = 0 term (radial mean).
	REM File truncated from http://www.ipgp.fr/~wieczor/SH/VenusTopo180.txt.zip
	REM Wieczorek, M. A., Gravity and topography of the terrestrial planets,
	REM   Treatise on Geophysics, 10, 165-205, doi:10.1016/B978-044452748-6/00156-5, 2007

	gmt sph2grd @VenusTopo180.txt -I1 -Rg -Ng -Gv1.nc -F1/1/25/30
	gmt sph2grd @VenusTopo180.txt -I1 -Rg -Ng -Gv2.nc -F1/1/85/90
	gmt sph2grd @VenusTopo180.txt -I1 -Rg -Ng -Gv3.nc -F1/1/170/180
	gmt grd2cpt v3.nc -Crainbow -E
	gmt grdimage v1.nc -I+a45+nt0.75 -JG90/30/5i -Bg -X3i -Y1.1i
	echo 4 4.5 L = 30 | gmt text -R0/6/0/6 -Jx1i -Dj0.2i -F+f16p+jLM -N
	gmt colorbar --FORMAT_FLOAT_MAP="%%'g" -Dx1.25i/-0.2i+jTC+w5.5i/0.1i+h -Bxaf -By+lm
	gmt grdimage v2.nc -I+a45+nt0.75 -JG -Bg -X-1.25i -Y1.9i
	echo 4 4.5 L = 90 | gmt text -R0/6/0/6 -Jx1i -Dj0.2i -F+f16p+jLM -N
	gmt grdimage v3.nc -I+a45+nt0.75 -JG -Bg -X-1.25i -Y1.9i
	echo 4 4.5 L = 180 | gmt text -R0/6/0/6 -Jx1i -Dj0.2i -F+f16p+jLM -N
	echo 3.75 5.4 Venus Spherical Harmonic Model | gmt text -F+f24p+jCM -N
	del v?.nc
gmt end show
