#!/usr/bin/env bash
#
# Plot psxy basic single-parameter geometric symbols for use on man page

ps=GMT_base_symbols1.ps

cat << EOF > tmp.txt
# All the basic geometric psxy symbols
1	2	1c	-
2	2	1c	+
3	2	1c	a
4	2	1c	c
5	2	1c	d
6	2	1c	g
7	2	1c	h
1	1	1c	i
2	1	1c	n
3	1	1c	p
4	1	1c	s
5	1	1c	t
6	1	1c	x
7	1	1c	y
EOF
gmt psxy tmp.txt -R0.6/7.4/0.6/2.4 -B0g1 -B+n -Jx1.5c -Sc1c -W0.25p -P -K --PROJ_LENGTH_UNIT=cm -i0,1 --MAP_GRID_PEN_PRIMARY=default,dashed > $ps
gmt psxy tmp.txt -R -J -S -Glightblue -W1p -O -K --PROJ_LENGTH_UNIT=cm >> $ps
$AWK '{if ($2 == 1) print $1, $2, $NF}' tmp.txt | gmt pstext -R -J -N -O -K -F+f10p,Helvetica-Bold+jTC -Dj0/0.75c >> $ps
$AWK '{if ($2 == 2) print $1, $2, $NF}' tmp.txt | gmt pstext -R -J -N -O -F+f10p,Helvetica-Bold+jBC -Dj0/0.75c >> $ps
