#!/usr/bin/env bash
# Test -SP sphere symbol
ps=sphere.ps
echo 3 3 3 | gmt psxyz -R0/6/0/6/0/6 -JX10c -JZ4c -p135/45 -SP2c -Gblue -W0.2p,red -Ba -Bza -P -K > $ps
echo 5 2 4 | gmt psxyz -R -J -JZ -p -SP1.5c+f -Ggreen -W0.5p -O -K >> $ps
echo 2 5 2 | gmt psxyz -R -J -JZ -p -SP1c+n -Gred -W0.3p,black -O >> $ps
