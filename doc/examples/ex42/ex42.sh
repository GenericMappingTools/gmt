#!/usr/bin/env bash
#               GMT EXAMPLE 42
#
# Purpose:      Illustrate Antarctica and stereographic projection
# GMT modules:  makecpt, grdimage, coast, legend, colorbar, text, plot
# Unix progs:   [curl grdconvert]
#
gmt begin ex42
	gmt set FONT_ANNOT_PRIMARY 12p FONT_LABEL 12p PROJ_ELLIPSOID WGS-84 FORMAT_GEO_MAP dddF
	# Data obtained via website and converted to netCDF thus:
	# curl http://www.antarctica.ac.uk//bas_research/data/access/bedmap/download/bedelev.asc.gz
	# gunzip bedelev.asc.gz
	# grdconvert bedelev.asc BEDMAP_elevation.nc=ns -V
	gmt makecpt -Cearth -T-7000/4000
	gmt grdimage @BEDMAP_elevation.nc -Jx1:60000000 -Q
	gmt coast -R-180/180/-90/-60 -Js0/-90/-71/1:60000000 -Bafg -Di -W0.25p
	gmt colorbar -DJRM+w2.5i/0.2i+o0.5i/0+mc -F+p+i -Bxa1000+lELEVATION -By+lm
	# GSHHG
	gmt coast -Glightblue -Sroyalblue2 -X2i -Y4.75i
	gmt coast -Glightbrown -A+ag -Bafg
	gmt legend -DjLM+w1.7i+jRM+o0.5i/0 -F+p+i <<- EOF
	H 18p,Times-Roman Legend
	D 0.1i 1p
	S 0.15i s 0.2i blue  0.25p 0.3i Ocean
	S 0.15i s 0.2i lightblue  0.25p 0.3i Ice front
	S 0.15i s 0.2i lightbrown  0.25p 0.3i Grounding line
	EOF
	# Fancy line
	gmt plot -R0/7.5/0/10 -Jx1i -B0 -W2p -X-2.5i -Y-5.25i <<- EOF
	0	5.55
	2.5	5.55
	5.0	4.55
	7.5	4.55
	EOF
	gmt text -F+f18p+jBL -Dj0.1i/0 <<- EOF
	0 5.2 BEDMAP
	0 9.65 GSHHG
	EOF
gmt end show

