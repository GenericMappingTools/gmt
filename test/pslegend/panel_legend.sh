#!/usr/bin/env bash
#
# Testing gmt pslegend within subplot panels after fixing the
# bug reported in https://forum.generic-mapping-tools.org/t/legends-in-subplots/535

gmt begin panel_legend ps
	gmt subplot begin 2x2 -Fs8c -M5p -A -Scb -Srl -Bwstr -R0/90/0/10
	gmt basemap -c0
	gmt legend -Dx1c/1c+w3c -F+gblue <<- EOF
	L 10 C Legend
	EOF
	gmt basemap -c1
	gmt basemap -c2
	gmt basemap -c3
	gmt legend -Dn0.9/0.9+w3c+jTR -F+gblue <<- EOF
	L 10 C Legend
	EOF
	gmt subplot end
gmt end show
