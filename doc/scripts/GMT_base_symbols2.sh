#!/usr/bin/env bash
#
# Plot psxy basic multi-parameter geometric symbols for use on man page

gmt begin GMT_base_symbols2
    gmt set GMT_THEME cookbook
cat << EOF > tmp.txt
# All the basic geometric psxy symbols
1	1	30	1.5c	0.75c	e
2	1	30	1.5c	0.75c	j
3	1	1.5c	0.75c	r
4	1	0.75c	1.5c	0.35c	R
5	1	1.5c	-30	70	w
EOF
    gmt plot tmp.txt -R0.6/5.4/0.6/1.4 -B0g1 -B+n -Jx2c -Sc1.5c -W0.25p --PROJ_LENGTH_UNIT=cm -i0,1 --MAP_GRID_PEN_PRIMARY=default,dashed
    gmt plot tmp.txt -S -Glightblue -W1p --PROJ_LENGTH_UNIT=cm 
$AWK '{if ($2 == 1) print $1, $2, $NF}' tmp.txt | gmt pstext -N -F+f10p,Helvetica-Bold+jTC -Dj0/1c
gmt end show
