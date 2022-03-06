#!/usr/bin/env bash
gmt begin GMT_tut_5
	gmt set GMT_THEME cookbook
	gmt coast -Rg -JG280/30/6i -Bag -Dc -A5000 -Gwhite -SDarkTurquoise
gmt end show
