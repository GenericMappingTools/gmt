#!/bin/bash
#	$Id$
# Testing gmt grdfilter's weights at a given point for a given
# filter diameter.  Specify which output you want (a|c|r|w).
# Change args below to pick another filter.

if [ "$HAVE_GMT_DEBUG_SYMBOLS" != "TRUE" ]; then
	echo "grdfilter -A option is not available without -DDEBUG"
	exit
fi

ps=filtertest.ps

if [[ ${HAVE_GLIB_GTHREAD} =~ TRUE|ON ]]; then
  _thread_opt=-x+a
fi

FILT=g			# Gaussian filter
INC=1			# 1x1 degree output
DATA="../genper/etopo10.nc" # Test on ETOP10 data
lon=150
lat=-80
D=5000
mode=c
no_U=1
gmt set PROJ_ELLIPSOID Sphere
# Set contour limits so we just draw the filter radius
lo=`gmt gmtmath -Q $D 2 DIV 0.5 SUB =`
hi=`gmt gmtmath -Q $D 2 DIV 0.5 ADD =`
# Run gmt grdfilter as specified
gmt grdfilter -A${mode}${lon}/$lat -D4 -F${FILT}$D -I$INC $DATA -Gt.nc -fg ${_thread_opt}
n_conv=`cat n_conv.txt`
if [ $lat -lt 0 ]; then	# S hemisphere view
	plat=-90
	range=-90/0
else	# N hemisphere view
	plat=90
	range=0/90
fi
if [ $mode = r ]; then	# Set a different cpt for radius in km
	gmt grdmath t.nc 0 NAN = t.nc
	gmt grd2cpt -Crainbow t.nc -E20 -N -Z > t.cpt
	t=500
else	# Just normalize the output to 0-1 and make a cpt to fit
	gmt grdmath t.nc DUP UPPER DIV 0 NAN = t.nc
	gmt makecpt -Crainbow -T0/1/0.02 -N -Z > t.cpt
	t=0.1
fi
echo "N white" >> t.cpt	# White is NaN
# Compute radial distances from our point
gmt grdmath -fg -R0/360/-90/90 -I1 $lon $lat SDIST = r.nc
# Plot polar map of result
if [ $no_U -eq 1 ]; then
	gmt grdimage  t.nc -JA0/$plat/7i -P -B30g10 -Ct.cpt -R0/360/$range -K -X0.75i -Y0.5i > $ps
else
	gmt grdimage  t.nc -JA0/$plat/7i -P -B30g10 -Ct.cpt -R0/360/$range -K -X0.75i -Y0.5i "$U" > $ps
fi
# Plot location of our test point
echo ${lon} $lat | gmt psxy -R -J -O -K -Sx0.1 -W1p >> $ps
# Draw filter boundary
gmt grdcontour r.nc -J -O -K -C1 -L$lo/$hi -W1p -R0/360/$range >> $ps
# Repeat plots in rectangular gmt projection
gmt grdimage t.nc -JQ0/7i -B30g10 -BWsNe+t"$D km Gaussian at ($lon, $lat)" -Ct.cpt -O -K -Y7.8i -R-180/180/$range --FONT_TITLE=18p >> $ps
echo ${lon} $lat | gmt psxy -R -J -O -K -Sx0.1 -W1p >> $ps
gmt grdcontour -R r.nc -J -O -K -C1 -L$lo/$hi -W1p >> $ps
gmt psscale -Ct.cpt -D3.5i/-0.15i+w6i/0.05i+h+jTC -O -K -Bx$t -By+l"${mode} [$n_conv]" >> $ps
gmt psxy -R -J -O -T >> $ps
