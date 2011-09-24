#!/bin/sh
#	$Id$
# Testing grdfilter's weights at a given point for a given
# filter diameter.  Specify which output you want (a|c|r|w).
# Change args below to pick another filter.

. ../functions.sh
header "Test grdfilter for filter weights (-DDEBUG only)"

grep DEBUG ../../src/config.mk > tmp
if [ ! -s tmp ]; then
	echo "[N/A]"
	rm -f tmp
	exit
fi
FILT=g			# Gaussian filter
INC=1			# 1x1 degree output
DATA=../genper/etopo10.nc	# Test on ETOP10 data
ps=filtertest.ps
pdf=`basename $ps ".ps"`.pdf
if [ $# -ne 4 ]; then
	lon=150
	lat=-80
	D=5000
	mode=c
	no_U=1
else
	lon=$1
	lat=$2
	D=$3
	mode=$4
	U="-U/-0.5i/-0.25i/$0 $*" 
	no_U=
fi
# Set contour limits so we just draw the filter radius
lo=`gmtmath -Q $D 2 DIV 0.5 SUB =`
hi=`gmtmath -Q $D 2 DIV 0.5 ADD =`
# Run grdfilter as specified
grdfilter -A${mode}${lon}/$lat -D4 -F${FILT}$D -I$INC $DATA -Gt.nc -fg
n_conv=`cat n_conv.txt`
if [ $lat -lt 0 ]; then	# S hemisphere view
	plat=-90
	range=-90/0
else	# N hemisphere view
	plat=90
	range=0/90
fi
if [ $mode = r ]; then	# Set a different cpt for radius in km
	grdmath t.nc 0 NAN = t.nc
	grd2cpt -Crainbow t.nc -E20 -N -Z > t.cpt
	t=500
else	# Just normalize the output to 0-1 and make a cpt to fit
	grdmath t.nc DUP UPPER DIV 0 NAN = t.nc
	makecpt -Crainbow -T0/1/0.02 -N -Z > t.cpt
	t=0.1
fi
echo "N white" >> t.cpt	# White is NaN
# Compute radial distances from our point
grdmath -fg -R0/360/-90/90 -I1 $lon $lat SDIST DEG2KM = r.nc
# Plot polar map of result
if [ $no_U -eq 1 ]; then
	grdimage  t.nc -JA0/$plat/7i -P -B30g10 -Ct.cpt -R0/360/$range -K -X0.75i -Y0.5i > $ps
else
	grdimage  t.nc -JA0/$plat/7i -P -B30g10 -Ct.cpt -R0/360/$range -K -X0.75i -Y0.5i "$U" > $ps
fi
# Plot location of our test point
echo ${lon} $lat | psxy -R -J -O -K -Sx0.1 -W1p >> $ps
# Draw filter boundary
grdcontour r.nc -J -O -K -C1 -L$lo/$hi -W1p -R0/360/$range >> $ps
# Repeat plots in rectangular projection
grdimage t.nc -JQ00/7i -B30g10:."$D km Gaussian at ($lon, $lat)":WsNe -Ct.cpt -O -K -Y7.8i -R0/360/$range --FONT_TITLE=18p >> $ps
echo ${lon} $lat | psxy -R -J -O -K -Sx0.1 -W1p >> $ps
grdcontour  -R0/360/$range r.nc -J -O -K -C1 -L$lo/$hi -W1p >> $ps
psscale -Ct.cpt -D3.5i/-0.15i/6i/0.05ih -O -K -B$t/:"${mode} [$n_conv]": >> $ps
psxy -R -J -O -T >> $ps
if [ $# -eq 4 ]; then
	ps2raster $ps -Tf
	open $pdf
fi
rm -f t.nc r.nc t.cpt n_conv.txt tmp
pscmp
