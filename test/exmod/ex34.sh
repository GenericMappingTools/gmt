#!/bin/bash
#               GMT EXAMPLE 34
#
# Purpose:      Illustrate pscoast with DCW country polygons
# GMT modules:  gmtset, pscoast, makecpt, grdimage
# Unix progs:   rm
#
#export GMT_PPID=$$
export GMT_PPID=1
gmt begin ex34 ps
  gmt set FORMAT_GEO_MAP dddF FONT_HEADING 24p
  gmt subplot begin 2x1 -Fs4.5i/4i -M0.1i -LRl -LCb -Lwesn -T"Franco-Italian Union, 2042-45"
  gmt pscoast -JM -R-6/20/35/52 -EFR,IT+gP300/8 -Glightgray -c2,1
  # Extract a subset of ETOPO2m for this part of Europe
  # gmt grdcut etopo2m_grd.nc -R -GFR+IT.nc=ns
  gmt makecpt -Cglobe -T-5000/5000 > z.cpt
  gmt grdimage @FR+IT.nc -I+a15+ne0.75 -Cz.cpt -c1,1
  gmt pscoast -EFR,IT+gred@60
  gmt subplot end
gmt end
rm -f z.cpt
