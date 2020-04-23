/*
 * Copyright (c) 2012-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_suppl_modules.h declares the prototypes for the supplements modules
 * and the library purpose string.  It is included by gmt_dev.h.
 */

#pragma once
#ifndef GMT_SUPPL_MODULES_H
#define GMT_SUPPL_MODULES_H

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

/* Prototypes of all modules in the GMT supplements library */
gmt_M_module_prototype (earthtide);
gmt_M_module_prototype (gpsgridder);
gmt_M_module_prototype (psvelo);
gmt_M_module_prototype (gshhg);
gmt_M_module_prototype (img2grd);
gmt_M_module_prototype (mgd77convert);
gmt_M_module_prototype (mgd77header);
gmt_M_module_prototype (mgd77info);
gmt_M_module_prototype (mgd77list);
gmt_M_module_prototype (mgd77magref);
gmt_M_module_prototype (mgd77manage);
gmt_M_module_prototype (mgd77path);
gmt_M_module_prototype (mgd77sniffer);
gmt_M_module_prototype (mgd77track);
gmt_M_module_prototype (gmtflexure);
gmt_M_module_prototype (gmtgravmag3d);
gmt_M_module_prototype (gravfft);
gmt_M_module_prototype (grdflexure);
gmt_M_module_prototype (grdgravmag3d);
gmt_M_module_prototype (grdredpol);
gmt_M_module_prototype (grdseamount);
gmt_M_module_prototype (talwani2d);
gmt_M_module_prototype (talwani3d);
gmt_M_module_prototype (pssegy);
gmt_M_module_prototype (segy2grd);
gmt_M_module_prototype (pssegyz);
gmt_M_module_prototype (pscoupe);
gmt_M_module_prototype (psmeca);
gmt_M_module_prototype (pspolar);
gmt_M_module_prototype (pssac);
gmt_M_module_prototype (backtracker);
gmt_M_module_prototype (gmtpmodeler);
gmt_M_module_prototype (grdpmodeler);
gmt_M_module_prototype (grdrotater);
gmt_M_module_prototype (grdspotter);
gmt_M_module_prototype (hotspotter);
gmt_M_module_prototype (originater);
gmt_M_module_prototype (polespotter);
gmt_M_module_prototype (rotconverter);
gmt_M_module_prototype (rotsmoother);
gmt_M_module_prototype (x2sys_binlist);
gmt_M_module_prototype (x2sys_cross);
gmt_M_module_prototype (x2sys_datalist);
gmt_M_module_prototype (x2sys_get);
gmt_M_module_prototype (x2sys_init);
gmt_M_module_prototype (x2sys_list);
gmt_M_module_prototype (x2sys_merge);
gmt_M_module_prototype (x2sys_put);
gmt_M_module_prototype (x2sys_report);
gmt_M_module_prototype (x2sys_solve);

#ifdef __cplusplus
}
#endif

#endif /* !GMT_SUPPL_MODULES_H */
