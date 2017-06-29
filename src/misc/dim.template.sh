#!/bin/bash
#
# $Id$
#
# Seung-Sep Kim, Chungnam National University, Daejeon, South Korea [seungsep@cnu.kr]
# $Revision$    $Date$

# This is a template script showing the steps for DiM-based
# regional-residual separation.
#
# For details, see Kim, S.-S., and Wessel, P. (2008), "Directional Median Filtering
# for Regional-Residual Separation of Bathymetry, Geochem. Geophys. Geosyst.,
# 9(Q03005), doi:10.1029/2007GC001850.

# 0. System defaults (Change if you know what you are doing):
dim_dist=2		# How we compute distances on the grid [Flat Earth approximation]
dim_sectors=8		# Number of sectors to use [8]
dim_filter=m		# Primary filter [m for median]
dim_quantity=l		# Secondary filter [l for lower]
dim_smooth_type=m	# Smoothing filter type [m for median]
dim_smooth_width=50	# Smoothing filter width, in km [50]

# 1. Setting up the region:
#    To prevent edge effects, the input grid domain must be
#    larger than the area of interest.  Make sure the input
#    grid exceeds the area of interest by > 1/2 max filter width.
box=-R	# Area of interest, a subset of data domain

# 2. Specify names for input and output files
bathy=	# Input bathymetry grid file for the entire data domain
ors=	# Intermediate Optimal Robust Separator analysis results (table)
orsout=	# ORS output work folder
dim=	# Final output DiM-based regional grid
err= 	# Final output DiM-based MAD uncertainty grid

gmt set ELLIPSOID Sphere

# A) ORS analysis first
# if ORS does not give you the reasonable range of filter widths,
# use the length scale of the largest feature in your domain as the
# standard to choose the filter widths

if [ ! -f $ors ]; then

	mkdir -p $orsout

	gmt grdcut $bathy $box -G/tmp/$$.t.nc  # the area of interest

	# A.1. Set filter parameters for an equidistant set of filters:
	minW= 	# Minimum filter width candidate for ORS  (e.g., 60) in km
	maxW= 	# Maximum filter width candidate for ORS  (e.g., 600) in km
	intW= 	# Filter width step (e.g., 20) in km
	level=  # Base contour used to compute the volume and area of the residual (e.g., 300m)
	#------stop A.1. editing here--------------------------------------

	STEP=`gmt math -T$minW/$maxW/$intW -N1/0 =`

	for width in $STEP
	do
		echo "W = $width km"
		gmt dimfilter $bathy $box -G/tmp/$$.dim.nc -F${dim_filter}${width} -D${dim_dist} -N${dim_quantity}${dim_sectors} # DiM filter
		gmt grdfilter /tmp/$$.dim.nc -G$orsout/dim.${width}.nc -F${dim_smooth_type}${dim_smooth_width} -D${dim_dist} # smoothing

		gmt grdmath /tmp/$$.t.nc $orsout/dim.${width}.nc SUB = /tmp/$$.sd.nc # residual from DiM
		gmt grdvolume /tmp/$$.sd.nc -Sk -C$level -Vl | awk '{print r,$2,$3,$4}' r=${width} >> $ors  # ORS from DiM
	done

fi

# B) Compute DiM-based regional

if [ ! -f $dim ]; then

	# B.1. Set filter parameters for an equidistant set of filters:
	minW=  	# Minimum optimal filter width (e.g., 200) in km
	maxW= 	# Maximum optimal filter width (e.g., 240) in km
	intW= 	# Filter width step (e.g., 5) in km
	alldepth= 	# for MAD analysis
	#------stop B.1. editing here--------------------------------------
	width=`gmt math -N1/0 -T$minW/$maxW/$intW =`
	let n_widths=0
	for i in $width
	do
		if [ ! -f $orsout/dim.${i}.nc ]; then
			echo "filtering W = ${i} km"
			gmt dimfilter $bathy $box -G/tmp/$$.dim.nc -F${dim_filter}${i} -D${dim_dist} -N${dim_quantity}${dim_sectors}	# DiM filter
			gmt grdfilter /tmp/$$.dim.nc -G$orsout/dim.${i}.nc -F${dim_smooth_type}${dim_smooth_width} -D${dim_dist} 	# smoothing
		fi

		if [ ! -f $alldepth ]; then
			gmt grd2xyz -Z $orsout/dim.${i}.nc > /tmp/$$.${i}.depth
		fi
		let n_widths=n_widths+1
	done

	if [ ! -f $alldepth ]; then
		paste /tmp/$$.*.depth > /tmp/$$.t.depth
		# the number of columns can be different for each case
		awk '{print $1," ",$2," ",$3," ",$4," ",$5," ",$6," ",$7," ",$8," ",$9}' /tmp/$$.t.depth > $alldepth
		awk '{for (k = 1; k <= '"$n_widths"', k++) print $1," ",$2," ",$3," ",$4," ",$5," ",$6," ",$7," ",$8," ",$9}' /tmp/$$.t.depth > $alldepth
		gmt grd2xyz $bathy $box -V > $bathy.xyz
	fi

	gmt dimfilter $alldepth -Q${n_widths} > /tmp/$$.out
	wc -l /tmp/$$.out $bathy.xyz

	paste $bathy.xyz /tmp/$$.out | awk '{print $1,$2,$4}' > /tmp/$$.dim.xyz
	paste $bathy.xyz /tmp/$$.out | awk '{print $1,$2,$5}' > /tmp/$$.err.xyz

	gmt xyz2grd /tmp/$$.dim.xyz -G$dim -I1m $box -V -r
	gmt xyz2grd /tmp/$$.err.xyz -G$err -I1m $box -V -r

fi
