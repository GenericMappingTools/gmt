#!/usr/bin/env bash
# Test automatic CPT scaling
gmt begin viewpluscpt ps
	gmt grdview @earth_relief_05m -RMG+r2 -Cgeo -I+ -Qi100i
	gmt coast -Wthin -N1/thick,red -BWSne -B
	gmt colorbar -DJTC -B
gmt end show
