/*--------------------------------------------------------------------
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*

psvelo will read <x,y> pairs (or <lon,lat>) from inputfile and
plot symbols on a map. Velocity ellipses, strain
crosses, or strain wedges, may be specified, some of which require
additional columns of data.  Only one symbol may be plotted at a time.
PostScript code is written to stdout.


 Author:	Kurt Feigl
 Date:		7 July 1998
 Version:	5
 Roots:		based on psxy.c
 Adapted to version 3.3 by Genevieve Patau (25 June 1999)
 Last modified : 18 February 2000.  Ported to GMT 5 by P. Wessel

*/

#include "gmt_dev.h"
#include "utilvelo.h"

#define THIS_MODULE_CLASSIC_NAME	"psvelo"
#define THIS_MODULE_MODERN_NAME	"velo"
#define THIS_MODULE_LIB		"geodesy"
#define THIS_MODULE_PURPOSE	"Plot velocity vectors, crosses, and wedges"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BHJKOPRUVXYdehiqt" GMT_OPT("c")

#define CINE 1
#define ANISO 2
#define WEDGE 3
#define CROSS 4

#define DEFAULT_FONTSIZE	9.0	/* In points */

#define READ_ELLIPSE	0
#define READ_ROTELLIPSE	1
#define READ_ANISOTROPY	2
#define READ_WEDGE	4
#define READ_CROSS	8

/* parameters for writing text */
#define ANGLE		0.0
#define FORM		0

/* Control structure for psvelo */

