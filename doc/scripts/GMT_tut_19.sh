#!/bin/bash
#	$Id$
#
gmt grdview "${tut:-../tutorial}"/us.nc -JM6i -p135/35 -Qi50 -Ius_i.nc -Ctopo.cpt -V -Ba -JZ0.5i > GMT_tut_19.ps
