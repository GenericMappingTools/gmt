#!/usr/bin/env bash
# Test FISHER operator in grdmath 
# Centered on (90,30) with kappa of 40
# DVC_TEST
gmt begin fisher ps
	gmt grdmath -Rg -I1 90 30 40 FISHER DUP 0.05 GT MUL 0 NAN = fisher.grd
	gmt coast -JG60/30/15c -A1000 -Bafg30 -Glightgray
	gmt makecpt -Cturbo -T0/6.5/0.5
	gmt grdimage fisher.grd -Q -t50
	gmt grdcontour fisher.grd -A5 -C1 -Wa2p -Wc1p -GlZ+/90/-90
gmt end show
