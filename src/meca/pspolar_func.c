/*--------------------------------------------------------------------
 *    $Id: pspolar_func.c,v 1.14 2011-06-30 08:45:18 guru Exp $ 
 *
 *    Copyright (c) 1996-2011 by G. Patau
 *    Distributed under the GNU Public Licence
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

#include "pslib.h"	/* to have pslib environment */
#include "gmt_meca.h"	/* to have gmt_meca supplements */

#define DEFAULT_POINTSIZE	0.005
#define DEFAULT_FONTSIZE	9.0
#define DEFAULT_OFFSET		3.0      /* In points */

/* Control structure for pspolar */

struct PSPOLAR_CTRL {
	struct C {	/* -C */
		GMT_LONG active;
		double lon, lat, size;
		struct GMT_PEN pen;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
		double lon, lat;
	} D;
 	struct E {	/* -E<fill> */
		GMT_LONG active;
		GMT_LONG outline;
		struct GMT_FILL fill;
		struct GMT_PEN pen;
	} E;
	struct F {	/* -F<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
		struct GMT_PEN pen;
	} F;
 	struct G {	/* -G<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
		struct GMT_PEN pen;
	} G;
	struct M {	/* -M */
		GMT_LONG active;
		double ech;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q only -h for Hypo71 */
		GMT_LONG active;
	} Q;
	struct S {	/* -r<fill> */
		GMT_LONG active;
		GMT_LONG symbol;
		char type;
		double scale, size;
		struct GMT_FILL fill;
	} S;
	struct S2 {	/* -r<fill> */
		GMT_LONG active;
		GMT_LONG symbol;
		GMT_LONG outline;
		GMT_LONG scolor;
		GMT_LONG vector;
		char type;
		double size;
		double width, length, head;
		double vector_shape;
		struct GMT_FILL fill;
	} S2;
	struct T {
		GMT_LONG active;
		double angle, fontsize;
		GMT_LONG form, justify;
		struct GMT_PEN pen;
 	} T;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} W;
};

void *New_pspolar_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSPOLAR_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSPOLAR_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

        C->E.pen = C->F.pen = C->G.pen  = GMT->current.setting.map_default_pen;
    
	C->C.size = DEFAULT_POINTSIZE;
	GMT_init_fill (GMT, &C->E.fill, 250.0 / 255.0, 250.0 / 255.0, 250.0 / 255.0);
	GMT_init_fill (GMT, &C->F.fill, -1.0, -1.0, -1.0); 
	GMT_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	GMT_init_fill (GMT, &C->S2.fill, -1.0, -1.0, -1.0); 
	C->T.justify = 5;
	C->T.fontsize = 12.0;
	return ((void *)C);
}

