#!/bin/bash
#
#	$Id$
# This test is based on a query by Eduardo Suarez

. ./functions.sh
header "Test grdview in combination with psxyz"

makecpt -Crainbow -T65/130/20 > cuerpo.cpt
grdview "$src"/super2.nc -G"$src"/domo-filtrado.nc  -R-70:10:17.05/-70:10:0.10/-42:40:31.10/-42:40:19.95/550/760 \
	-JM-70:10:13.525/-42:40:25.525/3.5i -JZ1.5i -Ba10sf10s/a10sf10s/a100f100WESNZ+ -Ccuerpo.cpt -Qi -p315/30/550 -K > $ps
psxyz "$src"/limites-domo-xyz.dat -R -JM -JZ -Wthick,red,solid -p -L -O >> $ps

pscmp
