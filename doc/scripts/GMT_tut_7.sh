#!/usr/bin/env bash
gmt begin GMT_tut_7
	gmt plot @tut_data.txt -R0/6/0/6 -Jx1i -B -Wthinner
gmt end show
