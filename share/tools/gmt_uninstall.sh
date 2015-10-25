# $Id$
#
# Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script removes the entire GMT installation. If the bin, share
# include, and lib dirs become empty after removing the GMT files then
# we also remove those parent directories since presumably under build dir.
#
# Run this script on the command line with:
#   $(gmt --show-datadir)/tools/gmt_uninstall.sh
#
# It expects the GMT executable to be in the search path and that
# you have permission to perform the changes in the bin directory.

# check for bash
[ -z "$BASH_VERSION" ] && return

inc=`gmt-config --includedir`
share=`gmt --show-sharedir`
bin=`gmt --show-bindir`
lib=`gmt --show-plugindir`

cwd=`pwd`

gmt_modules="backtracker blockmean blockmedian blockmode dimfilter filter1d \
fitcircle gmt2kml gmtconnect gmtconvert gmtdefaults gmtflexure gmtget \
gmtgravmag3d gmtinfo gmtlogo gmtmath gmtregress \
gmtselect gmtset gmtsimplify gmtspatial gmtvector gmtwhich gravfft \
grd2cpt grd2rgb grd2xyz grdblend grdclip grdcontour grdcut grdedit grdfft \
grdfilter grdflexure grdgradient grdgravmag3d grdhisteq grdimage grdinfo grdlandmask \
grdmask grdmath grdpaste grdpmodeler grdproject grdraster grdredpol grdconvert \
grdrotater grdsample grdseamount grdspotter grdtrack grdtrend grdvector \
grdview grdvolume greenspline gshhg hotspotter img2grd kml2gmt makecpt \
mapproject mgd77convert mgd77info mgd77list mgd77magref mgd77manage mgd77path \
mgd77sniffer mgd77track nearneighbor originator project psconvert \
psbasemap psclip pscoast pscontour pscoupe pshistogram psimage pslegend \
psmask psmeca pspolar psrose psscale pssegy pssegyz pstext psvelo pswiggle \
psxy psxyz rotconverter sample1d segy2grd spectrum1d sph2grd sphdistance \
sphinterpolate sphtriangulate splitxyz surface talwani2d talwani3d trend1d trend2d triangulate \
x2sys_binlist x2sys_cross x2sys_datalist x2sys_get x2sys_init x2sys_list \
x2sys_merge x2sys_put x2sys_report x2sys_solve xyz2grd \
minmax gmt2rgb gmtstitch gmtdp grdreformat ps2raster"

# 2. Remove include directory
cd $inc
cd ..
here=`pwd`
printf "Remove: %s\n" $inc
rm -rf $inc
if find "$here" -mindepth 1 -print -quit | grep -q .; then
	# include not empty, leave as is
	printf "Ignore: %s\n" $here
else
	printf "Remove: %s\n" $here
	rm -rf $here
fi

# 3. Remove share directory
for dir in conf cpt custom dbase doc localization man mgd77 mgg postscriptlight tools x2sys; do
	printf "Remove: %s/%s\n" $share $dir
	rm -rf $share/$dir
done
rm -f $share/VERSION
if find "$share" -mindepth 1 -print -quit | grep -q .; then
	# share not empty, leave as is
	printf "Ignore: %s\n" $share
else
	printf "Remove: %s\n" $share
	rm -rf $share
fi

# 4. Remove executables [and any links to them]
cd $bin
for module in ${gmt_modules}; do
	if [ -h $module ]; then
		printf "Remove: Link %14s -> gmt\n" $module
		rm -f $module
	fi
done
# Remove main executable
printf "Remove: gmt\n"
rm -f gmt
# Remove scripts
printf "Remove: gmt scripts\n"
rm -rf gmt-config gmt_shell_functions.sh gmtswitch isogmt gmt.dSYM
if find "$bin" -mindepth 1 -print -quit | grep -q .; then
	# bin not empty, leave as is
	printf "Ignore: %s\n" $bin
else
	printf "Remove: %s\n" $bin
	rm -rf $bin
fi

# 5. Lastly remove libs and plugin directory
cd $lib
cd ../..
here=`pwd`
if [ -d gmt ]; then	# plugin directory inside a gmt directory; delete gmt instead
	printf "Remove: %s\n" $here
	rm -rf gmt
else			# Just delete plugin directory
	printf "Remove: %s\n" $lib
	rm -rf $lib
fi
# Check for GMT libs here	
printf "Remove: libgmt.*\n"
printf "Remove: libpostscriptlight.*\n"
rm -rf libgmt.* libpostscriptlight.*
if find "$here" -mindepth 1 -print -quit | grep -q .; then
	# lib dir not empty, leave as is
	printf "Ignore: %s\n" $here
else
	printf "Remove: %s\n" $here
	rm -rf $here
fi

# Back to where we were
cd $cwd
