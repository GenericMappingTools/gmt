#!/bin/bash
#	$Id$
#
gmt blockmedian -R245/255/20/30 -I5m -V ../tutorial/ship.xyz > ship_5m.xyz
gmt surface ship_5m.xyz -R245/255/20/30 -I5m -Gship.nc -V
gmt psmask -R245/255/20/30 -I5m ship_5m.xyz -JM6i -Ba -P -K -V > GMT_tut_13.ps
gmt grdcontour ship.nc -J -O -K -C250 -A1000 >> GMT_tut_13.ps
gmt psmask -C -O >> GMT_tut_13.ps