struct PSVELO_CTRL {
	struct A {	/* -A */
		bool active;
		struct GMT_SYMBOL S;
	} A;
	struct D {	/* -D */
		bool active;
		double scale;
	} D;
 	struct E {	/* -E<fill> */
		bool active;
		struct GMT_FILL fill;
	} E;
 	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct L {	/* -L */
		bool active;
	} L;
	struct N {	/* -N */
		bool active;
	} N;
	struct S {	/* -r<fill> */
		bool active;
		int symbol;
		unsigned int readmode;
		unsigned int n_cols;
		double scale, wedge_amp, conrad;
		double confidence;
		struct GMT_FILL fill;
		struct GMT_FONT font;
	} S;
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSVELO_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSVELO_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
	C->A.S.v.h_length = (float)C->A.S.size_x;	/* 9p */
	C->A.S.v.v_angle = 30.0f;
	C->A.S.v.status = PSL_VEC_END + PSL_VEC_FILL + PSL_VEC_OUTLINE;
	C->A.S.v.pen = GMT->current.setting.map_default_pen;
	if (gmt_M_compat_check (GMT, 4)) GMT->current.setting.map_vector_shape = 0.4;	/* Historical reasons */
	C->A.S.v.v_shape = (float)GMT->current.setting.map_vector_shape;
	C->D.scale = 1.0;
	gmt_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	C->S.wedge_amp = 1.e7;
	C->S.conrad = 1.0;
	C->S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	C->S.font.size = 9;
	C->W.pen = GMT->current.setting.map_default_pen;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSVELO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psvelo synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s [-A<vecpar>] [%s] [-D<sigscale>]\n", name, GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<fill>] %s[-L] [-N] %s%s[-S<symbol><args>[+f<font>]]\n", API->K_OPT, API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-V] [-W<pen>] [%s]\n", GMT_U_OPT, GMT_X_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] %s[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n", GMT_Y_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify arrow head attributes:\n");
	gmt_vector_syntax (API->GMT, 15);
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is %gp+gblack+p1p\n", VECTOR_HEAD_LENGTH);
	GMT_Message (API, GMT_TIME_NONE, "\t-D Multiply uncertainties by <sigscale>. (Se and Sw only)i\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set color used for uncertainty wedges in -Sw option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify color (for symbols/polygons) or pattern (for polygons). fill can be either\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1) <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2) p[or P]<iconsize>/<pattern> for predefined patterns (0-90).\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Draw line or symbol outline using the current pen (see -W).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and scale (plus optional font; see documentation). Choose between:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     e  Velocity ellipses: in X,Y,Vx,Vy,SigX,SigY,CorXY,name format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r  Velocity ellipses: in X,Y,Vx,Vy,a,b,theta,name format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     n  Anisotropy : in X,Y,Vx,Vy.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     w  Rotational wedges: in X,Y,Spin,Spinsig.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x  Strain crosses : in X,Y,Eps1,Eps2,Theta.\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE,  "\t-W Set pen attributes [%s].\n", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X,c,di,e,h,i,qi,t,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSVELO_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psvelo and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_set;
	int n;
	bool no_size_needed, got_A = false;
	char txt[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, symbol, *c = NULL;
	struct GMT_OPTION *opt = NULL;

	symbol = (gmt_M_is_geographic (GMT, GMT_IN)) ? '=' : 'v';	/* Type of vector */

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change size of arrow head */
				got_A = true;
				if (gmt_M_compat_check (GMT, 4) && (strchr (opt->arg, '/') && !strchr (opt->arg, '+'))) {	/* Old-style args */
					sscanf (opt->arg, "%[^/]/%[^/]/%s", txt, txt_b, txt_c);
					Ctrl->A.S.v.v_width = (float)gmt_M_to_inch (GMT, txt);
					Ctrl->A.S.v.h_length = (float)gmt_M_to_inch (GMT, txt_b);
					Ctrl->A.S.v.h_width = (float)gmt_M_to_inch (GMT, txt_c);
					Ctrl->A.S.v.v_angle = (float)atand (0.5 * Ctrl->A.S.v.h_width / Ctrl->A.S.v.h_length);
					Ctrl->A.S.v.status |= PSL_VEC_OUTLINE2;
					Ctrl->A.S.symbol = GMT_SYMBOL_VECTOR_V4;
				}
				else {
					if (opt->arg[0] == '+') {	/* No size (use default), just attributes */
						n_errors += gmt_parse_vector (GMT, symbol, opt->arg, &Ctrl->A.S);
					}
					else {	/* Size, plus possible attributes */
						n = sscanf (opt->arg, "%[^+]%s", txt, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
						if (n == 1) txt_b[0] = 0;	/* No modifiers present, set txt_b to empty */
						Ctrl->A.S.size_x = gmt_M_to_inch (GMT, txt);	/* Length of vector */
						n_errors += gmt_parse_vector (GMT, symbol, txt_b, &Ctrl->A.S);
					}
					Ctrl->A.S.symbol = PSL_VECTOR;
				}
				break;
			case 'D':	/* Rescale Sigmas */
				Ctrl->D.active = true;
				sscanf (opt->arg, "%lf",&Ctrl->D.scale);
				break;
			case 'E':	/* Set color for error ellipse  */
				if (gmt_getfill (GMT, opt->arg, &Ctrl->E.fill)) {
					gmt_fill_syntax (GMT, 'E', NULL, " ");
					n_errors++;
				}
				Ctrl->E.active = true;
				break;
			case 'G':	/* Set Gray shade for polygon */
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'L':	/* Draw the outline */
				Ctrl->L.active = true;
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get symbol [and size] */
 				txt_b[0] = '\0';
				if ((c = strstr (opt->arg, "+f"))) {	/* Gave font directly */
					n_errors += gmt_getfont (GMT, &c[2], &(Ctrl->S.font));
					c[0] = '\0';	/* Temporarily chop off the font specification */
				}
 				if (opt->arg[0] == 'e' || opt->arg[0] == 'r') {
					strncpy (txt, &opt->arg[1], GMT_LEN256);
					n = 0; while (txt[n] && txt[n] != '/') n++; txt[n] = 0;
					Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
					sscanf (strchr(&opt->arg[1],'/')+1, "%lf/%s", &Ctrl->S.confidence, txt_b);
					/* confidence scaling */
					Ctrl->S.conrad = sqrt (-2.0 * log (1.0 - Ctrl->S.confidence));
					if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
				}
				if (opt->arg[0] == 'n' || opt->arg[0] == 'x' ) Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[1]);
				if (opt->arg[0] == 'w' && strlen(opt->arg) > 3) {
					strncpy(txt, &opt->arg[1], GMT_LEN256);
					n=0; while (txt[n] && txt[n] != '/') n++; txt[n]=0;
					Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
					sscanf(strchr(&opt->arg[1],'/')+1, "%lf", &Ctrl->S.wedge_amp);
				}
				switch (opt->arg[0]) {
					case 'e':
						Ctrl->S.symbol = CINE;	Ctrl->S.n_cols = 7;
						Ctrl->S.readmode = READ_ELLIPSE;
						break;
					case 'r':
						Ctrl->S.symbol = CINE;	Ctrl->S.n_cols = 7;
						Ctrl->S.readmode = READ_ROTELLIPSE;
						break;
					case 'n':
						Ctrl->S.symbol = ANISO;	Ctrl->S.n_cols = 4;
						Ctrl->S.readmode = READ_ANISOTROPY;
						break;
					case 'w':
						Ctrl->S.symbol = WEDGE;	Ctrl->S.n_cols = 4;
						Ctrl->S.readmode = READ_WEDGE;
						break;
					case 'x':
						Ctrl->S.symbol = CROSS;	Ctrl->S.n_cols = 5;
						Ctrl->S.readmode = READ_CROSS;
						break;
					default:
						n_errors++;
						break;
				}
				if (c) c[0] = '+';	/* Restore font specification */
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				break;

			/* Illegal options */

		}
	}

	no_size_needed = (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_ROTELLIPSE || Ctrl->S.readmode == READ_ANISOTROPY || Ctrl->S.readmode == READ_CROSS || Ctrl->S.readmode == READ_WEDGE );
        /* Only one allowed */
	n_set = (Ctrl->S.readmode == READ_ELLIPSE) + (Ctrl->S.readmode == READ_ROTELLIPSE) + (Ctrl->S.readmode == READ_ANISOTROPY) + (Ctrl->S.readmode == READ_CROSS) + (Ctrl->S.readmode == READ_WEDGE);
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, n_set > 1, "Syntax error: Only one -S setting is allowed.\n");
	n_errors += gmt_M_check_condition (GMT, !no_size_needed && (Ctrl->S.symbol > 1 && Ctrl->S.scale <= 0.0), "Syntax error: Must specify symbol size.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && ! (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_WEDGE), "Syntax error: -D requires -Se|w.\n");

	if (!got_A && Ctrl->W.active) Ctrl->A.S.v.pen = Ctrl->W.pen;	/* Set vector pen to that given by -W  */
	if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2 && Ctrl->W.active) gmt_M_rgb_copy (Ctrl->A.S.v.pen.rgb, Ctrl->W.pen.rgb);	/* Set vector pen color from -W but not thickness */
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_velo (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: velo\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psvelo (V_API, mode, args);
}

