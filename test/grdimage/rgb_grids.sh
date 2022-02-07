#!/usr/bin/env bash
#
# Testing gmt grdimage if given red, green, blue grids

ps=rgb_grids.ps
gmt grdmath -R0/5/0/5 -I0.1 -r X DUP UPPER DIV 255 MUL = r.grd
gmt grdmath -R0/5/0/5 -I0.1 -r Y DUP UPPER DIV 255 MUL = g.grd
gmt grdmath -R0/5/0/5 -I0.1 -r 5 X SUB 5 Y SUB MUL DUP UPPER DIV 255 MUL = b.grd
gmt grdmath -R0/5/0/5 -I0.1 -r 4 2 2 CDIST SUB 2 DIV 1 SUB = i.grd
gmt grdimage r.grd g.grd b.grd -Ii.grd -JX16c -Baf -BWSne+t"Plot r,g,b + i grids via grdimage" -P -Xc > $ps
