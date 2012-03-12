#!/bin/bash
#	$Id$
#
# Paste pixel registered grids along X & Y

. functions.sh
header "Test grdpaste to paste pixel registered grids horizontal and vertically"

ps=paste_pixreg.ps
# The final grid is just f(x,y) = x*y
grdmath -R-15/15/-15/15 -I0.5 X Y MUL -r = lixo.nc
# The 4 pieces to assemble into final grid
grdmath -R-15/0/-15/0 -I0.5 X Y MUL -r = lixo_x1.nc
grdmath -R0/15/-15/0  -I0.5 X Y MUL -r = lixo_x2.nc
grdmath -R-15/0/0/15  -I0.5 X Y MUL -r = lixo_y1.nc
grdmath -R0/15/0/15   -I0.5 X Y MUL -r = lixo_y2.nc
grdpaste lixo_x1.nc lixo_x2.nc -Glixo_x.nc
grdpaste lixo_y1.nc lixo_y2.nc -Glixo_y.nc
grdpaste lixo_x.nc lixo_y.nc -Glixo_xy.nc
# Top is single source, bottom is assembled
grdcontour lixo_xy.nc -JX10c -C10 -B5 -P -K -Xc > $ps
grdcontour lixo.nc -J -C10 -B5 -O -Y12c >> $ps

pscmp
