#!/bin/sh
#	$Id: GMT_polar.sh,v 1.2 2002-02-23 00:53:15 pwessel Exp $
#

grdmath -R0/360/2/4 -I6/0.1 X 4 MUL PI MUL 180 DIV COS Y 2 POW MUL = test.grd
grdcontour test.grd -JP3i -B30Ns -P -C2 -S4 > GMT_polar.ps
rm -f test.grd
