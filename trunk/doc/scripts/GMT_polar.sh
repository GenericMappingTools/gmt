#!/bin/sh
#	$Id: GMT_polar.sh,v 1.4 2008-02-29 03:23:52 remko Exp $
#

grdmath -R0/360/2/4 -I6/0.1 X 4 MUL PI MUL 180 DIV COS Y 2 POW MUL = $$.grd
grdcontour $$.grd -JP3i -B30Ns -P -C2 -S4 --PLOT_DEGREE_FORMAT=+ddd > GMT_polar.ps
rm -f $$.grd
