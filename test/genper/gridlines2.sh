#!/usr/bin/env bash
# More testing of issue #667, item #5
ps=gridlines2.ps
lon0=225.989251068
lat0=80.3192229138
gmt psbasemap -Rg -JG$lon0/$lat0/6166.78/0/0/54.6281322919/68.2/68.2/6i -Bxa30f10g10 -Bya5f5g5 -BWSne -P -Xc > $ps
