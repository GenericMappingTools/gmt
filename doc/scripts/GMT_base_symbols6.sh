#!/usr/bin/env bash
#
# Plot psxy custom symbols for use on man page

ps=GMT_base_symbols6.ps

cat << EOF > tmp.txt
# All the basic geometric psxy symbols
1	1	1.5c	kdeltoid
2	1	1.5		ksectoid
3	1	1.5c	kvolcano
4	1	1.5c	ktrirot4
5	1	1.5c	khurricane
6	1	1.5c	kflash
7	1	1.5c	kQR
EOF
gmt psxy tmp.txt -R0.6/7.4/0.6/1.4 -B0g1 -B+n -Jx2c -Sc1.5c -W0.25p -P -K --PROJ_LENGTH_UNIT=cm -i0,1 --MAP_GRID_PEN_PRIMARY=default,dashed > $ps
gmt psxy tmp.txt -R -J -S -Glightblue -W1p -O -K --PROJ_LENGTH_UNIT=cm >> $ps
$AWK '{if ($2 == 1) print $1, $2, substr($NF,2)}' tmp.txt | gmt pstext -R -J -N -O -F+f10p,Helvetica-Bold+jTC -Dj0/1c >> $ps
