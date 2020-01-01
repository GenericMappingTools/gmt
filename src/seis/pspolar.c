/*--------------------------------------------------------------------
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
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
 * Version:	5
 * Roots:	heavily based on psxy.c; ported to GMT5 by P. Wessel
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"pspolar"
#define THIS_MODULE_MODERN_NAME	"polar"
#define THIS_MODULE_LIB		"seis"
#define THIS_MODULE_PURPOSE	"Plot polarities on the lower hemisphere of the focal sphere"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BHJKOPRUVXYdehit" GMT_OPT("c")

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
	struct M {	/* -M<scale>[+m<magnitude>] */
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
		struct GMT_SYMBOL S;
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

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSPOLAR_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSPOLAR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

        C->C.pen = C->E.pen = C->F.pen = C->G.pen = GMT->current.setting.map_default_pen;

	C->C.size = GMT_DOT_SIZE;
	gmt_init_fill (GMT, &C->E.fill, 250.0 / 255.0, 250.0 / 255.0, 250.0 / 255.0);
	gmt_init_fill (GMT, &C->F.fill, -1.0, -1.0, -1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	gmt_init_fill (GMT, &C->S2.fill, -1.0, -1.0, -1.0);
	C->T.justify = 5;
	C->T.fontsize = 12.0;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSPOLAR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pspolar synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s -D<lon>/<lat>\n", name, GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t-M<size>[c|i|p][+m<mag>] -S<symbol><size>[c|i|p] [-A] [%s]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<lon>/<lat>[+p<pen>][+s<pointsize>]] [-E<fill>] [-F<fill>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<fill>] %s[-N] %s%s[-Qe[<pen>]] [-Qf[<pen>]] [-Qg[<pen>]]\n", API->K_OPT, API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Qh] [-Qs<half-size>[+v<size>[+<specs>]] [-Qt<pen>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T[<labelinfo>]] [%s] [%s] [-W<pen>]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] %s[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set longitude/latitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Set size of beachball in %s. Append +m<mag> to specify its magnitude, and beachball size is <mag> / 5.0 * <size>.\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and symbol size (in %s).  Choose between:\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   st(a)r, (c)ircle, (d)iamond, (h)exagon, (i)nvtriangle\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (p)oint, (s)quare, (t)riangle, and (x)cross.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set new_longitude/new_latitude[+p<pen>][+s<pointsize>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A line will be plotted between both positions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is current pen and pointsize = 0.015i.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Specify color symbol for station in extensive part.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Fill can be either <r/g/b> (each 0-255) for color \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or <gray> (0-255) for gray-shade [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is light gray.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify background color of beach ball. It can be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify color symbol for station in compressive part.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Fill can be either <r/g/b> (each 0-255) for color\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or <gray> (0-255) for gray-shade [0].\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   s Plot S polarity azimuth: Append <half-size>[+v<size>[+<specs>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Azimuth of S polarity is in last column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Specify a vector (with +v modifier) [Default is segment line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Default definition of vector is +v0.3i+e+gblack if just +v is given.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t Set pen attributes to write station codes [default is current pen].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T [<info about label printing>] to write station code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <angle/form/justify/fontsize in points>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [Default is 0.0/0/5/12].\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE,  "\t-W Set pen attributes [%s].\n", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X,c,di,e,h,i,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int old_Q_parser (struct GMT_CTRL *GMT, char *arg, struct PSPOLAR_CTRL *Ctrl) {
	/* Deal with the old syntax: -Qs<half-size>/[V[<v_width/h_length/h_width/shape>]][G<r/g/b>][L] */
	char *c = NULL, *text = strdup (arg);	/* Work on a copy */
	unsigned int n_errors = 0;
	GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-QsV<v_width>/<h_length>/<h_width>/<shape> is deprecated; use -Qs+v<vecpar> instead.\n");
	if ((c = strchr (text, 'L'))) {	/* Found trailing L for outline */
		Ctrl->S2.outline = true;
		c[0] = '\0';	/* Chop off L */
	}
	if ((c = strchr (text, 'G'))) {	/* Found trailing G for fill */
		if (gmt_getfill (GMT, &c[1], &Ctrl->S2.fill)) {
			gmt_fill_syntax (GMT, ' ', "QsG", " ");
			n_errors++;
		}
		Ctrl->S2.scolor = true;
		c[0] = '\0';	/* Chop off G */
	}

	if ((c = strchr (text, 'V'))) {	/* Got the Vector specs */
		Ctrl->S2.vector = true;
		if (c[1] == '\0') {	/* Provided no size information - set defaults */
			Ctrl->S2.width = 0.03; Ctrl->S2.length = 0.12; Ctrl->S2.head = 0.1;
			Ctrl->S2.vector_shape = GMT->current.setting.map_vector_shape;
			if (!GMT->current.setting.proj_length_unit) {
				Ctrl->S2.width = 0.075; Ctrl->S2.length = 0.3; Ctrl->S2.head = 0.25;
				Ctrl->S2.vector_shape = GMT->current.setting.map_vector_shape;
			}
		}
		else {
			char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""};
			char txt_c[GMT_LEN64] = {""}, txt_d[GMT_LEN64] = {""};
			sscanf (&c[1], "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
			Ctrl->S2.width  = gmt_M_to_inch (GMT, txt_a);
			Ctrl->S2.length = gmt_M_to_inch (GMT, txt_b);
			Ctrl->S2.head   = gmt_M_to_inch (GMT, txt_c);
			Ctrl->S2.vector_shape = atof (txt_d);
		}
		Ctrl->S2.S.symbol = GMT_SYMBOL_VECTOR_V4;
	}
	gmt_M_str_free (text);
	return (n_errors);

}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSPOLAR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pspolar and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n;
	char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, *p = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* New coordinates */
				Ctrl->C.active = true;
				sscanf(opt->arg, "%lf/%lf", &Ctrl->C.lon, &Ctrl->C.lat);
				if ((p = strstr (opt->arg, "+p"))) {
					char *q = strstr (p, "+s");
					if (q) q[0] = '\0';	/* Chop off the +s modifier */
					if (gmt_getpen (GMT, &p[2], &Ctrl->C.pen)) {
						gmt_pen_syntax (GMT, 'C', NULL, "Line connecting new and old point [Default current pen]", 0);
						n_errors++;
					}
					if (q) q[0] = '+';	/* Restore the +s modifier */
				}
				else if ((p = strchr (opt->arg, 'W')) && gmt_getpen (GMT, &p[1], &Ctrl->C.pen)) {	/* Old syntax */
					gmt_pen_syntax (GMT, 'C', NULL, "Line connecting new and old point [Default current pen]", 0);
					n_errors++;
				}
				if ((p = strstr (opt->arg, "+s"))) {	/* Found +s<size>[unit] */
					char *q = strstr (p, "+p");
					if (q) q[0] = '\0';	/* Chop off the +p modifier */
					if ((Ctrl->C.size = gmt_M_to_inch (GMT, &p[2]))) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Could not decode pointsize %s\n", &p[1]);
						n_errors++;
					}
					if (q) q[0] = '+';	/* Restore the +p modifier */
				}
				else if ((p = strchr (opt->arg, 'P')) && sscanf (&p[1], "%lf", &Ctrl->C.size)) {	/* Old syntax */
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Could not decode pointsize %s\n", &p[1]);
					n_errors++;
				}
				break;
			case 'D':	/* Coordinates */
				Ctrl->D.active = true;
				sscanf (opt->arg, "%lf/%lf", &Ctrl->D.lon, &Ctrl->D.lat);
				break;
			case 'E':	/* Set color for station in extensive part */
				Ctrl->E.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->E.fill)) {
					gmt_fill_syntax (GMT, 'E', NULL, " ");
					n_errors++;
				}
				break;
			case 'F':	/* Set background color of beach ball */
				Ctrl->F.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->F.fill)) {
					gmt_fill_syntax (GMT, 'F', NULL, " ");
					n_errors++;
				}
				break;
			case 'G':	/* Set color for station in compressive part */
				Ctrl->C.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'Q':	/* Repeatable; Controls various symbol attributes  */
				Ctrl->Q.active = true;
				switch (opt->arg[0]) {
					case 'e':	/* Outline station symbol in extensive part */
						Ctrl->E.active = true;
						if (strlen (&opt->arg[1]) && gmt_getpen (GMT, &opt->arg[1], &Ctrl->E.pen)) {
							gmt_pen_syntax (GMT, ' ', "Qe", "Outline station symbol (extensive part) [Default current pen]", 0);
							n_errors++;
						}
						break;
					case 'f':	/* Outline beach ball */
						Ctrl->F.active = true;
						if (strlen (&opt->arg[1]) && gmt_getpen (GMT, &opt->arg[1], &Ctrl->F.pen)) {
							gmt_pen_syntax (GMT, ' ', "Qf", "Outline beach ball [Default current pen]", 0);
							n_errors++;
						}
						break;
					case 'g':	/* Outline station symbol in compressive part */
						Ctrl->G.active = true;
						if (strlen (&opt->arg[1]) && gmt_getpen (GMT, &opt->arg[1], &Ctrl->G.pen)) {
							gmt_pen_syntax (GMT, ' ', "Qg", "Outline station symbol (compressive part) [Default current pen]", 0);
							n_errors++;
						}
						break;
					case 'h':	/* Use HYPO71 format */
						Ctrl->H2.active = true;
						break;
					case 's':	/* Get S polarity */
						Ctrl->S2.active = true;
						p = strchr (opt->arg, '/');	/* Find the first slash */
						if (p) p[0] = '\0';	/* Temporarily remove the slash */
						Ctrl->S2.size = gmt_M_to_inch (GMT, opt->arg);
						if (p) p[0] = '/';	/* Restore the slash */
						if (p && p[0] == '/' && (strchr (opt->arg, 'V') || strchr (opt->arg, 'G') || strchr (opt->arg, 'L')))	/* Clearly got the old syntax */
							n_errors += old_Q_parser (GMT, &p[1], Ctrl);
						else {	/* New syntax: -Qs[+v[<size>][+parameters]] */
							char symbol = (gmt_M_is_geographic (GMT, GMT_IN)) ? '=' : 'v';	/* Type of vector */
							if ((p = strstr (opt->arg, "+v"))) {	/* Got vector specification +v<size>[+<attributes>] */
								if (p[2] == '\0') {	/* Nothing, use defaults */
									Ctrl->S2.S.size_x = 0.3;	/* Length of vector */
									n_errors += gmt_parse_vector (GMT, symbol, "+e+gblack", &Ctrl->S2.S);
								}
								else if (p[2] == '+') {	/* No size (use default), just attributes */
									Ctrl->S2.S.size_x = 0.3;	/* Length of vector */
									n_errors += gmt_parse_vector (GMT, symbol, &p[2], &Ctrl->S2.S);
								}
								else {	/* Size, plus possible attributes */
									n = sscanf (&p[2], "%[^+]%s", txt_a, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
									if (n == 1) txt_b[0] = '\0';	/* No modifiers were present, set txt_b to empty */
									Ctrl->S2.S.size_x = gmt_M_to_inch (GMT, txt_a);	/* Length of vector */
									n_errors += gmt_parse_vector (GMT, symbol, txt_b, &Ctrl->S2.S);
								}
							}
						}
						break;
					case 't':	/* Set color for station label */
						if (gmt_getpen (GMT, &opt->arg[1], &Ctrl->T.pen)) {
							gmt_pen_syntax (GMT, ' ', "Qt", "Station code symbol[Default current pen]", 0);
							n_errors++;
						}
						break;
				}
				break;
			case 'M':	/* Focal sphere size -M<scale>+m<magnitude>*/
				Ctrl->M.active = true;
				sscanf(opt->arg, "%[^+]%s", txt_a, txt_b);
				Ctrl->M.ech = gmt_M_to_inch (GMT, txt_a);
				if ((p = strstr (txt_b, "+m")) != NULL && p[2]) {
					/* if +m<mag> is used, the focal sphere size is <mag>/5.0 * <size> */
					double magnitude;
					sscanf ((p+2), "%lf", &magnitude);
					Ctrl->M.ech *= (magnitude / 5.0);
				}
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get symbol [and size] */
				Ctrl->S.type = opt->arg[0];
				Ctrl->S.size = gmt_M_to_inch (GMT, &opt->arg[1]);
				Ctrl->S.active = true;
				switch (Ctrl->S.type) {
					case 'a':
						Ctrl->S.symbol = PSL_STAR;
						break;
					case 'c':
						Ctrl->S.symbol = PSL_CIRCLE;
						break;
					case 'd':
						Ctrl->S.symbol = PSL_DIAMOND;
						break;
					case 'h':
						Ctrl->S.symbol = PSL_HEXAGON;
						break;
					case 'i':
						Ctrl->S.symbol = PSL_INVTRIANGLE;
						break;
					case 'p':
						Ctrl->S.symbol = PSL_DOT;
						break;
					case 's':
						Ctrl->S.symbol = PSL_SQUARE;
						break;
					case 't':
						Ctrl->S.symbol = PSL_TRIANGLE;
						break;
					case 'x':
						Ctrl->S.symbol = PSL_CROSS;
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
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;

		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.ech <= 0.0, "Syntax error: -M must specify a size\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active + Ctrl->M.active + Ctrl->S.active < 3, "Syntax error: -D, -M, -S must be set together\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_polar (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: polar\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_pspolar (V_API, mode, args);
}

int GMT_pspolar (void *V_API, int mode, void *args) {
	int n = 0, error = 0;
	bool old_is_world;

	double plot_x, plot_y, symbol_size2 = 0, plot_x0, plot_y0, azS = 0, si, co;
	double new_plot_x0, new_plot_y0, radius, azimut = 0, ih = 0, plongement = 0.0;

	char col[4][GMT_LEN64], pol, stacode[GMT_LEN64] = {""};

	struct GMT_RECORD *In = NULL;
	struct PSPOLAR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;				/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pspolar main code ----------------------------*/

	gmt_M_memset (col, GMT_LEN64*4, char);
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
 	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	PSL_setfont (PSL, GMT->current.setting.font_annot[GMT_PRIMARY].id);

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	old_is_world = GMT->current.map.is_world;

	GMT->current.map.is_world = true;

        gmt_geo_to_xy (GMT, Ctrl->D.lon, Ctrl->D.lat, &plot_x0, &plot_y0);
	if (Ctrl->C.active) {
		gmt_setpen (GMT, &Ctrl->C.pen);
		gmt_geo_to_xy (GMT, Ctrl->C.lon, Ctrl->C.lat, &new_plot_x0, &new_plot_y0);
		PSL_plotsymbol (PSL, plot_x0, plot_y0, &(Ctrl->C.size), PSL_CIRCLE);
		PSL_plotsegment (PSL, plot_x0, plot_y0, new_plot_x0, new_plot_y0);
		plot_x0 = new_plot_x0;
		plot_y0 = new_plot_y0;
	}
	if (Ctrl->N.active) {
		gmt_map_outside (GMT, Ctrl->D.lon, Ctrl->D.lat);
		if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Point given by -D is outside map; no plotting occurs.");
			Return (GMT_NOERROR);
		};
	}

	gmt_setpen (GMT, &Ctrl->F.pen);
	gmt_setfill (GMT, &(Ctrl->F.fill), Ctrl->F.active);
	PSL_plotsymbol (PSL, plot_x0, plot_y0, &(Ctrl->M.ech), PSL_CIRCLE);

	GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX);	/* Only text expected */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_TEXT, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */

		if (Ctrl->H2.active) {
			if (Ctrl->S2.active) {
				n = sscanf (In->text, "%s %s %s %s %lf %lf %c %lf", col[0], col[1], col[2], stacode, &azimut, &ih, col[3], &azS);
				pol = col[3][2];
				if (n == 7)
					azS = -1.0;
			}
			else { /* !Ctrl->S2.active */
				sscanf (In->text, "%s %s %s %s %lf %lf %c", col[0], col[1], col[2], stacode, &azimut, &ih, col[3]);
				pol = col[3][2];
			}
		}
		else { /* !Ctrl->H2.active */
			if (Ctrl->S2.active) {
				n = sscanf (In->text, "%s %lf %lf %c %lf", stacode, &azimut, &ih, &pol, &azS);
				if (n == 4)
					azS = -1.0;
			}
			else /* !Ctrl->S2.active */
				sscanf (In->text, "%s %lf %lf %c", stacode, &azimut, &ih, &pol);
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
		if (Ctrl->S.symbol == PSL_CROSS || Ctrl->S.symbol == PSL_DOT) PSL_setcolor (PSL, GMT->session.no_rgb, PSL_IS_STROKE);

		if (Ctrl->T.active) {
			gmt_setpen (GMT, &Ctrl->T.pen);
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
		if (Ctrl->S.symbol == PSL_DOT) symbol_size2 = GMT_DOT_SIZE;

		if (pol == 'u' || pol == 'U' || pol == 'c' || pol == 'C' || pol == '+') {
			gmt_setpen (GMT, &Ctrl->G.pen);
			gmt_setfill (GMT, &(Ctrl->G.fill), Ctrl->G.active);
			PSL_plotsymbol (PSL, plot_x, plot_y, &symbol_size2, Ctrl->S.symbol);
		}
		else if (pol == 'r' || pol == 'R' || pol == 'd' || pol == 'D' || pol == '-') {
			gmt_setpen (GMT, &Ctrl->E.pen);
			gmt_setfill (GMT, &(Ctrl->E.fill), Ctrl->E.active);
			PSL_plotsymbol (PSL, plot_x, plot_y, &symbol_size2, Ctrl->S.symbol);
		}
		else {
			gmt_setpen (GMT, &Ctrl->W.pen);
			PSL_plotsymbol (PSL, plot_x, plot_y, &symbol_size2, Ctrl->S.symbol);
		}
		if (Ctrl->S2.active && azS >= 0.0) {
			gmt_setpen (GMT, &Ctrl->W.pen);
			sincosd (azS, &si, &co);
			if (Ctrl->S2.vector) {
				double dim[PSL_MAX_DIMS];
				gmt_M_memset (dim, PSL_MAX_DIMS, double);
				dim[0] = plot_x + Ctrl->S2.size*si; dim[1] = plot_y + Ctrl->S2.size*co;
				dim[2] = Ctrl->S2.width; dim[3] = Ctrl->S2.length; dim[4] = Ctrl->S2.head;
				dim[5] = Ctrl->S2.vector_shape; dim[6] = PSL_VEC_END | PSL_VEC_FILL;
				if (Ctrl->S2.S.symbol == GMT_SYMBOL_VECTOR_V4)
					psl_vector_v4 (PSL, plot_x - Ctrl->S2.size*si, plot_y - Ctrl->S2.size*co, dim, Ctrl->S2.fill.rgb, Ctrl->S2.outline);
				else {	/* Modern vector */
					gmt_setfill (GMT, &(Ctrl->S2.fill), Ctrl->S2.outline);
					PSL_plotsymbol (PSL, plot_x - Ctrl->S2.size*si, plot_y - Ctrl->S2.size*co, dim, PSL_VECTOR);
				}
			}
			else {	/* Just draw a segment line */
				if (Ctrl->S2.scolor) PSL_setcolor (PSL, Ctrl->S2.fill.rgb, PSL_IS_STROKE);
				else PSL_setcolor (PSL, Ctrl->W.pen.rgb, PSL_IS_STROKE);
				PSL_plotsegment (PSL, plot_x - Ctrl->S2.size*si, plot_y - Ctrl->S2.size*co, plot_x + Ctrl->S2.size*si, plot_y + Ctrl->S2.size*co);
			}
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	GMT->current.map.is_world = old_is_world;

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	PSL_setdash (PSL, NULL, 0);

	gmt_map_basemap (GMT);

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
