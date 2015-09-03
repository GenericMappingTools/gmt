/*--------------------------------------------------------------------
 *    $Id$ 
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * pspolar will read azimuth, take-off angle of seismic ray and polarities
 * in stations and plots polarities on the inferior focal half-sphere.
 * A variety of symbols may be specified.
 * Only one event may be plotted at a time.
 * PostScript code is written to stdout.
 *
 * Author:	Genevieve Patau
 * Date:	19-OCT-1995 (psprojstations)
 * Version:	4
 * Roots:	heavily based on psxy.c
 *
 */

#define THIS_MODULE_NAME	"pspolar"
#define THIS_MODULE_LIB		"meca"
#define THIS_MODULE_PURPOSE	"Plot polarities on the inferior focal half-sphere on maps"
#define THIS_MODULE_KEYS	"<DI,>XO,RG-"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>BHJKOPRUVXYcdhixy"

#define DEFAULT_FONTSIZE	9.0	/* In points */

/* Control structure for pspolar */

struct PSPOLAR_CTRL {
	struct C {	/* -C */
		bool active;
		double lon, lat, size;
		struct GMT_PEN pen;
	} C;
	struct D {	/* -D */
		bool active;
		double lon, lat;
	} D;
 	struct E {	/* -E<fill> */
		bool active;
		int outline;
		struct GMT_FILL fill;
		struct GMT_PEN pen;
	} E;
	struct F {	/* -F<fill> */
		bool active;
		struct GMT_FILL fill;
		struct GMT_PEN pen;
	} F;
 	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
		struct GMT_PEN pen;
	} G;
	struct M {	/* -M */
		bool active;
		double ech;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* Repeatable: -Q<mode>[<args>] for various symbol parameters */
		bool active;
	} Q;
	struct H2 {	/* -Qh for Hypo71 */
		bool active;
	} H2;
	struct S {	/* -r<fill> */
		bool active;
		int symbol;
		char type;
		double scale, size;
		struct GMT_FILL fill;
	} S;
	struct S2 {	/* -r<fill> */
		bool active;
		bool scolor;
		bool vector;
		int symbol;
		int outline;
		char type;
		double size;
		double width, length, head;
		double vector_shape;
		struct GMT_FILL fill;
	} S2;
	struct T {
		bool active;
		double angle, fontsize;
		int form, justify;
		struct GMT_PEN pen;
 	} T;
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
};

void *New_pspolar_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSPOLAR_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSPOLAR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

        C->E.pen = C->F.pen = C->G.pen  = GMT->current.setting.map_default_pen;
    
	C->C.size = GMT_DOT_SIZE;
	GMT_init_fill (GMT, &C->E.fill, 250.0 / 255.0, 250.0 / 255.0, 250.0 / 255.0);
	GMT_init_fill (GMT, &C->F.fill, -1.0, -1.0, -1.0); 
	GMT_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	GMT_init_fill (GMT, &C->S2.fill, -1.0, -1.0, -1.0); 
	C->T.justify = 5;
	C->T.fontsize = 12.0;
	return (C);
}

