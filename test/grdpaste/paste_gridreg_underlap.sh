#!/bin/bash
#	$Id$
#
# Paste grid registered grids that do NOT share a row/column along X & Y edges
# We simulate this by creating pastable pixreg grids and toggle their registration

ps=paste_gridreg_underlap.ps

# The final grid is just f(x,y) = x*y
gmt grdmath -R-14.75/14.75/-14.75/14.75 -I0.5 X Y MUL = lixo.nc
# The 4 pieces to assemble into final grid
gmt grdmath -R-15/0/-15/0 -I0.5 X Y MUL -r = lixo_x1.nc
gmt grdedit lixo_x1.nc -T
gmt grdmath -R0/15/-15/0  -I0.5 X Y MUL -r = lixo_x2.nc
gmt grdedit lixo_x2.nc -T
gmt grdmath -R-15/0/0/15  -I0.5 X Y MUL -r = lixo_y1.nc
gmt grdedit lixo_y1.nc -T
gmt grdmath -R0/15/0/15   -I0.5 X Y MUL -r = lixo_y2.nc
gmt grdedit lixo_y2.nc -T
gmt grdpaste lixo_x1.nc lixo_x2.nc -Glixo_x.nc
gmt grdpaste lixo_y1.nc lixo_y2.nc -Glixo_y.nc
gmt grdpaste lixo_x.nc lixo_y.nc -Glixo_xy.nc
# Top is single source, bottom is assembled
gmt grdcontour lixo_xy.nc -JX10c -C10 -B5 -P -K -Xc > $ps
gmt grdcontour lixo.nc -J -C10 -B5 -O -Y12c >> $ps
