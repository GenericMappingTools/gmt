#
# Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates a function for each GMT module which invalidates
# direct module call.  This is used to make sure test scripts properly
# prefix modules with "gmt", which is needed to make sure that the
# shell does not execute modules from PATH but instead uses the gmt
# executable from the build dir.
#

gmt_modules="blockmean blockmedian blockmode filter1d fitcircle 
	gmt2kml gmtconnect gmtconvert gmtdefaults gmtget 
	gmtinfo gmtlogo gmtmath gmtregress gmtselect 
	gmtset gmtsimplify gmtspatial gmtvector gmtwhich 
	grd2cpt grd2kml grd2xyz grdblend grdclip 
	grdcontour grdconvert grdcut grdedit grdfft 
	grdfill grdfilter grdgradient grdhisteq grdimage 
	grdinfo grdlandmask grdmask grdmath grdpaste 
	grdproject grdsample grdtrack grdtrend grdvector 
	grdview grdvolume greenspline inset kml2gmt 
	makecpt mapproject movie nearneighbor project 
	psbasemap psclip pscoast pscontour psconvert 
	pshistogram psimage pslegend psmask psrose 
	psscale pssolar psternary pstext pswiggle 
	psxyz psxy revert sample1d spectrum1d 
	sph2grd sphdistance sphinterpolate sphtriangulate splitxyz 
	subplot surface trend1d 
	trend2d triangulate xyz2grd gshhg img2grd 
	pscoupe psmeca pspolar pssac psvelo 
	mgd77convert mgd77header mgd77info mgd77list mgd77magref 
	mgd77manage mgd77path mgd77sniffer mgd77track dimfilter 
	grdppa earthtide gmtflexure gmtgravmag3d gpsgridder 
	gravfft grdflexure grdgravmag3d grdredpol grdseamount 
	talwani2d talwani3d pssegyz pssegy segy2grd 
	backtracker gmtpmodeler grdpmodeler grdrotater grdspotter 
	hotspotter originater polespotter rotconverter rotsmoother 
	x2sys_binlist x2sys_cross x2sys_datalist x2sys_get x2sys_init 
	x2sys_list x2sys_merge x2sys_put x2sys_report x2sys_solve"

for module in ${gmt_modules}; do
  eval "function ${module} () { false; }"
  eval "export -f ${module}"
done
