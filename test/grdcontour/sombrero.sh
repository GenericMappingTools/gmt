#!/usr/bin/env bash
# Test implementation of feature #1141
ps=sombrero.ps

gmt grdmath -R-15/15/-15/15 -I0.3 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL 0.001 SUB = sombrero.nc
gmt grdimage sombrero.nc -P -JX4.5i -K -Xc -Y0.75i > $ps
gmt grdcontour sombrero.nc -C1 -Bafg -O -K -J -T -Wthick >> $ps
gmt grdimage sombrero.nc -O -J -K -Y5i -BWsNE -Bafg >> $ps
gmt grdcontour sombrero.nc -C1 -O -J -T+a -Wthick >> $ps
