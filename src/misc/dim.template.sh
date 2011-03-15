#!/bin/sh
#
# $Id: dim.template.sh,v 1.3 2011-03-15 02:06:37 guru Exp $
#
# Seung-Sep Kim, GG/SOEST/UHM
# $Revision: 1.3 $    $Date: 2011-03-15 02:06:37 $

# This is a template file showing the steps for DiM-based
# regional-residual separation
#
# For details, see Kim, S.-S., and Wessel, P. (2008), "Directional Median Filtering
# for Regional-Residual Separation of Bathymetry, Geochem. Geophys. Geosyst.,
# 9(Q03005), doi:10.1029/2007GC001850.

# setting up the region
# to prevent edge effects, the filtering domain had better be
# larger than the area of interest
pad=-R	# filtering domain
box=-R	# area of interest

bathy=	# input bathy file with $pad domain
dim=	# DiM-based regional
err= 	# DiM-based MAD
ors=	# Optimal Robust Separator anaysis 
	
gmtset ELLIPSOID Sphere

# ORS analysis first
# if ORS does not give you the reasonable range of filter widths,
# use the length scale of the largest feature in your domain as the
# standard to choose the filter widths
		
if [ ! -f $ors ]; then
	
	orsout=		# ors output folder
		
	mkdir -p $orsout
		
	grdcut $bathy $box -G/tmp/$$.t.nc  # the area of interest
	
	minW= 	# minimum filter width candidate for ORS  (e.g., 60)
	maxW= 	# maximum filter width candidate for ORS  (e.g., 600)
	intW= 	# filter width step (e.g., 20)
	level=  # base contour to compute the volume and area of the residual (e.g., 300m)
	STEP=`gmtmath -T$minW/$maxW/$intW -N1/0 =`
	
	for width in $STEP
	do
		echo "W = $width km"			
		dimfilter $bathy $box -G/tmp/$$.dim.nc -Fm${width} -D2 -Nl8 # DiM filter
		grdfilter /tmp/$$.dim.nc -G$orsout/dim.${width}.nc -Fm50 -D2 # smoothing

		grdmath /tmp/$$.t.nc $orsout/dim.${width}.nc SUB = /tmp/$$.sd.nc # residual from DiM
		grdvolume /tmp/$$.sd.nc -Sk -C$level -Vl | awk '{print r,$2,$3,$4}' r=${width} >> $ors  # ORS from DiM
	done
	
fi

# compute DiM-based regional

if [ ! -f $dim ]; then

	minW=  	# minimum optimal filter width (e.g., 200)
	maxW= 	# maximum optimal filter width (e.g., 240)
	intW= 	# filter width step (e.g., 5)
	width=`gmtmath -N1/0 -T$minW/$maxW/$intW =`
	alldepth= 	# for MAD analysis
	
	for i in $width
	do
		if [ ! -f $orsout/dim.${i}.nc ]; then
			echo "filtering W = ${i} km"
			dimfilter $bathy $box -G/tmp/$$.dim.nc -Fm${i} -D2 -Nl8 # DiM filter
			grdfilter /tmp/$$.dim.nc -G$orsout/dim.${i}.nc -Fm50 -D2 # smoothing
		fi
		
		if [ ! -f $alldepth ]; then
			grd2xyz -Z $orsout/dim.${i}.nc > /tmp/$$.${i}.depth
		fi
		
	done
		
	if [ ! -f $alldepth ]; then
		paste /tmp/$$.*.depth > /tmp/$$.t.depth
		# the number of columns can be different for each case
		awk '{print $1," ",$2," ",$3," ",$4," ",$5," ",$6," ",$7," ",$8," ",$9}' /tmp/$$.t.depth > $alldepth
		grd2xyz $bathy $box -V > $bathy.xyz
	fi
	
	dimfilter $alldepth -Q9 > /tmp/$$.out
	wc -l /tmp/$$.out $bathy.xyz
		
	paste $bathy.xyz /tmp/$$.out | awk '{print $1,$2,$4}' > /tmp/$$.dim.xyz
	paste $bathy.xyz /tmp/$$.out | awk '{print $1,$2,$5}' > /tmp/$$.err.xyz

	xyz2grd /tmp/$$.dim.xyz -G$dim -I1m $box -V -F
	xyz2grd /tmp/$$.err.xyz -G$err -I1m $box -V -F
		
fi
	
