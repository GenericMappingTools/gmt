#!/usr/bin/env bash
# Illustrate 4 ways to build a CPT form a list of colors
gmt begin GMT_colorlist
	gmt makecpt -T0,2,6 -Cred,yellow,purple -Z
	gmt colorbar -R0/4/0/4 -Jx1i -C -Bx -By+l"d)" -Dx0/0+w5i/0.2i+h
	gmt makecpt -T0,4,5.5,6 -Cred,yellow,purple
	gmt colorbar -C -Bxf1 -By+l"c)" -Dx0/0+w5i/0.2i+h -Y0.3i
	gmt makecpt -T0/6/2 -Cred,yellow,purple
	gmt colorbar -C -Bxf1 -By+l"b)" -Dx0/0+w5i/0.2i+h -Y0.3i
	gmt makecpt -T0/6 -Cred,yellow,purple
	gmt colorbar -C -Bxf1 -By+l"a)" -Dx0/0+w5i/0.2i+h -Y0.3i
gmt end show
