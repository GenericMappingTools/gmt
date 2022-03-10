#!/usr/bin/env bash

ps=hovmuller.ps

gmt set TIME_EPOCH 2000-01-01T00 TIME_UNIT y
gmt set FORMAT_DATE_MAP o FORMAT_TIME_PRIMARY_MAP c FONT_ANNOT_PRIMARY 10p FONT_ANNOT_SECONDARY 12p
gmt set MAP_ANNOT_ORTHO ""

gmt makecpt -Crainbow -T-1/1/0.05 > tmp.cpt
# Make a fake longitude/time grid
gmt grdmath -R-180/180/0/3 -f0x,1t -I10/37+n X SIND Y 2 MUL PI MUL SIN MUL = tmp.nc
gmt grdimage tmp.nc -Ctmp.cpt -JX15c/22cT -Bx30f10 -By1O -Bsy1Y -E100 -P -Xc > $ps
