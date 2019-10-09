REM               GMT EXAMPLE 42
REM
REM Purpose:      Illustrate Antarctica and stereographic projection
REM GMT modules:  makecpt, grdimage, coast, legend, colorbar, text, plot
REM
gmt begin ex42
	gmt set FONT_ANNOT_PRIMARY 12p FONT_LABEL 12p PROJ_ELLIPSOID WGS-84 FORMAT_GEO_MAP dddF
	REM Data obtained via website and converted to netCDF thus:
	REM curl http://www.antarctica.ac.uk//bas_research/data/access/bedmap/download/bedelev.asc.gz
	REM gunzip bedelev.asc.gz
	REM grdconvert bedelev.asc BEDMAP_elevation.nc=ns -V
	gmt makecpt -Cearth -T-7000/4000
	gmt grdimage @BEDMAP_elevation.nc -Jx1:60000000 -Q
	gmt coast -R-180/180/-90/-60 -Js0/-90/-71/1:60000000 -Bafg -Di -W0.25p
	gmt colorbar -DJRM+w2.5i/0.2i+o0.5i/0+mc -F+p+i -Bxa1000+lELEVATION -By+lm
	REM GSHHG
	gmt coast -Glightblue -Sroyalblue2 -X2i -Y4.75i
	gmt coast -Glightbrown -A+ag -Bafg

	echo H 18p,Times-Roman Legend > legend.txt
	echo D 0.1i 1p >> legend.txt
	echo S 0.15i s 0.2i blue  0.25p 0.3i Ocean >> legend.txt
	echo S 0.15i s 0.2i lightblue  0.25p 0.3i Ice front >> legend.txt
	echo S 0.15i s 0.2i lightbrown  0.25p 0.3i Grounding line >> legend.txt
	gmt legend legend.txt -DjLM+w1.7i+jRM+o0.5i/0 -F+p+i
	REM Fancy line
	echo 0	5.55		> lines.txt
	echo 2.5	5.55	>> lines.txt
	echo 5.0	4.55	>> lines.txt
	echo 7.5	4.55	>> lines.txt
	gmt plot lines.txt -R0/7.5/0/10 -Jx1i -B0 -W2p -X-2.5i -Y-5.25i
	echo 0 5.2 BEDMAP > tmp.txt
	echo 0 9.65 GSHHG >> tmp.txt
	gmt text tmp.txt -F+f18p+jBL -Dj0.1i/0
gmt end show

