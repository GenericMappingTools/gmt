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
 * Brief synopsis: psstereonet reads structural geology observations given as
 * pairs of angles (planes as strike/dip or dip-direction/dip, or lines as
 * trend/plunge) and plots them on a stereonet, i.e., the projection of the
 * lower (or upper) hemisphere of a unit sphere onto the horizontal plane.
 *
 * A stereonet is not a new map projection: it is simply a hemisphere seen from
 * above, centered on the nadir.  Hence we use the standard GMT azimuthal
 * projections centered on (0,0), whose default 90-degree horizon gives exactly
 * one hemisphere:
 *
 *   -JA0/0/<width>	Lambert azimuthal equal-area -> Schmidt net (equal-area)
 *   -JS0/0/<width>	Stereographic (equal-angle)   -> Wulff net (equal-angle)
 *
 * With that setup the north pole of the sphere (lat = +90) plots at the top of
 * the net, so azimuth is measured clockwise from the top of the plot while the
 * angular distance from the center of the net is 90 minus the plunge of the
 * line being plotted.  A line with trend T and plunge P is therefore the point
 * (lon, lat) obtained from the unit vector
 *
 *	x = cos (90-P), y = sin (90-P) * sin (T), z = sin (90-P) * cos (T)
 *
 * while the cyclographic trace (great circle) of a plane with strike S and dip
 * D is the meridian lon = 90-D rotated by S about the x-axis, i.e., about the
 * axis that points at the center of the net.  A point on that trace is given
 * by sweeping a parameter t from -90 to +90; since the standard geological
 * rake (or pitch) R of a lineation on the plane runs from 0 at the strike
 * azimuth, through 90 at the down-dip direction, to 180 at the opposite end
 * of the strike, the two parameters are simply related by t = 90-R.
 *
 * Author:	Federico Esteban
 * Date:	22-AUG-2026
 * Version:	6 API
 *
 * Based on the recipe worked out by Rom1, Joaquim Luis and Federico Esteban in
 * https://forum.generic-mapping-tools.org/t/plot-a-schmidt-stereonet/6067
 */

#include "gmt_dev.h"
#include "longopt/psstereonet_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"psstereonet"
#define THIS_MODULE_MODERN_NAME	"stereonet"
#define THIS_MODULE_LIB		"geology"
#define THIS_MODULE_PURPOSE	"Plot structural geology data on a stereonet"
#define THIS_MODULE_KEYS	"<D{,>X},>DM,CC("
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYbdefghipqstxy"

/* What the two input angles mean and hence what we can draw */
enum psstereonet_types {
	PSSTEREONET_PLANE = 0,	/* -Tp: azimuth is the strike (right-hand rule) */
	PSSTEREONET_DIPDIR,	/* -Td: azimuth is the dip direction */
	PSSTEREONET_LINE};	/* -Tl: azimuth is the trend and dip is the plunge */

/* Which of the derived data sets -M should write */
enum psstereonet_dumps {
	PSSTEREONET_DUMP_AUTO = 0,
	PSSTEREONET_DUMP_TRACE,	/* -Mc: the cyclographic traces (great circles) */
	PSSTEREONET_DUMP_POINT};	/* -Mp: the poles, or the lines if -Tl */

#define PSSTEREONET_N_TRACE	181	/* Points used to draw one great circle (i.e., 1 degree steps) */
#define PSSTEREONET_DEF_WIDTH	15.0	/* Default width (diameter) of the net, in cm */
#define PSSTEREONET_DEF_ANNOT	30.0	/* Default azimuth annotation interval */
#define PSSTEREONET_DEF_TICK	10.0	/* Default azimuth tick interval */
#define PSSTEREONET_DEF_SYMBOL	"c0.15c"	/* Default symbol for the poles and lines */
#define PSSTEREONET_DEF_PEN	"default"	/* Default pen for traces and symbol outlines */
#define PSSTEREONET_DEF_FRAME	2		/* Number of default -B options we may add */

