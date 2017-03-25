/*--------------------------------------------------------------------
 *    $Id$
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*

psmeca will read <x,y> pairs (or <lon,lat>) from inputfile and
plot symbols on a map. Focal mechanisms are specified (double couple or
moment tensor).
PostScript code is written to stdout.


 Author:	Genevieve Patau
 Date:		7 July 1998
 Version:	5
 Roots:		based on psxy.c, ported to GMT 5 by P. Wessel
 */

#include "gmt_dev.h"
#include "meca.h"
#include "utilmeca.h"

#define THIS_MODULE_NAME	"psmeca"
#define THIS_MODULE_LIB		"meca"
#define THIS_MODULE_PURPOSE	"Plot focal mechanisms on maps"
#define THIS_MODULE_KEYS	"<T{,>X}"
#define THIS_MODULE_NEEDS	"dJ"
#define THIS_MODULE_OPTIONS "-:>BHJKOPRUVXYdhit" GMT_OPT("c")

#define DEFAULT_FONTSIZE	9.0	/* In points */
#define DEFAULT_OFFSET		3.0	/* In points */
#define DEFAULT_SIZE		6.0 /* In points */

#define READ_CMT	0
#define READ_AKI	1
#define READ_PLANES	2
#define READ_AXIS	4
#define READ_TENSOR	8

#define PLOT_DC		1
#define PLOT_AXIS	2
#define PLOT_TRACE	4

/* Control structure for psmeca */

