#!/bin/bash
#	$Id$
#
gmt makecpt -Crainbow -T1000/5000 > topo.cpt
gmt grdgradient @us.nc -Ne0.8 -A100 -fg -Gus_i.nc
gmt grdimage us.nc -Ius_i.nc -JM6i -P -Ba -BWSnE -Ctopo.cpt -K > GMT_tut_16.ps
gmt psscale -DJTC -Rus.nc -J -Ctopo.cpt -I0.4 -Bxa -By+lm -O >> GMT_tut_16.ps
