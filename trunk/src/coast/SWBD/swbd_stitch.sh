#!/bin/sh
# Make coastline polygons from SRTM's SWBD files
#	$Id: swbd_stitch.sh,v 1.12 2009-06-17 22:09:04 guru Exp $
#
# Usage: swbd_stitch.sh w e s n JOBDIR
#
# This script accepts w/e/s/n and a job dir as the first 5 arguments and
# looks for SWBD files in a specified DIR.  The *.zip files are then unzipped
# and ogr2ogr is used to convert shapefiles to GMT tables.  These are
# modified slightly by the rearranger so that all the GIS comments are
# placed on the multisegment header record instead of comment records
# follwoing the segment header.  We then use the SWBD codes to separate
# coastlines, lakes, and rivers so we can process them separately.
# We seek to remove portions of thetile outline that has been added to
# many features by looking for horizontal or vertical line segments that
# occur along the w/e/s/n borders exceeding a minimum length.  We allow
# some play here: ~1 arc sec deviation from w/e/s/n, in dx and dy, and use
# 0 meter as limit.
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
JOBDIR=$5
list=1
extract=1
stitch=1
combine=1

mkdir -p $JOBDIR
cd $JOBDIR

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
		rearranger < $name.gmt > tmp
		gmtconvert -m -fg -S"BA040" -F0,1 --D_FORMAT=%.6f tmp > raw_c/$name.gmt
		gmtconvert -m -fg -S"BH080" -F0,1 --D_FORMAT=%.6f tmp > raw_l/$name.gmt
		gmtconvert -m -fg -S"BH140" -F0,1 --D_FORMAT=%.6f tmp > raw_r/$name.gmt
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
			gmtpoly -L0/2.8e-4/2.8e-4 -m -fg -R$w/$e/$s/$n raw_${type}/$name.gmt --D_FORMAT=%.6f >> SWBD.raw_${type}.d
		done
	done < files.lis
	rm -rf raw_[clr]
#	Stitch together those segments that form closed polygons (coast, lakes, rivers separately)
	for type in c l r; do
		if [ -f SWBD.raw_${type}.d ]; then
			mkdir -p pol_${type}
			gmtstitch -fg -T0 -Dpol_${type}/srtm_pol_%c_%8.8d.txt -L -m SWBD.raw_${type}.d -Qlist_${type}_%c.lis --D_FORMAT=%.6f
			while read file; do
				echo "> $type segment" >> SWBD_open_${type}.d
				cat $file >> SWBD_open_${type}.d
			done < list_${type}_O.lis
			while read file; do
				echo "> $type polygon" >> SWBD_closed_${type}.d
				cat $file >> SWBD_closed_${type}.d
			done < list_${type}_C.lis
			rm -rf pol_${type} SWBD.raw_${type}.d list_${type}_[CO].lis
		fi
	done
fi
if [ $combine -eq 1 ]; then
#	See if the new batch of open segments may be combinable with those already in the file
	closed=""
	open=""
	for type in c l r; do
		if [ -f SWBD_open_${type}.d ]; then
			mkdir -p polc_${type}
			gmtstitch -fg -T0 -Dpolc_${type}/srtm_pol_%c_%8.8d.txt -L -m SWBD_open_${type}.d -Qlist_${type}_%c.lis --D_FORMAT=%.6f
			rm -f SWBD_open_${type}.d
			while read file; do
				echo "> $type segment" >> SWBD_open_${type}.d
				cat $file >> SWBD_open_${type}.d
			done < list_${type}_O.lis
			while read file; do
				echo "> $type polygon" >> SWBD_closed_${type}.d
				cat $file >> SWBD_closed_${type}.d
			done < list_${type}_C.lis
			rm -rf polc_${type} list_${type}_[CO].lis
		fi
		nc=0
		no=0
		if [ -f SWBD_closed_${type}.d ]; then
			nc=`grep '^>' SWBD_closed_${type}.d | wc -l`
		fi
		if [ -f SWBD_open_${type}.d ]; then
			no=`grep '^>' SWBD_open_${type}.d | wc -l`
		fi
		closed="$closed $nc"
		open="$open $no"
		mv links.d links_${type}.d
	done
	printf "%17s : C L R polygons Closed: %s Open: %s\n"  "$WEST/$EAST/$SOUTH/$NORTH" "$closed" "$open"
fi
rm -f files.d
cd ..
