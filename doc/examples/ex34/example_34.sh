#!/bin/bash
#               GMT EXAMPLE 34
#               $Id$
#
# Purpose:      Illustrate pscoast with DCW country polygons
# GMT progs:    pscoast, makecpt, grdimage, grdgradient
# Unix progs:   rm
#
ps=example_34.ps
gmtset FORMAT_GEO_MAP dddF
pscoast -JM4.5i -R-6/20/35/52 -FFR,IT+fP300/8 -Glightgray -Baf -BWSne -P -K \
	-X2i -U-1.75i/-0.75i/"Example 34 in Cookbook" > $ps
# Extract a subset of ETOPO2m for this part of Europe
# grdcut etopo2m_grd.nc -R -GFR+IT.nc=ns
makecpt -Cglobe -T-5000/5000/500 -Z > z.cpt
grdgradient FR+IT.nc -A15 -Ne0.75 -GFR+IT_int.nc
grdimage FR+IT.nc -IFR+IT_int.nc -Cz.cpt -J -O -K -Y4.5i \
	-Baf -BWsnE+t"Franco-Italian Union, 2042-45" >> $ps
pscoast -J -R -FFR,IT+fred@60 -O >> $ps
# cleanup
rm -f gmt.conf FR+IT_int.nc z.cpt
