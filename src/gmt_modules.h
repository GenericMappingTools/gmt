/*
 * Copyright (c) 2012-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_modules.h declares the prototypes for the core modules
 * and the library purpose string.  It is included by gmt_dev.h.
 */

#pragma once
#ifndef GMT_MODULES_H
#define GMT_MODULES_H

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

#define GMT_CORE_STRING "GMT core: The main modules of the Generic Mapping Tools"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

/* Prototypes of all modules in the GMT core library */
gmt_M_module_prototype (psbasemap);
gmt_M_module_prototype (begin);
gmt_M_module_prototype (blockmean);
gmt_M_module_prototype (blockmedian);
gmt_M_module_prototype (blockmode);
gmt_M_module_prototype (clear);
gmt_M_module_prototype (psclip);
gmt_M_module_prototype (pscoast);
gmt_M_module_prototype (psscale);
gmt_M_module_prototype (pscontour);
gmt_M_module_prototype (dimfilter);
gmt_M_module_prototype (docs);
gmt_M_module_prototype (end);
gmt_M_module_prototype (psevents);
gmt_M_module_prototype (figure);
gmt_M_module_prototype (filter1d);
gmt_M_module_prototype (fitcircle);
gmt_M_module_prototype (gmt2kml);
gmt_M_module_prototype (gmtconnect);
gmt_M_module_prototype (gmtconvert);
gmt_M_module_prototype (gmtdefaults);
gmt_M_module_prototype (gmtget);
gmt_M_module_prototype (gmtinfo);
gmt_M_module_prototype (gmtlogo);
gmt_M_module_prototype (gmtmath);
gmt_M_module_prototype (gmtread);
gmt_M_module_prototype (gmtregress);
gmt_M_module_prototype (gmtselect);
gmt_M_module_prototype (gmtset);
gmt_M_module_prototype (gmtsimplify);
gmt_M_module_prototype (gmtspatial);
gmt_M_module_prototype (gmtvector);
gmt_M_module_prototype (gmtwhich);
gmt_M_module_prototype (gmtwrite);
gmt_M_module_prototype (grd2cpt);
gmt_M_module_prototype (grd2kml);
gmt_M_module_prototype (grd2xyz);
gmt_M_module_prototype (grdblend);
gmt_M_module_prototype (grdclip);
gmt_M_module_prototype (grdcontour);
gmt_M_module_prototype (grdconvert);
gmt_M_module_prototype (grdcut);
gmt_M_module_prototype (grdedit);
gmt_M_module_prototype (grdfft);
gmt_M_module_prototype (grdfill);
gmt_M_module_prototype (grdfilter);
gmt_M_module_prototype (grdgdal);
gmt_M_module_prototype (grdgradient);
gmt_M_module_prototype (grdhisteq);
gmt_M_module_prototype (grdimage);
gmt_M_module_prototype (grdinfo);
gmt_M_module_prototype (grdinterpolate);
gmt_M_module_prototype (grdlandmask);
gmt_M_module_prototype (grdmask);
gmt_M_module_prototype (grdmath);
gmt_M_module_prototype (grdpaste);
gmt_M_module_prototype (grdproject);
gmt_M_module_prototype (grdsample);
gmt_M_module_prototype (grdtrack);
gmt_M_module_prototype (grdtrend);
gmt_M_module_prototype (grdvector);
gmt_M_module_prototype (grdview);
gmt_M_module_prototype (grdvolume);
gmt_M_module_prototype (greenspline);
gmt_M_module_prototype (pshistogram);
gmt_M_module_prototype (psimage);
gmt_M_module_prototype (inset);
gmt_M_module_prototype (kml2gmt);
gmt_M_module_prototype (pslegend);
gmt_M_module_prototype (makecpt);
gmt_M_module_prototype (mapproject);
gmt_M_module_prototype (psmask);
gmt_M_module_prototype (movie);
gmt_M_module_prototype (nearneighbor);
gmt_M_module_prototype (psxy);
gmt_M_module_prototype (psxyz);
gmt_M_module_prototype (project);
gmt_M_module_prototype (psconvert);
gmt_M_module_prototype (psrose);
gmt_M_module_prototype (sample1d);
gmt_M_module_prototype (pssolar);
gmt_M_module_prototype (spectrum1d);
gmt_M_module_prototype (sph2grd);
gmt_M_module_prototype (sphdistance);
gmt_M_module_prototype (sphinterpolate);
gmt_M_module_prototype (sphtriangulate);
gmt_M_module_prototype (splitxyz);
gmt_M_module_prototype (subplot);
gmt_M_module_prototype (surface);
gmt_M_module_prototype (psternary);
gmt_M_module_prototype (pstext);
gmt_M_module_prototype (trend1d);
gmt_M_module_prototype (trend2d);
gmt_M_module_prototype (triangulate);
gmt_M_module_prototype (pswiggle);
gmt_M_module_prototype (xyz2grd);

#ifdef __cplusplus
}
#endif

#endif /* !GMT_MODULES_H */
