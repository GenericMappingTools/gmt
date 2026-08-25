#!/usr/bin/env bash
#
# Example 1, Linear x-y plot -JX
#

ps=grdbarb_01.ps
title="Linear x-y plot -JX"

gmt grdmath -Rg -f0f,1y -I10 X           = dir.grd  # -f0f is needed for poles
gmt grdmath -Rg -f0f,1y -I10 Y ABS 2 DIV = spd.grd
gmt grdmath dir.grd 180 SUB SIND spd.grd MUL = u.grd
gmt grdmath dir.grd 180 SUB COSD spd.grd MUL = v.grd

gmt grdbarb u.grd v.grd -W -JX15c/0 -Rg -Ba -BWSen+t"$title" -P > $ps

rm -f *.grd
