#!/bin/bash
#
#	$Id: units.sh,v 1.2 2011-04-26 19:29:44 remko Exp $

. ../functions.sh
header "Test grdproject with SI and US units"

# Make a small geographic grid
grdmath -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95 -I1.31151346331e-05/1.29590887959e-05 X Y MUL = tmp.nc
grdproject -JM-70:10:13.525/-42:40:25.525/8 -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95 tmp.nc -Gc.nc -Sb --PROJ_LENGTH_UNIT=cm
w_cm=`grdinfo c.nc -C | cut -f3`
grdproject -JM-70:10:13.525/-42:40:25.525/8 -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95 tmp.nc -Gi.nc -Sb --PROJ_LENGTH_UNIT=inch
w_inch=`grdinfo i.nc -C | cut -f3`
err=`gmtmath -Q $w_inch $w_cm NEQ =`
if [ $err -eq 1 ]; then
	echo "cm gave $w_cm cm and inch gave $w_inch inc" > fail
fi
passfail test
rm -f tmp.nc c.nc i.nc
