#!/bin/bash
#	$Id: paste_x.sh,v 1.1 2011-06-18 16:15:00 jluis Exp $
#
# Paste grids along X & Y

. ../functions.sh
header "Test grdpaste to paste grids horizontal and vertically"

ps=paste.ps
grdmath -R-15/0/-15/0 -I0.5 X = lixo_x1.grd
grdmath -R0/15/-15/0  -I0.5 X = lixo_x2.grd
grdmath -R-15/0/0.5/15 -I0.5 X = lixo_y1.grd
grdmath -R0/15/0.5/15  -I0.5 X = lixo_y2.grd
grdpaste lixo_x1.grd lixo_x2.grd -Glixo_x.grd
grdpaste lixo_y1.grd lixo_y2.grd -Glixo_y.grd
grdpaste lixo_x.grd lixo_y.grd -Glixo_xy.grd

grdcontour lixo_xy.grd -R-15/15/-15/15 -JX10c -C2 -B5 -P > $ps

rm -f lixo_*.grd

#pscmp
