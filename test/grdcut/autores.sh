#!/usr/bin/env bash
# Test grdcut -D returned rectangular region for a -JG -Rd selection
# for all the remote tiled data.  We compare the output to known answers.

cat << EOF > answer.txt
8.33333	21.66667	13.89583	25.90833	0.00417	0.00417
8.33333	21.66667	13.88333	25.91667	0.01667	0.01667
8.33333	21.66667	13.89583	25.90833	0.00417	0.00417
8.33333	21.66667	13.89167	25.90833	0.00833	0.00833
8.33333	21.66667	13.89167	25.90833	0.00833	0.00833
EOF
gmt set FORMAT_FLOAT_OUT %.5lf
PROJ=-JG15/20/2500/0/0/0/30/30/15c
gmt grdcut @earth_relief -Rg $PROJ -D >  result.txt
gmt grdcut @earth_age    -Rg $PROJ -D >> result.txt
gmt grdcut @earth_mask   -Rg $PROJ -D >> result.txt
gmt grdcut @earth_day    -Rg $PROJ -D >> result.txt
gmt grdcut @earth_night  -Rg $PROJ -D >> result.txt
diff result.txt answer.txt --strip-trailing-cr > fail
