#!/bin/sh
#	$Id$
# Testing grdfilter if openmp is used.

ps=openmp.ps

if [ -z "$HAVE_OPENMP" ]; then
  echo "[N/A]"
  exit 0
fi
FILT=g			# Gaussian filter
INC=1			# 1x1 degree output
D=1000			# 1000 km filter width
DATA=../genper/etopo10.nc	# Test on ETOP10 data

# Run grdfilter as specified
grdfilter -D4 -F${FILT}$D -I$INC $DATA -Gt.nc -fg -V
makecpt -Cglobe -Z > t.cpt
grdimage t.nc -JQ0/7i -Ba:."$D km Gaussian filter":WSne -Ct.cpt -P -K -Xc -Y1.5i > $ps
psscale -Ct.cpt -D3.5i/-0.5i/6i/0.1ih -O -K -Ba/:m: >> $ps
grdimage $DATA -JQ0/7i -Ba:."Original data":WSne -Ct.cpt -O -K -Y4.75i >> $ps
psxy -Rt.nc -J -O -T >> $ps

