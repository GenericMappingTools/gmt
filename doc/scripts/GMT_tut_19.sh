#!/bin/bash
#	$Id$
#
gmt grdgradient "${tut:-../tutorial}"/us.nc -Ne0.8 -A100 -fg -Gus_i.nc
gmt grdview "${tut:-../tutorial}"/us.nc -JM6i -p135/35 -Qi50 -Ius_i.nc -Ctopo -V -Ba -JZ0.5i > GMT_tut_19.ps
