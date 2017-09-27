#!/bin/bash
#	$Id$
#
# Test the C API for conversions between datasets, matrices, and vectors.
# Also read groups of items just to free them,

# Make to dataset tables with headers and segments
cat << EOF > A.txt
# Some dumb header
# that happens to take up two lines
> Some header for A
1	2
2	3
3	4
> Another header for A
6	7
7	8
EOF
cat << EOF > B.txt
# Another dumb header
> Some header for B
11	2
21	3
31	4
> Another header for B
61	7
71	8
EOF
# Make 3 grids, CPT, and PS
gmt grdmath -R0/5/0/5 -I1 X = x.nc
gmt grdmath -R0/5/0/5 -I1 Y = y.nc
gmt grdmath -R0/5/0/5 -I1 X Y MUL = x.nc
gmt makecpt -Cjet -T0/10 > a.cpt
gmt makecpt -Cpolar -T0/100 > b.cpt
gmt makecpt -Cgray -T0/1000/100 > last.cpt
gmt psbasemap -R0/20/0/20 -JM6i -P -Baf > first.ps
gmt psbasemap -R0/20/20/40 -JM6i -P -Baf > second.ps
gmt psbasemap -R0/20/40/60 -JM6i -P -Baf > third.ps
testapiconv
cat *AB*.txt > results.txt
diff -q --strip-trailing-cr results.txt "${src:-.}"/testapiconv_answer.txt > fail
