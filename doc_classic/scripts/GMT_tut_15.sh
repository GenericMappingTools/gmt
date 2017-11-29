#!/bin/bash
#	$Id$
#
gmt makecpt -Crainbow -T1000/5000 > topo.cpt
gmt grdimage @tut_relief.nc -JM6i -P -Ba -BWSnE -Ctopo.cpt -K > GMT_tut_15.ps
gmt psscale -DJTC -Rtut_relief.nc -J -Ctopo.cpt -I0.4 -Bxa -By+lm -O >> GMT_tut_15.ps
