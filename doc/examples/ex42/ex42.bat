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
	gmt colorbar -DJRM+w6.5c/0.5c+o1.5c/0+mc -F+p+i -Bxa1000+lELEVATION -By+lm
	REM GSHHG
	gmt coast -Glightblue -Sroyalblue2 -X5c -Y12c
	gmt coast -Glightbrown -A+ag -Bafg

	echo H 18p,Times-Roman Legend > legend.txt
	echo D 0.25c 1p >> legend.txt
	echo S 0.4c s 0.5c blue       0.25p 0.75c Ocean >> legend.txt
	echo S 0.4c s 0.5c lightblue  0.25p 0.75c Ice front >> legend.txt
	echo S 0.4c s 0.5c lightbrown 0.25p 0.75c Grounding line >> legend.txt
	gmt legend legend.txt -DjLM+w4c+jRM+o1c/0 -F+p+i
	REM Fancy line
	echo 0	14		> lines.txt
	echo 6.5 14		>> lines.txt
	echo 13	11.5	>> lines.txt
	echo 19	11.5	>> lines.txt
	gmt plot lines.txt -R0/19/0/25 -Jx1c -B0 -W2p -X-6c -Y-13.5c
	echo 0 13 BEDMAP > tmp.txt
	echo 0 24 GSHHG >> tmp.txt
	gmt text tmp.txt -F+f18p+jBL -Dj8p/0
gmt end show
