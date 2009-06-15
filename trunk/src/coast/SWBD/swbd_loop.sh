#!/bin/sh
# Make coastline polygons from SRTM's SWBD files
#	$Id: swbd_loop.sh,v 1.1 2009-06-15 20:15:49 guru Exp $

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
