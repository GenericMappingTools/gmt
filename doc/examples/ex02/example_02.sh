#!/bin/bash
#		GMT EXAMPLE 02
#		$Id$
#
# Purpose:	Make two color images based gridded data
# GMT progs:	gmtset, grd2cpt, grdgradient, grdimage, makecpt, psscale, pstext
# Unix progs:	rm
#
ps=example_02.ps
gmt gmtset FONT_TITLE 30p MAP_ANNOT_OBLIQUE 0
gmt makecpt -Crainbow -T-2/14/2 > g.cpt
gmt grdimage HI_geoid2.nc -R160/20/220/30r -JOc190/25.5/292/69/4.5i -E50 -K -P \
            -B10 -Cg.cpt -X1.5i -Y1.25i > $ps
gmt psscale -Cg.cpt -DjRM+o0.6i/0+w2.88i/0.4i+mc+e -R -J -O -K -Bx2+lGEOID -By+lm >> $ps
gmt grd2cpt HI_topo2.nc -Crelief -Z > t.cpt
gmt grdgradient HI_topo2.nc -A0 -Nt -GHI_topo2_int.nc
gmt grdimage HI_topo2.nc -IHI_topo2_int.nc -R -J -B+t"H@#awaiian@# T@#opo and @#G@#eoid" \
            -B10 -E50 -O -K -Ct.cpt -Y4.5i --MAP_TITLE_OFFSET=0.5i >> $ps
gmt psscale -Ct.cpt -DjRM+o0.6i/0+w2.88i/0.4i+mc -R -J -O -K -I0.3 -Bx2+lTOPO -By+lkm >> $ps
gmt pstext -R0/8.5/0/11 -Jx1i -F+f30p,Helvetica-Bold+jCB -O -N -Y-4.5i >> $ps << END
-0.4 7.5 a)
-0.4 3.0 b)
END
rm -f HI_topo2_int.nc ?.cpt gmt.conf
