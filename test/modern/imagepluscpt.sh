#!/usr/bin/env bash
# Test automatic CPT scaling
gmt begin imagepluscpt ps
	gmt grdimage @earth_relief_01m -RMG+r2 -Cgeo -I+
	gmt coast -Wthin -N1/thick,red -BWSne -B
	gmt colorbar -DJTC -B
gmt end show
