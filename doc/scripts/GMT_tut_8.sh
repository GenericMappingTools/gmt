#!/usr/bin/env bash
gmt begin GMT_tut_8
	gmt plot @tut_data.txt -R0/6/0/6 -Jx1i -B -Wthinner
	gmt plot tut_data.txt -W -Si0.2i
gmt end show
