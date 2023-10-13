#!/usr/bin/env bash
# Demonstrate filling of area between two sinusoidal curves with a section of NaNs
gmt begin GMT_fill_curves
	gmt set GMT_THEME cookbook
	gmt math -T0/720/5 -N3/0 T -C1 COSD -C2 3 MUL SIND 4 DIV -C1,2 1 T 200 250 INRANGE SUB 0 NAN MUL = both_NaN.txt
	gmt plot -Mc+gblue+r+l"Short @~l@~ exceeds long" -Bxa90g90+u@. -Byafg -BWStr -JX15c/3c -R0/720/-1.25/2.5 both_NaN.txt -Gred -l"Long @~l@~ exceeds short"
gmt end show
