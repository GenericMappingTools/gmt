#!/bin/sh
# Make coastline polygons from SRTM's SWBD files
#	$Id$
#
# Usage: swbd_stitch.sh type
#
# This script follows on from swbd_separate and will try to find more closed
# polygons from the open segments


#	See if the new batch of open segments may be combinable with those already in the file
closed=""
open=""
for type in c l r; do
	if [ -f SWBD_open_${type}.d ]; then
		mkdir -p polc_${type}
		gmtstitch -fg -T0 -Dpolc_${type}/srtm_pol_%c_%8.8d.txt -L -V SWBD_open_${type}.d -Qlist_${type}_%c.lis --D_FORMAT=%.6f --OUTPUT_DEGREE_FORMAT=D
		rm -f SWBD_open2_${type}.d
		while read file; do
			cat $file >> SWBD_open2_${type}.d
		done < list_${type}_O.lis
		rm -f SWBD_closed2_${type}.d
		while read file; do
			cat $file >> SWBD_closed2_${type}.d
		done < list_${type}_C.lis
		rm -rf polc_${type} list_${type}_[CO].lis
		mv links.d links_${type}.d
	fi
	nc=0
	no=0
	if [ -f SWBD_closed_${type}.d ]; then
		nc=`grep '^>' SWBD_closed2_${type}.d | wc -l`
	fi
	if [ -f SWBD_open_${type}.d ]; then
		no=`grep '^>' SWBD_open2_${type}.d | wc -l`
	fi
	closed="$closed $nc"
	open="$open $no"
done
printf "%17s : New C L R polygons Closed: %s Open: %s\n"  "$WEST/$EAST/$SOUTH/$NORTH" "$closed" "$open"
