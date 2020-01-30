#!/usr/bin/env bash
#
# Makes the data for GMT_App_O_[1-9].sh
#
gmt grdcut @osu91a1f_16.nc -R50/160/-15/15 -GApp_O_geoid.nc
# Complex line algorithm points
cat << EOF > App_O_cross.txt
> 1st branch
59	-12
62	-7
66	-3
71	-1
77	3
> 2nd branch
94	-11
100	-10.5
105	-9.5
109	-8.6
114	-6.5
119	-4
126	2
> 3rd branch
148	3
158	13
EOF
info=$(gmt grdinfo -M -Cn App_O_geoid.nc)
x0=$(echo $info | cut -f11 -d ' ')
y0=$(echo $info | cut -f12 -d ' ')
x1=$(echo $info | cut -f13 -d ' ')
y1=$(echo $info | cut -f14 -d ' ')
gmt project -C$x0/$y0 -E$x1/$y1 -G10 -Q > tt.d
dist=$(gmt convert tt.d --FORMAT_FLOAT_OUT=%.0lf -El -o2)
R=$(gmt info -I1 tt.d)
echo "# Geoid Extrema Separation is $dist km" > App_O_transect.txt
gmt grdtrack tt.d -GApp_O_geoid.nc | gmt grdtrack -GGMT_App_O.nc >> App_O_transect.txt
rm -f tt.d gmt.history
