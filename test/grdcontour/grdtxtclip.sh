#!/bin/bash
ps=grdtxtclip.ps
grdcontour test.grd -R-74.25/-63.75/-44.0/-39.5 \
       -JB-69/-41.75/-40.625/-42.875/6i -Ba1 \
       -C500 -A1000+e+p+ggreen@50 -K -P > $ps 
psclip -C -O >> $ps
