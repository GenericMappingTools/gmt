#!/usr/bin/env bash
#
# Show all splines and their derivatives for a basic data set
cat << EOF > t.txt
0	0
1	1
2	1.5
3	1.25
4	1.5
4.5	3
5	2
6	2.5
EOF
# Splines
gmt begin doc_splines png
  gmt basemap -R-0.1/6.1/-0.1/3.1 -JX15c/6c -Baf+l"u" -By+l"u(x)" -BWSne
  gmt sample1d t.txt -T0.01 -Fc	| gmt plot -W1p		  -l"Cubic spline (-Fc)"
  gmt sample1d t.txt -T0.01 -Fs1	| gmt plot -W1p,orange	  -l"Smooth cubic spline (-Fs1)"
  gmt sample1d t.txt -T0.01 -Fl	| gmt plot -W1p,blue	  -l"Linear spline (-Fl)"
  gmt sample1d t.txt -T0.01 -Fa	| gmt plot -W1p,red	  -l"Akima spline (-Fa)"
  gmt sample1d t.txt -T0.01 -Fn	| gmt plot -W1p,darkgreen -l"Nearest neighbor (-Fn)"
  gmt plot t.txt -Sc0.25c -Gred -Wthin -l"Data"
  gmt legend -DjTL+w4.9c+o0.2c -F+p1p+gwhite+s 
gmt end show
