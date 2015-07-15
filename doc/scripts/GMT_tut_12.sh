#!/bin/bash
#	$Id$
#
gmt nearneighbor -R245/255/20/30 -I5m -S40k -Gship.nc -V ../tutorial/ship.xyz
gmt grdcontour ship.nc -JM6i -P -Ba -C250 -A1000 > GMT_tut_12.ps
