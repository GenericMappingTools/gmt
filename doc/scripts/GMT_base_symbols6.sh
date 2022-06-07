#!/usr/bin/env bash
#
# Plot psxy custom symbols for use on man page

gmt begin GMT_base_symbols6
    gmt set GMT_THEME cookbook
cat << EOF > tmp.txt
# All the basic geometric plot symbols
1	1	1.5c	kdeltoid
2	1	1.5		ksectoid
3	1	1.5c	kvolcano
4	1	1.5c	ktrirot4
5	1	1.5c	khurricane
6	1	1.5c	kflash
7	1	1.5c	kQR
EOF
    gmt plot tmp.txt -R0.6/7.4/0.6/1.4 -B0g1 -B+n -Jx2c -Sc1.5c -W0.25p --PROJ_LENGTH_UNIT=cm -i0,1 --MAP_GRID_PEN_PRIMARY=default,dashed
    gmt plot tmp.txt -S -Glightblue -W1p --PROJ_LENGTH_UNIT=cm
    $AWK '{if ($2 == 1) print $1, $2, substr($NF,2)}' tmp.txt | gmt text -N -F+f10p,Helvetica-Bold+jTC -Dj0/1c
gmt end show
