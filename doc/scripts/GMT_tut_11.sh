#!/usr/bin/env bash
gmt begin GMT_tut_11
	gmt grdcontour @tut_bathy.nc -JM6i -C250 -A1000 -B
gmt end show
