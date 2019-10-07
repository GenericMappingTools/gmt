#!/usr/bin/env bash
#               GMT EXAMPLE 34
#
# Purpose:      Illustrate coast with DCW country polygons
# GMT modules:  set, coast, makecpt, grdimage
# Unix progs:   rm
#
gmt begin ex34 ps
  gmt set FORMAT_GEO_MAP dddF FONT_HEADING 24p
  gmt subplot begin 2x1 -Fs4.5i/0 -M0.05i -JM4.5i -R-6/20/35/52 -SRl -SCb -Bwesn -T"Franco-Italian Union, 2042-45"
  gmt coast -EFR,IT+gP300/8 -Glightgray -c2,1
  # Extract a subset of ETOPO2m for this part of Europe
  # gmt grdcut etopo2m_grd.nc -R -GFR+IT.nc=ns
  gmt makecpt -Cglobe -T-5000/5000
  gmt grdimage @FR+IT.nc -I+a15+ne0.75 -C -c1,1
  gmt coast -EFR,IT+gred@60
  gmt subplot end
gmt end
