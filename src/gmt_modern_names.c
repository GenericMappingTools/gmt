/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Enables new module names without the leading "ps" but only when used 
 * in modern mode.  This was done because since in modern mode the default
 * output format is no longer PostScript but PFD and furthermore can be
 * set to any other format (e.g., PNG) as well or in addition to others.
 *
 * Author:	Paul Wessel
 * Date:	03-NOV-2017
 * Version:	6.x
 */

#include "gmt_dev.h"


int GMT_basemap (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: basemap\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psbasemap", mode, args));
}

int GMT_clip (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: clip\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psclip", mode, args));
}

int GMT_coast (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: coast\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pscoast", mode, args));
}

int GMT_contour (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: contour\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pscontour", mode, args));
}

int GMT_histogram (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: histogram\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pshistogram", mode, args));
}

int GMT_image (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: image\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psimage", mode, args));
}

int GMT_legend (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: legend\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pslegend", mode, args));
}

int GMT_mask (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: mask\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psmask", mode, args));
}

int GMT_rose (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: rose\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psrose", mode, args));
}

int GMT_scale (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: scale\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psscale", mode, args));
}

int GMT_solar (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: solar\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pssolar", mode, args));
}

int GMT_ternary (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: ternary\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psternary", mode, args));
}

int GMT_text (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: text\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pstext", mode, args));
}

int GMT_wiggle (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: wiggle\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pswiggle", mode, args));
}

int GMT_plot (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: plot\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psxy", mode, args));
}

int GMT_plot3d (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: plot3d\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psxyz", mode, args));
}

int GMT_coupe (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: coupe\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pscoupe", mode, args));
}

int GMT_meca (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: meca\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psmeca", mode, args));
}

int GMT_polar (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: polar\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pspolar", mode, args));
}

int GMT_sac (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: sac\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pssac", mode, args));
}

int GMT_velo (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: velo\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "psvelo", mode, args));
}

int GMT_segy (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: segy\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pssegy", mode, args));
}

int GMT_segyz (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: segyz\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return (GMT_Call_Module (API, "pssegyz", mode, args));
}
