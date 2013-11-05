# $Id$
#
# Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates an alias for each GMT module which prepends the module
# name with "gmt". For example: alias psxy "gmt psxy"
#
# Include this file in your GMT (t)csh script or on the command line with:
#   source path/to/gmt/share/tools/gmt_aliases.sh
# If the GMT executable is not in the search path, set an extra alias:
#   alias gmt path/to/gmt

set gmt_modules = (backtracker blockmean blockmedian blockmode dimfilter \
filter1d fitcircle gmt2kml gmtconvert gmtdefaults gmtget gmtgravmag3d gmtinfo \
gmtmath gmtselect gmtset gmtsimplify gmtspatial gmtstitch gmtvector gmtwhich \
gravfft grd2cpt grd2rgb grd2xyz grdblend grdclip grdcontour grdcut grdedit \
grdfft grdfilter grdgradient grdgravmag3d grdhisteq grdimage grdinfo \
grdlandmask grdmask grdmath grdpaste grdpmodeler grdproject grdraster \
grdredpol grdreformat grdrotater grdsample grdseamount grdspotter grdtrack \
grdtrend grdvector grdview grdvolume greenspline gshhg hotspotter img2grd \
kml2gmt makecpt mapproject mgd77convert mgd77info mgd77list mgd77magref \
mgd77manage mgd77path mgd77sniffer mgd77track minmax nearneighbor originator \
project ps2raster psbasemap psclip pscoast pscontour pscoupe pshistogram \
psimage pslegend psmask psmeca pspolar psrose psscale pssegy pssegyz pstext \
psvelo pswiggle psxy psxyz rotconverter sample1d segy2grd spectrum1d sph2grd \
sphdistance sphinterpolate sphtriangulate splitxyz surface trend1d trend2d \
triangulate x2sys_binlist x2sys_cross x2sys_datalist x2sys_get x2sys_init \
x2sys_list x2sys_merge x2sys_put x2sys_report x2sys_solve xyz2grd)

foreach module ( $gmt_modules )
  eval 'alias $module "gmt $module"'
end
