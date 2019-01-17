#!/usr/bin/env bash
#
# This test is based on a query by Eduardo Suarez

ps=cuerpo.ps

gmt makecpt -Crainbow -T65/125/20 > cuerpo.cpt
gmt grdview super2.nc -Gdomo-filtrado.nc  -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95/550/760 \
	-JM-70:10:13.525/-42:40:25.525/3.5i -JZ1.5i -Ba10sf10s -Bza100f100 -BWESNZ+b -Ccuerpo.cpt -Qi -p315/30/550 -K > $ps
gmt psxyz limites-domo-xyz.dat -R -JM -JZ -Wthick,red,solid -p -L -O >> $ps

