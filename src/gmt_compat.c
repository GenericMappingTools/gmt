/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * Provides compatibility with module names no longer used.
 *
 * Author:	Paul Wessel
 * Date:	24-JUN-2013
 * Version:	5.x
 */

#include "gmt_dev.h"

EXTERN_MSC int GMT_gmtdp (void *V_API, int mode, void *args);
EXTERN_MSC int GMT_grdreformat (void *V_API, int mode, void *args);
EXTERN_MSC int GMT_minmax (void *V_API, int mode, void *args);
EXTERN_MSC int GMT_gmtstitch (void *V_API, int mode, void *args);
EXTERN_MSC int GMT_gmt2rgb (void *V_API, int mode, void *args);
EXTERN_MSC int GMT_ps2raster (void *V_API, int mode, void *args);

int GMT_gmtdp (void *V_API, int mode, void *args)
{	/* This was the GMT4 name */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (GMT_compat_check (API->GMT, 4)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Warning: module gmtdp is deprecated; use gmtsimplify.\n");
		return (GMT_Call_Module (API, "gmtsimplify", mode, args));
	}
	GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: gmtdp\n");
	return (GMT_NOT_A_VALID_MODULE);
}

int GMT_grdreformat (void *V_API, int mode, void *args)
{	/* This was the GMT5.1 name */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (GMT_compat_check (API->GMT, 5)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Warning: module grdreformat is deprecated; use grdconvert.\n");
		return (GMT_Call_Module (API, "grdconvert", mode, args));
	}
	GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: grdreformat\n");
	return (GMT_NOT_A_VALID_MODULE);
}

int GMT_minmax (void *V_API, int mode, void *args)
{	/* This was the GMT4 name */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (GMT_compat_check (API->GMT, 4)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Warning: module minmax is deprecated; use gmtinfo.\n");
		return (GMT_Call_Module (API, "gmtinfo", mode, args));
	}
	GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: minmax\n");
	return (GMT_NOT_A_VALID_MODULE);
}

int GMT_gmtstitch (void *V_API, int mode, void *args)
{	/* This was the GMT4 name */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (GMT_compat_check (API->GMT, 4)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Warning: module gmtstitch is deprecated; use gmtconnect.\n");
		return (GMT_Call_Module (API, "gmtconnect", mode, args));
	}
	GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: gmtstitch\n");
	return (GMT_NOT_A_VALID_MODULE);
}

int GMT_gmt2rgb (void *V_API, int mode, void *args)
{	/* This was the GMT4 name */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (GMT_compat_check (API->GMT, 4)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Warning: module gmt2rgb is deprecated; use grd2rgb.\n");
		return (GMT_Call_Module (API, "grd2rgb", mode, args));
	}
	GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: gmt2rgb\n");
	return (GMT_NOT_A_VALID_MODULE);
}

int GMT_ps2raster (void *V_API, int mode, void *args)
{	/* This was the GMT5.1 name */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (GMT_compat_check (API->GMT, 5)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Warning: module ps2raster is deprecated; use psconvert.\n");
		return (GMT_Call_Module (API, "psconvert", mode, args));
	}
	GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: ps2raster\n");
	return (GMT_NOT_A_VALID_MODULE);
}
