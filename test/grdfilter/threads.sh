#!/bin/bash
#	$Id$
# Testing gmt grdfilter if openmp is used.

ps=openmp.ps # same as test: openmp.sh

if [[ ${HAVE_GLIB_GTHREAD} =~ TRUE|ON ]]; then
  _thread_opt=-x+a
fi

FILT=g			# Gaussian filter
INC=1			# 1x1 degree output
D=1000			# 1000 km filter width
DATA=../genper/etopo10.nc	# Test on ETOP10 data

# Run gmt grdfilter as specified
gmt grdfilter -D4 -F${FILT}$D -I$INC $DATA -Gt.nc -fg ${_thread_opt}
gmt makecpt -Cglobe -Z > t.cpt
gmt grdimage t.nc -JQ0/7i -Ba -BWSne+t"$D km Gaussian filter" -Ct.cpt -P -K -Xc -Y1.5i > $ps
gmt psscale -Ct.cpt -D3.5i/-0.5i/6i/0.1ih -O -K -Bxa -By+l"m" >> $ps
gmt grdimage $DATA -JQ0/7i -Ba -BWSne+t"Original data" -Ct.cpt -O -K -Y4.75i >> $ps
gmt psxy -Rt.nc -J -O -T >> $ps

