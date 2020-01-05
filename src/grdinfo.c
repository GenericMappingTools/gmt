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
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: grdinfo reads one or more grid file and [optionally] prints
 * out various statistics like mean/standard deviation and median/scale.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdinfo"
#define THIS_MODULE_MODERN_NAME	"grdinfo"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Extract information from grids"
#define THIS_MODULE_KEYS	"<G{+,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->RVfho"

/* Control structure for grdinfo */

enum Opt_I_modes {
	GRDINFO_GIVE_INCREMENTS = 0,
	GRDINFO_GIVE_REG_ORIG,
	GRDINFO_GIVE_REG_IMG,
	GRDINFO_GIVE_REG_ROUNDED,
	GRDINFO_GIVE_BOUNDBOX};

enum Opt_C_modes {
	GRDINFO_TRADITIONAL	= 0,
	GRDINFO_NUMERICAL	= 1,
	GRDINFO_TRAILING	= 2};

struct GRDINFO_CTRL {
	struct GRDINFO_C {	/* -C[n|t] */
		bool active;
		unsigned int mode;
	} C;
	struct GRDINFO_D {	/* -D[dx[/dy]][+i] */
		bool active;
		unsigned int mode;
		double inc[2];
	} D;
	struct GRDINFO_F {	/* -F */
		bool active;
	} F;
	struct GRDINFO_I {	/* -Ir|b|i|dx[/dy] */
		bool active;
		unsigned int status;
		double inc[2];
	} I;
	struct GRDINFO_M {	/* -M */
		bool active;
	} M;
	struct GRDINFO_L {	/* -L[0|1|2|p] */
		bool active;
		unsigned int norm;
	} L;
	struct GRDINFO_T {	/* -T[s]<dz>  -T[<dz>][+s][+a[<alpha>]] */
		bool active;
		unsigned int mode;
		double inc;
		double alpha;
	} T;
	struct GRDINFO_G {	/*  */
		bool active;
		char *opts;
	} G;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDINFO_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDINFO_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->T.alpha = 2.0;	/* 2 % alpha trim is default if selected */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDINFO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid> [-C[n|t]] [-D[<offx>[/<offy>]][+i]] [-F] [-I[<dx>[/<dy>]|b|i|r]] [-L[a|0|1|2|p]] [-M]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "	[%s] [-T[<dz>][+a[<alpha>]][+s]] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_ho_OPT, GMT_o_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> may be one or more grid files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Format report in fields on a single line using the format\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <file w e s n z0 z1 dx dy n_columns n_rows [x0 y0 x1 y1] [med L1scale] [mean std rms] [n_nan] [mode LMSscale]>,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   where -M adds [x0 y0 x1 y1] and [n_nan], -L1 adds [median L1scale], -L2 adds [mean std rms],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and -Lp adds [mode LMSscale]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ct to place <file> at the end of the output record, or -Cn to write only numerical columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Report tiles using tile size set in -I. Optionally, extend each tile region by <offx>/<offy>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +i to only report tiles if the subregion has data (limited to one input grid).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no grid is given then -R must be given and we tile based on the given region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ct to append the region string as trailing text to the numerical columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Report domain in world mapping format [Default is generic].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Return textstring -Rw/e/s/n to nearest multiple of dx/dy.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -C is set then rounding off will occur but no -R string is issued.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no argument is given then the -I<xinc>/<yinc> string is issued.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -Ib is given then the grid's bounding box polygon is issued.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -Ii is given then the original img2grd -R string is issued, if available.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If the grid is not an img grid then the regular -R string is issued.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -Ir is given then the grid's -R string is issued.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set report mode:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L0 reports range of data by actually reading them (not from header).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L1 reports median and L1 scale (MAD w.r.t. median) of data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L[2] reports mean, standard deviation, and rms of data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Lp reports mode (lms) and LMS-scale (MAD w.r.t. mode) of data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -La all of the above.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If grid is geographic then we report area-weighted statistics.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Search for the global min and max locations (x0,y0) and (x1,y1).\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Print global -Tzmin/zmax[/dz] (in rounded multiples of dz, if given).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +a[<alpha>] to trim grid range by excluding the two <alpha>/2 tails [2 %%].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Note: +a is limited to a single grid.  Give <alpha> in percent.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s to force a symmetrical range about zero.\n");
	GMT_Option (API, "V,f,h,o,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDINFO_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	bool no_file_OK, num_report;
	unsigned int n_errors = 0, n_files = 0;
	char text[GMT_LEN32] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					n_files++;
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Column format */
				Ctrl->C.active = true;
				if (opt->arg[0] == 'n')
					Ctrl->C.mode = GRDINFO_NUMERICAL;
				else if (opt->arg[0] == 't')
					Ctrl->C.mode = GRDINFO_TRAILING;
				if (GMT->parent->external && Ctrl->C.mode == GRDINFO_TRADITIONAL) Ctrl->C.mode = GRDINFO_NUMERICAL;
				break;
			case 'D':	/* Tiling output w/ optional overlap */
				Ctrl->D.active = true;
				if (opt->arg[0]) {
					if ((c = strstr (opt->arg, "+i"))) {
						c[0] = '\0';	/* Temporarily chop off modifier */
						Ctrl->D.mode = 1;
					}
					if (opt->arg[0] && gmt_getinc (GMT, opt->arg, Ctrl->D.inc)) {
						gmt_inc_syntax (GMT, 'D', 1);
						n_errors++;
					}
				}
				break;
			case 'F':	/* World mapping format */
				Ctrl->F.active = true;
				break;
			case 'G':	/* List of GDAL options */
				Ctrl->G.active = true;
				Ctrl->G.opts = strdup(opt->arg);
				break;
			case 'I':	/* Increment rounding */
				Ctrl->I.active = true;
				if (!opt->arg[0])	/* No args given, we want to output the -I string */
					Ctrl->I.status = GRDINFO_GIVE_INCREMENTS;
				else if (opt->arg[0] == 'b' && opt->arg[1] == '\0')	/* -Ib means return grid perimeter as bounding box */
					Ctrl->I.status = GRDINFO_GIVE_BOUNDBOX;
				else if (opt->arg[0] == 'i' && opt->arg[1] == '\0')	/* -Ii means return -R string from original img2grd */
					Ctrl->I.status = GRDINFO_GIVE_REG_IMG;
				else if ((opt->arg[0] == 'r' || opt->arg[0] == '-') && opt->arg[1] == '\0')	/* -Ir: we want to output the actual -R string */
					Ctrl->I.status = GRDINFO_GIVE_REG_ORIG;
				else {	/* Report -R to nearest given multiple increment */
					Ctrl->I.status = GRDINFO_GIVE_REG_ROUNDED;
					if (gmt_getinc (GMT, opt->arg, Ctrl->I.inc)) {
						gmt_inc_syntax (GMT, 'I', 1);
						n_errors++;
					}
				}
				break;
			case 'L':	/* Selects norm */
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case '\0': case '2':
						Ctrl->L.norm |= 2; break;
					case '1':
						Ctrl->L.norm |= 1; break;
					case 'p':
						Ctrl->L.norm |= 4; break;
					case 'a':	/* All three */
						Ctrl->L.norm |= (1+2+4); break;
				}
				break;
			case 'M':	/* Global extrema */
				Ctrl->M.active = true;
				break;
			case 'T':	/* CPT range */
				Ctrl->T.active = true;
				if (opt->arg[0] == 's' && gmt_M_compat_check (GMT, 5)) {	/* Old-style format, cast in new syntax */
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-Ts option is deprecated; please use -T[<dz>][+s][+a[<alpha>]] next time.\n");
					sprintf (text, "%s+s", &opt->arg[1]);
				}
				else
					strncpy (text, opt->arg, GMT_LEN32-1);
				if (gmt_validate_modifiers (GMT, text, opt->option, "as")) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Error -T: Syntax is -T[<dz>][+s][+a[<alpha>]] next time.\n");
					n_errors++;
				}
				else {
					char string[GMT_LEN32] = {""};
					if (text[0] && text[0] != '+')
						Ctrl->T.inc = atof (text);
					if (gmt_get_modifier (text, 's', string))	/* Want symmetrical range about 0, i.e., -3500/3500[/500] */
						Ctrl->T.mode |= 1;
					if (gmt_get_modifier (text, 'a', string)) {	/* Want alpha-trimmed range before determining limits */
						Ctrl->T.mode |= 2;
						if (string[0]) Ctrl->T.alpha = atof (string);
					}
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	num_report = (Ctrl->C.active && (Ctrl->D.active || Ctrl->C.mode != GRDINFO_TRADITIONAL));
	no_file_OK = (Ctrl->D.active && Ctrl->D.mode == 0 && GMT->common.R.active[RSET]);
	n_errors += gmt_M_check_condition (GMT, n_files == 0 && !no_file_OK,
	                                   "Syntax error: Must specify one or more input files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.mode && n_files != 1,
	                                   "Syntax error -D: The +n modifier requires a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.inc < 0.0,
	                                   "Syntax error -T: The optional increment must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.mode & 2 && n_files != 1,
	                                   "Syntax error -T: The optional alpha-trim value can only work with a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && (Ctrl->T.alpha < 0.0 || Ctrl->T.alpha > 100.0),
	                                   "Syntax error -T: The optional alpha-trim value must be in the 0 < alpha < 100 %% range\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.status == GRDINFO_GIVE_REG_ROUNDED &&
	                                   (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0),
									   "Syntax error -I: Must specify a positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->I.active || Ctrl->T.active) && Ctrl->M.active,
	                                   "Syntax error -M: Not compatible with -I or -T\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->I.active || Ctrl->T.active) && Ctrl->L.active,
	                                   "Syntax error -L: Not compatible with -I or -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->I.active,
	                                   "Syntax error: Only one of -I -T can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->F.active,
	                                   "Syntax error: Only one of -C, -F can be specified\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.o.active && !num_report,
	                                   "Syntax error: The -o option requires -Cn\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

struct GMT_TILES {
	double wesn[4];
};

GMT_LOCAL void report_tiles (struct GMT_CTRL *GMT, struct GMT_GRID *G, double w, double e, double s, double n, struct GRDINFO_CTRL *Ctrl) {
	/* Find the tiles covering the present grid, if given */
	bool use = true, num_report;
	unsigned int nx, ny, i, j, js = 0, jn = 0, ie, iw;
	uint64_t row, col, node;
	double wesn[4], out[4], box[4];
	char text[GMT_LEN64] = {""}, record[GMT_BUFSIZ] = {""};
	struct GMT_RECORD *Out = NULL;
	
	num_report = (Ctrl->C.active);
	Out = gmt_new_record (GMT, num_report ? out : NULL, (num_report && Ctrl->C.mode != GRDINFO_TRAILING) ? NULL : record);
	
	nx = gmt_M_get_n (GMT, w, e, Ctrl->I.inc[GMT_X], 1);
	ny = gmt_M_get_n (GMT, s, n, Ctrl->I.inc[GMT_Y], 1);
	for (j = 0; j < ny; j++) {
		box[YLO] = wesn[YLO] = s + j * Ctrl->I.inc[GMT_Y];	box[YHI] = wesn[YHI] = wesn[YLO] + Ctrl->I.inc[GMT_Y];
		if (G && Ctrl->D.mode) {	/* Must determine if there is at least one data point inside this subset */
			if (wesn[YLO] < G->header->wesn[YLO]) wesn[YLO] = G->header->wesn[YLO];
			if (wesn[YHI] > G->header->wesn[YHI]) wesn[YHI] = G->header->wesn[YHI];
			js = gmt_M_grd_y_to_row (GMT, wesn[YLO], G->header);
			jn = gmt_M_grd_y_to_row (GMT, wesn[YHI], G->header);
		}
		for (i = 0; i < nx; i++) {
			box[XLO] = wesn[XLO] = w + i * Ctrl->I.inc[GMT_X];	box[XHI] = wesn[XHI] = wesn[XLO] + Ctrl->I.inc[GMT_X];
			if (G && Ctrl->D.mode) {	/* Must determine if there is at least one data point inside this subset */
				if (wesn[XLO] < G->header->wesn[XLO]) wesn[XLO] = G->header->wesn[XLO];
				if (wesn[XHI] > G->header->wesn[XHI]) wesn[XHI] = G->header->wesn[XHI];
				iw = gmt_M_grd_x_to_col (GMT, wesn[XLO], G->header);
				ie = gmt_M_grd_x_to_col (GMT, wesn[XHI], G->header);
				use = true;
				for (row = jn; row <= js; row++) {
					for (col = iw; col <= ie; col++) {
						node = gmt_M_ijp (G->header, row, col);
						if (!gmt_M_is_fnan (G->data[node]))
							goto L_use_it;
					}
				}
				use = false;	/* Could not find a single valid node */
L_use_it:			row = 0;	/* Get here by goto and use is still true */	
			}
			if (use) {
				gmt_M_memcpy (out, box, 4, double);
				out[XLO] -= Ctrl->D.inc[GMT_X];
				out[XHI] += Ctrl->D.inc[GMT_X];
				out[YLO] -= Ctrl->D.inc[GMT_Y];
				out[YHI] += Ctrl->D.inc[GMT_Y];
				if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must make sure we don't get outside valid bounds */
					if (out[YLO] < -90.0)
						out[YLO] = -90.0;
					if (out[YHI] > 90.0)
						out[YHI] = 90.0;
					if (fabs (out[XHI] - out[XLO]) > 360.0) {
						out[XLO] = (out[XLO] < 0.0) ? -180.0 : 0.0;
						out[XHI] = (out[XHI] < 0.0) ? +180.0 : 360.0;
					}
				}
				if (num_report && Ctrl->C.mode != GRDINFO_TRAILING)	/* Report numerical only */
					GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, Out);
				else {
					sprintf (record, "-R");
					gmt_ascii_format_col (GMT, text, out[XLO], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
					gmt_ascii_format_col (GMT, text, out[XHI], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
					gmt_ascii_format_col (GMT, text, out[YLO], GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, "/");
					gmt_ascii_format_col (GMT, text, out[YHI], GMT_OUT, GMT_X);	strcat (record, text);
					GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, Out);
				}
			}
		}
	}
	gmt_M_free (GMT, Out);
}

GMT_LOCAL void smart_increments (struct GMT_CTRL *GMT, double inc[], unsigned int which, char *text) {
	char tmptxt[GMT_LEN64] = {""};
	if (gmt_M_is_geographic (GMT, GMT_IN) && ((which == 2 && inc[GMT_X] < 1.0 && inc[GMT_Y] < 1.0) || (which != 2 && inc[which] < 1.0))) {	/* See if we can report smart increments */
		unsigned int col, s, k, int_inc[2], use_unit[2], kind;
		bool ok[2] = {false, false};
		double new_inc;
		static int try[10] = {30, 20, 15, 10, 6, 5, 4, 3, 2, 1}, scl[2] = {60, 3600};
		static char *unit[2][2] = {{"m", "s"}, {" min", " sec"}};
		kind = (which == 2) ? 0 : 1;
		/* See if we can use arc units */
		for (col = ((which == 2) ? GMT_X : which); col <= ((which == 2) ? GMT_Y : which); col++) {
			for (s = 0, ok[col] = false; !ok[col] && s < 2; s++) {
				for (k = 0; !ok[col] && k < 10; k++) {
					new_inc = inc[col] * scl[s];	/* Now in minutes or seconds */
					int_inc[col] = try[k] * irint (new_inc / try[k]);	/* Nearest in given multiple */
					if (fabs ((new_inc - int_inc[col])/new_inc) < GMT_CONV4_LIMIT) {	/* Yes, hit bingo */
						ok[col] = true;	use_unit[col] = s;
					}
				}
			}
		}
		if (which != 2)	{	/* Single increment only, no slash situation */
			if (ok[which]) {	/* Single increment and it works */
				gmt_ascii_format_col (GMT, tmptxt, inc[which], GMT_OUT, GMT_Z);
				sprintf (text, "%s (%d%s)", tmptxt, int_inc[which], unit[kind][use_unit[which]]);
			}
			else
				gmt_ascii_format_col (GMT, text, inc[which], GMT_OUT, GMT_Z);
		}
		else if (ok[GMT_X] && ok[GMT_Y]) {	/* Both can use arc min or sec */
			if (int_inc[GMT_X] == int_inc[GMT_Y] && use_unit[GMT_X] == use_unit[GMT_Y])	/* Same increments */
				sprintf (text, "%d%s", int_inc[GMT_X], unit[kind][use_unit[GMT_X]]);
			else	/* Different so must use slash */
				sprintf (text, "%d%s/%d%s", int_inc[GMT_X], unit[kind][use_unit[GMT_X]], int_inc[GMT_Y], unit[kind][use_unit[GMT_Y]]);
		}
		else if (ok[GMT_X]) {	/* Only x-inc can use arc units */
			gmt_ascii_format_col (GMT, tmptxt, inc[GMT_Y], GMT_OUT, GMT_Z);
			sprintf (text, "%d%s/%s", int_inc[GMT_X], unit[kind][use_unit[GMT_X]], tmptxt);
		}
		else if (ok[GMT_Y]) {	/* Only y-inc can use arc units */
			gmt_ascii_format_col (GMT, tmptxt, inc[GMT_X], GMT_OUT, GMT_Z);
			sprintf (text, "%s/%d%s", tmptxt, int_inc[GMT_Y], unit[kind][use_unit[GMT_Y]]);
		}
		else {	/* Neither can use arc units */
			gmt_ascii_format_col (GMT, text, inc[GMT_X], GMT_OUT, GMT_Z);
			gmt_ascii_format_col (GMT, tmptxt, inc[GMT_Y], GMT_OUT, GMT_Z);
			strcat (text, "/");	strcat (text, tmptxt);
		}
	}
	else {	/* Cartesian */
		if (which == 2) {
			if (doubleAlmostEqual (inc[GMT_X], inc[GMT_Y]))
				gmt_ascii_format_col (GMT, text, inc[GMT_X], GMT_OUT, GMT_Z);
			else {
				gmt_ascii_format_col (GMT, text, inc[GMT_X], GMT_OUT, GMT_Z);
				gmt_ascii_format_col (GMT, tmptxt, inc[GMT_Y], GMT_OUT, GMT_Z);
				strcat (text, "/");	strcat (text, tmptxt);
			}
		}
		else
			gmt_ascii_format_col (GMT, text, inc[which], GMT_OUT, GMT_Z);
	}
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

#if defined(HAVE_GDAL) && (GDAL_VERSION_MAJOR >= 2) && (GDAL_VERSION_MINOR >= 1)
#include <gdal_utils.h>
#include "gmt_gdal_librarified.c"
#endif

int GMT_grdinfo (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int n_grds = 0, n_cols = 0, col, i_status, cmode = GMT_COL_FIX, geometry = GMT_IS_TEXT;
	bool subset, delay, num_report;

	uint64_t ij, n_nan = 0, n = 0;

	double x_min = 0.0, y_min = 0.0, z_min = 0.0, x_max = 0.0, y_max = 0.0, z_max = 0.0, wesn[4];
	double global_xmin, global_xmax, global_ymin, global_ymax, global_zmin, global_zmax;
	double z_mean = 0.0, z_median = 0.0, z_mode = 0.0, z_stdev = 0.0, z_scale = 0.0, z_lmsscl = 0.0, z_rms = 0.0, out[22];

	char format[GMT_BUFSIZ] = {""}, text[GMT_LEN512] = {""}, record[GMT_BUFSIZ] = {""}, grdfile[PATH_MAX] = {""};
	char *type[2] = { "Gridline", "Pixel"}, *sep = NULL, *projStr = NULL, *answer[2] = {"", " no"};

	struct GRDINFO_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL, *W = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdinfo main code ----------------------------*/

	/* OK, done parsing, now process all input grids in a loop */
	
	sep = GMT->current.setting.io_col_separator;
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	if (Ctrl->D.active && Ctrl->D.mode == 0 && GMT->common.R.active[RSET]) {
		global_xmin = GMT->common.R.wesn[XLO]; global_ymin = GMT->common.R.wesn[YLO];
		global_xmax = GMT->common.R.wesn[XHI] ; global_ymax = GMT->common.R.wesn[YHI];
		global_zmin = DBL_MAX;		global_zmax = -DBL_MAX;
	}
	else {
		global_xmin = global_ymin = global_zmin = DBL_MAX;
		global_xmax = global_ymax = global_zmax = -DBL_MAX;
	}
	delay = (Ctrl->D.mode == 1 || (Ctrl->T.mode & 2));	/* Delay the freeing of the (single) grid we read */
	num_report = (Ctrl->C.active && (Ctrl->D.active || Ctrl->C.mode != GRDINFO_TRADITIONAL));
	
	if (Ctrl->C.active) {
		n_cols = 6;	/* w e s n z0 z1 */
		if (!Ctrl->I.active) {
			n_cols += 4;				/* Add dx dy n_columns n_rows */
			if (Ctrl->M.active) n_cols += 5;	/* Add x0 y0 x1 y1 nnan */
			if (Ctrl->L.norm & 1) n_cols += 2;	/* Add median scale */
			if (Ctrl->L.norm & 2) n_cols += 3;	/* Add mean stdev rms */
			if (Ctrl->L.norm & 4) n_cols += 2;	/* Add mode lmsscale */
		}
		if (Ctrl->C.mode == GRDINFO_NUMERICAL) cmode = GMT_COL_FIX_NO_TEXT;
		geometry = GMT_IS_NONE;
		if (Ctrl->C.mode == GRDINFO_TRADITIONAL) geometry = GMT_IS_TEXT;
		if (geometry == GMT_IS_TEXT) n_cols = 0;	/* A single string, unfortunately */
		if (Ctrl->D.active) {
			n_cols = 4;	/* Just reporting w e s n per tile */
			if (Ctrl->C.mode != GRDINFO_TRAILING) cmode = GMT_COL_FIX_NO_TEXT;
		}
	}
	else if (Ctrl->I.status == GRDINFO_GIVE_BOUNDBOX) {
		n_cols = 2;
		cmode = GMT_COL_FIX_NO_TEXT;
		geometry = GMT_IS_POLY;
	}
	GMT_Report (API, GMT_MSG_DEBUG, "Will write output record(s) with %d leading numerical columns and with%s trailing text\n", n_cols, answer[cmode>0]);
	
	GMT_Set_Columns (GMT->parent, GMT_OUT, n_cols, cmode);

	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	
	Out = gmt_new_record (GMT, (n_cols) ? out : NULL, (cmode) == GMT_COL_FIX ? record : NULL);	/* the two items */
	
	for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */ 

		if (opt->option != '<') continue;	/* We are only processing filenames here */

#if defined(HAVE_GDAL) && (GDAL_VERSION_MAJOR >= 2) && (GDAL_VERSION_MINOR >= 1)
		if (Ctrl->G.active)
			grid_gdal_librarified (GMT, opt->arg, Ctrl->G.opts);
#endif

		gmt_set_cartesian (GMT, GMT_IN);	/* Reset since we may get a bunch of files, some geo, some not */

		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL) {
			Return (API->error);
		}
		subset = gmt_M_is_subset (GMT, G->header, wesn);	/* Subset requested */
		if (subset) gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, G->header), "");	/* Make sure wesn matches header spacing */
		HH = gmt_get_H_hidden (G->header);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing grid %s\n", HH->name);

		if (G->header->ProjRefPROJ4 && !Ctrl->C.active && !Ctrl->T.active && !Ctrl->I.active)
			projStr = strdup (G->header->ProjRefPROJ4);		/* Copy proj string to print at the end */
		else if (G->header->ProjRefWKT && !Ctrl->C.active && !Ctrl->T.active && !Ctrl->I.active) 
			projStr = strdup (G->header->ProjRefWKT);

		for (n = 0; n < GMT_Z; n++)
			GMT->current.io.col_type[GMT_OUT][n] = gmt_M_type (GMT, GMT_IN, n);	/* Since grids may differ in types */

		if (num_report && n_grds == 0) {	/* Need to prepare col_type for output */
			if (Ctrl->M.active) {
				GMT->current.io.col_type[GMT_OUT][10] = GMT->current.io.col_type[GMT_OUT][12] = GMT->current.io.col_type[GMT_OUT][GMT_X];
				GMT->current.io.col_type[GMT_OUT][11] = GMT->current.io.col_type[GMT_OUT][13] = GMT->current.io.col_type[GMT_OUT][GMT_Y];
			}
			GMT->current.io.col_type[GMT_OUT][2] = GMT->current.io.col_type[GMT_OUT][3] = GMT->current.io.col_type[GMT_OUT][GMT_Y];
			GMT->current.io.col_type[GMT_OUT][1] = GMT->current.io.col_type[GMT_OUT][GMT_X];
		}
		n_grds++;

		if (Ctrl->M.active || Ctrl->L.active || subset || Ctrl->D.mode || (Ctrl->T.mode & 2)) {	/* Need to read the data (all or subset) */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, opt->arg, G) == NULL) {
				Return (API->error);
			}
		}

		if (Ctrl->T.mode & 2) strncpy (grdfile, opt->arg, PATH_MAX-1);
		
		if (Ctrl->M.active || Ctrl->L.active) {	/* Must determine the location of global min and max values */
			uint64_t ij_min, ij_max;
			unsigned int col, row;

			z_min = DBL_MAX;	z_max = -DBL_MAX;
			ij_min = ij_max = n = 0;
			gmt_M_grd_loop (GMT, G, row, col, ij) {
				if (gmt_M_is_fnan (G->data[ij])) continue;
				if (G->data[ij] < z_min) {
					z_min = G->data[ij];	ij_min = ij;
				}
				if (G->data[ij] > z_max) {
					z_max = G->data[ij];	ij_max = ij;
				}
				n++;
			}

			n_nan = G->header->nm - n;
			if (n) {	/* Meaning at least one non-NaN node was found */
				col = (unsigned int)gmt_M_col (G->header, ij_min);
				row = (unsigned int)gmt_M_row (G->header, ij_min);
				x_min = gmt_M_grd_col_to_x (GMT, col, G->header);
				y_min = gmt_M_grd_row_to_y (GMT, row, G->header);
				col = (unsigned int)gmt_M_col (G->header, ij_max);
				row = (unsigned int)gmt_M_row (G->header, ij_max);
				x_max = gmt_M_grd_col_to_x (GMT, col, G->header);
				y_max = gmt_M_grd_row_to_y (GMT, row, G->header);
			}
			else	/* Not a single valid node */
				x_min = x_max = y_min = y_max = GMT->session.d_NaN;
		}

		if (Ctrl->L.norm && gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
			W = gmt_duplicate_grid (GMT, G, GMT_DUPLICATE_ALLOC);
			gmt_get_cellarea (GMT, W);
		}
		
		if (Ctrl->L.norm & 1) {	/* Calculate the median and MAD */
			z_median = gmt_grd_median (GMT, G, W, false);
			z_scale = gmt_grd_mad (GMT, G, W, &z_median, false);
		}
		if (Ctrl->L.norm & 2) {	/* Calculate the mean, standard deviation, and rms */
			z_mean = gmt_grd_mean (GMT, G, W);	/* Compute the [weighted] mean */
			z_stdev = gmt_grd_std (GMT, G, W);	/* Compute the [weighted] stdev */
			z_rms = gmt_grd_rms (GMT, G, W);		/* Compute the [weighted] rms */
		}
		if (Ctrl->L.norm & 4) {	/* Calculate the mode and lmsscale */
			z_mode = gmt_grd_mode (GMT, G, W, false);
			z_lmsscl = gmt_grd_lmsscl (GMT, G, W, &z_mode, false);
		}
		if (W) gmt_free_grid (GMT, &W, true);

		if (gmt_M_is_geographic (GMT, GMT_IN)) {
			if (gmt_grd_is_global(GMT, G->header) || (G->header->wesn[XLO] < 0.0 && G->header->wesn[XHI] <= 0.0))
				GMT->current.io.geo.range = GMT_IS_GIVEN_RANGE;
			else if ((G->header->wesn[XHI] - G->header->wesn[XLO]) > 180.0)
				GMT->current.io.geo.range = GMT_IS_GIVEN_RANGE;
			else if (G->header->wesn[XLO] < 0.0 && G->header->wesn[XHI] >= 0.0)
				GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
			else
				GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
		}

		/* OK, time to report results */

		i_status = Ctrl->I.status;
		if (Ctrl->I.active && Ctrl->I.status == GRDINFO_GIVE_REG_IMG) {
			if (!(strstr (G->header->command, "img2grd") || strstr (G->header->command, "img2mercgrd") ||
			      strstr (G->header->remark, "img2mercgrd"))) {
				GMT_Report (API, GMT_MSG_NORMAL,
				            "Could not find a -Rw/e/s/n string produced by img tools - returning regular grid -R\n");
				i_status = GRDINFO_GIVE_REG_ORIG;
			}
		}

		if (Ctrl->I.active && i_status == GRDINFO_GIVE_REG_ORIG) {
			sprintf (record, "-R");
			gmt_ascii_format_col (GMT, text, G->header->wesn[XLO], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, G->header->wesn[XHI], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, G->header->wesn[YLO], GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, G->header->wesn[YHI], GMT_OUT, GMT_Y);	strcat (record, text);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else if (Ctrl->I.active && i_status == GRDINFO_GIVE_REG_IMG) {
			char *c = strrchr (G->header->remark, 'R');
			sprintf (record, "-%s", c);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else if (Ctrl->I.active && i_status == GRDINFO_GIVE_INCREMENTS) {
			sprintf (record, "-I");
			smart_increments (GMT, G->header->inc, 2, text);	strcat (record, text);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else if (Ctrl->I.active && i_status == GRDINFO_GIVE_BOUNDBOX) {
			sprintf (record, "> Bounding box for %s", HH->name);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);
			/* LL */
			out[GMT_X] = G->header->wesn[XLO];	out[GMT_Y] = G->header->wesn[YLO];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			/* LR */
			out[GMT_X] = G->header->wesn[XHI];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			/* UR */
			out[GMT_Y] = G->header->wesn[YHI];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			/* UL */
			out[GMT_X] = G->header->wesn[XLO];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			/* LL (repeat to close polygon) */
			out[GMT_X] = G->header->wesn[XLO];	out[GMT_Y] = G->header->wesn[YLO];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else if (Ctrl->C.active && !Ctrl->I.active) {
			if (num_report) {	/* External interface, return as data with no leading text */
				/* w e s n z0 z1 dx dy n_columns n_rows [x0 y0 x1 y1] [med scale] [mean std rms] [n_nan] */
				gmt_M_memcpy (out, G->header->wesn, 4, double);	/* Place the w/e/s/n limits */
				out[ZLO]   = G->header->z_min;		out[ZHI]   = G->header->z_max;
				out[ZHI+1] = G->header->inc[GMT_X];	out[ZHI+2] = G->header->inc[GMT_Y];
				out[ZHI+3] = G->header->n_columns;		out[ZHI+4] = G->header->n_rows;
				col = ZHI+5;
				if (Ctrl->M.active) {
					out[col++] = x_min;	out[col++] = y_min;
					out[col++] = x_max;	out[col++] = y_max;
				}
				if (Ctrl->L.norm & 1) {
					out[col++] = z_median;	out[col++] = z_scale;
				}
				if (Ctrl->L.norm & 2) {
					out[col++] = z_mean;	out[col++] = z_stdev;	out[col++] = z_rms;
				}
				if (Ctrl->M.active) {
					out[col++] = (double)n_nan;
				}
				if (Ctrl->L.norm & 4) {
					out[col++] = z_mode;	out[col++] = z_lmsscl;
				}
				if (Ctrl->C.mode == GRDINFO_TRAILING)
					sprintf (record, "%s", HH->name);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			else {	/* Command-line usage */
				record[0] = '\0';
				if (Ctrl->C.mode != GRDINFO_TRAILING)
					sprintf (record, "%s%s", HH->name, sep);
				gmt_ascii_format_col (GMT, text, G->header->wesn[XLO], GMT_OUT, GMT_X);
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, G->header->wesn[XHI], GMT_OUT, GMT_X);
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, G->header->wesn[YLO], GMT_OUT, GMT_Y);
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, G->header->wesn[YHI], GMT_OUT, GMT_Y);
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, G->header->z_min, GMT_OUT, GMT_Z);
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, G->header->z_max, GMT_OUT, GMT_Z);
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, G->header->inc[GMT_X], GMT_OUT, GMT_X);
				if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, G->header->inc[GMT_Y], GMT_OUT, GMT_Y);
				if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, (double)G->header->n_columns, GMT_OUT, GMT_Z);
				strcat (record, text);	strcat (record, sep);
				gmt_ascii_format_col (GMT, text, (double)G->header->n_rows, GMT_OUT, GMT_Z);
				strcat (record, text);

				if (Ctrl->M.active) {
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, x_min, GMT_OUT, GMT_X);	strcat (record, text);
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, y_min, GMT_OUT, GMT_Y);	strcat (record, text);
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, x_max, GMT_OUT, GMT_X);	strcat (record, text);
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, y_max, GMT_OUT, GMT_Y);	strcat (record, text);
				}
				if (Ctrl->L.norm & 1) {
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, z_median, GMT_OUT, GMT_Z);	strcat (record, text);
					strcat (record, sep);	gmt_ascii_format_col (GMT, text,  z_scale, GMT_OUT, GMT_Z);	strcat (record, text);
				}
				if (Ctrl->L.norm & 2) {
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, z_mean, GMT_OUT, GMT_Z);	strcat (record, text);
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, z_stdev, GMT_OUT, GMT_Z);	strcat (record, text);
					strcat (record, sep);	gmt_ascii_format_col (GMT, text,   z_rms, GMT_OUT, GMT_Z);	strcat (record, text);
				}
				if (Ctrl->M.active) { sprintf (text, "%s%" PRIu64, sep, n_nan);	strcat (record, text); }
				if (Ctrl->L.norm & 4) {
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, z_mode,   GMT_OUT, GMT_Z);	strcat (record, text);
					strcat (record, sep);	gmt_ascii_format_col (GMT, text, z_lmsscl, GMT_OUT, GMT_Z);	strcat (record, text);
				}
				if (Ctrl->C.mode == GRDINFO_TRAILING) { sprintf (text, "%s%s", sep, HH->name);	strcat (record, text); }
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
		}
		else if (!(Ctrl->T.active || (Ctrl->I.active && Ctrl->I.status == GRDINFO_GIVE_REG_ROUNDED))) {
			char *gtype[2] = {"Cartesian grid", "Geographic grid"};
			sprintf (record, "%s: Title: %s", HH->name, G->header->title);		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			sprintf (record, "%s: Command: %s", HH->name, G->header->command);	GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			sprintf (record, "%s: Remark: %s", HH->name, G->header->remark);	GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			if (G->header->registration == GMT_GRID_NODE_REG || G->header->registration == GMT_GRID_PIXEL_REG)
				sprintf (record, "%s: %s node registration used [%s]", HH->name, type[G->header->registration], gtype[gmt_M_is_geographic (GMT, GMT_IN)]);
			else
				sprintf (record, "%s: Unknown registration! Probably not a GMT grid", HH->name);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			if (G->header->type != k_grd_unknown_fmt)
				sprintf (record, "%s: Grid file format: %s", HH->name, GMT->session.grdformat[G->header->type]);
			else
				sprintf (record, "%s: Unrecognized grid file format! Probably not a GMT grid", HH->name);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			if (Ctrl->F.active) {
				if ((fabs (G->header->wesn[XLO]) < 500.0) && (fabs (G->header->wesn[XHI]) < 500.0) &&
				    (fabs (G->header->wesn[YLO]) < 500.0) && (fabs (G->header->wesn[YHI]) < 500.0)) {
					sprintf (record, "%s: x_min: %.7f", HH->name, G->header->wesn[XLO]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: x_max: %.7f", HH->name, G->header->wesn[XHI]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: x_inc: %.7f", HH->name, G->header->inc[GMT_X]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: name: %s",    HH->name, G->header->x_units);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: n_columns: %d", HH->name, G->header->n_columns);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: y_min: %.7f", HH->name, G->header->wesn[YLO]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: y_max: %.7f", HH->name, G->header->wesn[YHI]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: y_inc: %.7f", HH->name, G->header->inc[GMT_Y]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: name: %s",    HH->name, G->header->y_units);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: n_rows: %d",  HH->name, G->header->n_rows);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
				else {
					sprintf (record, "%s: x_min: %.2f", HH->name, G->header->wesn[XLO]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: x_max: %.2f", HH->name, G->header->wesn[XHI]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: x_inc: %.2f", HH->name, G->header->inc[GMT_X]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: name: %s",    HH->name, G->header->x_units);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: n_columns: %d",  HH->name, G->header->n_columns);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: y_min: %.2f", HH->name, G->header->wesn[YLO]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: y_max: %.2f", HH->name, G->header->wesn[YHI]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: y_inc: %.2f", HH->name, G->header->inc[GMT_Y]);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: name: %s",    HH->name, G->header->y_units);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					sprintf (record, "%s: n_rows: %d",  HH->name, G->header->n_rows);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
			else {
				char *c = NULL;
				sprintf (record, "%s: x_min: ", HH->name);
				gmt_ascii_format_col (GMT, text, G->header->wesn[XLO], GMT_OUT, GMT_X);	strcat (record, text);
				strcat (record, " x_max: ");
				gmt_ascii_format_col (GMT, text, G->header->wesn[XHI], GMT_OUT, GMT_X);	strcat (record, text);
				strcat (record, " x_inc: ");	smart_increments (GMT, G->header->inc, GMT_X, text);	strcat (record, text);
				strcat (record, " name: ");
				if ((c = strstr (G->header->x_units, " [degrees"))) {	/* Strip off [degrees ...] from the name of the longitude variable */
					c[0] = '\0'; strcat (record, G->header->x_units); if (c) c[0] = ' ';
				}
				else	/* Just output what it says */
					strcat (record, G->header->x_units);
				sprintf (text, " n_columns: %d", G->header->n_columns);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				sprintf (record, "%s: y_min: ", HH->name);
				gmt_ascii_format_col (GMT, text, G->header->wesn[YLO], GMT_OUT, GMT_Y);	strcat (record, text);
				strcat (record, " y_max: ");
				gmt_ascii_format_col (GMT, text, G->header->wesn[YHI], GMT_OUT, GMT_Y);	strcat (record, text);
				strcat (record, " y_inc: ");	smart_increments (GMT, G->header->inc, GMT_Y, text);	strcat (record, text);
				strcat (record, " name: ");
				if ((c = strstr (G->header->y_units, " [degrees"))) {	/* Strip off [degrees ...] from the name of the latitude variable */
					c[0] = '\0'; strcat (record, G->header->y_units); if (c) c[0] = ' ';
				}
				else	/* Just output what it says */
					strcat (record, G->header->y_units);
				sprintf (text, " n_rows: %d", G->header->n_rows);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}

			if (Ctrl->M.active) {
				if (z_min == DBL_MAX) z_min = GMT->session.d_NaN;
				if (z_max == -DBL_MAX) z_max = GMT->session.d_NaN;
				sprintf (record, "%s: z_min: ", HH->name);
				gmt_ascii_format_col (GMT, text, z_min, GMT_OUT, GMT_Z);	strcat (record, text);	strcat (record, " at x = ");
				gmt_ascii_format_col (GMT, text, x_min, GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, " y = ");
				gmt_ascii_format_col (GMT, text, y_min, GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, " z_max: ");
				gmt_ascii_format_col (GMT, text, z_max, GMT_OUT, GMT_Z);	strcat (record, text);	strcat (record, " at x = ");
				gmt_ascii_format_col (GMT, text, x_max, GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, " y = ");
				gmt_ascii_format_col (GMT, text, y_max, GMT_OUT, GMT_Y);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			else if (Ctrl->F.active) {
				sprintf (record, "%s: zmin: %g", HH->name, G->header->z_min);	GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				sprintf (record, "%s: zmax: %g", HH->name, G->header->z_max);	GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				sprintf (record, "%s: name: %s", HH->name, G->header->z_units);	GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			else {
				sprintf (record, "%s: z_min: ", HH->name);
				gmt_ascii_format_col (GMT, text, G->header->z_min, GMT_OUT, GMT_Z);	strcat (record, text);
				strcat (record, " z_max: ");
				gmt_ascii_format_col (GMT, text, G->header->z_max, GMT_OUT, GMT_Z);	strcat (record, text);
				strcat (record, " name: ");	strcat (record, G->header->z_units);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}

			/* print scale and offset */
			sprintf (format, "%s: scale_factor: %s add_offset: %s",
			         HH->name, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			sprintf (record, format, G->header->z_scale_factor, G->header->z_add_offset);
			if (G->header->z_scale_factor != 1.0 || G->header->z_add_offset != 0) {
				/* print packed z-range */
				sprintf (format, "%s packed z-range: [%s,%s]", record,
				         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
				sprintf (record, format,
				         (G->header->z_min - G->header->z_add_offset) / G->header->z_scale_factor,
				         (G->header->z_max - G->header->z_add_offset) / G->header->z_scale_factor);
			}
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			if (n_nan) {
				double percent = 100.0 * n_nan / G->header->nm;
				sprintf (record, "%s: %" PRIu64 " nodes (%.1f%%) set to NaN", HH->name, n_nan, percent);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			if (Ctrl->L.norm & 1) {
				sprintf (record, "%s: median: ", HH->name);
				gmt_ascii_format_col (GMT, text, z_median, GMT_OUT, GMT_Z);	strcat (record, text);
				strcat (record, " scale: ");
				gmt_ascii_format_col (GMT, text, z_scale, GMT_OUT, GMT_Z);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			if (Ctrl->L.norm & 2) {
				sprintf (record, "%s: mean: ", HH->name);
				gmt_ascii_format_col (GMT, text,  z_mean, GMT_OUT, GMT_Z);	strcat (record, text);
				strcat (record, " stdev: ");
				gmt_ascii_format_col (GMT, text, z_stdev, GMT_OUT, GMT_Z);	strcat (record, text);
				strcat (record, " rms: ");
				gmt_ascii_format_col (GMT, text,   z_rms, GMT_OUT, GMT_Z);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			if (Ctrl->L.norm & 4) {
				sprintf (record, "%s: mode: ", HH->name);
				gmt_ascii_format_col (GMT, text,   z_mode, GMT_OUT, GMT_Z);	strcat (record, text);
				strcat (record, " lmsscale: ");
				gmt_ascii_format_col (GMT, text, z_lmsscl, GMT_OUT, GMT_Z);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			if (strspn(GMT->session.grdformat[G->header->type], "nc") != 0) {
				/* type is netCDF: report chunk size and deflation level */
				if (HH->is_netcdf4) {
					sprintf (text, " chunk_size: %" PRIuS ",%" PRIuS " shuffle: %s deflation_level: %u",
					         HH->z_chunksize[0], HH->z_chunksize[1],
					         HH->z_shuffle ? "on" : "off", HH->z_deflate_level);
				}
				else
					text[0] = '\0';
				sprintf (record, "%s: format: %s%s",
						HH->name, HH->is_netcdf4 ? "netCDF-4" : "classic", text);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
		} /* !(Ctrl->T.active || (Ctrl->I.active && Ctrl->I.status == GRDINFO_GIVE_REG_ROUNDED))) */
		else {
			if (G->header->z_min < global_zmin) global_zmin = G->header->z_min;
			if (G->header->z_max > global_zmax) global_zmax = G->header->z_max;
			if (G->header->wesn[XLO] < global_xmin) global_xmin = G->header->wesn[XLO];
			if (G->header->wesn[XHI] > global_xmax) global_xmax = G->header->wesn[XHI];
			if (G->header->wesn[YLO] < global_ymin) global_ymin = G->header->wesn[YLO];
			if (G->header->wesn[YHI] > global_ymax) global_ymax = G->header->wesn[YHI];
		}
		if (!delay && GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	if (global_zmin == DBL_MAX) global_zmin = GMT->session.d_NaN;	/* Never got set */
	if (global_zmax == -DBL_MAX) global_zmax = GMT->session.d_NaN;

	if (Ctrl->C.active && (Ctrl->I.active && Ctrl->I.status == GRDINFO_GIVE_REG_ROUNDED)) {
		if (!Ctrl->D.active) {	/* Don't want to round if getting tile regions */
			global_xmin = floor (global_xmin / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
			global_xmax = ceil  (global_xmax / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
			global_ymin = floor (global_ymin / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
			global_ymax = ceil  (global_ymax / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		}
		if (!Ctrl->D.active && gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must make sure we don't get outside valid bounds */
			if (global_ymin < -90.0) {
				global_ymin = -90.0;
				GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused south to become < -90.  Reset to -90.\n");
			}
			if (global_ymax > 90.0) {
				global_ymax = 90.0;
				GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused north to become > +90.  Reset to +90.\n");
			}
			if (fabs (global_xmax - global_xmin) > 360.0) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused longitude range to exceed 360.  Reset to a range of 360.\n");
				global_xmin = (global_xmin < 0.0) ? -180.0 : 0.0;
				global_xmax = (global_xmin < 0.0) ? +180.0 : 360.0;
			}
		}
		if (Ctrl->D.active)
			report_tiles (GMT, G, global_xmin, global_xmax, global_ymin, global_ymax, Ctrl);
		else if (num_report) {	/* External interface, return as data with no leading text */
			/* w e s n z0 z1 */
			out[XLO] = global_xmin;		out[XHI] = global_xmax;
			out[YLO] = global_ymin;		out[YHI] = global_ymax;
			out[ZLO] = global_zmin;		out[ZHI] = global_zmax;
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else {	/* Command-line usage */
			sprintf (record, "%d%s", n_grds, sep);
			gmt_ascii_format_col (GMT, text, global_xmin, GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, sep);
			gmt_ascii_format_col (GMT, text, global_xmax, GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, sep);
			gmt_ascii_format_col (GMT, text, global_ymin, GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, sep);
			gmt_ascii_format_col (GMT, text, global_ymax, GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, sep);
			gmt_ascii_format_col (GMT, text, global_zmin, GMT_OUT, GMT_Z);	strcat (record, text);	strcat (record, sep);
			gmt_ascii_format_col (GMT, text, global_zmax, GMT_OUT, GMT_Z);	strcat (record, text);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
	}
	else if (Ctrl->T.active) {
		if (Ctrl->T.mode & 2) {	/* Must do alpha trimming first */
			gmt_grdfloat *tmp_grid = NULL;
			char *file_ptr = grdfile;	/* To avoid a warning */
			if (gmt_M_file_is_memory (file_ptr)) {	/* Must operate on a copy since sorting is required */
				tmp_grid = gmt_M_memory_aligned (GMT, NULL, G->header->size, gmt_grdfloat);
				gmt_M_memcpy (tmp_grid, G->data, G->header->size, gmt_grdfloat);
			}
			else
				tmp_grid = G->data;
			gmt_sort_array (GMT, tmp_grid, G->header->size, GMT_FLOAT);	/* Sort so we can find quantiles */
			global_zmin = gmt_quantile_f (GMT, tmp_grid, 0.5 * Ctrl->T.alpha, G->header->size);			/* "Left" quantile */
			global_zmax = gmt_quantile_f (GMT, tmp_grid, 100.0-0.5* Ctrl->T.alpha, G->header->size);	/* "Right" quantile */
			if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {	/* Delayed destroy due to alpha trimming */
				gmt_M_free (GMT, tmp_grid);
				Return (API->error);
			}
			if (gmt_M_file_is_memory (file_ptr))	/* Now free temp grid */
				gmt_M_free (GMT, tmp_grid);
		}
		if (Ctrl->T.mode & 1) {	/* Get a symmetrical range */
			if (Ctrl->T.inc > 0.0) {	/* Round limits first */
				global_zmin = floor (global_zmin / Ctrl->T.inc) * Ctrl->T.inc;
				global_zmax = ceil  (global_zmax / Ctrl->T.inc) * Ctrl->T.inc;
			}
			global_zmax = MAX (fabs (global_zmin), fabs (global_zmax));
			global_zmin = -global_zmax;
		}
		else {	/* Just use reported min/max values (possibly alpha-trimmed above) */
			if (Ctrl->T.inc > 0.0) {	/* Round limits first */
				global_zmin = floor (global_zmin / Ctrl->T.inc) * Ctrl->T.inc;
				global_zmax = ceil  (global_zmax / Ctrl->T.inc) * Ctrl->T.inc;
			}
		}
		sprintf (record, "-T");
		gmt_ascii_format_col (GMT, text, global_zmin, GMT_OUT, GMT_Z);	strcat (record, text);	strcat (record, "/");
		gmt_ascii_format_col (GMT, text, global_zmax, GMT_OUT, GMT_Z);	strcat (record, text);
		if (Ctrl->T.inc > 0.0) {
			strcat (record, "/");
			gmt_ascii_format_col (GMT, text, Ctrl->T.inc, GMT_OUT, GMT_Z);	strcat (record, text);
		}
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
	}
	else if ((Ctrl->I.active && Ctrl->I.status == GRDINFO_GIVE_REG_ROUNDED)) {
		if (!Ctrl->D.active) {	/* Don't want to round if getting tile regions */
			global_xmin = floor (global_xmin / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
			global_xmax = ceil  (global_xmax / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
			global_ymin = floor (global_ymin / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
			global_ymax = ceil  (global_ymax / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		}
		if (!Ctrl->D.active && gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must make sure we don't get outside valid bounds */
			if (global_ymin < -90.0) {
				global_ymin = -90.0;
				GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused south to become < -90.  Reset to -90.\n");
			}
			if (global_ymax > 90.0) {
				global_ymax = 90.0;
				GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused north to become > +90.  Reset to +90.\n");
			}
			if (fabs (global_xmax - global_xmin) > 360.0) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused longitude range to exceed 360.  Reset to a range of 360.\n");
				global_xmin = (global_xmin < 0.0) ? -180.0 : 0.0;
				global_xmax = (global_xmin < 0.0) ? +180.0 : 360.0;
			}
		}
		if (Ctrl->D.active)
			report_tiles (GMT, G, global_xmin, global_xmax, global_ymin, global_ymax, Ctrl);
		else {
			sprintf (record, "-R");
			gmt_ascii_format_col (GMT, text, global_xmin, GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, global_xmax, GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, global_ymin, GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, global_ymax, GMT_OUT, GMT_Y);	strcat (record, text);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
	}

	if (delay && GMT_Destroy_Data (API, &G) != GMT_NOERROR) {	/* Delayed destroy due to -D+n */
		Return (API->error);
	}

	if (!Ctrl->C.active && !Ctrl->T.active && projStr) {		/* Print the referencing info */
		Out->text = projStr;
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		gmt_M_str_free (projStr);
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	gmt_M_free (GMT, Out);

	Return (GMT_NOERROR);
}
