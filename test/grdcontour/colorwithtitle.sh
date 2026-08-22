#!/usr/bin/env bash
# Address issue #1231
ps=colorwithtitle.ps
gmt makecpt -Cgeo -T-2000/2000/200 > col.cpt
gmt grdcontour -P -Bafg -BWesN+t"Title problem" @earth_relief_15m -JM6i -RIS -Ccol.cpt -N -An -Xc > $ps
