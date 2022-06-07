#!/usr/bin/env bash
gmt begin GMT_pow
	gmt set GMT_THEME cookbook
	gmt math -T0/100/1 T SQRT = sqrt.txt
	gmt math -T0/100/10 T SQRT = sqrt10.txt
	gmt plot -R0/100/0/10 -Jx0.75cp0.5/0.4c -Bxa1p -Bya2f1 -BWSne+givory -Wthick sqrt.txt
	gmt plot -Sc0.2c -Ggreen -W sqrt10.txt
gmt end show
