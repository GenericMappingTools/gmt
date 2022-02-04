#!/usr/bin/env bash
# Test automatic CPT scaling
# DVC_TEST
gmt begin imagepluscpt ps
	gmt grdimage @earth_relief_05m -RMG+r2 -Cgeo -I+d
	gmt coast -Wthin -N1/thick,red -BWSne -B
	gmt colorbar -DJTC -B
gmt end show