int GMT_psvelo (void *V_API, int mode, void *args) {
	int ix = 0, iy = 1, n_rec = 0, justify;
	int des_ellipse = true, des_arrow = true, error = false;

	double plot_x, plot_y, vxy[2], plot_vx, plot_vy, length, s, dim[PSL_MAX_DIMS];
	double eps1 = 0.0, eps2 = 0.0, spin = 0.0, spinsig = 0.0, theta = 0.0, *in = NULL;
	double direction = 0, small_axis = 0, great_axis = 0, sigma_x, sigma_y, corr_xy;
	double t11 = 1.0, t12 = 0.0, t21 = 0.0, t22 = 1.0, hl, hw, vw, ssize, headpen_width = 0.0;

	char *station_name = NULL;

	struct GMT_RECORD *In = NULL;
	struct PSVELO_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psvelo main code ----------------------------*/

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	gmt_M_memset (dim, PSL_MAX_DIMS, double);
	gmt_setpen (GMT, &Ctrl->W.pen);
	PSL_setfont (PSL, GMT->current.setting.font_annot[GMT_PRIMARY].id);
	if (Ctrl->E.active) Ctrl->L.active = true;

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	gmt_init_vector_param (GMT, &Ctrl->A.S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);
	if (Ctrl->A.S.symbol == PSL_VECTOR) Ctrl->A.S.v.v_width = (float)(Ctrl->A.S.v.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);

	ix = (GMT->current.setting.io_lonlat_toggle[0]);	iy = 1 - ix;

	GMT_Set_Columns (API, GMT_IN, Ctrl->S.n_cols, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->S.readmode == READ_ELLIPSE || Ctrl->S.readmode == READ_ROTELLIPSE) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "psvelo: 2-D confidence interval and scaling factor %f %f\n", Ctrl->S.confidence, Ctrl->S.conrad);

	if (Ctrl->S.symbol == CINE || Ctrl->S.symbol == CROSS) {
		if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head outline pen specified separately */
			PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->A.S.v.pen.width, Ctrl->A.S.v.pen.style, Ctrl->A.S.v.pen.offset, Ctrl->A.S.v.pen.rgb);
			headpen_width = 0.5*Ctrl->A.S.v.pen.width;
		}
		else {	/* Reset to default pen */
			if (Ctrl->W.active) {	/* Vector head outline pen default is half that of stem pen */
				PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->W.pen.width, Ctrl->W.pen.style, Ctrl->W.pen.offset, Ctrl->W.pen.rgb);
				headpen_width = 0.5 * Ctrl->W.pen.width;
			}
		}
	}
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_MIXED, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			assert (In->text != NULL);						/* Should never get here */
		}

		in = In->data;
		station_name = In->text;

		/* Data record to process */

		n_rec++;

		if (Ctrl->S.readmode == READ_ELLIPSE) {
			vxy[ix] = in[2];
			vxy[iy] = in[3];
			sigma_x = in[4];
			sigma_y = in[5];
			corr_xy = in[6];
			/* rescale uncertainties if necessary */
			if (Ctrl->D.active) {
				sigma_x = Ctrl->D.scale * sigma_x;
				sigma_y = Ctrl->D.scale * sigma_y;
			}
			if (fabs (sigma_x) < EPSIL && fabs (sigma_y) < EPSIL)
				des_ellipse = false;
			else {
				des_ellipse = true;
				velo_ellipse_convert (sigma_x, sigma_y, corr_xy, Ctrl->S.conrad, &small_axis, &great_axis, &direction);

				/* convert to degrees */
				direction = direction * R2D;
			}
		}
		else if (Ctrl->S.readmode == READ_ROTELLIPSE) {
			vxy[ix] = in[2];
			vxy[iy] = in[3];
			great_axis = Ctrl->S.conrad*in[4];
			small_axis = Ctrl->S.conrad*in[5];
			direction = in[6];
			if (fabs (great_axis) < EPSIL && fabs (small_axis) < EPSIL)
				des_ellipse = false;
			else
				des_ellipse = true;
		}
		else if (Ctrl->S.readmode == READ_ANISOTROPY) {
			vxy[ix] = in[2];
			vxy[iy] = in[3];
		}
		else if (Ctrl->S.readmode == READ_CROSS) {
			eps1  = in[2];
			eps2  = in[3];
			theta = in[4];
		}
		else if (Ctrl->S.readmode == READ_WEDGE) {
			spin    = in[2];
			spinsig = in[3];
			if (Ctrl->D.active) spinsig = spinsig * Ctrl->D.scale;
		}

		if (!Ctrl->N.active) {
			gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
			if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
		}

		gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);

		switch (Ctrl->S.symbol) {
			case CINE:
				des_arrow = hypot (vxy[0], vxy[1]) < 1.e-8 ? false : true;
				velo_trace_arrow (GMT, in[GMT_X], in[GMT_Y], vxy[0], vxy[1], Ctrl->S.scale, &plot_x, &plot_y, &plot_vx, &plot_vy);
				velo_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
				if (des_ellipse) {
					if (Ctrl->E.active)
						velo_paint_ellipse (GMT, plot_vx, plot_vy, direction, great_axis, small_axis, Ctrl->S.scale,
							t11,t12,t21,t22, Ctrl->E.active, &Ctrl->E.fill, Ctrl->L.active);
					else
						velo_paint_ellipse (GMT, plot_vx, plot_vy, direction, great_axis, small_axis, Ctrl->S.scale,
							t11,t12,t21,t22, Ctrl->E.active, &Ctrl->G.fill, Ctrl->L.active);
				}
				if (des_arrow) {	/* verify that arrow is not ridiculously small */
					length = hypot (plot_x-plot_vx, plot_y-plot_vy);	/* Length of arrow */
					if (length < Ctrl->A.S.v.h_length && Ctrl->A.S.v.v_norm < 0.0)	/* No shrink requested yet head length exceeds total vector length */
						GMT_Report (API, GMT_MSG_VERBOSE, "Vector head length exceeds overall vector length near line %d. Consider adding +n<norm> to -A\n", n_rec);
					s = (length < Ctrl->A.S.v.v_norm) ? length / Ctrl->A.S.v.v_norm : 1.0;
					hw = s * Ctrl->A.S.v.h_width;
					hl = s * Ctrl->A.S.v.h_length;
					vw = s * Ctrl->A.S.v.v_width;
					if (vw < 2.0/PSL_DOTS_PER_INCH) vw = 2.0/PSL_DOTS_PER_INCH;	/* Minimum width set */
					if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) gmt_setpen (GMT, &Ctrl->A.S.v.pen);
					dim[0] = plot_vx, dim[1] = plot_vy;
					dim[2] = vw, dim[3] = hl, dim[4] = hw;
					dim[5] = Ctrl->A.S.v.v_shape;
					if (Ctrl->A.S.symbol == GMT_SYMBOL_VECTOR_V4) {
						double *this_rgb = NULL;
						if (Ctrl->G.active)
							this_rgb = Ctrl->G.fill.rgb;
						else
							this_rgb = GMT->session.no_rgb;
						if (Ctrl->L.active) gmt_setpen (GMT, &Ctrl->W.pen);
						psl_vector_v4 (PSL, plot_x, plot_y, dim, this_rgb, Ctrl->L.active);
					}
					else {
						dim[6] = (double)Ctrl->A.S.v.status;
						dim[7] = (double)Ctrl->A.S.v.v_kind[0];	dim[8] = (double)Ctrl->A.S.v.v_kind[1];
						dim[11] = (headpen_width > 0.0) ? headpen_width : 0.5 * Ctrl->W.pen.width;
						if (Ctrl->A.S.v.status & PSL_VEC_FILL2)
							gmt_setfill (GMT, &Ctrl->A.S.v.fill, Ctrl->L.active);
						else if (Ctrl->G.active)
							gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->L.active);
						PSL_plotsymbol (PSL, plot_x, plot_y, dim, PSL_VECTOR);
					}
					if (Ctrl->A.S.v.status & PSL_VEC_OUTLINE2) gmt_setpen (GMT, &Ctrl->W.pen);

					justify = plot_vx - plot_x > 0. ? PSL_MR : PSL_ML;
					if (Ctrl->S.font.size > 0.0 && station_name)	/* 1 inch = 2.54 cm */
						PSL_plottext (PSL, plot_x + (6 - justify) / 25.4 , plot_y, Ctrl->S.font.size, station_name, ANGLE, justify, FORM);
				}
				else {
					gmt_setfill (GMT, &Ctrl->G.fill, 1);
					ssize = GMT_DOT_SIZE;
					PSL_plotsymbol (PSL, plot_x, plot_y, &ssize, PSL_CIRCLE);
					justify = PSL_TC;
					if (Ctrl->S.font.size > 0.0 && station_name) {
						PSL_plottext (PSL, plot_x, plot_y - 1. / 25.4, Ctrl->S.font.size, station_name, ANGLE, justify, FORM);
					}
					/*  1 inch = 2.54 cm */
				}
				break;
			case ANISO:
				velo_trace_arrow (GMT, in[GMT_X], in[GMT_Y], vxy[0], vxy[1], Ctrl->S.scale, &plot_x, &plot_y, &plot_vx, &plot_vy);
				PSL_plotsegment (PSL, plot_x, plot_y, plot_vx, plot_vy);
				break;
			case CROSS:
				/* triangular arrowheads */
				velo_trace_cross (GMT, in[GMT_X],in[GMT_Y],eps1,eps2,theta,Ctrl->S.scale,Ctrl->A.S.v.v_width,Ctrl->A.S.v.h_length,
					Ctrl->A.S.v.h_width,0.1,Ctrl->L.active,&(Ctrl->W.pen));
				break;
			case WEDGE:
				PSL_comment (PSL, "begin wedge number %li", n_rec);
				gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);
				velo_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
				velo_paint_wedge (PSL, plot_x, plot_y, spin, spinsig, Ctrl->S.scale, Ctrl->S.wedge_amp, t11,t12,t21,t22,
					Ctrl->G.active, Ctrl->G.fill.rgb, Ctrl->E.active, Ctrl->E.fill.rgb, Ctrl->L.active);
				break;
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Number of records read: %li\n", n_rec);

	if (Ctrl->D.active)  GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Rescaling uncertainties by a factor of %f\n", Ctrl->D.scale);

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	PSL_setdash (PSL, NULL, 0);

	gmt_map_basemap (GMT);

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
