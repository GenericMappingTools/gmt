#!/bin/bash
#	$Id$
#
# Paste grid registered grids that do NOT share a row/column along X & Y edges
# We simulate this by creating pastable pixreg grids and toggle their registration

ps=paste_gridreg_uderlap.ps

# The final grid is just f(x,y) = x*y
grdmath -R-15/15/-15/15 -I0.5 X Y MUL -r = lixo.nc
# The 4 pieces to assemble into final grid
grdmath -R-15/0/-15/0 -I0.5 X Y MUL -r = lixo_x1.nc
grdedit lixo_x1.nc -T
grdmath -R0/15/-15/0  -I0.5 X Y MUL -r = lixo_x2.nc
grdedit lixo_x2.nc -T
grdmath -R-15/0/0/15  -I0.5 X Y MUL -r = lixo_y1.nc
grdedit lixo_y1.nc -T
grdmath -R0/15/0/15   -I0.5 X Y MUL -r = lixo_y2.nc
grdedit lixo_y2.nc -T
grdpaste lixo_x1.nc lixo_x2.nc -Glixo_x.nc
grdpaste lixo_y1.nc lixo_y2.nc -Glixo_y.nc
grdpaste lixo_x.nc lixo_y.nc -Glixo_xy.nc
# Top is single source, bottom is assembled
grdcontour lixo_xy.nc -JX10c -C10 -B5 -P -K -Xc > $ps
grdcontour lixo.nc -J -C10 -B5 -O -Y12c >> $ps

