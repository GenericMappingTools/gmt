#!/bin/bash
#	$Id$
#
gmt grdcontour "${tut:-../tutorial}"/bermuda.nc -JM6i -C250 -A1000 -P -Ba > GMT_tut_11.ps
