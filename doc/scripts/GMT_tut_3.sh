#!/usr/bin/env bash
gmt begin GMT_tut_3
	gmt set GMT_THEME cookbook
	gmt coast -R-90/-70/0/20 -JM6i -B -Gchocolate
gmt end show
