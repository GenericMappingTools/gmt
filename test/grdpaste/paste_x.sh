#!/bin/bash
#	$Id: paste_x.sh,v 1.2 2011-06-18 19:48:27 guru Exp $
#
# Paste grids along X & Y

. ../functions.sh
header "Test grdpaste to paste grids horizontal and vertically"

ps=paste.ps
# The final grid
grdmath -R-15/15/-15/15 -I0.5 X = lixo.nc
# The 4 pieces to assemble into final grid
grdmath -R-15/0/-15/0 -I0.5 X = lixo_x1.nc
grdmath -R0/15/-15/0  -I0.5 X = lixo_x2.nc
grdmath -R-15/0/0/15 -I0.5 X = lixo_y1.nc
grdmath -R0/15/0/15  -I0.5 X = lixo_y2.nc
grdpaste lixo_x1.nc lixo_x2.nc -Glixo_x.nc
grdpaste lixo_y1.nc lixo_y2.nc -Glixo_y.nc
grdpaste lixo_x.nc lixo_y.nc -Glixo_xy.nc
# Top is single source, bottom is assembled
grdcontour lixo_xy.nc -JX10c -C2 -B5 -P -K -Xc > $ps
grdcontour lixo.nc -J -C2 -B5 -O -Y12c >> $ps

rm -f lixo*.nc

pscmp
