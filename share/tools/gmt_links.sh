# $Id$
#
# Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates, removes, or lists symbolic links for each GMT
# module to the main GMT executable that allow users to run modules
# directly.
#
# Run this script on the command line with:
#   $(gmt --show-datadir)/tools/gmt_links.sh create|remove
#
# With no arguments we simply check for the links.
#
# It expects the GMT executable to be in the search path and that
# you have permission to perform the changes in the bin directory.

# check for bash
[ -z "$BASH_VERSION" ] && return

if [ "X$1" = "Xdelete" ]; then
	mode=1
elif [ "X$1" = "Xcreate" ]; then
	mode=2
else
	mode=0
fi
bin=`gmt --show-bindir`
cwd=`pwd`

gmt_modules="backtracker blockmean blockmedian blockmode dimfilter filter1d \
fitcircle gmt2kml gmtconnect gmtconvert gmtdefaults gmtflexure gmtget \
gmtgravmag3d gmtinfo gmtlogo gmtmath \
gmtselect gmtset gmtsimplify gmtspatial gmtstitch gmtvector gmtwhich gravfft \
grd2cpt grd2rgb grd2xyz grdblend grdclip grdcontour grdcut grdedit grdfft \
grdfilter grdflexure grdgradient grdgravmag3d grdhisteq grdimage grdinfo grdlandmask \
grdmask grdmath grdpaste grdpmodeler grdproject grdraster grdredpol grdconvert \
grdrotater grdsample grdseamount grdspotter grdtrack grdtrend grdvector \
grdview grdvolume greenspline gshhg hotspotter img2grd kml2gmt makecpt \
mapproject mgd77convert mgd77info mgd77list mgd77magref mgd77manage mgd77path \
mgd77sniffer mgd77track minmax nearneighbor originator project psconvert \
psbasemap psclip pscoast pscontour pscoupe pshistogram psimage pslegend \
psmask psmeca pspolar psrose psscale pssegy pssegyz pstext psvelo pswiggle \
psxy psxyz rotconverter sample1d segy2grd spectrum1d sph2grd sphdistance \
sphinterpolate sphtriangulate splitxyz surface trend1d trend2d triangulate \
x2sys_binlist x2sys_cross x2sys_datalist x2sys_get x2sys_init x2sys_list \
x2sys_merge x2sys_put x2sys_report x2sys_solve xyz2grd"

cd $bin
for module in ${gmt_modules}; do
	if [ $mode -eq 1 ]; then	# Delete links
		echo "rm -f $module"
	elif [ $mode -eq 2 ]; then	# Create new links (remove old if present)
		echo "ls -sf gmt $module"
	else				# List what we find	
		if [ -h $module ]; then
			printf "Link for module %16s: %s\n" $module "Present"
		else
			printf "Link for module %16s: %s\n" $module "Absent"
		fi
	fi
done
cd $cwd
