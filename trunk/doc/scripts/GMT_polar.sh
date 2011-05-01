#!/bin/bash
#	$Id: GMT_polar.sh,v 1.8 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

grdmath -R0/360/2/4 -I6/0.1 X 4 MUL PI MUL 180 DIV COS Y 2 POW MUL = $$.nc
grdcontour $$.nc -JP3i -B30Ns -P -C2 -S4 --FORMAT_GEO_MAP=+ddd > GMT_polar.ps
rm -f $$.nc
