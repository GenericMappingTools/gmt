#!/bin/bash
#
#	$Id$

# Make a small geographic grid
gmt grdmath -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95 -I1.31151346331e-05/1.29590887959e-05 X Y MUL = tmp.nc
gmt grdproject -JM-70:10:13.525/-42:40:25.525/8 -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95 tmp.nc -Gc.nc -nb --PROJ_LENGTH_UNIT=cm
w_cm=`gmt grdinfo c.nc -C | head -1 | cut -f3`
gmt grdproject -JM-70:10:13.525/-42:40:25.525/8 -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95 tmp.nc -Gi.nc -nb --PROJ_LENGTH_UNIT=inch
w_inch=`gmt grdinfo i.nc -C | head -1 | cut -f3`
err=`gmt gmtmath -Q $w_inch $w_cm NEQ =`
if [ $err -eq 1 ]; then
	echo "cm gave $w_cm cm and inch gave $w_inch inc" > fail
else
	touch fail
fi
