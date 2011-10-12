#!/bin/sh
# Combine the tile files
#	$Id$

closed=""
open=""
for type in c l r; do
	rm -f SWBD_open_${type}.d SWBD_closed_${type}.d
	for TILE in NWa NWb NE SW SE; do
		cat ${TILE}/SWBD_${TILE}_open_${type}.d >> SWBD_open_${type}.d
		cat ${TILE}/SWBD_${TILE}_closed_${type}.d >> SWBD_closed_${type}.d
	done
	nc=`grep '^>' SWBD_closed_${type}.d | wc -l`
	no=`grep '^>' SWBD_open_${type}.d | wc -l`
	closed="$closed $nc"
	open="$open $no"
done
printf "\n%17s : C L R polygons Closed: %s Open: %s\n"  "-180/180/-60/60" "$closed" "$open"
