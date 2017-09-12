#!/bin/bash
#		GMT EXAMPLE 02
#		$Id$
#
# Purpose:	Make two color images based gridded data
# GMT modules:	set, grd2cpt, grdimage, makecpt, psscale, subplot
# Unix progs:	rm
#
gmt begin ex02 ps
  gmt set MAP_ANNOT_OBLIQUE 0 PS_MEDIA letter
  gmt makecpt -Crainbow -T-2/14/2 > g.cpt
  gmt grd2cpt @HI_topo_02.nc -Crelief -Z > t.cpt
  gmt subplot begin 2x1 -A+JTL+o0.1i/0 -Fs6i/3.5i -M0.2i -R160/20/220/30+r -JOc190/25.5/292/69/6i -B10 -T"H@#awaiian@# T@#opo and @#G@#eoid@#"
    gmt subplot 2,1 -Ce1.1i
    gmt grdimage @HI_geoid_02.nc -Cg.cpt
    gmt psscale -Cg.cpt -DJRM+o0.6i/0+e+mc -Bx2+lGEOID -By+lm
    gmt subplot 1,1 -Ce1.15i
    gmt grdimage @HI_topo_02.nc -I+a0 -Ct.cpt
    gmt psscale -Ct.cpt -DJRM+o0.6i/0+mc -I0.3 -Bx2+lTOPO -By+lkm
  gmt subplot end
gmt end
rm -f ?.cpt
