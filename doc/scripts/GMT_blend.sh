#!/usr/bin/env bash
#

# "Inner" regions when blending (dashed lines)
cat << EOF > info.txt
a.nc	-R1/5/1/5	1
b.nc	-R5/10/1/4	1
c.nc	-R5/9/5/9	1
d.nc	-R1/5/5/9	1
EOF
# Make 4 constant grids
gmt grdmath -R0/6/0/6 -I0.1   0 = a.nc
gmt grdmath -R4/10/0/5 -I0.1  2 = b.nc
gmt grdmath -R0/6/4/10 -I0.1  8 = c.nc
gmt grdmath -R4/10/4/10 -I0.1 4 = d.nc

gmt begin GMT_blend
	gmt set GMT_THEME cookbook FONT_TAG 10p,Helvetica-Bold,black FONT_ANNOT_PRIMARY 8p
	gmt makecpt -Crainbow -T0/8
	gmt subplot begin 2x3 -Fs5c/3c -Sct -Srl -R0/10/0/10 -A0+jTR
	gmt subplot set 0 -Aaverage
	# 0. Just add them up
	gmt grdblend ?.nc -R0/10/0/10 -I0.1 -Gblend.nc
	gmt grdimage blend.nc
	# Draw grid outlines
	cat <<- EOF > lines.txt
	> a
	6	0
	6	6
	0	6
	> b
	4	0
	4	5
	10	5
	> c
	4	10
	4	4
	10	4
	> d
	6	10
	6	4
	0	4
	EOF
	gmt plot lines.txt -W1p
	# 1. Blend the overlapping grids
	gmt grdblend info.txt -I0.1 -Gblend.nc
	gmt subplot set 1 -Ablend
	gmt grdimage blend.nc
	gmt plot lines.txt -W1p
	# Draw the inside regions
	gmt plot -W0.5p,- -L <<- EOF
	> a
	1	1
	5	1
	5	5
	1	5
	> b
	5	1
	10	1
	10	4
	5	4
	> c
	5	5
	9	5
	9	9
	5	9
	> d
	1	5
	5	5
	5	9
	1	9
	EOF
	# 2. Last grid encountered matters
	gmt grdblend ?.nc -Co -I0.1 -Gblend.nc
	gmt subplot set 2 -Alast
	gmt grdimage blend.nc 
	gmt plot lines.txt -W1p
	# 3. First grid encountered matters
	gmt grdblend ?.nc -Cf -I0.1 -Gblend.nc
	gmt subplot set 3 -Afirst
	gmt grdimage blend.nc
	gmt plot  lines.txt -W1p
	# 4. Grid with lowest value matters
	gmt grdblend ?.nc -Cl -I0.1 -Gblend.nc
	gmt subplot set 4 -Alow
	gmt grdimage blend.nc
	gmt plot lines.txt -W1p
	# 5. Grid with highest value matters
	gmt grdblend ?.nc -Cu -I0.1 -Gblend.nc
	gmt subplot set 5 -Ahigh
	gmt grdimage blend.nc
	gmt plot lines.txt -W1p
	gmt subplot end
	gmt colorbar -DJRM -B1
gmt end show


