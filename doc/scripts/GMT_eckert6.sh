#!/usr/bin/env bash
gmt begin GMT_eckert6
	gmt set GMT_THEME cookbook
	gmt coast -Rg -JKs4.5i -Bg -Dc -A10000 -Wthinnest -Givory -Sbisque3
gmt end show
