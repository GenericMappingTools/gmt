#!/bin/sh
#		GMT EXAMPLE 02
#
#		$Id: job02.sh,v 1.2 2001-09-24 02:30:55 pwessel Exp $
#
# Purpose:	Make two color images based gridded data
# GMT progs:	gmtset grd2cpt grdgradient grdimage makecpt psscale pstext
# Unix progs:	cat rm
#
gmtset HEADER_FONT_SIZE 30 OBLIQUE_ANOTATION 0
#get gridded data using GMT supplemental program grdraster
#grdraster 1 -R160/20/220/30r -JOc190/25.5/292/69/4.5i -GHI_topo2.grd=0/0.001/0
#grdraster 4 -R -JO -GHI_geoid2.grd
makecpt -Crainbow -T-2/14/2 > g.cpt
grdimage HI_geoid2.grd -R160/20/220/30r -JOc190/25.5/292/69/4.5i -E50 -K -P -U/-1.25i/-1i/"Example 2 in Cookbook" -B10 -Cg.cpt -X1.5i -Y1.25i > example_02.ps
psscale -Cg.cpt -D5.1i/1.35i/2.88i/0.4i -O -K -L -B2:GEOID:/:m: -E >> example_02.ps
grd2cpt HI_topo2.grd -Crelief -Z > t.cpt
grdgradient HI_topo2.grd -A0 -Nt -GHI_topo2_int.grd
grdimage HI_topo2.grd -IHI_topo2_int.grd -R -JO -E50 -B10:."H@#awaiian@# T@#opo and @#G@#eoid:" -O -K -Ct.cpt -Y4.5i >> example_02.ps
psscale -Ct.cpt -D5.1i/1.35i/2.88i/0.4i -O -K -I0.3 -B2:TOPO:/:km: >> example_02.ps
cat << EOF | pstext -R0/8.5/0/11 -Jx1i -O -N -Y-4.5i >> example_02.ps
-0.4 7.5 30 0.0 1 2 a)
-0.4 3.0 30 0.0 1 2 b)
EOF
\rm -f .gmtcommands .gmtdefaults HI_topo2_int.grd ?.cpt
