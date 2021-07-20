#!/usr/bin/env bash
# Test a 2x2 basemap matrix with different history settings
# at different hierarchical levels, showing of splines
cat << EOF > t.txt
5	0.5
12	7
30	5
50	4
75	1
EOF

gmt begin panels_history ps
  gmt set FONT_TITLE Helvetica-Bold
  gmt subplot begin 2x2 -Fs3i -M5p -A -Scb -Srl -Bwstr -R0/80/0/10 -T"SPLINE INTERPOLATIONS"
    gmt set FONT_ANNOT_PRIMARY=Times-Roman
    gmt subplot set 0 -ALINEAR
    gmt plot t.txt -Sc0.2c -Gblack
    gmt set GMT_INTERPOLANT linear
    gmt sample1d -I1 t.txt | gmt plot -W1p,green
    gmt subplot set 1 -ACUBIC
    gmt basemap
    gmt plot t.txt -Sc0.2c -Gblack
    gmt set GMT_INTERPOLANT cubic
    gmt sample1d -I1 t.txt | gmt plot -W1p,orange
    gmt subplot set 2 -A"BEZIER (In PostScript)"
    gmt basemap
    gmt plot t.txt -Sc0.2c -Gblack
    gmt plot t.txt -W1p,blue+s
    gmt subplot set 3 -AAKIMA
    gmt basemap --FONT_ANNOT_PRIMARY=Helvetica-Bold
    gmt plot t.txt -Sc0.2c -Gblack
    gmt sample1d -I1 t.txt | gmt plot -W1p,red
  gmt subplot end
gmt end show
