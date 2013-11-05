#!/bin/bash
#               GMT EXAMPLE 34
#               $Id$
#
# Purpose:      Illustrate pscoast with DCW country polygons
# GMT progs:    pscoast, makecpt, grdimage, grdgradient
# Unix progs:   rm
#
ps=example_34.ps
gmt gmtset FORMAT_GEO_MAP dddF
gmt pscoast -JM4.5i -R-6/20/35/52 -FFR,IT+gP300/8 -Glightgray -Baf -BWSne -P -K -X2i > $ps
# Extract a subset of ETOPO2m for this part of Europe
# gmt grdcut etopo2m_grd.nc -R -GFR+IT.nc=ns
gmt makecpt -Cglobe -T-5000/5000/500 -Z > z.cpt
gmt grdgradient FR+IT.nc -A15 -Ne0.75 -GFR+IT_int.nc
gmt grdimage FR+IT.nc -IFR+IT_int.nc -Cz.cpt -J -O -K -Y4.5i \
	-Baf -BWsnE+t"Franco-Italian Union, 2042-45" >> $ps
gmt pscoast -J -R -FFR,IT+gred@60 -O >> $ps
# cleanup
rm -f gmt.conf FR+IT_int.nc z.cpt