struct PSSTEREONET_CTRL {
	struct PSSTEREONET_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct PSSTEREONET_A {	/* -A[<annot>[/<tick>]] */
		bool active;
		bool draw;	/* False if -A0 was given */
		double annot, tick;
	} A;
	struct PSSTEREONET_C {	/* -C<cpt> */
		bool active;
		char *string;	/* Since we will simply pass this on to plot */
	} C;
	struct PSSTEREONET_G {	/* -G<fill> */
		bool active;
		char *string;	/* Since we will simply pass this on to plot */
	} G;
	struct PSSTEREONET_L {	/* -L<pen> */
		bool active;
		char *string;	/* Since we will simply pass this on to plot */
	} L;
	struct PSSTEREONET_M {	/* -M[c|p] */
		bool active;
		unsigned int mode;
	} M;
	struct PSSTEREONET_S {	/* -S<symbol>[<size>] */
		bool active;
		char *string;	/* Since we will simply pass this on to plot */
	} S;
	struct PSSTEREONET_T {	/* -T[d|l|p][+r][+u] */
		bool active;
		bool upper;	/* True if +u, i.e., plot on the upper hemisphere */
		bool rake;	/* True if +r, i.e., a third column gives the rake of a lineation on the plane */
		unsigned int mode;
	} T;
	struct PSSTEREONET_W {	/* -W<pen> */
		bool active;
		char *string;	/* Since we will simply pass this on to plot */
	} W;
	struct PSSTEREONET_l {	/* -l<label>[<modifiers>], may be repeated once */
		unsigned int n;
		char *string[2];	/* Since we will simply pass these on to plot */
	} l;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSTEREONET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSSTEREONET_CTRL);

	C->A.draw = false;	/* Only annotate the azimuth ring if -A was actually given */
	C->A.annot = PSSTEREONET_DEF_ANNOT;
	C->A.tick  = PSSTEREONET_DEF_TICK;

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSSTEREONET_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->C.string);
	gmt_M_str_free (C->G.string);
	gmt_M_str_free (C->L.string);
	gmt_M_str_free (C->S.string);
	gmt_M_str_free (C->W.string);
	gmt_M_str_free (C->l.string[0]);
	gmt_M_str_free (C->l.string[1]);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psstereonet synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] [-JA|S<width>] [-A[<annot>[/<tick>]]] [%s] "
		"[-G<fill>] %s[-L<pen>] %s%s[-S<symbol>[<size>]] [-T[d|l|p][+u]] [%s] [%s] "
		"[-W<pen>] [%s] [%s] [%s] %s[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_U_OPT, GMT_V_OPT,
		GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT,
		GMT_h_OPT, GMT_i_OPT, GMT_l_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_s_OPT, GMT_t_OPT, GMT_colon_OPT,
		GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<table>");
	GMT_Usage (API, -2, "One or more data files with two columns holding a pair of angles in degrees. What "
		"the two angles mean is set by -T [the strike and dip of planes]. If no files are given we read "
		"standard input.");

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-JA|S<width>");
	GMT_Usage (API, -2, "A stereonet is just a hemisphere seen from above, so select one of the two "
		"azimuthal projections whose default 90-degree horizon is a full hemisphere:");
	GMT_Usage (API, 3, "A: Lambert azimuthal equal-area, i.e., a Schmidt net [equal-area; Default].");
	GMT_Usage (API, 3, "S: Stereographic, i.e., a Wulff net [equal-angle].");
	GMT_Usage (API, -2, "Note: <width> is the diameter of the net [-JA%gc]. The net is always centered on "
		"0/0, so give the width only, without a center.", PSSTEREONET_DEF_WIDTH);
	GMT_Usage (API, 1, "\n-A[<annot>[/<tick>]]");
	GMT_Usage (API, -2, "Annotate the azimuth around the perimeter of the net every <annot> degrees, with "
		"ticks every <tick> degrees [%g/%g if -A is given with no argument]. Without -A no azimuth ring "
		"is drawn at all; -A0 is the same as omitting -A.",
		PSSTEREONET_DEF_ANNOT, PSSTEREONET_DEF_TICK);
	GMT_Option (API, "B-");
	GMT_Usage (API, -2, "Note: The gridlines requested via -B are the net itself: the meridians are the "
		"cyclographic traces of planes striking N-S with dips in steps of the grid interval, while the "
		"parallels are the small circles of constant plunge. Since the net has no meaningful longitude "
		"or latitude annotations you will normally only ask for gridlines. Without -B no frame at all is "
		"drawn, not even the perimeter of the net; give a bare -B for the classic two-level mesh "
		"[-Bpg10 -Bsg30].");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify a fill for the symbols.");
	GMT_Usage (API, 1, "\n-L<pen>");
	GMT_Usage (API, -2, "Set the pen used to outline the symbols [%s].", PSSTEREONET_DEF_PEN);
	GMT_Usage (API, 1, "\n-S<symbol>[<size>]");
	GMT_Usage (API, -2, "Plot the pole to each plane (or the line itself if -Tl) with this symbol; see the "
		"plot module for the available symbol codes [%s]. Without -S no symbols are plotted for planes.",
		PSSTEREONET_DEF_SYMBOL);
	GMT_Usage (API, 1, "\n-T[d|l|p][+u]");
	GMT_Usage (API, -2, "Select what the two input angles mean:");
	GMT_Usage (API, 3, "d: Planes given as dip direction and dip.");
	GMT_Usage (API, 3, "l: Lines given as trend and plunge.");
	GMT_Usage (API, 3, "p: Planes given as strike and dip, with the strike following the right-hand rule, "
		"i.e., the plane dips to the right of the strike direction [Default].");
	GMT_Usage (API, -2, "Optionally, append modifier:");
	GMT_Usage (API, 3, "+u Plot the data on the upper hemisphere [Default is the lower hemisphere].");
	GMT_Usage (API, 1, "\n-W<pen>");
	GMT_Usage (API, -2, "Set the pen used to draw the cyclographic traces (great circles) of the planes "
		"[%s]. Ignored if -Tl since a line has no trace.", PSSTEREONET_DEF_PEN);
	GMT_Option (API, "U,V,X,bi2,c,di,e,f,g,h,i");
	GMT_Usage (API, 1, "\n%s", GMT_l_OPT);
	GMT_Usage (API, -2, "Add a legend entry for the item drawn. Repeat the option to label both the "
		"cyclographic traces (first) and the symbols (second).");
	GMT_Option (API, "p,qi,s,t,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct PSSTEREONET_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psstereonet and sets parameters in Ctrl.
	 * Note there are no error checks here, just parsing errors.
	 */

	unsigned int n_errors = 0;
	int n;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files after checking they exist */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;
			case '>':	/* Got named output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Out.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->Out.file));
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Azimuth annotations around the perimeter */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.draw = true;	/* -A was given, so annotate using the default or given intervals */
				if (opt->arg[0] == '\0') break;	/* Just use the default intervals */
				if ((n = sscanf (opt->arg, "%lg/%lg", &Ctrl->A.annot, &Ctrl->A.tick)) < 1) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -A: Unable to parse <annot>[/<tick>] from %s\n", opt->arg);
					n_errors++;
				}
				else if (Ctrl->A.annot < 0.0 || Ctrl->A.tick < 0.0) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -A: The intervals cannot be negative\n");
					n_errors++;
				}
				else if (gmt_M_is_zero (Ctrl->A.annot))	/* -A0 means skip the azimuth ring */
					Ctrl->A.draw = false;
				else if (n == 1)	/* No tick interval given, so use a third of the annotation interval */
					Ctrl->A.tick = Ctrl->A.annot / 3.0;
				break;
			case 'C':	/* Use CPT for coloring symbols and traces */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				gmt_M_str_free (Ctrl->C.string);
				if (opt->arg[0]) Ctrl->C.string = strdup (opt->arg);
				break;
			case 'G':	/* Symbol fill */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &Ctrl->G.string);
				break;
			case 'L':	/* Pen used to outline the symbols */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &Ctrl->L.string);
				break;
			case 'M':	/* Dump the converted coordinates instead of plotting */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				switch (opt->arg[0]) {
					case '\0': Ctrl->M.mode = PSSTEREONET_DUMP_AUTO;  break;
					case 'c':  Ctrl->M.mode = PSSTEREONET_DUMP_TRACE; break;
					case 'p':  Ctrl->M.mode = PSSTEREONET_DUMP_POINT; break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -M: Unrecognized directive %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'S':	/* Symbol to plot at the poles or lines */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &Ctrl->S.string);
				break;
			case 'T':	/* What the two input angles mean */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				if ((c = gmt_first_modifier (GMT, opt->arg, "ru"))) {	/* Got the +r and/or +u modifiers */
					if (gmt_get_modifier (c, 'r', NULL)) Ctrl->T.rake = true;
					if (gmt_get_modifier (c, 'u', NULL)) Ctrl->T.upper = true;
					c[0] = '\0';	/* Temporarily chop off the modifiers */
				}
				switch (opt->arg[0]) {
					case '\0': case 'p': Ctrl->T.mode = PSSTEREONET_PLANE;  break;
					case 'd': Ctrl->T.mode = PSSTEREONET_DIPDIR; break;
					case 'l': Ctrl->T.mode = PSSTEREONET_LINE;   break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -T: Unrecognized directive %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (c) c[0] = '+';	/* Restore the modifier */
				break;
			case 'W':	/* Pen used to draw the cyclographic traces */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &Ctrl->W.string);
				break;
			case 'l':	/* Legend entry; we intercept it here so that we can pass it on to plot */
				if (Ctrl->l.n == 2) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -l: Given more than twice\n");
					n_errors++;
				}
				else
					Ctrl->l.string[Ctrl->l.n++] = strdup (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.string));

	if (!Ctrl->M.active) {	/* Need a proper stereonet setup for anything but dumping */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify -JA<width> or -JS<width>\n");
		if (GMT->common.J.active) {
			if (!(GMT->current.proj.projection == GMT_LAMB_AZ_EQ || GMT->current.proj.projection == GMT_STEREO)) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -J: Only -JA (Schmidt net) or -JS (Wulff net) are available for this module\n");
				n_errors++;
			}
			else if (!(gmt_M_is_zero (GMT->current.proj.pars[0]) && gmt_M_is_zero (GMT->current.proj.pars[1]))) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -J: A stereonet must be centered on 0/0\n");
				n_errors++;
			}
		}
	}
	if (Ctrl->T.mode == PSSTEREONET_LINE) {	/* Lines have no cyclographic trace */
		n_errors += gmt_M_check_condition (GMT, Ctrl->M.mode == PSSTEREONET_DUMP_TRACE,
			"Option -Mc: Lines (-Tl) have no cyclographic trace\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.rake, "Option -T+r: Lines (-Tl) have no plane to measure a rake on\n");
		if (Ctrl->W.active) {	/* -W has nothing to draw here; use -L to outline the symbols instead */
			GMT_Report (API, GMT_MSG_WARNING, "Option -W: Lines (-Tl) have no cyclographic trace; ignored (use -L to outline the symbols)\n");
			Ctrl->W.active = false;
		}
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->S.string == NULL, "Option -S: Must specify a symbol\n");

	n_errors += gmt_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int psstereonet_prep_options (struct GMTAPI_CTRL *API, struct GMT_OPTION **options) {
	/* A stereonet is fully determined once the user has picked equal-area or equal-angle and a width, so
	 * we fill in the parts of -J and -R that the user should not have to think about, and we default to a
	 * grid-only frame since the net has no meaningful longitude or latitude annotations. */
	bool dump = (GMT_Find_Option (API, 'M', *options) != NULL);	/* If -M we are not plotting at all */
	char string[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;
	static char *frame[PSSTEREONET_DEF_FRAME] = {"pg10", "sg30"};
	unsigned int k;

	if ((opt = GMT_Find_Option (API, 'J', *options))) {	/* Expand the -JA|S<width> shorthand, if that is what we got */
		if (strchr ("AaSs", opt->arg[0])) {
			if (opt->arg[1] && !strchr (opt->arg, '/'))
				sprintf (string, "%c0/0/%s", opt->arg[0], &opt->arg[1]);
			else if (strchr (opt->arg, '/')) {	/* An explicit center was given; the net is always centered on 0/0 */
				GMT_Report (API, GMT_MSG_ERROR, "Option -J: A stereonet is always centered on 0/0; give the width "
					"only, e.g. -JA<width> or -JS<width>\n");
				return (GMT_PARSE_ERROR);
			}
			if (string[0] && GMT_Update_Option (API, opt, string)) return (GMT_PARSE_ERROR);
		}
	}
	else if (!dump) {	/* No -J given, so default to a Schmidt net */
		sprintf (string, "A0/0/%gc", PSSTEREONET_DEF_WIDTH);
		if ((opt = GMT_Make_Option (API, 'J', string)) == NULL) return (GMT_PARSE_ERROR);
		if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return (GMT_PARSE_ERROR);
	}
	if (dump) return (GMT_NOERROR);	/* The remaining settings only matter for a plot */

	if ((opt = GMT_Find_Option (API, 'R', *options))) {	/* A stereonet always covers a full hemisphere, so -R has no effect */
		GMT_Report (API, GMT_MSG_ERROR, "Option -R: A stereonet always covers a full hemisphere and cannot be restricted\n");
		return (GMT_PARSE_ERROR);
	}
	if ((opt = GMT_Make_Option (API, 'R', "g")) == NULL) return (GMT_PARSE_ERROR);
	if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return (GMT_PARSE_ERROR);

	if ((opt = GMT_Find_Option (API, 'B', *options)) != NULL && opt->arg[0] == '\0') {
		/* A bare -B was given, so lay down the classic two-level mesh of a stereonet */
		GMT_Delete_Option (API, opt, options);
		for (k = 0; k < PSSTEREONET_DEF_FRAME; k++) {
			if ((opt = GMT_Make_Option (API, 'B', frame[k])) == NULL) return (GMT_PARSE_ERROR);
			if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return (GMT_PARSE_ERROR);
		}
	}

	return (GMT_NOERROR);
}

GMT_LOCAL void psstereonet_geo_format (double annot, char *fmt, size_t len) {
	/* Build a FORMAT_GEO_MAP template with just enough decimal digits to show <annot> exactly,
	 * so that a fractional annotation interval (e.g., -A22.5) is not rounded to whole degrees. */
	unsigned int n = 0;
	double scaled = annot;
	while (n < 3 && fabs (scaled - rint (scaled)) > GMT_CONV6_LIMIT) {
		scaled *= 10.0;
		n++;
	}
	if (n == 0)
		snprintf (fmt, len, "+ddd");
	else
		snprintf (fmt, len, "+ddd.%.*s", n, "xxx");
}

GMT_LOCAL void psstereonet_add_option (char *cmd, size_t len, char option, char *arg) {
	/* Append -<option><arg> to the command we build for the plot module.  Arguments such as a legend
	 * label may contain spaces, so those must be passed on inside quotes to survive the option parser. */
	char item[GMT_LEN256] = {""};
	size_t used = strlen (cmd);
	if ((used + 1) >= len) return;	/* No room left, so nothing to do */
	if (arg && strchr (arg, ' '))
		snprintf (item, GMT_LEN256, " \"-%c%s\"", option, arg);
	else
		snprintf (item, GMT_LEN256, " -%c%s", option, (arg) ? arg : "");
	strncat (cmd, item, len - used - 1);
}

GMT_LOCAL void psstereonet_line_to_lonlat (double trend, double plunge, double *lon, double *lat) {
	/* Return the point where a line with the given trend and plunge (in degrees, positive downwards)
	 * pierces the hemisphere.  It sits an angular distance 90-plunge from the center of the net, in the
	 * direction of the trend, which for our 0/0-centered azimuthal projection means: */
	double x, y, z, sd, cd, st, ct;
	sincosd (90.0 - plunge, &sd, &cd);
	sincosd (trend, &st, &ct);
	x = cd;	y = sd * st;	z = sd * ct;
	*lon = d_atan2d (y, x);
	*lat = d_asind (z);	/* Safe since (x,y,z) is a unit vector */
}

GMT_LOCAL void psstereonet_plane_to_lonlat (double strike, double dip, double t, double *lon, double *lat) {
	/* Return the point at parameter t (which runs from -90 to +90) along the cyclographic trace of a
	 * plane with the given strike and dip.  A plane striking due north is the meridian lon = 90-dip, and
	 * a general strike is that meridian rotated by the strike about the axis that points at the center
	 * of the net, i.e., about the x-axis. */
	double x, y, z, yr, zr, sd, cd, st, ct, ss, cs;
	sincosd (90.0 - dip, &sd, &cd);
	sincosd (t, &st, &ct);
	sincosd (strike, &ss, &cs);
	x = ct * cd;	y = ct * sd;	z = st;
	yr =  y * cs + z * ss;
	zr = -y * ss + z * cs;
	*lon = d_atan2d (yr, x);
	*lat = d_asind (zr);	/* Safe since (x,yr,zr) is a unit vector */
}

GMT_LOCAL int psstereonet_convert (struct GMT_CTRL *GMT, struct PSSTEREONET_CTRL *Ctrl, struct GMT_DATASET *D,
                                 struct GMT_DATASET **Trace, struct GMT_DATASET **Point, struct GMT_DATASET **Zval) {
	/* Convert the (azimuth, dip) pairs in D to the great circles and points that we can plot on the net.
	 * Trace gets one segment per plane [NULL if -Tl], Point gets a single segment with one row per input
	 * record, and Zval gets the third column, if any, with one value per plane [for plot -Z]. */
	bool do_trace = (Ctrl->T.mode != PSSTEREONET_LINE);
	unsigned int zcol = 2 + (Ctrl->T.rake ? 1 : 0);	/* The rake, if present, shifts the z-column out by one */
	bool do_z = (Ctrl->C.active && D->n_columns > zcol);
	uint64_t tbl, seg, row, p, n = 0, dim[4] = {1, 1, 0, 2};
	double azimuth, dip, rake, strike = 0.0, trend, plunge, lon, lat;
	struct GMT_DATASEGMENT *S = NULL, *Sout = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	*Trace = *Point = *Zval = NULL;

	dim[GMT_SEG] = 1;	dim[GMT_ROW] = D->n_records;	dim[GMT_COL] = (do_z) ? 3 : 2;
	if ((*Point = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
		return (API->error);
	if (do_trace) {	/* One great circle per record */
		dim[GMT_SEG] = D->n_records;	dim[GMT_ROW] = PSSTEREONET_N_TRACE;	dim[GMT_COL] = 2;
		if ((*Trace = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
			return (API->error);
	}
	if (do_z) {	/* One z-value per great circle, as needed by plot -Z */
		dim[GMT_SEG] = 1;	dim[GMT_ROW] = D->n_records;	dim[GMT_COL] = 1;
		if ((*Zval = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
			return (API->error);
	}

	Sout = (*Point)->table[0]->segment[0];
	for (tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
			S = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
			for (row = 0; row < S->n_rows; row++, n++) {
				azimuth = S->data[GMT_X][row];
				dip = S->data[GMT_Y][row];
				/* A dip or plunge is 0-90 by definition.  An impossible value would silently project
				 * onto the far hemisphere, where it is clipped away, so we stop rather than hand back
				 * a figure that is quietly missing data.  Azimuths need no such check since they wrap. */
				if (dip < 0.0 || dip > 90.0) {
					GMT_Report (API, GMT_MSG_ERROR, "Record %" PRIu64 ": %s of %g is outside the 0-90 range\n",
						n, (Ctrl->T.mode == PSSTEREONET_LINE) ? "Plunge" : "Dip", dip);
					return (GMT_RUNTIME_ERROR);
				}
				/* The upper hemisphere is the lower one reflected through the center of the net */
				if (Ctrl->T.upper) azimuth += 180.0;
				if (Ctrl->T.mode == PSSTEREONET_LINE) {	/* The two angles are the trend and plunge of a line */
					trend = azimuth;	plunge = dip;
					psstereonet_line_to_lonlat (trend, plunge, &lon, &lat);
				}
				else {	/* A plane: get the right-hand-rule strike, then the trace and the pole (or the rake) */
					strike = (Ctrl->T.mode == PSSTEREONET_DIPDIR) ? azimuth - 90.0 : azimuth;
					for (p = 0; p < PSSTEREONET_N_TRACE; p++) {	/* Walk along the cyclographic trace */
						psstereonet_plane_to_lonlat (strike, dip, -90.0 + 180.0 * p / (PSSTEREONET_N_TRACE - 1), &lon, &lat);
						(*Trace)->table[0]->segment[n]->data[GMT_X][p] = lon;
						(*Trace)->table[0]->segment[n]->data[GMT_Y][p] = lat;
					}
					if (Ctrl->T.rake) {	/* Plot the lineation given by its rake instead of the pole; rake
					                     * runs 0-180 from the strike azimuth, through the down-dip direction
					                     * at 90, to the opposite end of the strike at 180 - i.e., t = 90-rake */
						rake = S->data[2][row];
						/* A negative rake is the common shorthand for measuring from the other end of
						 * the strike line, so fold it into our 0-180 range instead of rejecting it */
						if (rake < 0.0) rake += 180.0;
						if (rake < 0.0 || rake > 180.0) {
							GMT_Report (API, GMT_MSG_ERROR, "Record %" PRIu64 ": Rake of %g is outside the 0-180 range\n",
								n, S->data[2][row]);
							return (GMT_RUNTIME_ERROR);
						}
						psstereonet_plane_to_lonlat (strike, dip, 90.0 - rake, &lon, &lat);
					}
					else {	/* The pole plunges 90-dip in the direction opposite to the dip direction */
						trend = strike - 90.0;	plunge = 90.0 - dip;
						psstereonet_line_to_lonlat (trend, plunge, &lon, &lat);
					}
				}
				Sout->data[GMT_X][n] = lon;
				Sout->data[GMT_Y][n] = lat;
				if (do_z) {
					Sout->data[GMT_Z][n] = S->data[zcol][row];
					(*Zval)->table[0]->segment[0]->data[GMT_X][n] = S->data[zcol][row];
				}
			}
		}
	}
	gmt_set_dataset_minmax (GMT, *Point);
	if (do_trace) gmt_set_dataset_minmax (GMT, *Trace);
	if (do_z) gmt_set_dataset_minmax (GMT, *Zval);

	return (GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_psstereonet (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int n_l = 0;
	bool do_trace, do_point;

	char cmd[GMT_LEN1024] = {""}, vfile[GMT_VF_LEN] = {""}, zfile[GMT_VF_LEN] = {""};

	struct PSSTEREONET_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMT_DATASET *D = NULL, *Trace = NULL, *Point = NULL, *Zval = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	if ((error = psstereonet_prep_options (API, &options))) bailout (error);	/* Complete -J, -R and -B before the common parser sees them */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error);	/* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psstereonet main code ----------------------------*/

	/* Decide what to draw: -W asks for the cyclographic traces of the planes while -S, -G and -L ask for
	 * the poles (or the lines when -Tl).  If the user asked for neither we draw the one that makes sense. */
	do_point = Ctrl->S.active || Ctrl->G.active || Ctrl->L.active || Ctrl->T.mode == PSSTEREONET_LINE;
	do_trace = (Ctrl->T.mode != PSSTEREONET_LINE) && (Ctrl->W.active || !do_point);
	if (do_trace && Ctrl->W.string == NULL) Ctrl->W.string = strdup (PSSTEREONET_DEF_PEN);
	if (do_point && Ctrl->S.string == NULL) Ctrl->S.string = strdup (PSSTEREONET_DEF_SYMBOL);
	if (do_point && !(Ctrl->G.active || Ctrl->C.active || Ctrl->L.active))	/* Must give the symbol some paint */
		Ctrl->L.string = strdup (PSSTEREONET_DEF_PEN);

	GMT_Report (API, GMT_MSG_INFORMATION, "Expecting the %s\n", (Ctrl->T.mode == PSSTEREONET_LINE) ? "trend and plunge of lines" :
		((Ctrl->T.mode == PSSTEREONET_DIPDIR) ? "dip direction and dip of planes" : "strike and dip of planes"));

	if (GMT_Set_Columns (API, GMT_IN, 2 + (Ctrl->T.rake ? 1 : 0) + (Ctrl->C.active ? 1 : 0), GMT_COL_FIX_NO_TEXT) != GMT_NOERROR)
		Return (API->error);
	gmt_set_cartesian (GMT, GMT_IN);	/* The input angles are plain numbers, not longitudes and latitudes */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)	/* Register data input */
		Return (API->error);
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL)
		Return (API->error);
	if (D->n_records == 0)
		GMT_Report (API, GMT_MSG_WARNING, "No data records found; only the empty net will be drawn\n");
	else if ((error = psstereonet_convert (GMT, Ctrl, D, &Trace, &Point, &Zval)))
		Return (error);
	gmt_set_geographic (GMT, GMT_OUT);	/* The converted coordinates are longitudes and latitudes */

	if (Ctrl->M.active) {	/* Just write the converted coordinates and exit */
		unsigned int which = Ctrl->M.mode;
		struct GMT_DATASET *Out = NULL;
		if (which == PSSTEREONET_DUMP_AUTO) which = (Ctrl->T.mode == PSSTEREONET_LINE) ? PSSTEREONET_DUMP_POINT : PSSTEREONET_DUMP_TRACE;
		Out = (which == PSSTEREONET_DUMP_TRACE) ? Trace : Point;
		if (Out && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, (which == PSSTEREONET_DUMP_TRACE) ? GMT_IS_LINE : GMT_IS_POINT,
		                           GMT_WRITE_NORMAL, NULL, Ctrl->Out.file, Out) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to write the converted lon, lat data\n");
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	/* Here we are plotting a stereonet */

	/* The input-related common options only concern the angles we just read, so make sure they do not
	 * also get applied when plot reads back the coordinates we computed from them */
	gmt_disable_bghio_opts (GMT);	/* Suspends -b, -g, -h, -i and -o */
	GMT->common.e.active = GMT->common.s.active = GMT->common.q.active[GMT_IN] = false;

	if (gmt_map_setup (GMT, GMT->common.R.wesn))
		Return (GMT_PROJECTION_ERROR);

	if (gmt_plotinit (GMT, options) == NULL)
		Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_BEFORE);
	gmt_plotcanvas (GMT);		/* Fill canvas if requested */
	gmt_map_basemap (GMT);		/* Lay down the net itself, i.e., the gridlines */

	if (do_trace && Trace) {	/* Draw the cyclographic trace (great circle) of each plane */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|GMT_IS_REFERENCE, Trace, vfile) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create a virtual data set for the traces\n");
			Return (API->error);
		}
		snprintf (cmd, GMT_LEN1024, "-R%s -J%s -O -K %s", GMT->common.R.string, GMT->common.J.string, vfile);
		if (Ctrl->C.active && Zval) {	/* Color each trace via its z-value */
			char pen[GMT_LEN128] = {""};
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, Zval, zfile) == GMT_NOTSET) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a virtual data set for the z-values\n");
				Return (API->error);
			}
			snprintf (pen, GMT_LEN128, "%s+z", Ctrl->W.string);	/* Let the CPT set the color of the pen */
			psstereonet_add_option (cmd, GMT_LEN1024, 'W', pen);
			psstereonet_add_option (cmd, GMT_LEN1024, 'C', Ctrl->C.string);
			psstereonet_add_option (cmd, GMT_LEN1024, 'Z', zfile);
		}
		else
			psstereonet_add_option (cmd, GMT_LEN1024, 'W', Ctrl->W.string);
		if (Ctrl->l.n) psstereonet_add_option (cmd, GMT_LEN1024, 'l', Ctrl->l.string[n_l++]);
		if ((error = GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to plot the cyclographic traces\n");
			Return (API->error);
		}
		if (zfile[0] && GMT_Close_VirtualFile (API, zfile) != GMT_NOERROR)
			Return (API->error);
		if (GMT_Close_VirtualFile (API, vfile) != GMT_NOERROR)
			Return (API->error);
	}

	if (do_point && Point) {	/* Plot a symbol at each pole, or at each line if -Tl */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, Point, vfile) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create a virtual data set for the points\n");
			Return (API->error);
		}
		snprintf (cmd, GMT_LEN1024, "-R%s -J%s -O -K %s", GMT->common.R.string, GMT->common.J.string, vfile);
		psstereonet_add_option (cmd, GMT_LEN1024, 'S', Ctrl->S.string);
		if (Ctrl->C.active) psstereonet_add_option (cmd, GMT_LEN1024, 'C', Ctrl->C.string);
		if (Ctrl->G.active) psstereonet_add_option (cmd, GMT_LEN1024, 'G', Ctrl->G.string);
		if (Ctrl->L.string) psstereonet_add_option (cmd, GMT_LEN1024, 'W', Ctrl->L.string);
		if (n_l < Ctrl->l.n) psstereonet_add_option (cmd, GMT_LEN1024, 'l', Ctrl->l.string[n_l++]);
		if ((error = GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to plot the %s\n", (Ctrl->T.mode == PSSTEREONET_LINE) ? "lines" : "poles");
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, vfile) != GMT_NOERROR)
			Return (API->error);
	}

	gmt_map_basemap (GMT);	/* Draw the perimeter of the net */

	if (Ctrl->A.draw) {	/* Annotate azimuth around the perimeter via a matching polar basemap */
		/* MAP_FRAME_AXES must go back to auto so that the polar projection picks the outer arc to annotate,
		 * and FORMAT_GEO_MAP must use the 0-360 range so that azimuths are not reported as 0-180 west/east.
		 * The format itself must carry enough decimals for a fractional -A (e.g., -A22.5), or the annotation
		 * would round to the nearest whole degree even though the tick lands exactly on the given azimuth. */
		char geo_fmt[GMT_LEN16] = {""};
		psstereonet_geo_format (Ctrl->A.annot, geo_fmt, GMT_LEN16);
		snprintf (cmd, GMT_LEN1024, "-R0/360/0/1 -JP%gi+a -Bxa%gf%g -O -K --MAP_FRAME_AXES=auto --FORMAT_GEO_MAP=%s",
			GMT->current.map.width, Ctrl->A.annot, Ctrl->A.tick, geo_fmt);
		gmt_init_B (GMT);
		/* Forget our own two-level -B or the polar basemap would inherit a secondary axis with no intervals,
		 * and rewind the basemap order since we already used up both of our before/after passes above */
		GMT->common.B.active[GMT_PRIMARY] = GMT->common.B.active[GMT_SECONDARY] = false;
		GMT->current.map.frame.order = 0;
		if ((error = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to annotate the azimuth around the net\n");
			Return (API->error);
		}
	}

	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}

EXTERN_MSC int GMT_stereonet (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: stereonet\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psstereonet (V_API, mode, args);
}
