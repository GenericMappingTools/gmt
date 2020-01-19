/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------
 *
 * Author:	Paul Wessel
 * Date:	23-JUN-2017
 * Version:	6
 */

/*!
 * \file gmt_modern.h
 * \brief Definitions of constants used through GMT for modern mode.
 */

#ifndef GMT_MODERN_H
#define GMT_MODERN_H

#define GMT_HISTORY_FILE	"gmt.history"
#define GMT_SESSION_FILE	"gmt.session"

/* Session settings for default plot file prefix and format (extension) */

#define GMT_SESSION_NAME	"gmtsession"	/* Default prefix for single-figure modern sessions */
#define GMT_SESSION_FORMAT	0		/* Default entry into gmt_session_format|code arrays -> PDF */
#define GMT_SESSION_CONVERT	"A"		/* Default psconvert options in gmt figure */

/* Declarations of modern mode module names.  These functions will
 * call the classically-named modules (e.g., plot will call psxy) but
 * only if modern mode is in effect. */

/* Modern names: core */

EXTERN_MSC int GMT_basemap (void *API, int mode, void *args);
EXTERN_MSC int GMT_clip (void *API, int mode, void *args);
EXTERN_MSC int GMT_coast (void *API, int mode, void *args);
EXTERN_MSC int GMT_contour (void *API, int mode, void *args);
EXTERN_MSC int GMT_events (void *API, int mode, void *args);
EXTERN_MSC int GMT_histogram (void *API, int mode, void *args);
EXTERN_MSC int GMT_image (void *API, int mode, void *args);
EXTERN_MSC int GMT_legend (void *API, int mode, void *args);
EXTERN_MSC int GMT_mask (void *API, int mode, void *args);
EXTERN_MSC int GMT_rose (void *API, int mode, void *args);
EXTERN_MSC int GMT_colorbar (void *API, int mode, void *args);
EXTERN_MSC int GMT_solar (void *API, int mode, void *args);
EXTERN_MSC int GMT_ternary (void *API, int mode, void *args);
EXTERN_MSC int GMT_text (void *API, int mode, void *args);
EXTERN_MSC int GMT_wiggle (void *API, int mode, void *args);
EXTERN_MSC int GMT_plot (void *API, int mode, void *args);
EXTERN_MSC int GMT_plot3d (void *API, int mode, void *args);

/* Modern names: supplements */

EXTERN_MSC int GMT_coupe (void *API, int mode, void *args);
EXTERN_MSC int GMT_meca (void *API, int mode, void *args);
EXTERN_MSC int GMT_polar (void *API, int mode, void *args);
EXTERN_MSC int GMT_sac (void *API, int mode, void *args);
EXTERN_MSC int GMT_velo (void *API, int mode, void *args);
EXTERN_MSC int GMT_segyz (void *API, int mode, void *args);
EXTERN_MSC int GMT_segy (void *API, int mode, void *args);

EXTERN_MSC const char *gmt_current_name (const char *module, char modname[]);
EXTERN_MSC const char *gmt_get_full_name (struct GMTAPI_CTRL *API, const char *module);

#endif  /* GMT_MODERN_H */
