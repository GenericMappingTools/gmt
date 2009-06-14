#!/bin/sh
# Make coastline polygons from SRTM's SWBD files

DIR=/Volumes/GEOWAREHD/TMP/DATA/SWBD
extract_es=0
extract_wn=0
stitch_es=1
stitch_wn=0
#-----------------------------------------------
if [ $extract_es -eq 1 ]; then
	ls $DIR/E0??S*.ZIP > es.lis
	ls $DIR/E1??S.ZIP >> es.lis
	mkdir -p es
	while read file; do
		cp -f $file .
		name=`basename $file '.ZIP'`
		echo "$name"
		unzip $name.ZIP
		name=`echo $name | tr 'A-Z' 'a-z'`
		ogr2ogr -f "GMT" $name.d $name.shp
		mv $name.gmt es
		rm -f $name.* *.ZIP
	done < es.lis
fi
if [ $extract_wn -eq 1 ]; then
	ls $DIR/W0??N*.ZIP > wn.lis
	ls $DIR/W1??N.ZIP >> wn.lis
	mkdir -p wn
	while read file; do
		cp -f $file .
		name=`basename $file '.ZIP'`
		echo "$name"
		unzip $name.ZIP
		name=`echo $name | tr 'A-Z' 'a-z'`
		ogr2ogr -f "GMT" $name.d $name.shp
		mv $name.gmt wn
		rm -f $name.* *.ZIP
	done < wn.lis
fi
if [ $stitch_es -eq 1 ]; then
	rm -f SWBD.es.d
	while read file; do
		name=`basename $file '.ZIP' | tr 'A-Z' 'a-z'`
		grep -v '^#' es/$name.gmt >> SWBD.es.d
	done < es.lis
	mkdir -p es_pol
	cd es_pol
	gmtstitch -fg -T0 -Dsrtm_pol_%8.8d_%c.txt -L -m -Vl ../SWBD.es.d
fi
