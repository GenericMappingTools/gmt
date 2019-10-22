#!/usr/bin/env bash
gmt begin GMT_tut_17
	gmt makecpt -Cno_green -T-2/30/2
	gmt grdimage -Rg -JW180/9i "@otemp.anal1deg.nc?otemp[2,0]" -Bag
gmt end show
