# $Id$
#
# Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates a function for each GMT module which calls the module
# via the main GMT executable (gmt <module>).
#
# Include this file in your GMT bash script or on the command line with:
#   source $(gmt --show-datadir)/tools/gmt_functions.sh
# If the GMT executable is not in the search path, set an extra function:
#   function gmt () { path/to/gmt "$@"; }
#   export -f gmt

gmt_modules="backtracker blockmean blockmedian blockmode dimfilter filter1d \
fitcircle gmt2kml gmtconvert gmtdefaults gmtget gmtgravmag3d gmtinfo gmtmath \
gmtselect gmtset gmtsimplify gmtspatial gmtstitch gmtvector gmtwhich gravfft \
grd2cpt grd2rgb grd2xyz grdblend grdclip grdcontour grdcut grdedit grdfft \
grdfilter grdgradient grdgravmag3d grdhisteq grdimage grdinfo grdlandmask \
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

for module in ${gmt_modules}; do
  eval "function ${module} () { gmt ${module} \"\$@\"; }"
  eval "export -f ${module}"
done
