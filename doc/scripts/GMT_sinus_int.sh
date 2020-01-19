#!/usr/bin/env bash
gmt begin GMT_sinus_int
	gmt coast -R200/340/-90/90 -Ji0.014i -Bg -A10000 -Dc -Gdarkred -Sazure
	gmt coast -R-20/60/-90/90 -Bg -Dc -A10000 -Gdarkgreen -Sazure -X1.96i
	gmt coast -R60/200/-90/90 -Bg -Dc -A10000 -Gdarkblue -Sazure -X1.12i
gmt end show
