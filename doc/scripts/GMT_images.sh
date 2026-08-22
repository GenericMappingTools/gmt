#!/usr/bin/env bash
gmt begin GMT_images
	gmt image @nsf1.jpg -R0/2/0/1 -JX5i/1.6i -B0 -DjML+w1.5i+o0.1i/0i
	gmt image @soest.eps -DjMR+o0.1i/0i+w2i
gmt end show
