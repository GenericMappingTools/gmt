#!/bin/bash
#	$Id$
#
gmt grd2cpt ../tutorial/bermuda.nc -Cocean > bermuda.cpt
gmt grdview ../tutorial/bermuda.nc -JM5i -P -JZ2i -p135/30 -Ba -Cbermuda.cpt > GMT_tut_18.ps
