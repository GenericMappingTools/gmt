#!/bin/sh
# Make coastline polygons from SRTM's SWBD files
#	$Id: swbd_loop.sh,v 1.2 2009-06-18 23:02:15 guru Exp $

WEST=$1
EAST=$2
SOUTH=$3
NORTH=$4
TILE=$5

w=$WEST
while [ $w -lt $EAST ]; do
	e=`expr $w + 10`
	n=$NORTH
	while [ $n -gt $SOUTH ]; do
		s=`expr $n - 10`
		swbd_stitch.sh $w $e $s $n $TILE
		n=$s
	done
	w=$e
done
closed=""
open=""
for type in c l r; do
	nc=0
	no=0
	if [ -f ${TILE}/SWBD_${TILE}_open_${type}.d ]; then
		nc=`grep '^>' ${TILE}/SWBD_${TILE}_closed_${type}.d | wc -l`
	fi
	if [ -f ${TILE}/SWBD_${TILE}_open_${type}.d ]; then
		no=`grep '^>' ${TILE}/SWBD_${TILE}_open_${type}.d | wc -l`
	fi
	closed="$closed $nc"
	open="$open $no"
done
printf "\n%17s : C L R polygons Closed: %s Open: %s\n"  "$WEST/$EAST/$SOUTH/$NORTH" "$closed" "$open"
