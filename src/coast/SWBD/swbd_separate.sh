#!/bin/sh
# Make coastline polygons from SRTM's SWBD files
#	$Id: swbd_separate.sh,v 1.3 2011-03-15 02:06:37 guru Exp $
#
# Usage: swbd_separate.sh w e s n TILE
#
# This script accepts w/e/s/n and a job dir as the first 5 arguments and
# looks for SWBD files in a specified DIR.  The *.zip files are then unzipped
# and ogr2ogr is used to convert shapefiles to GMT tables.  These are
# modified slightly by the rearranger so that all the GIS comments are
# placed on the multisegment header record instead of comment records
# follwoing the segment header.  We then use the SWBD codes to separate
# coastlines, lakes, and rivers so we can process them separately.
# We seek to remove portions of the tile outline that has been added to
# many features by looking for horizontal or vertical line segments that
# occur along the w/e/s/n borders exceeding a minimum length.  We allow
# some play here: ~1 arc sec deviation from w/e/s/n, in dx and dy, and use
# 0 meter as limit.
# We then write out closed and open segments to separate files.
#
# SWBD codes:
#	BA040 Coastline
#	BH080 Lake
#	BH140 Riverlake

filename () {	# w,s
	if [ $1 -lt 0 ]; then
		w=`gmtmath -Q $1 NEG =`
		x=w
	else
		w=$1
		x=e
	fi
	if [ $2 -lt 0 ]; then
		s=`gmtmath -Q $2 NEG =`
		y=s
	else
		s=$2
		y=n
	fi
        printf "%s%3.3d%s%2.2d?\n" "$x" $w "$y" $s
}
DIR=$GMT_DATADIR/SWBD
WEST=$1
EAST=$2
SOUTH=$3
NORTH=$4
TILE=$5
list=1
extract=1
stitch=1

mkdir -p $TILE
cd $TILE

if [ $list -eq 1 ]; then
#	Create a list (files.lis) of the zip files found inside this region
	w=$WEST
	rm -f files.lis
	while [ $w -lt $EAST ]; do
		e=`expr $w + 1`
		n=$NORTH
		while [ $n -gt $SOUTH ]; do
			s=`expr $n - 1`
			name=`filename $w $s`
			(ls $DIR/SWBDeast/$name.zip >> files.lis) 2> /dev/null
			(ls $DIR/SWBDwest/$name.zip >> files.lis) 2> /dev/null
			n=$s
		done
		w=$e
	done
	rm -rf raw_[clr]
#	nf=`wc -l files.lis`
#	echo "Found zip files : $nf"
fi
#-----------------------------------------------
if [ $extract -eq 1 ]; then
#	Extract info from the zipped shapefiles and separate into coast, lakes, and river files
	mkdir -p raw_c raw_l raw_r
	while read file; do
		name=`basename $file '.zip'`
		unzip -q $file
		ogr2ogr -f "GMT" $name.d $name.shp
		rearranger $name < $name.gmt > tmp
		gmtconvert -fg -S"BA040" -o0,1 --D_FORMAT=%.8f --OUTPUT_DEGREE_FORMAT=D tmp > raw_c/$name.gmt
		gmtconvert -fg -S"BH080" -o0,1 --D_FORMAT=%.8f --OUTPUT_DEGREE_FORMAT=D tmp > raw_l/$name.gmt
		gmtconvert -fg -S"BH140" -o0,1 --D_FORMAT=%.8f --OUTPUT_DEGREE_FORMAT=D tmp > raw_r/$name.gmt
		rm -f tmp $name.* *.zip
	done < files.lis
fi
#-----------------------------------------------
if [ $stitch -eq 1 ]; then
#	Clip all polygons to fit inside 1x1 tile to eliminate closed boundingbox portions.
	rm -f SWBD.raw_[clr].d
	while read file; do
		name=`basename $file '.zip'`
		w=`echo $name | awk '{if (substr($1,1,1) == "w") {printf "-%d\n", substr($1, 2, 3)} else {printf "%d\n", substr($1, 2, 3)}}'`
		e=`expr $w + 1`
		s=`echo $name | awk '{if (substr($1,5,1) == "s") {printf "-%d\n", substr($1, 6, 2)} else {printf "%d\n", substr($1, 6, 2)}}'`
		n=`expr $s + 1`
		for type in c l r; do
			gmtpoly -L0/2.8e-4/2.8e-4 -fg -R$w/$e/$s/$n raw_${type}/$name.gmt --D_FORMAT=%.6f --OUTPUT_DEGREE_FORMAT=D >> SWBD.raw_${type}.d
		done
	done < files.lis
	rm -rf raw_[clr]
#	Separate out those segments that already form closed polygons (coast, lakes, rivers separately)
	closed=""
	open=""
	for type in c l r; do
		if [ -f SWBD.raw_${type}.d ]; then
			rm -f closed
			gmtstitch -fg -Cclosed SWBD.raw_${type}.d --D_FORMAT=%.6f --OUTPUT_DEGREE_FORMAT=D > open
			nc=0
			if [ -f closed ]; then
				cat closed >> SWBD_${TILE}_closed_${type}.d
				nc=`grep '^>' closed | wc -l`
			fi
			cat open >> SWBD_${TILE}_open_${type}.d
			no=`grep '^>' open | wc -l`
			rm -f open closed SWBD.raw_${type}.d
			closed="$closed $nc"
			open="$open $no"
		else
			closed="$closed        -"
			open="$open        -"
		fi
	done
	printf "%17s : C L R polygons Closed: %s Open: %s\n"  "$WEST/$EAST/$SOUTH/$NORTH" "$closed" "$open"
fi
rm -f files.d
cd ..
