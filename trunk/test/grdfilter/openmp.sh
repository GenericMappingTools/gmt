#!/bin/sh
#	$Id: openmp.sh,v 1.1 2011-07-08 04:04:37 guru Exp $
# Testing grdfilter if openmp is used.

. ../functions.sh
header "Test grdfilter for parallel operations (-fopenmp only)"

grep '\-fopenmp' ../../src/config.mk > tmp
if [ ! -s tmp ]; then
	echo "[N/A]"
	rm -f tmp
	exit
fi
FILT=g			# Gaussian filter
INC=1			# 1x1 degree output
D=1000			# 1000 km filter width
DATA=../genper/etopo10.nc	# Test on ETOP10 data
ps=openmp.ps
# Run grdfilter as specified
grdfilter -D4 -F${FILT}$D -I$INC $DATA -Gt.nc -fg
makecpt -Cglobe -Z > t.cpt
grdimage t.nc -JQ0/7i -Ba:."$D km Gaussian filter":WSne -Ct.cpt -P -K -Xc -Y1.5i > $ps
psscale -Ct.cpt -D3.5i/-0.5i/6i/0.1ih -O -K -Ba/:m: >> $ps
grdimage $DATA -JQ0/7i -Ba:."Original data":WSne -Ct.cpt -O -K -Y4.75i >> $ps
psxy -Rt.nc -J -O -T >> $ps
#rm -f t.nc t.cpt tmp
pscmp
