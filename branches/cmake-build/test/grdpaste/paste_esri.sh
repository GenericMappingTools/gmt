#!/bin/bash
#	$Id: $
#
# Paste esri ascii grids along Y

. ../functions.sh
header "Test grdpaste with ESRI ASCII files. Currently segfaults"

#ps=paste_esri.ps
# The final grid is just f(x,y) = x
grdmath -R-15/15/-15/0 -I0.5 X = lixo_y1.asc=ei
grdmath -R-15/15/0/15 -I0.5 X = lixo_y2.asc=ei

grdpaste lixo_y1.asc lixo_y2.asc -Glixo_y.nc

rm -f lixo*
