/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2026 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *--------------------------------------------------------------------*/
/*
 * gmtlib_get_history_value: Shared lookup used to fetch the value of a
 * "history" parameter, i.e., either:
 *   - a plain gmt.history key (R, J, B, @L, @G, @S, ...), or
 *   - the special keys w/h, which are NOT stored in gmt.history but are
 *     the bounding-box width/height (in the session's PROJ_LENGTH_UNIT)
 *     of the last object plotted in the current modern-mode figure, as
 *     used by -Xw... and -Yh... (see gmtinit_set_last_dimensions and
 *     gmtinit_get_last_dimensions in gmt_init.c, which persist these to
 *     "<gwf_dir>/gmt.canvas.<fig>").
 *
 * This is factored out so both the gmtget module (-H option) and the
 * public GMT_Get_History API function share one implementation instead
 * of duplicating the file-parsing logic.
 *
 * Author: Federico (esteban82), with Claude
 */

#include "gmt_dev.h"

EXTERN_MSC void gmtlib_get_graphics_item (struct GMTAPI_CTRL *API, int *fig, int *subplot, char *panel, int *inset);

GMT_LOCAL void gmtlib_history_path (struct GMTAPI_CTRL *API, char *path) {
	/* Locate the gmt.history in effect: workflow dir in modern mode, else cwd */
	struct GMT_CTRL *GMT = API->GMT;
	if (GMT->current.setting.run_mode == GMT_MODERN && API->gwf_dir && API->gwf_dir[0])
		snprintf (path, PATH_MAX, "%s/gmt.history", API->gwf_dir);
	else
		snprintf (path, PATH_MAX, "gmt.history");
}

GMT_LOCAL bool gmtlib_is_wh_key (const char *key) {
	return (!strcmp (key, "w") || !strcmp (key, "h"));
}

GMT_LOCAL bool gmtlib_get_wh (struct GMTAPI_CTRL *API, double *width, double *height) {
	/* Read the last plotted object's canvas dimensions (in inches) for the current figure, if any */
	char file[PATH_MAX] = {""}, panel[GMT_LEN64] = {""};
	int fig = 0, subplot = 0, inset = 0, dim = 0;
	FILE *fp = NULL;

	if (API->gwf_dir == NULL || API->gwf_dir[0] == '\0') return false;	/* Not modern mode / no session */

	gmtlib_get_graphics_item (API, &fig, &subplot, panel, &inset);	/* Current figure number */
	snprintf (file, PATH_MAX, "%s/gmt.canvas.%d", API->gwf_dir, fig);
	if (access (file, R_OK)) return false;		/* Nothing plotted yet in this figure */
	if ((fp = fopen (file, "r")) == NULL) return false;
	if (fscanf (fp, "%lg %lg %d", width, height, &dim) != 3) {
		fclose (fp);
		return false;
	}
	fclose (fp);
	return true;
}

/* gmtlib_get_history_value: Look up a single history key (or w/h) and write
 * its string value into value (caller-supplied buffer, at least GMT_LEN256
 * bytes). Returns GMT_NOERROR on success, or a GMT error code if the key
 * could not be resolved (file missing, key not found, no prior plot, etc.).
 * Emits a GMT_Report warning/error describing the failure. */
int gmtlib_get_history_value (struct GMTAPI_CTRL *API, const char *key, char *value) {
	struct GMT_CTRL *GMT = API->GMT;
	FILE *fp = NULL;
	char path[PATH_MAX] = {""}, line[GMT_BUFSIZ] = {""};
	bool found = false;

	if (key == NULL || key[0] == '\0' || value == NULL) return GMT_ARG_IS_NULL;

	if (gmtlib_is_wh_key (key)) {	/* Special case: previous plot bounding box width/height */
		double width = 0.0, height = 0.0, factor, val;
		if (!gmtlib_get_wh (API, &width, &height)) {
			GMT_Report (API, GMT_MSG_WARNING, "gmtlib_get_history_value: No previous plot found in this figure to get %s from\n", key);
			return GMT_RUNTIME_ERROR;
		}
		/* Internally GMT always stores lengths in inches; convert to the
		 * session's PROJ_LENGTH_UNIT (default "c") for a human/script-friendly value */
		factor = GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
		val = ((key[0] == 'w') ? width : height) * factor;
		snprintf (value, GMT_LEN256, "%.16g", val);
		return GMT_NOERROR;
	}

	/* Regular key: look it up in gmt.history */
	gmtlib_history_path (API, path);
	if ((fp = fopen (path, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_history_value: Unable to open history file %s\n", path);
		return GMT_ERROR_ON_FOPEN;
	}
	while (fgets (line, GMT_BUFSIZ, fp) != NULL) {
		char *p = line, *tab = NULL;
		size_t len = strlen (p);
		while (len && (p[len-1] == '\n' || p[len-1] == '\r')) p[--len] = '\0';
		if (len == 0 || p[0] == '#') continue;				/* Skip blank lines and comments */
		if (!strncmp (p, "BEGIN", 5U) || !strcmp (p, "END")) continue;	/* Skip session brackets */
		if ((tab = strchr (p, '\t')) == NULL) continue;		/* Malformed line - skip */
		*tab = '\0';
		if (!strcmp (p, key)) {
			strncpy (value, tab + 1, GMT_LEN256 - 1);
			value[GMT_LEN256-1] = '\0';
			found = true;
			break;
		}
	}
	fclose (fp);

	if (!found) {
		GMT_Report (API, GMT_MSG_WARNING, "gmtlib_get_history_value: No history found for parameter %s in %s\n", key, path);
		return GMT_NOT_A_VALID_PARAMETER;
	}
	return GMT_NOERROR;
}

/* --------------------------------------------------------------------
 * GMT_Get_History: Public API wrapper, mirroring GMT_Get_Default's
 * structure exactly (same macros, same error-handling pattern) so that
 * external wrappers (PyGMT, GMT.jl, GMT/MATLAB, etc.) can fetch a
 * gmt.history value (or w/h) directly via FFI, without spawning a
 * `gmt get -H` subprocess and parsing its stdout.
 * -------------------------------------------------------------------- */

EXTERN_MSC void gmtlib_report_error (void *V_API, int error);
EXTERN_MSC struct GMTAPI_CTRL * gmt_get_api_ptr (struct GMTAPI_CTRL *ptr);

#define gmthist_return_error(API,err) { gmtlib_report_error (API, err); return (err); }

int GMT_Get_History (void *V_API, const char *keyword, char *value) {
	/* Given a history "key" (a plain gmt.history entry such as R, J, B, @L, ...,
	 * or the special keys w/h used by -Xw.../-Yh...), return its value as a
	 * string in the caller-supplied buffer (at least GMT_LEN256 bytes). */
	int error = GMT_NOERROR;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) gmthist_return_error (V_API, GMT_NOT_A_SESSION);
	if (keyword == NULL) gmthist_return_error (V_API, GMT_NO_PARAMETERS);
	if (value == NULL) gmthist_return_error (V_API, GMT_NO_PARAMETERS);
	API = gmt_get_api_ptr (V_API);
	error = gmtlib_get_history_value (API, keyword, value);
	gmthist_return_error (V_API, error);
}
