#!/bin/sh
#		GMT EXAMPLE 16
#
#		$Id: job16.sh,v 1.3 2003-04-11 23:49:54 pwessel Exp $
#
# Purpose:	Illustrates interpolation methods using same data as Example 12.
# GMT progs:	gmtset, grdview, grdfilter, pscontour, psscale, pstext, surface, triangulate
# Unix progs:	$AWK, echo, rm
#
# First make a cpt file as in example 12:
#
#z0=`minmax table_5.11 -C -I25 | $AWK '{print $5}'`
#z1=`minmax table_5.11 -C -I25 | $AWK '{print $6}'`
#makecpt -Crainbow -T$z0/$z1/25 > ex16.cpt
#Hand edit to add patterns and skips
#
# Now illustrate various means of contouring, using triangulate and surface.
#
gmtset MEASURE_UNIT INCH ANOT_FONT_SIZE 9
#
pscontour -R0/6.5/-0.2/6.5 -Jx0.45i -P -K -Y5.5i -Ba2f1WSne table_5.11 -Cex16.cpt -I > example_16.ps
echo "3.25 7 18 0 4 CB pscontour (triangulate)" | pstext -R -Jx -O -K -N >> example_16.ps
#
surface table_5.11 -R -I0.1 -Graws0.grd
grdview raws0.grd -R -Jx -Ba2f1WSne -Cex16.cpt -Qs -O -K -X3.5i >> example_16.ps
echo "3.25 7 18 0 4 CB surface (tension = 0)" | pstext -R -Jx -O -K -N >> example_16.ps
#
surface table_5.11 -R -I0.1 -Graws5.grd -T0.5
grdview raws5.grd -R -Jx -Ba2f1WSne -Cex16.cpt -Qs -O -K -Y-3.75i -X-3.5i -U"Example 16 in Cookbook" >> example_16.ps
echo "3.25 7 18 0 4 CB surface (tension = 0.5)" | pstext -R -Jx -O -K -N >> example_16.ps
#
triangulate table_5.11 -Grawt.grd -R -I0.1 > /dev/null
grdfilter rawt.grd -Gfiltered.grd -D0 -Fc1
grdview filtered.grd -R -Jx -Ba2f1WSne -Cex16.cpt -Qs -O -K -X3.5i >> example_16.ps
echo "3.25 7 18 0 4 CB triangulate @~\256@~ grdfilter" | pstext -R -Jx -O -K -N >> example_16.ps
echo "3.2125 7.5 32 0 4 CB Gridding of Data" | pstext -R0/10/0/10 -Jx1i -O -K -N -X-3.5i >> example_16.ps
psscale -D3.21/0.35/5/0.25h -Cex16.cpt -O -U"Example 16 in Cookbook" -Y-0.75i >> example_16.ps
#
rm -f *.grd .gmt*
