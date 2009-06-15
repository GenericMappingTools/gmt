#!/bin/sh
# Make coastline polygons from SRTM's SWBD files
#	$Id: swbd_stitch.sh,v 1.2 2009-06-15 18:15:34 guru Exp $

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
DIR=/home/gmt1/data/misc/SWBD
WEST=$1
EAST=$2
SOUTH=$3
NORTH=$4
list=1
extract=1
stitch=1
combine=1
if [ $list -eq 1 ]; then
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
	rm -rf raw
fi
#-----------------------------------------------
if [ $extract -eq 1 ]; then
	mkdir -p raw
	while read file; do
		cp -f $file .
		name=`basename $file '.zip'`
		unzip -q $name.zip
		ogr2ogr -f "GMT" $name.d $name.shp
		mv $name.gmt raw
		rm -f $name.* *.zip
	done < files.lis
fi
#-----------------------------------------------
if [ $stitch -eq 1 ]; then
	rm -f SWBD.raw.d
	while read file; do
		name=`basename $file '.zip'`
		w=`echo $name | awk '{if (substr($1,1,1) == "w") {printf "-%d\n", substr($1, 2, 3)} else {printf "%d\n", substr($1, 2, 3)}}'`
		e=`expr $w + 1`
		s=`echo $name | awk '{if (substr($1,5,1) == "s") {printf "-%d\n", substr($1, 6, 2)} else {printf "%d\n", substr($1, 6, 2)}}'`
		n=`expr $s + 1`
		gmtpoly -T -m -fg -R$w/$e/$s/$n raw/$name.gmt >> SWBD.raw.d
	done < files.lis
	if [ -f SWBD.raw.d ]; then
		mkdir -p pol
		cd pol
		gmtstitch -fg -T0 -Dsrtm_pol_%8.8d_%c.txt -L -m ../SWBD.raw.d --D_FORMAT=%.6f
		(ls srtm_pol_*_O.txt > t.lis) 2> /dev/null
		while read file; do
			echo "> Open $file" >> ../SWBD_open.d
			cat $file >> ../SWBD_open.d
		done < t.lis
		(ls srtm_pol_*_C.txt > t.lis) 2> /dev/null
		while read file; do
			echo "> Closed $file" >> ../SWBD_closed.d
			cat $file >> ../SWBD_closed.d
		done < t.lis
		cd ..
		rm -rf pol raw SWBD.raw.d
	fi
fi
if [ $combine -eq 1 ]; then
	mkdir -p pol
	cd pol
	gmtstitch -fg -T0 -Dsrtm_pol_%8.8d_%c.txt -L -m ../SWBD_open.d --D_FORMAT=%.6f
	rm -f ../SWBD_open.d
	(ls srtm_pol_*_O.txt > t.lis) 2> /dev/null
	while read file; do
		echo "> Open $file" >> ../SWBD_open.d
		cat $file >> ../SWBD_open.d
	done < t.lis
	(ls srtm_pol_*_C.txt > t.lis) 2> /dev/null
	while read file; do
		echo "> Closed $file" >> ../SWBD_closed.d
		cat $file >> ../SWBD_closed.d
	done < t.lis
	cd ..
	rm -rf pol
	nc=`grep '^>' SWBD_closed.d | wc -l`
	no=`grep '^>' SWBD_open.d | wc -l`
	echo "$WEST/$EAST/$SOUTH/$NORTH : Closed polygons: $nc Open polygons: $no"
fi
