#!/usr/bin/env bash
#
# Test greenspline -A...+f6, which derives gradient constraints (midpoint,
# gradient magnitude, and azimuth) from successive x,y,z points along a
# profile or track.  We check that this matches feeding greenspline the
# equivalent, manually precomputed -A...+f2 (x, y, gradient, azimuth) data.

cat << EOF > gspline_9_data.txt
0	0	0
4	0	4
EOF

# Profile: three points along y = 0 with piecewise-linear z
cat << EOF > gspline_9_profile.txt
0	0	0
2	0	4
4	0	4
EOF

# Same information as above, precomputed by hand into format 2:
# midpoint (1,0): dz=4, ds=2 -> gradient=2, azimuth=90
# midpoint (3,0): dz=0, ds=2 -> gradient=0, azimuth=90
cat << EOF > gspline_9_format2.txt
1	0	2	90
3	0	0	90
EOF

gmt greenspline gspline_9_data.txt -R0/4/-2/2 -I0.5 -Sl -Z1 -Agspline_9_profile.txt+f6 -Ggspline_9_mode6.grd
gmt greenspline gspline_9_data.txt -R0/4/-2/2 -I0.5 -Sl -Z1 -Agspline_9_format2.txt+f2 -Ggspline_9_mode2.grd

gmt grdmath gspline_9_mode6.grd gspline_9_mode2.grd SUB ABS = gspline_9_diff.grd
gmt grdinfo -C gspline_9_diff.grd > gspline_9_diff.txt

$AWK '{if ($6 > 1e-10 || $7 > 1e-10) print $0}' gspline_9_diff.txt > fail
