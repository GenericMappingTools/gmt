#!/bin/bash
#	$Id$
# Testing gmt grdfilter if openmp is used.

ps=openmp.ps

if ! [[ ${HAVE_OPENMP} =~ TRUE|ON ]]; then
  echo "[N/A]"
  exit 0
fi
FILT=g			# Gaussian filter
INC=1			# 1x1 degree output
D=1000			# 1000 km filter width
DATA=../genper/etopo10.nc	# Test on ETOP10 data

# Run gmt grdfilter as specified
gmt grdfilter -D4 -F${FILT}$D -I$INC $DATA -Gt.nc -fg
gmt makecpt -Cglobe -Z > t.cpt
gmt grdimage t.nc -JQ0/7i -Ba -BWSne+t"$D km Gaussian filter" -Ct.cpt -P -K -Xc -Y1.5i > $ps
gmt psscale -Ct.cpt -D3.5i/-0.5i+w6i/0.1i+h+jTC -O -K -Bxa -By+l"m" >> $ps
gmt grdimage $DATA -JQ0/7i -Ba -BWSne+t"Original data" -Ct.cpt -O -K -Y4.75i >> $ps
gmt psxy -Rt.nc -J -O -T >> $ps

