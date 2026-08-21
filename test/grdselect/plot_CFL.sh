#!/usr/bin/env bash
# Test grdselect for finding grids that overlap with a set of
# points, lines or polygons

# Create a simple data sets that may be a polygon, line or points
# Here it happens to be a closed polygon
# We will see that only one grid (G1) actually contains a data point
# while two grids (G1 and G3) are crossed by the lines, and that
# three grids (G1, G3, and G4) overlap with the polygon area.
cat << EOF > data.txt
-0.35	0.25
-0.35	0
-0.25	-0.2
0	-0.4
0.45	-0.4
0.45	-0.3
0.1	-0.3
-0.1	0
0.35	0
0.35	0.25
-0.35	0.25
EOF

# Create 4 dummy grids
gmt grdmath -R0.05/0.25/-0.35/-0.15 -I0.01 1 = G1.grd
gmt grdmath -R0.3/0.5/-0.25/-0.05   -I0.01 2 = G2.grd
gmt grdmath -R-0.1/0.25/0.15/0.35   -I0.01 3 = G3.grd
gmt grdmath -R-0.3/-0.15/0/0.15     -I0.01 4 = G4.grd
gmt grdinfo G[1234].grd -Ib > all.txt

gmt begin plot_CFL
	gmt set FONT_TAG 14p,Helvetica,black
	gmt subplot begin 4x3 -R-0.5/0.5/-0.5/0.5 -Fs6c -A -Sct -Srl+p -X1.2c -Y0.8c
	gmt subplot set -A"All grids"
	gmt plot -W2p -Glightblue all.txt
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Point overlap"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Cdata.txt > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Line intersection"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Ldata.txt > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Polygon overlap"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Fdata.txt > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Polygon inclusion"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Fdata.txt+i > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Polygon exclusion"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Fdata.txt+o > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Invert Point overlap"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Cdata.txt -IC > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Invert Line intersection"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Ldata.txt -IL > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Invert Polygon overlap"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Fdata.txt -IF > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Invert Polygon inclusion"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Fdata.txt+i -IF > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Invert Polygon exclusion"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Fdata.txt+o -IF > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot set -A"Line and inverted point"
	gmt plot -W0.5,dashed all.txt
	gmt grdselect G[1234].grd -Cdata.txt -Ldata.txt+o -IC > tmp.lis
	gmt grdinfo $(cat tmp.lis) -Ib | gmt plot -W2p -Glightblue
	gmt plot data.txt -W2p
	gmt plot data.txt -Sc6p -Gred -W0.25p
	gmt subplot end
gmt end show
