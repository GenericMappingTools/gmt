#!/usr/bin/env bash
#
# Tests gmt project to make Cartesian or geographic ellipses.
# Cartesian case expects direction while geographic expects azimuth
# This matches the expectations in psxy

gmt begin ellipses ps
	gmt project -N -Z4/3/0+e -G0.1 -C2/1 | gmt plot -R-2/5/-2/5 -Jx1.5c -Bafg1 -W1p -Glightgray
	gmt project -Z400/300/0+e -G10 -C2/1 -Q | gmt plot -Jm1.5c -Bafg1 -W1p -Glightgray -Y13c
gmt end show
