#!/bin/bash
#	$Id: GMT_polar.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

grdmath -R0/360/2/4 -I6/0.1 X 4 MUL PI MUL 180 DIV COS Y 2 POW MUL = $$.nc
grdcontour $$.nc -JP3i -B30Ns -P -C2 -S4 --PLOT_DEGREE_FORMAT=+ddd > GMT_polar.ps
rm -f $$.nc