struct PSMECA_CTRL {
	struct C {	/* -C[<pen>][P<pointsize>] */
		bool active;
		double size;
		struct GMT_PEN pen;
	} C;
 	struct D {	/* -D<min/max> */
		bool active;
		double depmin, depmax;
	} D;
 	struct E {	/* -E<fill> */
		bool active;
		struct GMT_FILL fill;
	} E;
 	struct F {	/* Repeatable -F<mode>[<args>] */
		bool active;
	} F;
 	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct L {	/* -L<pen> */
		bool active;
		struct GMT_PEN pen;
	} L;
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
	struct S {	/* -S<format><scale>[/fontsize[/justify/offset/angle/form]] */
		bool active;
		bool no_label;
		unsigned int readmode;
		unsigned int plotmode;
		unsigned int justify;
		double scale;
		double fontsize, offset;
		struct GMT_FILL fill;
	} S;
	struct T {	/* -Tnplane[/<pen>] */
		bool active;
		unsigned int n_plane;
		struct GMT_PEN pen;
	} T;
	struct Z2 {	/* -z<pen>] */
		bool active;
		struct GMT_PEN pen;
	} Z2;
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<cpt> */
		bool active;
		char *file;
	} Z;
	struct A2 {	/* -a[size][/Psymbol[Tsymbol]] */
		bool active;
		char P_sym_type, T_sym_type;
		char P_symbol, T_symbol;
		double size;
	} A2;
 	struct E2 {	/* -e<fill> */
		bool active;
		struct GMT_FILL fill;
	} E2;
 	struct G2 {	/* -g<fill> */
		bool active;
		struct GMT_FILL fill;
	} G2;
 	struct P2 {	/* -p[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} P2;
	struct R2 {	/* -r[<fill>] */
		bool active;
		struct GMT_FILL fill;
	} R2;
 	struct T2 {	/* -t[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} T2;
 	struct O2 {	/* -Fo */
		bool active;
	} O2;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSMECA_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSMECA_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->C.size = GMT_DOT_SIZE;
	C->C.pen = C->L.pen = C->T.pen = C->T2.pen = C->P2.pen = C->Z2.pen = C->W.pen = GMT->current.setting.map_default_pen;
	/* Set width temporarily to -1. This will indicate later that we need to replace by W.pen */
	C->C.pen.width = C->L.pen.width = C->T.pen.width = C->T2.pen.width = C->P2.pen.width = C->Z2.pen.width = -1.0;
	C->D.depmax = 900.0;
	C->L.active = true;
	gmt_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	gmt_init_fill (GMT, &C->R2.fill, 1.0, 1.0, 1.0);
	C->S.fontsize = DEFAULT_FONTSIZE;
	C->S.offset = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
	C->S.justify = PSL_BC;
	C->A2.size = DEFAULT_SIZE * GMT->session.u2u[GMT_PT][GMT_INCH];
	C->A2.P_symbol = C->A2.T_symbol = PSL_CIRCLE;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSMECA_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Z.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psmeca synopsis and optionally full usage information */

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psmeca [<table>] %s %s\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t-S<format><scale>[/<fontsize>[/<justify>/<offset>/<angle>/<form>]] [%s]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C[<pen>][P<pointsize>]] [-D<depmin>/<depmax>] [-E<fill>] [-G<fill>] [-K] [-L<pen>] [-M]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fa[<size>[/<Psymbol>[<Tsymbol>]]] [-Fe<fill>] [-Fg<fill>] [-Fo] [-Fr<fill>] [-Fp[<pen>]] [-Ft[<pen>]] [-Fz[<pen>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N] [-O] [-P] [-T<nplane>[/<pen>]] [%s] [%s] [-W<pen>]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-Z<cpt>] [-z[<pen>]]\n", GMT_X_OPT, GMT_Y_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_di_OPT, GMT_h_OPT, GMT_i_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Offset focal mechanisms to the latitude and longitude specified in the last two columns of the input file before label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default pen attributes are set by -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A line is plotted between both positions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A small circle is plotted at the initial location. Add P<pointsize> to change the size of the circle.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Plot events between <depmin> and <depmax> deep.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set color used for extensive parts [default is white].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Sets various attributes of symbols depending on <mode>:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a Plot axis. Default symbols are circles; otherwise append <size>[/<Psymbol>[<Tsymbol>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g Append color used for P_symbol [default as set by -G].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e Append color used for T_symbol [default as set by -E].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p Draw P_symbol outline using the default pen (see -W) or append pen attribute for outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t Draw T_symbol outline using the default pen (see -W) or append pen attribute for outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   o Use psvelomeca format (Without depth in third column).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r Draw box behind labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z Overlay zero trace moment tensor using default pen (see -W) or append outline pen.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set color used for compressive parts [default is black].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Sets pen attribute for outline other than the default set by -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Same size for any magnitude. Size is given with -S.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select format type and symbol size (in %s).\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose format between:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c  Focal mechanisms in Harvard CMT convention\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, strike1, dip1, rake1, strike2, dip2, rake2, moment, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      with moment in 2 columns : mantissa and exponent corresponding to seismic moment in dynes-cm\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a  Focal mechanism in Aki & Richard's convention:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, strike, dip, rake, mag, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p  Focal mechanism defined with:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, strike1, dip1, strike2, fault, mag, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      fault = -1/+1 for a normal/inverse fault\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m  Seismic moment tensor (Harvard CMT, with zero trace):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z  Anisotropic part of seismic moment tensor (Harvard CMT, with zero trace):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d  Best double couple defined from seismic moment tensor (Harvard CMT, with zero trace):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x  Principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, T_value, T_azim, T_plunge, N_value, N_azim, N_plunge\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      P_value, P_azim, P_plunge, exp, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t  Zero trace moment tensor defined from principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, T_value, T_azim, T_plunge, N_value, N_azim, N_plunge\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      P_value, P_azim, P_plunge, exp, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   y  Best double couple defined from principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      X, Y, depth, T_value, T_azim, T_plunge, N_value, N_azim, N_plunge\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      P_value, P_azim, P_plunge, exp, newX, newY, event_title\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Fo option for old (psvelomeca) format (no depth in third column).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally add /fontsize[/offset][u]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default values are /%g/%fp\n", DEFAULT_FONTSIZE, DEFAULT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t   fontsize < 0 : no label written;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   offset is from the limit of the beach ball.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default label is above the beach ball. Add u to plot it under.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Tn[/<pen>] Draw nodal planes and circumference only to provide a transparent\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   beach ball using the default pen (see -W) or sets pen attribute. \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 1 the only first nodal plane is plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 2 the only second nodal plane is plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 0 both nodal planes are plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If moment tensor is required, nodal planes overlay moment tensor.\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set pen attributes [%s].\n", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Use CPT to assign colors based on depth-value in 3rd column.\n");
	GMT_Option (API, "X,di,h,i,t,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSMECA_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psmeca and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	bool no_size_needed;
	char txt[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, *p = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Change position [set line attributes] */
				Ctrl->C.active = true;
				if (!opt->arg[0]) break;
				strncpy (txt, opt->arg, GMT_LEN256-1);
				if ((p = strchr (txt, 'P')) != NULL) Ctrl->C.size = gmt_M_to_inch (GMT, (p+1));
				if (txt[0] != 'P') {	/* Have a pen up front */
					if (p) p[0] = '\0';
					if (gmt_getpen (GMT, txt, &Ctrl->C.pen)) {
						gmt_pen_syntax (GMT, 'C', " ", 0);
						n_errors++;
					}
				}
				break;
			case 'D':	/* Plot events between depmin and depmax deep */
				Ctrl->D.active = true;
				sscanf (opt->arg, "%lf/%lf", &Ctrl->D.depmin, &Ctrl->D.depmax);
				break;
			case 'E':	/* Set color for extensive parts  */
				Ctrl->E.active = true;
				if (!opt->arg[0] || (opt->arg[0] && gmt_getfill (GMT, opt->arg, &Ctrl->E.fill))) {
					gmt_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'F':	/* Repeatable; Controls various symbol attributes  */
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'a':	/* plot axis */
						Ctrl->A2.active = true;
						strncpy (txt, &opt->arg[2], GMT_LEN256-1);
						if ((p = strchr (txt, '/')) != NULL) p[0] = '\0';
						if (txt[0]) Ctrl->A2.size = gmt_M_to_inch (GMT, txt);
						if (p) {	/* Also specified symbols */
							p++;
							switch (strlen (p)) {
								case 1:
									Ctrl->A2.P_symbol = Ctrl->A2.T_symbol = p[0];
									break;
								case 2:
									Ctrl->A2.P_symbol = p[0], Ctrl->A2.T_symbol = p[1];
									break;
							}
						}
						break;
					case 'e':	/* Set color for T axis symbol */
						Ctrl->E2.active = true;
						if (gmt_getfill (GMT, &opt->arg[1], &Ctrl->E2.fill)) {
							gmt_fill_syntax (GMT, 'e', " ");
							n_errors++;
						}
						break;
					case 'g':	/* Set color for P axis symbol */
						Ctrl->G2.active = true;
						if (gmt_getfill (GMT, &opt->arg[1], &Ctrl->G2.fill)) {
							gmt_fill_syntax (GMT, 'g', " ");
							n_errors++;
						}
						break;
					case 'p':	/* Draw outline of P axis symbol [set outline attributes] */
						Ctrl->P2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->P2.pen)) {
							gmt_pen_syntax (GMT, 'p', " ", 0);
							n_errors++;
						}
						break;
					case 'r':	/* draw box around text */
						Ctrl->R2.active = true;
						if (opt->arg[1] && gmt_getfill (GMT, &opt->arg[1], &Ctrl->R2.fill)) {
							gmt_fill_syntax (GMT, 'r', " ");
							n_errors++;
						}
						break;
					case 't':	/* Draw outline of T axis symbol [set outline attributes] */
						Ctrl->T2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->T2.pen)) {
							gmt_pen_syntax (GMT, 't', " ", 0);
							n_errors++;
						}
						break;
					case 'o':	/* use psvelomeca format (without depth in 3rd column) */
						Ctrl->O2.active = true;
						break;
					case 'z':	/* overlay zerotrace moment tensor */
						Ctrl->Z2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->Z2.pen)) { /* Set pen attributes */
							gmt_pen_syntax (GMT, 'z', " ", 0);
							n_errors++;
						}
						break;
				}
				break;
			case 'G':	/* Set color for compressive parts */
				Ctrl->G.active = true;
				if (!opt->arg[0] || (opt->arg[0] && gmt_getfill (GMT, opt->arg, &Ctrl->G.fill))) {
					gmt_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'L':	/* Draw outline [set outline attributes] */
				Ctrl->L.active = true;
				if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					gmt_pen_syntax (GMT, 'L', " ", 0);
					n_errors++;
				}
				break;
			case 'M':	/* Same size for any magnitude */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get symbol [and size] */
				Ctrl->S.active = true;
				if (opt->arg[strlen(opt->arg)-1] == 'u') Ctrl->S.justify = PSL_TC, opt->arg[strlen(opt->arg)-1] = '\0';
				txt[0] = txt_b[0] = txt_c[0] = '\0';
				sscanf (&opt->arg[1], "%[^/]/%[^/]/%s", txt, txt_b, txt_c);
				if (txt[0]) Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
				if (txt_b[0]) Ctrl->S.fontsize = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
				if (txt_c[0]) Ctrl->S.offset = gmt_convert_units (GMT, txt_c, GMT_PT, GMT_INCH);
				if (Ctrl->S.fontsize < 0.0) Ctrl->S.no_label = true;

				switch (opt->arg[0]) {
					case 'c':
						Ctrl->S.readmode = READ_CMT;
						break;
					case 'a':
						Ctrl->S.readmode = READ_AKI;
						break;
					case 'p':
						Ctrl->S.readmode = READ_PLANES;
						break;
					case 'x':
						Ctrl->S.readmode = READ_AXIS;
						break;
					case 'y':
						Ctrl->S.readmode = READ_AXIS;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 't':
						Ctrl->S.readmode = READ_AXIS;
						Ctrl->S.plotmode = PLOT_TRACE;
						break;
					case 'm':
						Ctrl->S.readmode = READ_TENSOR;
						break;
					case 'd':
						Ctrl->S.readmode = READ_TENSOR;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 'z':
						Ctrl->S.readmode = READ_TENSOR;
						Ctrl->S.plotmode = PLOT_TRACE;
						break;
					default:
						n_errors++;
						break;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				sscanf (opt->arg, "%d", &Ctrl->T.n_plane);
				if (strlen (opt->arg) > 2 && gmt_getpen (GMT, &opt->arg[2], &Ctrl->T.pen)) {	/* Set transparent attributes */
					gmt_pen_syntax (GMT, 'T', " ", 0);
					n_errors++;
				}
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', " ", 0);
					n_errors++;
				}
				break;
			case 'Z':	/* Vary symbol color with z */
				Ctrl->Z.active = true;
				Ctrl->Z.file = strdup (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	no_size_needed = (Ctrl->S.readmode == READ_CMT || Ctrl->S.readmode == READ_PLANES || Ctrl->S.readmode == READ_AKI ||
	                  Ctrl->S.readmode == READ_TENSOR || Ctrl->S.readmode == READ_AXIS);
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !no_size_needed && (Ctrl->S.active && Ctrl->S.scale <= 0.0),
	                                   "Syntax error: -S must specify scale\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->O2.active, "Syntax error: -Z cannot be combined with -Fo\n");

	/* Set to default pen where needed */

	if (Ctrl->C.pen.width  < 0.0) Ctrl->C.pen  = Ctrl->W.pen;
	if (Ctrl->L.pen.width  < 0.0) Ctrl->L.pen  = Ctrl->W.pen;
	if (Ctrl->T.pen.width  < 0.0) Ctrl->T.pen  = Ctrl->W.pen;
	if (Ctrl->T2.pen.width < 0.0) Ctrl->T2.pen = Ctrl->W.pen;
	if (Ctrl->P2.pen.width < 0.0) Ctrl->P2.pen = Ctrl->W.pen;
	if (Ctrl->Z2.pen.width < 0.0) Ctrl->Z2.pen = Ctrl->W.pen;

	/* Default -e<fill> and -g<fill> to -E<fill> and -G<fill> */

	if (!Ctrl->E2.active) Ctrl->E2.fill = Ctrl->E.fill;
	if (!Ctrl->G2.active) Ctrl->G2.fill = Ctrl->G.fill;

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psmeca (void *V_API, int mode, void *args) {
	/* High-level function that implements the psmeca task */
	int i, n, k, ix = 0, iy = 1, last = 0, form = 0, new_fmt;
	int n_rec = 0, n_plane_old = 0, error;
	bool transparence_old = false, not_defined = false;

	double plot_x, plot_y, plot_xnew, plot_ynew, delaz;
	double t11 = 1.0, t12 = 0.0, t21 = 0.0, t22 = 1.0, xy[2], xynew[2];
	double angle = 0.0, fault, depth, size, P_x, P_y, T_x, T_y;

	char string[GMT_BUFSIZ] = {""}, event_title[GMT_BUFSIZ] = {""}, *line = NULL, *p = NULL, col[15][GMT_LEN64];

	st_me meca;
	struct MOMENT moment;
	struct M_TENSOR mt;
	struct AXIS T, N, P;

	struct GMT_PALETTE *CPT = NULL;
	struct PSMECA_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psmeca main code ----------------------------*/

	gmt_M_memset (event_title, GMT_BUFSIZ, char);
	gmt_M_memset (&meca, 1, st_me);
	gmt_M_memset (&T, 1, struct AXIS);
	gmt_M_memset (&N, 1, struct AXIS);
	gmt_M_memset (&P, 1, struct AXIS);
	gmt_M_memset (col, GMT_LEN64*15, char);

	if (Ctrl->Z.active) {
		if ((CPT = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->Z.file, NULL)) == NULL) {
			Return (API->error);
		}
	}

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	PSL_setfont (PSL, GMT->current.setting.font_annot[GMT_PRIMARY].id);

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	ix = (GMT->current.setting.io_lonlat_toggle[0]);	iy = 1 - ix;

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		if ((line = GMT_Get_Record (API, GMT_READ_TEXT, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			assert (line != NULL);						/* Should never get here */
		}

		/* Data record to process */

		n_rec++;
		if (Ctrl->S.readmode == READ_CMT) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6],
				col[7], col[8], col[9], col[10], col[11], string);
			last = 11;
		}
		else if (Ctrl->S.readmode == READ_AKI) {
			sscanf (line, "%s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6],
				col[7], string);
			last = 7;
		}
		else if (Ctrl->S.readmode == READ_PLANES) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6],
				col[7], col[8], string);
			last = 8;
		}
		else if (Ctrl->S.readmode == READ_AXIS) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7],
				col[8], col[9], col[10], col[11], col[12], col[13], string);
			last = 13;
		}
		else if (Ctrl->S.readmode == READ_TENSOR) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7],
				col[8], col[9], col[10], string);
			last = 10;
		}
 		for (k = 0; k <= last; k++)
 			if ((p = strchr (col[k], ',')) != NULL) *p = '\0';	/* Chop of trailing command from input field deliminator */

		/* Immediately skip locations outside of the map area */

		if ((gmt_scanf (GMT, col[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X], &xy[ix]) == GMT_IS_NAN) ||
		                (gmt_scanf (GMT, col[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y], &xy[iy]) == GMT_IS_NAN)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad x and/or y coordinates, must exit)\n", n_rec);
			GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
		}

		if (!Ctrl->N.active) {
			gmt_map_outside (GMT, xy[GMT_X], xy[GMT_Y]);
			if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
		}

		/* In new (psmeca) input format, third column is depth.
		   Skip record when depth is out of range. Also read an extra column. */

		new_fmt = Ctrl->O2.active ? 0 : 1;
		if (new_fmt) {
			depth = atof (col[GMT_Z]);
			if (depth < Ctrl->D.depmin || depth > Ctrl->D.depmax) continue;
			if (Ctrl->Z.active) gmt_get_fill_from_z (GMT, CPT, depth, &Ctrl->G.fill);
			sscanf (string, "%s %[^\n]\n", col[last+1], event_title);
		}
		else
			strncpy (event_title, string, GMT_BUFSIZ-1);

		/* Gather and transform the input records, depending on type */

		if (Ctrl->S.readmode == READ_CMT) {
			meca.NP1.str = atof (col[2+new_fmt]);
			if (meca.NP1.str > 180.0)
				meca.NP1.str -= 360.0; else if (meca.NP1.str < -180.0) meca.NP1.str += 360.0;		/* Strike must be in -180/+180 range*/
			meca.NP1.dip = atof (col[3+new_fmt]);
			meca.NP1.rake = atof (col[4+new_fmt]);
			if (meca.NP1.rake > 180.0)
				meca.NP1.rake -= 360.0; else if (meca.NP1.rake < -180.0) meca.NP1.rake += 360.0;	/* Rake must be in -180/+180 range*/
			meca.NP2.str = atof (col[5+new_fmt]);
			if (meca.NP2.str > 180.0)
				meca.NP2.str -= 360.0; else if (meca.NP2.str < -180.0) meca.NP2.str += 360.0;		/* Strike must be in -180/+180 range*/
			meca.NP2.dip = atof (col[6+new_fmt]);
			meca.NP2.rake = atof (col[7+new_fmt]);
			if (meca.NP2.rake > 180.0)
				meca.NP2.rake -= 360.0; else if (meca.NP2.rake < -180.0) meca.NP2.rake += 360.0;	/* Rake must be in -180/+180 range*/
			meca.moment.mant = atof (col[8+new_fmt]);
			meca.moment.exponent = atoi(col[9+new_fmt]);
			if (meca.moment.exponent == 0) meca.magms = atof (col[8+new_fmt]);
		}
		else if (Ctrl->S.readmode == READ_AKI) {
			meca.NP1.str = atof (col[2+new_fmt]);
			if (meca.NP1.str > 180.0)
				meca.NP1.str -= 360.0; else if (meca.NP1.str < -180.0) meca.NP1.str += 360.0;		/* Strike must be in -180/+180 range*/
			meca.NP1.dip = atof (col[3+new_fmt]);
			meca.NP1.rake = atof (col[4+new_fmt]);
			if (meca.NP1.rake > 180.0)
				meca.NP1.rake -= 360.0; else if (meca.NP1.rake < -180.0) meca.NP1.rake += 360.0;	/* Rake must be in -180/+180 range*/
			if (gmt_M_is_zero (meca.NP1.rake)) meca.NP1.rake = 0.00001;	/* Fixing the issue http://gmt.soest.hawaii.edu/issues/894 */
			meca.magms = atof (col[5+new_fmt]);
			meca.moment.exponent = 0;
			meca_define_second_plane (meca.NP1, &meca.NP2);
		}
		else if (Ctrl->S.readmode == READ_PLANES) {
			meca.NP1.str = atof (col[2+new_fmt]);
			if (meca.NP1.str > 180.0)
				meca.NP1.str -= 360.0; else if (meca.NP1.str < -180.0) meca.NP1.str += 360.0;		/* Strike must be in -180/+180 range*/
			meca.NP1.dip = atof (col[3+new_fmt]);
			meca.NP2.str = atof (col[4+new_fmt]);
			if (meca.NP2.str > 180.0)
				meca.NP2.str -= 360.0; else if (meca.NP2.str < -180.0) meca.NP2.str += 360.0;		/* Strike must be in -180/+180 range*/
			fault = atof (col[5+new_fmt]);
			meca.magms = atof (col[6+new_fmt]);
			meca.moment.exponent = 0;
			meca.NP2.dip = meca_computed_dip2(meca.NP1.str, meca.NP1.dip, meca.NP2.str);
			if (meca.NP2.dip == 1000.0) {
				not_defined = true;
				transparence_old = Ctrl->T.active;
				n_plane_old = Ctrl->T.n_plane;
				Ctrl->T.active = true;
				Ctrl->T.n_plane = 1;
				meca.NP1.rake = 1000.;
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: second plane is not defined for event %s only first plane is plotted.\n", line);
			}
			else
				meca.NP1.rake = meca_computed_rake2(meca.NP2.str, meca.NP2.dip, meca.NP1.str, meca.NP1.dip, fault);
		}
		else if (Ctrl->S.readmode == READ_AXIS) {
			T.val = atof (col[2+new_fmt]);
			T.str = atof (col[3+new_fmt]);
			T.dip = atof (col[4+new_fmt]);
			T.e = atoi(col[11+new_fmt]);

			N.val = atof (col[5+new_fmt]);
			N.str = atof (col[6+new_fmt]);
			N.dip = atof (col[7+new_fmt]);
			N.e = atoi(col[11+new_fmt]);

			P.val = atof (col[8+new_fmt]);
			P.str = atof (col[9+new_fmt]);
			P.dip = atof (col[10+new_fmt]);
			P.e = atoi(col[11+new_fmt]);
			/*
			F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998, p.167.
			Definition of scalar moment.
			*/
			meca.moment.exponent = T.e;
			meca.moment.mant = sqrt (squared (T.val) + squared (N.val) + squared (P.val)) / M_SQRT2;
			meca.magms = 0.0;

			/* normalization by M0 */
			T.val /= meca.moment.mant;
			N.val /= meca.moment.mant;
			P.val /= meca.moment.mant;

			if (Ctrl->T.active || Ctrl->S.plotmode == PLOT_DC) meca_axe2dc (T, P, &meca.NP1, &meca.NP2);
		}
		else if (Ctrl->S.readmode == READ_TENSOR) {
			for (i = 2+new_fmt, n = 0; i < 8+new_fmt; i++, n++) mt.f[n] = atof (col[i]);
			mt.expo = atoi(col[i]);
			/*
			F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998, p.167.
			Definition of scalar moment.
			*/
			meca.moment.mant = sqrt(squared(mt.f[0]) + squared(mt.f[1]) + squared(mt.f[2]) +
			                        2.*(squared(mt.f[3]) + squared(mt.f[4]) + squared(mt.f[5]))) / M_SQRT2;
			meca.moment.exponent = mt.expo;
			meca.magms = 0.;

			/* normalization by M0 */
			for(i=0;i<=5;i++) mt.f[i] /= meca.moment.mant;

			meca_moment2axe (GMT, mt, &T, &N, &P);

			if (Ctrl->T.active || Ctrl->S.plotmode == PLOT_DC) meca_axe2dc (T, P, &meca.NP1, &meca.NP2);
		}

		/* Common to all input types ... */

		gmt_geo_to_xy (GMT, xy[GMT_X], xy[GMT_Y], &plot_x, &plot_y);

		/* If option -C is used, read the new position */

		if (Ctrl->C.active) {
			if ((gmt_scanf (GMT, col[last-1+new_fmt], GMT->current.io.col_type[GMT_IN][GMT_X], &xynew[ix]) == GMT_IS_NAN) || (gmt_scanf (GMT, col[last+new_fmt], GMT->current.io.col_type[GMT_IN][GMT_Y], &xynew[iy]) == GMT_IS_NAN)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad newX and/or newY coordinates, must exit)\n", n_rec);
				GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
			}
			if (fabs (xynew[ix]) > EPSIL || fabs (xynew[iy]) > EPSIL) {
				gmt_setpen (GMT, &Ctrl->C.pen);
				gmt_geo_to_xy (GMT, xynew[GMT_X], xynew[GMT_Y], &plot_xnew, &plot_ynew);
				gmt_setfill (GMT, &Ctrl->G.fill, true);
				PSL_plotsymbol (PSL, plot_x, plot_y, &Ctrl->C.size, PSL_CIRCLE);
				PSL_plotsegment (PSL, plot_x, plot_y, plot_xnew, plot_ynew);
				plot_x = plot_xnew;
				plot_y = plot_ynew;
			}
		}

		if (Ctrl->M.active) {
			meca.moment.mant = 4.0;
			meca.moment.exponent = 23;
		}

		moment.mant = meca.moment.mant;
		moment.exponent = meca.moment.exponent;
		size = (meca_computed_mw(moment, meca.magms) / 5.0) * Ctrl->S.scale;

		meca_get_trans (GMT, xy[GMT_X], xy[GMT_Y], &t11, &t12, &t21, &t22);
		delaz = atan2d(t12,t11);

		if ((Ctrl->S.readmode == READ_AXIS || Ctrl->S.readmode == READ_TENSOR) && Ctrl->S.plotmode != PLOT_DC) {

			T.str = meca_zero_360(T.str + delaz);
			N.str = meca_zero_360(N.str + delaz);
			P.str = meca_zero_360(P.str + delaz);

			gmt_setpen (GMT, &Ctrl->L.pen);
			if (fabs (N.val) < EPSIL && fabs (T.val + P.val) < EPSIL) {
				meca_axe2dc (T, P, &meca.NP1, &meca.NP2);
				meca_ps_mechanism (GMT, PSL, plot_x, plot_y, meca, size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
			}
			else
				meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active, Ctrl->S.plotmode == PLOT_TRACE, n_rec);
		}

		if (Ctrl->Z2.active) {
			gmt_setpen (GMT, &Ctrl->Z2.pen);
			meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, NULL, NULL, true, true, n_rec);
		}

		if (Ctrl->T.active) {
			meca.NP1.str = meca_zero_360(meca.NP1.str + delaz);
			meca.NP2.str = meca_zero_360(meca.NP2.str + delaz);
			gmt_setpen (GMT, &Ctrl->T.pen);
			meca_ps_plan (GMT, PSL, plot_x, plot_y, meca, size, Ctrl->T.n_plane);
			if (not_defined) {
				not_defined = false;
				Ctrl->T.active = transparence_old;
				Ctrl->T.n_plane = n_plane_old;
			}
		}
		else if (Ctrl->S.readmode == READ_AKI || Ctrl->S.readmode == READ_CMT || Ctrl->S.readmode == READ_PLANES || Ctrl->S.plotmode == PLOT_DC) {
			meca.NP1.str = meca_zero_360(meca.NP1.str + delaz);
			meca.NP2.str = meca_zero_360(meca.NP2.str + delaz);
			gmt_setpen (GMT, &Ctrl->L.pen);
			meca_ps_mechanism (GMT, PSL, plot_x, plot_y, meca, size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
		}

		if (!Ctrl->S.no_label) {
			gmt_setpen (GMT, &Ctrl->W.pen);
			i = (Ctrl->S.justify == PSL_BC ? 1 : -1);
			PSL_setfill (PSL, Ctrl->R2.fill.rgb, false);
			if (Ctrl->R2.active) PSL_plotbox (PSL, plot_x - size * 0.5, plot_y + i * (size * 0.5 + Ctrl->S.offset + Ctrl->S.fontsize / PSL_POINTS_PER_INCH), plot_x + size * 0.5, plot_y + i * (size * 0.5 + Ctrl->S.offset));
			PSL_plottext (PSL, plot_x, plot_y + i * (size * 0.5 + Ctrl->S.offset), Ctrl->S.fontsize, event_title, angle,
				Ctrl->S.justify, form);
		}

		if (Ctrl->A2.active) {
			if (Ctrl->S.readmode != READ_TENSOR && Ctrl->S.readmode != READ_AXIS) meca_dc2axe (meca, &T, &N, &P);
			meca_axis2xy (plot_x, plot_y, size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
			gmt_setpen (GMT, &Ctrl->P2.pen);
			gmt_setfill (GMT, &Ctrl->G2.fill, Ctrl->P2.active);
			PSL_plotsymbol (PSL, P_x, P_y, &Ctrl->A2.size, Ctrl->A2.P_symbol);
			gmt_setpen (GMT, &Ctrl->T2.pen);
			gmt_setfill (GMT, &Ctrl->E2.fill, Ctrl->T2.active);
			PSL_plotsymbol (PSL, T_x, T_y, &Ctrl->A2.size, Ctrl->A2.T_symbol);
		}
		event_title[0] = string[0] = '\0';		/* Reset these two in case next record misses "string" */
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Number of records read: %li\n", n_rec);

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
	PSL_setdash (PSL, NULL, 0);
	gmt_map_basemap (GMT);
	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
