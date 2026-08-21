#!/usr/bin/env bash
# Illustrate 4 ways to build a CPT form a list of colors
gmt begin GMT_colorlist
	gmt makecpt -T0,2,6 -Cred,yellow,purple -Z
	gmt colorbar -R0/4/0/4 -Jx1i -C -Bx -By+l"d)" -Dx0/0+w5i/0.2i+h+mu
	echo "5.1 0 -T0,2,6 -Cred,yellow,purple -Z" | gmt text -F+f12p+jLB -N
	gmt makecpt -T0,4,5.5,6 -Cred,yellow,purple
	gmt colorbar -C -Bxf1 -By+l"c)" -Dx0/0+w5i/0.2i+h+mu -Y0.3i
	echo "5.1 0 -T0,4,5.5,6 -Cred,yellow,purple" | gmt text -F+f12p+jLB -N
	gmt makecpt -T0/6/2 -Cred,yellow,purple
	gmt colorbar -C -Bxf1 -By+l"b)" -Dx0/0+w5i/0.2i+h+mu -Y0.3i
	echo "5.1 0 -T0/6/2 -Cred,yellow,purple" | gmt text -F+f12p+jLB -N
	gmt makecpt -T0/6 -Cred,yellow,purple
	gmt colorbar -C -Bxf1 -By+l"a)" -Dx0/0+w5i/0.2i+h+mu -Y0.3i
	echo "5.1 0 -T0/6 -Cred,yellow,purple" | gmt text -F+f12p+jLB -N
gmt end show