void Free_pspolar_Ctrl (struct GMT_CTRL *GMT, struct PSPOLAR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int GMT_pspolar_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the pspolar synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pspolar [<table>] %s %s -D<longitude>/<latitude>\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t-M<size>[i/c] -S<symbol><size>[i/c] [-A] [%s]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<longitude>/<latitude>[W<pen>][P<pointsize>]] [-E<fill>] [-F<fill>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<fill>] [-K] [-N] [-O] [-P] [-Qe[<pen>]] [-Qf[<pen>]] [-Qg[<pen>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Qh] [-Qs<half-size>/[V[<v_width>/<h_length>/<h_width>/<shape>]][G<fill>][L] [-Qt<pen>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T[<labelinfo>]] [%s] [%s] [-W<pen>]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_di_OPT, GMT_h_OPT, GMT_i_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "J-,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set longitude/latitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Set size of beach ball in %s.\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and symbol size (in %s).  Choose between:\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   st(a)r, (c)ircle, (d)iamond, (h)exagon, (i)nvtriangle\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (p)oint, (s)quare, (t)riangle, and (x)cross.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set new_longitude/new_latitude[W<pen>][Ppointsize].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A line will be plotted between both positions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is width = 3, color = current pen and pointsize = 0.015i.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Specify color symbol for station in extensive part.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Fill can be either <r/g/b> (each 0-255) for color \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or <gray> (0-255) for gray-shade [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is light gray.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify background color of beach ball. It can be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify color symbol for station in compressive part. Fill can be either\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Fill can be either <r/g/b> (each 0-255) for color\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or <gray> (0-255) for gray-shade [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add L[<pen>] to outline [Default is black].\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not skip/clip symbols that fall outside map border\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default will ignore those outside].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Sets various attributes of symbols depending on <mode>:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e Outline of station symbol in extensive part [Default is current pen].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f Outline beach ball.  Add <pen attributes> [Default is current pen].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g Outline of station symbol in compressive part.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Add <pen attributes> if not current pen.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   h Use special format derived from HYPO71 output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s Plot S polarity azimuth.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Azimuth of S polarity is in last column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     It may be a vector (V option) or a segment. Give half-size in cm.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     L option is for outline\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -s<half-size>/[V[<v_width>/<h_length></h_width>/<shape>]][G<fill>][L]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Default definition of v is 0.075/0.3/0.25/1\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Outline is current pen\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t Set pen attributes to write station codes [default is current pen].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T[<info about label printing>] to write station code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <angle/form/justify/fontsize in points>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is 0.0/0/5/12].\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE,  "\t-W Set pen attributes [%s].\n", GMT_putpen (API->GMT, API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X,c,di,h,i,.");

	return (EXIT_FAILURE);
}