void Free_pspolar_Ctrl (struct GMT_CTRL *GMT, struct PSPOLAR_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

GMT_LONG GMT_pspolar_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the pspolar synopsis and optionally full usage information */

	GMT_message (GMT,"pspolar %s - Plot polarities on the inferior focal half-sphere on maps\n\n", GMT_VERSION);
	GMT_message (GMT,"usage: pspolar [<table>] %s %s\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t-D<longitude>/<latitude> -M<size>[i/c] -S<symbol><size>[i/c]\n");
	GMT_message (GMT, "\t[-A] [%s] [-C<longitude>/<latitude>[W<pen>][P<pointsize>]] [-E<fill>]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[-e[<pen>]] [-F<fill>] [-f[<pen>]] [-G<fill>] [-g[<pen>]] [-K] [-N] [-O] [-P]\n");
	GMT_message (GMT, "\t[-s<half-size>/[V[<v_width>/<h_length>/<h_width>/<shape>]][G<fill>][L]\n");
	GMT_message (GMT, "\t[-T[<labelinfo>]] [-t<pen>] [%s] [-V] [-W<pen>]\n", GMT_U_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_h_OPT, GMT_i_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_explain_options (GMT, "jR");
	GMT_message (GMT, "\t-D Set longitude/latitude.\n");
	GMT_message (GMT, "\t-M Set size of beach ball in %s.\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t-S Select symbol type and symbol size (in %s).  Choose between:\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t   st(a)r, (c)ircle, (d)iamond, (h)exagon, (i)nvtriangle\n");
	GMT_message (GMT, "\t   (p)oint, (s)quare, (t)riangle, and (x)cross.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<b");
	GMT_message (GMT, "\t-C Set new_longitude/new_latitude[W<pen>][Ppointsize].\n");
	GMT_message (GMT, "\t   A line will be plotted between both positions.\n");
	GMT_message (GMT, "\t   Default is width = 3, color = current pen and pointsize = 0.015i.\n");
	GMT_message (GMT, "\t-E Specify color symbol for station in extensive part.\n");
	GMT_message (GMT, "\t   Fill can be either <r/g/b> (each 0-255) for color \n");
	GMT_message (GMT, "\t   or <gray> (0-255) for gray-shade [0].\n");
	GMT_message (GMT, "\t   Default is light gray.\n");
	GMT_message (GMT, "\t-e Outline of station symbol in extensive part.\n");
	GMT_message (GMT, "\t   Default is current pen.\n");
	GMT_message (GMT, "\t-F Specify background color of beach ball. It can be\n");
	GMT_message (GMT, "\t   <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_message (GMT, "\t   [Default is no fill].\n");
	GMT_message (GMT, "\t-f Outline beach ball.  Add <pen attributes> if not current pen.\n");
	GMT_message (GMT, "\t-G Specify color symbol for station in compressive part. Fill can be either\n");
	GMT_message (GMT, "\t   Fill can be either <r/g/b> (each 0-255) for color\n");
	GMT_message (GMT, "\t   or <gray> (0-255) for gray-shade [0].\n");
	GMT_message (GMT, "\t   Add L[<pen>] to outline [Default is black].\n");
	GMT_message (GMT, "\t-g Outline of station symbol in compressive part.\n");
	GMT_message (GMT, "\t   Add <pen attributes> if not current pen.\n");
	GMT_message (GMT, "\t-h Use special format derived from HYPO71 output.\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-N Do Not skip/clip symbols that fall outside map border\n");
	GMT_message (GMT, "\t   [Default will ignore those outside].\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-s Plot S polarity azimuth.\n");
	GMT_message (GMT, "\t   Azimuth of S polarity is in last column.\n");
	GMT_message (GMT, "\t   It may be a vector (V option) or a segment. Give half-size in cm.\n");
	GMT_message (GMT, "\t   L option is for outline\n");
	GMT_message (GMT, "\t   -s<half-size>/[V[<v_width>/<h_length></h_width>/<shape>]][G<fill>][L]\n");
	GMT_message (GMT, "\t   Default definition of v is 0.075/0.3/0.25/1\n");
	GMT_message (GMT, "\t   Outline is current pen\n");
	GMT_message (GMT, "\t-T[<info about labal printing>] to write station code.\n");
	GMT_message (GMT, "\t   <angle/form/justify/fontsize in points>\n");
	GMT_message (GMT, "\t   [Default is 0.0/0/5/12].\n");
	GMT_message (GMT, "\t-t Set pen attributes to write station codes [default is current pen].\n");
	GMT_explain_options (GMT, "UV");
	GMT_message (GMT,  "\t-W Set pen attributes [%s].\n", GMT_putpen (GMT, GMT->current.setting.map_default_pen));
	GMT_explain_options (GMT, "Xchi.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_pspolar_parse (struct GMTAPI_CTRL *C, struct PSPOLAR_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pspolar and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n;
	char txt[GMT_TEXT_LEN64],txt_b[GMT_TEXT_LEN64],txt_c[GMT_TEXT_LEN64], txt_d[GMT_TEXT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				break;

			/* Processes program-specific parameters */

			case 'C':	/* New coordinates */
				Ctrl->C.active = TRUE;
				sscanf(opt->arg, "%lf/%lf", &Ctrl->C.lon, &Ctrl->C.lat);
				if (strchr (opt->arg, 'W'))  GMT_getpen (GMT, strchr (opt->arg+1, 'W')+1, &Ctrl->C.pen);
				if (strchr (opt->arg, 'P')) sscanf(strchr (opt->arg+1, 'P')+1, "%lf", &Ctrl->C.size);
				break;
			case 'D':	/* Coordinates */
				Ctrl->D.active = TRUE;
				sscanf (opt->arg, "%lf/%lf", &Ctrl->D.lon, &Ctrl->D.lat);
				break;
			case 'E':	/* Set color for station in extensive part */
				Ctrl->E.active = TRUE;
				GMT_getfill (GMT, opt->arg, &Ctrl->E.fill);
				break;
			case 'e':	/* Outline station symbol in extensive part */
				Ctrl->E.active = TRUE;
				if (strlen (opt->arg)) GMT_getpen (GMT, opt->arg, &Ctrl->E.pen);
				break;
			case 'F':	/* Set background color of beach ball */
				Ctrl->F.active = TRUE;
				GMT_getfill (GMT, opt->arg, &Ctrl->F.fill);
				break;
			case 'f':	/* Outline beach ball */
				Ctrl->F.active = TRUE;
				if (strlen (opt->arg))  GMT_getpen (GMT, opt->arg, &Ctrl->F.pen);
				break;
			case 'G':	/* Set color for station in compressive part */
				Ctrl->C.active = TRUE;
				GMT_getfill (GMT, opt->arg, &Ctrl->G.fill);
				break;
			case 'g':	/* Outline station symbol in compressive part */
				Ctrl->G.active = TRUE;
				if (strlen (opt->arg)) GMT_getpen (GMT, opt->arg, &Ctrl->G.pen);
				break;
			case 'Q':	/* Use HYPO71 format */
				Ctrl->Q.active = TRUE;
				break;
			case 'M':	/* Focal sphere size */
				Ctrl->M.active = TRUE;
				Ctrl->M.ech = GMT_to_inch (GMT, opt->arg);
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = TRUE;
				break;
			case 'S':	/* Get symbol [and size] */
				Ctrl->S.type = opt->arg[0];
				Ctrl->S.size = GMT_to_inch (GMT, &opt->arg[1]);
				Ctrl->S.active = TRUE;
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
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -S option: Unrecognized symbol type %c\n", Ctrl->S.type);
						break;
				}
				break;
			case 's':	/* Get S polarity */
				Ctrl->S2.active = TRUE;
				strcpy (txt, &opt->arg[1]);
				n=0; while (txt[n] && txt[n] != '/' && txt[n] != 'V' && txt[n] != 'G' && txt[n] != 'L') n++; txt[n]=0;
				Ctrl->S2.size = GMT_to_inch (GMT, txt);
				if (strchr (opt->arg, 'V')) {
					Ctrl->S2.vector = TRUE;
					strcpy (txt, strchr (opt->arg, 'V'));
					if (strncmp (txt,"VG",(size_t)2) == 0 || strncmp(txt,"VL",(size_t)2) == 0 || strlen (txt) == 1) {
						Ctrl->S2.width = 0.03; Ctrl->S2.length = 0.12; Ctrl->S2.head = 0.1; Ctrl->S2.vector_shape = GMT->current.setting.map_vector_shape;
						if (!GMT->current.setting.proj_length_unit) {
							Ctrl->S2.width = 0.075; Ctrl->S2.length = 0.3; Ctrl->S2.head = 0.25; Ctrl->S2.vector_shape = GMT->current.setting.map_vector_shape;
						}
					}
					else {
						sscanf (strchr (opt->arg, 'V')+1, "%[^/]/%[^/]/%[^/]/%s", txt, txt_b, txt_c, txt_d);
						Ctrl->S2.width = GMT_to_inch (GMT, txt);
						Ctrl->S2.length = GMT_to_inch (GMT, txt_b);
						Ctrl->S2.head = GMT_to_inch (GMT, txt_c);
						Ctrl->S2.vector_shape = atof(txt_d);
					}
				}
				if (strchr (opt->arg, 'G')) {
					GMT_getfill (GMT, strchr (opt->arg+1,'G')+1, &Ctrl->S2.fill);
					Ctrl->S2.scolor = TRUE;
				}
				if (strchr (opt->arg, 'L')) Ctrl->S2.outline = TRUE;
				break;
			case 'T':	/* Information about label printing */
				Ctrl->T.active = TRUE;
				if (strlen (opt->arg)) {
					sscanf (opt->arg, "%lf/%ld/%ld/%lf/", &Ctrl->T.angle, &Ctrl->T.form, &Ctrl->T.justify, &Ctrl->T.fontsize);
				}
				break;
			case 't':	/* Set color for station label */
				GMT_getpen (GMT, opt->arg, &Ctrl->T.pen);
				break;

			case 'W':	/* Set line attributes */
				Ctrl->W.active = TRUE;
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

#define Return(code) {Free_pspolar_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_pspolar (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG n, ix, iy, n_fields, error = FALSE, greenwich, old_is_world;
   
	double plot_x, plot_y, symbol_size2 = 0, plot_x0, plot_y0, azS = 0, si, co;
	double new_plot_x0, new_plot_y0, radius, azimut = 0, ih = 0, plongement = 0.0;

	char *line, col[4][GMT_TEXT_LEN64];
	char pol, stacode[GMT_TEXT_LEN64];

	struct PSPOLAR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;				/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_pspolar_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_pspolar_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pspolar", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJR:", "BHKOPUVXhixYyc>", options))) Return (error);
	Ctrl = (struct PSPOLAR_CTRL *)New_pspolar_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pspolar_parse (API, Ctrl, options))) Return (error);
 	PSL = GMT->PSL;		/* This module also needs PSL */
  
	/*---------------------------- This is the pspolar main code ----------------------------*/

	greenwich = (GMT->common.R.wesn[XLO] < 0.0 || GMT->common.R.wesn[XHI] <= 0.0);
    
	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	GMT_plotinit (GMT, options);
 	GMT_plotcanvas (GMT);	/* Fill canvas if requested */
   
	PSL_setfont (PSL, GMT->current.setting.font_annot[0].id);

	if (!Ctrl->N.active) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
    
	old_is_world = GMT->current.map.is_world;
  
	GMT->current.map.is_world = TRUE;
        
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
		if (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Point give by -D is outside map; no plotting occours.");
			Return (GMT_OK);
		};
	}

	GMT_setpen (GMT, &Ctrl->F.pen);
	GMT_setfill (GMT, &(Ctrl->F.fill), Ctrl->F.active);
	PSL_plotsymbol (PSL, plot_x0, plot_y0, &(Ctrl->M.ech), GMT_SYMBOL_CIRCLE);

	ix = (GMT->current.setting.io_lonlat_toggle[0]);	iy = 1 - ix;

	if ((error = GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */

	while ((n_fields = GMT_Get_Record (API, GMT_READ_TEXT, (void **)&line)) != EOF) {	/* Keep returning records until we have no more files */

 		if (GMT_REC_IS_ERROR (GMT)) Return (EXIT_FAILURE);
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */
 
		switch (Ctrl->Q.active) {
			case 0 :
				if (!Ctrl->S2.active)
					sscanf (line, "%s %lf %lf %c", stacode, &azimut, &ih, &pol);
				else {
					n = sscanf (line, "%s %lf %lf %c %lf", stacode, &azimut, &ih, &pol, &azS);
					if (n == 4) azS = -1.0;
				}
				break;
			case 1 :
				if (!Ctrl->S2.active) {
					sscanf (line, "%s %s %s %s %lf %lf %c", col[0], col[1], col[2], stacode, &azimut, &ih, col[3]);
					pol = col[3][2];
				}
				else {
					n = sscanf (line, "%s %s %s %s %lf %lf %c %lf", col[0], col[1], col[2], stacode, &azimut, &ih, col[3], &azS);
					pol = col[3][2];
					if (n == 7) azS = -1.0;
				}
				break;
		}

		if (strcmp (col[0], "000000")) {
			plongement = (ih - 90.0) * M_PI / 180.0;
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
		if (Ctrl->S.symbol == GMT_SYMBOL_DOT) symbol_size2 = DEFAULT_POINTSIZE;

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
				double dim[7];
				dim[0] = plot_x + Ctrl->S2.size*si; dim[1] = plot_y + Ctrl->S2.size*co;
				dim[2] = Ctrl->S2.width; dim[3] = Ctrl->S2.length; dim[4] = Ctrl->S2.head;
				dim[5] = GMT->current.setting.map_vector_shape; dim[6] = 0.0;
				GMT_setfill (GMT, &(Ctrl->S2.fill), Ctrl->S2.outline);
				PSL_plotsymbol (PSL, plot_x - Ctrl->S2.size*si, plot_y - Ctrl->S2.size*co, dim, PSL_VECTOR);
			}
			else { 
				if (Ctrl->S2.scolor) PSL_setcolor (PSL, Ctrl->S2.fill.rgb, PSL_IS_STROKE);
				else PSL_setcolor (PSL, Ctrl->W.pen.rgb, PSL_IS_STROKE);
				PSL_plotsegment (PSL, plot_x - Ctrl->S2.size*si, plot_y - Ctrl->S2.size*co, plot_x + Ctrl->S2.size*si, plot_y + Ctrl->S2.size*co); 
			}
		}
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
    
	GMT->current.map.is_world = old_is_world;
    
	if (!Ctrl->N.active) GMT_map_clip_off (GMT);

	if (Ctrl->W.pen.style) PSL_setdash (PSL, CNULL, 0);

	GMT_map_basemap (GMT);

	GMT_plotend (GMT);

	Return (GMT_OK);
}
