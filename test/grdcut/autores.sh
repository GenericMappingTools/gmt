#!/usr/bin/env bash
# Test grdcut -D returned rectangular region for a -JG -Rd selection
# for all the remote tiled data.  We compare the output to known answers.

cat << EOF > answer.txt
8.23333	21.76667	13.79167	26.00833	0.00833	0.00833
8.23333	21.76667	13.78333	26.01667	0.01667	0.01667
8.23333	21.76667	13.79167	26.00833	0.00833	0.00833
8.23333	21.76667	13.79167	26.00833	0.00833	0.00833
8.23333	21.76667	13.79167	26.00833	0.00833	0.00833
EOF
gmt set FORMAT_FLOAT_OUT %.5lf
PROJ=-JG15/20/2500/0/0/0/30/30/15c
gmt grdcut @earth_relief -Rg $PROJ -D >  result.txt
gmt grdcut @earth_age    -Rg $PROJ -D >> result.txt
gmt grdcut @earth_mask   -Rg $PROJ -D >> result.txt
gmt grdcut @earth_day    -Rg $PROJ -D >> result.txt
gmt grdcut @earth_night  -Rg $PROJ -D >> result.txt
diff result.txt answer.txt --strip-trailing-cr > fail
