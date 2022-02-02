#!/usr/bin/env bash
#
# Computes the magnetic anomaly of a cube
# DVC_TEST

ps=cube_mag.ps

gmt gmtgravmag3d -R-15/15/-15/15 -I1 -H10/60/10/-10/40 -M+sprism,1/1/1/-5 -Gcube_mag.grd

gmt grd2cpt  cube_mag.grd -E20 -D > m.cpt
gmt grdimage cube_mag.grd -Cm.cpt -JX12c -Ba -P > $ps
