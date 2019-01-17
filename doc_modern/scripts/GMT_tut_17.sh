#!/usr/bin/env bash
gmt makecpt -Cno_green -T-2/30/2 > otemp.cpt
gmt grdimage -Rg -JW180/9i "@otemp.anal1deg.nc?otemp[2,0]" -Cotemp.cpt -Bag -ps  GMT_tut_17
