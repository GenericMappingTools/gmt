#!/bin/bash
#	$Id$
#
# Evaluate first 36 degrees of EGM96 geoid model

ps=EGM96.ps
sph2grd EGM96_to_36.txt -I1 -Rg -Dn -Ggrid.nc
grdcontour grid.nc -JH0/6i -P -B30 grid.nc -A10 -C5 > $ps
