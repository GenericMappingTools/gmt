#!/bin/bash
#	$Id$
#
gmt makecpt -Crainbow -T1000/5000/500 -Z > topo.cpt
gmt grdimage ../tutorial/us.nc -JM6i -P -Ba -Ctopo.cpt -V -K > GMT_tut_15.ps
gmt psscale -D3i/8.5i/5i/0.25ih -Ctopo.cpt -I0.4 -By+lm -O >> GMT_tut_15.ps
