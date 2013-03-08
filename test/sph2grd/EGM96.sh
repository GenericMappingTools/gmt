#!/bin/bash
#	$Id$
#
# Evaluate first 36 degrees of EGM96 geoid model

ps=EGM96.ps
sph2grd EGM96_to_36.txt -I1 -Rg -Nn -Ggrid.nc
grdmath grid.nc 1e5 MUL = grid.nc
grd2cpt grid.nc -Crainbow -E16 -Z > t.cpt
grdimage grid.nc -JH0/6i -P -B30 -Ct.cpt > $ps
