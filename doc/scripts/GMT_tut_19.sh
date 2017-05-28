#!/bin/bash
#	$Id$
#
gmt makecpt -Ctopo -T1000/5000 > t.cpt
gmt grdgradient @us.nc -Ne0.8 -A100 -fg -Gus_i.nc
gmt grdview us.nc -JM6i -p135/35 -Qi50 -Ius_i.nc -Ct.cpt -Ba -JZ0.5i > GMT_tut_19.ps
