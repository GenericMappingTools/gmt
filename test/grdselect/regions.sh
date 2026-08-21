#!/usr/bin/env bash
# Test grdselect for selection of grids

gmt grdmath -R212/242/0/30     -fg -I2 X = 1.grd
gmt grdmath -R-135/-105/10/40  -fg -rp -I5 X = 2.grd
# This creates a grid that has 85 NaN nodes
gmt grdmath -R137W/121W/5S/45N -fg -I1 X Y MUL DUP 0 GT 1 NAN ADD = 3.grd

# Set up known answer file
cat << EOF > answer.txt
Grids that have pixel registration only
2.grd
Grids that have gridline registration and unit grid spacing
3.grd
Grids that have more than 80 NaNs
3.grd
Grids that have more no NaNs
1.grd
2.grd
Grids that have data values in the range -150/-100
2.grd
3.grd
EOF
echo "Grids that have pixel registration only" > result.txt
gmt grdselect ?.grd -rp >> result.txt
echo "Grids that have gridline registration and unit grid spacing" >> result.txt
gmt grdselect ?.grd -rg -D1 >> result.txt
echo "Grids that have more than 80 NaNs" >> result.txt
gmt grdselect ?.grd -Nh80 >> result.txt
echo "Grids that have more no NaNs" >> result.txt
gmt grdselect ?.grd -Nl >> result.txt
echo "Grids that have data values in the range -150/-100" >> result.txt
gmt grdselect ?.grd -W-150/-100 >> result.txt
diff result.txt answer.txt --strip-trailing-cr  >> fail
