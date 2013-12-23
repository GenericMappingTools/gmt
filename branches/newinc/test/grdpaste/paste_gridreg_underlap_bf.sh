#!/bin/bash
#	$Id$
#
# Paste grid registered grids that do NOT share a row/column along X & Y edges
# We simulate this by creating pastable pixreg grids and toggle their registration
# This version tests for the non-netCDF format

ps=paste_gridreg_underlap_bf.ps

# The final grid is just f(x,y) = x*y
gmt grdmath -R-14.75/14.75/-14.75/14.75 -I0.5 X Y MUL = lixo.grd=bf
# The 4 pieces to assemble into final grid
gmt grdmath -R-15/0/-15/0 -I0.5 X Y MUL -r = lixo_x1.grd=bf
gmt grdedit lixo_x1.grd=bf -T
gmt grdmath -R0/15/-15/0  -I0.5 X Y MUL -r = lixo_x2.grd=bf
gmt grdedit lixo_x2.grd=bf -T
gmt grdmath -R-15/0/0/15  -I0.5 X Y MUL -r = lixo_y1.grd=bf
gmt grdedit lixo_y1.grd=bf -T
gmt grdmath -R0/15/0/15   -I0.5 X Y MUL -r = lixo_y2.grd=bf
gmt grdedit lixo_y2.grd=bf -T
gmt grdpaste lixo_x1.grd=bf lixo_x2.grd=bf -Glixo_x.grd=bf
gmt grdpaste lixo_y1.grd=bf lixo_y2.grd=bf -Glixo_y.grd=bf
gmt grdpaste lixo_x.grd=bf lixo_y.grd=bf -Glixo_xy.grd=bf
# Top is single source, bottom is assembled
gmt grdcontour lixo_xy.grd=bf -JX10c -C10 -B5 -P -K -Xc > $ps
gmt grdcontour lixo.grd=bf -J -C10 -B5 -O -Y12c >> $ps