int GMT_pspolar_parse (struct GMT_CTRL *GMT, struct PSPOLAR_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pspolar and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n;
	char txt[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, txt_c[GMT_LEN64] = {""}, txt_d[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* New coordinates */
				Ctrl->C.active = true;
				sscanf(opt->arg, "%lf/%lf", &Ctrl->C.lon, &Ctrl->C.lat);
				if (strchr (opt->arg, 'W'))  GMT_getpen (GMT, strchr (opt->arg+1, 'W')+1, &Ctrl->C.pen);
				if (strchr (opt->arg, 'P')) sscanf(strchr (opt->arg+1, 'P')+1, "%lf", &Ctrl->C.size);
				break;
			case 'D':	/* Coordinates */
				Ctrl->D.active = true;
				sscanf (opt->arg, "%lf/%lf", &Ctrl->D.lon, &Ctrl->D.lat);
				break;
			case 'E':	/* Set color for station in extensive part */
				Ctrl->E.active = true;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->E.fill)) {
					GMT_fill_syntax (GMT, 'E', " ");
					n_errors++;
				}
				break;
			case 'F':	/* Set background color of beach ball */
				Ctrl->F.active = true;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->F.fill)) {
					GMT_fill_syntax (GMT, 'F', " ");
					n_errors++;
				}
				break;
			case 'G':	/* Set color for station in compressive part */
				Ctrl->C.active = true;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'Q':	/* Repeatable; Controls various symbol attributes  */
				Ctrl->Q.active = true;
				switch (opt->arg[0]) {
					case 'e':	/* Outline station symbol in extensive part */
						Ctrl->E.active = true;
						if (strlen (&opt->arg[1])) GMT_getpen (GMT, &opt->arg[1], &Ctrl->E.pen);
						break;
					case 'f':	/* Outline beach ball */
						Ctrl->F.active = true;
						if (strlen (&opt->arg[1]))  GMT_getpen (GMT, &opt->arg[1], &Ctrl->F.pen);
						break;
					case 'g':	/* Outline station symbol in compressive part */
						Ctrl->G.active = true;
						if (strlen (&opt->arg[1])) GMT_getpen (GMT, &opt->arg[1], &Ctrl->G.pen);
						break;
					case 'h':	/* Use HYPO71 format */
						Ctrl->H2.active = true;
						break;
					case 's':	/* Get S polarity */
						Ctrl->S2.active = true;
						strncpy (txt, &opt->arg[2], GMT_LEN64);
						n=0; while (txt[n] && txt[n] != '/' && txt[n] != 'V' && txt[n] != 'G' && txt[n] != 'L') n++; txt[n]=0;
						Ctrl->S2.size = GMT_to_inch (GMT, txt);
						if (strchr (&opt->arg[1], 'V')) {
							Ctrl->S2.vector = true;
							strncpy (txt, strchr (&opt->arg[1], 'V'), GMT_LEN64);
							if (strncmp (txt,"VG",2U) == 0 || strncmp(txt,"VL",2U) == 0 || strlen (txt) == 1) {
								Ctrl->S2.width = 0.03; Ctrl->S2.length = 0.12; Ctrl->S2.head = 0.1; Ctrl->S2.vector_shape = GMT->current.setting.map_vector_shape;
								if (!GMT->current.setting.proj_length_unit) {
									Ctrl->S2.width = 0.075; Ctrl->S2.length = 0.3; Ctrl->S2.head = 0.25; Ctrl->S2.vector_shape = GMT->current.setting.map_vector_shape;
								}
							}
							else {
								sscanf (strchr (&opt->arg[1], 'V')+1, "%[^/]/%[^/]/%[^/]/%s", txt, txt_b, txt_c, txt_d);
								Ctrl->S2.width = GMT_to_inch (GMT, txt);
								Ctrl->S2.length = GMT_to_inch (GMT, txt_b);
								Ctrl->S2.head = GMT_to_inch (GMT, txt_c);
								Ctrl->S2.vector_shape = atof(txt_d);
							}
						}
						if (strchr (opt->arg, 'G')) {
							if (GMT_getfill (GMT, strchr (opt->arg+2,'G')+1, &Ctrl->S2.fill)) {
								GMT_fill_syntax (GMT, 's', " ");
								n_errors++;
							}
							Ctrl->S2.scolor = true;
						}
						if (strchr (&opt->arg[1], 'L')) Ctrl->S2.outline = true;
						break;
					case 't':	/* Set color for station label */
						GMT_getpen (GMT, &opt->arg[1], &Ctrl->T.pen);
						break;
				}
				break;
			case 'M':	/* Focal sphere size */
				Ctrl->M.active = true;
				Ctrl->M.ech = GMT_to_inch (GMT, opt->arg);
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get symbol [and size] */
				Ctrl->S.type = opt->arg[0];
				Ctrl->S.size = GMT_to_inch (GMT, &opt->arg[1]);
				Ctrl->S.active = true;
				switch (Ctrl->S.type) {
					case 'a':
						Ctrl->S.symbol = GMT_SYMBOL_STAR;
						break;
					case 'c':
						Ctrl->S.symbol = GMT_SYMBOL_CIRCLE;
						break;
					case 'd':
						Ctrl->S.symbol = GMT_SYMBOL_DIAMOND;
						break;
					case 'h':
						Ctrl->S.symbol = GMT_SYMBOL_HEXAGON;
						break;
					case 'i':
						Ctrl->S.symbol = GMT_SYMBOL_INVTRIANGLE;
						break;
					case 'p':
						Ctrl->S.symbol = GMT_SYMBOL_DOT;
						break;
					case 's':
						Ctrl->S.symbol = GMT_SYMBOL_SQUARE;
						break;
					case 't':
						Ctrl->S.symbol = GMT_SYMBOL_TRIANGLE;
						break;
					case 'x':
						Ctrl->S.symbol = GMT_SYMBOL_CROSS;
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Unrecognized symbol type %c\n", Ctrl->S.type);
						break;
				}
				break;
			case 'T':	/* Information about label printing */
				Ctrl->T.active = true;
				if (strlen (opt->arg)) {
					sscanf (opt->arg, "%lf/%d/%d/%lf/", &Ctrl->T.angle, &Ctrl->T.form, &Ctrl->T.justify, &Ctrl->T.fontsize);
				}
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg && GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;

		}
	}

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.ech <= 0.0, "Syntax error: -M must specify a size\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active + Ctrl->M.active + Ctrl->S.active < 3, "Syntax error: -D, -M, -S must be set together\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pspolar_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_pspolar (void *V_API, int mode, void *args)
{
	int k, n = 0, error = 0;
	bool old_is_world;
   
	double plot_x, plot_y, symbol_size2 = 0, plot_x0, plot_y0, azS = 0, si, co;
	double new_plot_x0, new_plot_y0, radius, azimut = 0, ih = 0, plongement = 0.0;

	char *line = NULL, col[4][GMT_LEN64];
	char pol, stacode[GMT_LEN64] = {""};

	struct PSPOLAR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;				/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pspolar_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pspolar_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pspolar_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pspolar_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pspolar_parse (GMT, Ctrl, options))) Return (error);
  
	/*---------------------------- This is the pspolar main code ----------------------------*/

	GMT_memset (col, GMT_LEN64*4, char);
	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	PSL = GMT_plotinit (GMT, options);
 	GMT_plotcanvas (GMT);	/* Fill canvas if requested */
   
	PSL_setfont (PSL, GMT->current.setting.font_annot[0].id);

	if (!Ctrl->N.active) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
    
	old_is_world = GMT->current.map.is_world;
  
	GMT->current.map.is_world = true;
        
        GMT_geo_to_xy (GMT, Ctrl->D.lon, Ctrl->D.lat, &plot_x0, &plot_y0);
	if (Ctrl->C.active) {
		GMT_setpen (GMT, &Ctrl->C.pen);
		GMT_geo_to_xy (GMT, Ctrl->C.lon, Ctrl->C.lat, &new_plot_x0, &new_plot_y0);
		PSL_plotsymbol (PSL, plot_x0, plot_y0, &(Ctrl->C.size), GMT_SYMBOL_CIRCLE);
		PSL_plotsegment (PSL, plot_x0, plot_y0, new_plot_x0, new_plot_y0);
		plot_x0 = new_plot_x0;
		plot_y0 = new_plot_y0;
	}
	if (Ctrl->N.active) {
		GMT_map_outside (GMT, Ctrl->D.lon, Ctrl->D.lat);
		if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Point give by -D is outside map; no plotting occours.");
			Return (GMT_OK);
		};
	}

	GMT_setpen (GMT, &Ctrl->F.pen);
	GMT_setfill (GMT, &(Ctrl->F.fill), Ctrl->F.active);
	PSL_plotsymbol (PSL, plot_x0, plot_y0, &(Ctrl->M.ech), GMT_SYMBOL_CIRCLE);

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		if ((line = GMT_Get_Record (API, GMT_READ_TEXT, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */

		for (k = 0; k < (int)strlen (line); k++) if (line[k] == ',') line[k] = ' ';	/* Replace commas with spaces */
		if (Ctrl->H2.active) {
			if (Ctrl->S2.active) {
				n = sscanf (line, "%s %s %s %s %lf %lf %c %lf", col[0], col[1], col[2], stacode, &azimut, &ih, col[3], &azS);
				pol = col[3][2];
				if (n == 7)
					azS = -1.0;
			}
			else { /* !Ctrl->S2.active */
				sscanf (line, "%s %s %s %s %lf %lf %c", col[0], col[1], col[2], stacode, &azimut, &ih, col[3]);
				pol = col[3][2];
			}
		}
		else { /* !Ctrl->H2.active */
			if (Ctrl->S2.active)
				n = sscanf (line, "%s %lf %lf %c %lf", stacode, &azimut, &ih, &pol, &azS);
				if (n == 4)
					azS = -1.0;
			else { /* !Ctrl->S2.active */
				sscanf (line, "%s %lf %lf %c", stacode, &azimut, &ih, &pol);
			}
		}

		if (strcmp (col[0], "000000")) {
			plongement = (ih - 90.0) * D2R;
			if (plongement  < 0.0) {
				plongement = -plongement;
				azimut += 180.0;
				symbol_size2 = Ctrl->S.size * 0.8;
			}
		}
                else
			symbol_size2 = Ctrl->S.size;
		radius = sqrt (1.0 - sin (plongement));
		if (radius >= 0.97) radius = 0.97;
		azimut += 180.0;
		sincosd (azimut, &si, &co);
		plot_x = radius * si * Ctrl->M.ech / 2.0 + plot_x0;
		plot_y = radius * co * Ctrl->M.ech / 2.0 + plot_y0;
		if (Ctrl->S.symbol == GMT_SYMBOL_CROSS || Ctrl->S.symbol == GMT_SYMBOL_DOT) PSL_setcolor (PSL, GMT->session.no_rgb, PSL_IS_STROKE);
            
		if (Ctrl->T.active) {
			GMT_setpen (GMT, &Ctrl->T.pen);
			switch (Ctrl->T.justify) {
				case PSL_TR:
					PSL_plottext (PSL, plot_x-symbol_size2-0.1, plot_y-symbol_size2-0.1, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_TR, Ctrl->T.form);
					break;
				case PSL_TC:
					PSL_plottext (PSL, plot_x, plot_y-symbol_size2-0.1, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_TC, Ctrl->T.form);
					break;
				case PSL_TL:
					PSL_plottext (PSL, plot_x+symbol_size2+0.1, plot_y-symbol_size2-0.1, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_TL, Ctrl->T.form);
					break;
				case PSL_MR:
					PSL_plottext (PSL, plot_x-symbol_size2-0.1, plot_y, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_MR, Ctrl->T.form);
					break;
				case PSL_MC:
					PSL_plottext (PSL, plot_x, plot_y, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_MC, Ctrl->T.form);
					break;
				case PSL_ML:
					PSL_plottext (PSL, plot_x+symbol_size2+0.1, plot_y, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_ML, Ctrl->T.form);
					break;
				case PSL_BR:
					PSL_plottext (PSL, plot_x-symbol_size2-0.1, plot_y+symbol_size2+0.1, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_BR, Ctrl->T.form);
					break;
				case PSL_BC:
					PSL_plottext (PSL, plot_x, plot_y+symbol_size2+0.1, Ctrl->T.fontsize, col[3], Ctrl->T.angle, PSL_BC, Ctrl->T.form);
					break;
				case PSL_BL:
					PSL_plottext (PSL, plot_x+symbol_size2+0.1, plot_y+symbol_size2+0.1, Ctrl->T.fontsize, stacode, Ctrl->T.angle, PSL_BL, Ctrl->T.form);
					break;
			}
		}
		if (Ctrl->S.symbol == GMT_SYMBOL_DOT) symbol_size2 = GMT_DOT_SIZE;

		if (pol == 'u' || pol == 'U' || pol == 'c' || pol == 'C' || pol == '+') {
			GMT_setpen (GMT, &Ctrl->G.pen);
			GMT_setfill (GMT, &(Ctrl->G.fill), Ctrl->G.active);
			PSL_plotsymbol (PSL, plot_x, plot_y, &symbol_size2, Ctrl->S.symbol);
		}
		else if (pol == 'r' || pol == 'R' || pol == 'd' || pol == 'D' || pol == '-') {
			GMT_setpen (GMT, &Ctrl->E.pen);
			GMT_setfill (GMT, &(Ctrl->E.fill), Ctrl->E.active);
			PSL_plotsymbol (PSL, plot_x, plot_y, &symbol_size2, Ctrl->S.symbol);
		}
		else {
			GMT_setpen (GMT, &Ctrl->W.pen);
			PSL_plotsymbol (PSL, plot_x, plot_y, &symbol_size2, Ctrl->S.symbol);
		}
		if (Ctrl->S2.active && azS >= 0.0) {
			GMT_setpen (GMT, &Ctrl->W.pen);
			sincosd (azS, &si, &co);
			if (Ctrl->S2.vector) {
				double dim[PSL_MAX_DIMS];
				GMT_memset (dim, PSL_MAX_DIMS, double);
				dim[0] = plot_x + Ctrl->S2.size*si; dim[1] = plot_y + Ctrl->S2.size*co;
				dim[2] = Ctrl->S2.width; dim[3] = Ctrl->S2.length; dim[4] = Ctrl->S2.head;
				dim[5] = GMT->current.setting.map_vector_shape; dim[6] = GMT_VEC_END | GMT_VEC_FILL;
				GMT_setfill (GMT, &(Ctrl->S2.fill), Ctrl->S2.outline);
				PSL_plotsymbol (PSL, plot_x - Ctrl->S2.size*si, plot_y - Ctrl->S2.size*co, dim, PSL_VECTOR);
			}
			else { 
				if (Ctrl->S2.scolor) PSL_setcolor (PSL, Ctrl->S2.fill.rgb, PSL_IS_STROKE);
				else PSL_setcolor (PSL, Ctrl->W.pen.rgb, PSL_IS_STROKE);
				PSL_plotsegment (PSL, plot_x - Ctrl->S2.size*si, plot_y - Ctrl->S2.size*co, plot_x + Ctrl->S2.size*si, plot_y + Ctrl->S2.size*co); 
			}
		}
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}
    
	GMT->current.map.is_world = old_is_world;
    
	if (!Ctrl->N.active) GMT_map_clip_off (GMT);

	if (Ctrl->W.pen.style) PSL_setdash (PSL, NULL, 0);

	GMT_map_basemap (GMT);

	GMT_plotend (GMT);

	Return (GMT_OK);
}
