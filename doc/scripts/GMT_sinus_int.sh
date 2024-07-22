#!/usr/bin/env bash
gmt begin GMT_sinus_int
	gmt coast -R200/340/-90/90 -Ji0.04c -Bg -A10000 -Dc -Gdarkred -Sazure
	gmt coast -R-20/60/-90/90 -Bg -Dc -A10000 -Gdarkgreen -Sazure -X5.6c
	gmt coast -R60/200/-90/90 -Bg -Dc -A10000 -Gdarkblue -Sazure -X3.2c
gmt end show
