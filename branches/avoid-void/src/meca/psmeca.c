/*--------------------------------------------------------------------
 *    $Id$
 *
 *    Copyright (c) 1996-2011 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*

psmeca will read <x,y> pairs (or <lon,lat>) from inputfile and
plot symbols on a map. Focal mechanisms are specified (double couple or
moment tensor).
PostScript code is written to stdout.


 Author:	Genevieve Patau
 Date:		7 July 1998
 Version:	4
 Roots:		based on psxy.c
 */

#include "pslib.h"      /* to have pslib environment */
#include "gmt_meca.h"	/* to have gmt_meca supplements */

#include "meca.h"
#include "utilmeca.h"

#define DEFAULT_POINTSIZE 0.005
#define DEFAULT_FONTSIZE 9.0
#define DEFAULT_OFFSET 3.0	/* In points */
#define DEFAULT_JUSTIFY 2

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
		GMT_LONG active;
		double size;
		struct GMT_PEN pen;
	} C;
 	struct D {	/* -D<min/max> */
		GMT_LONG active;
		double depmin, depmax;
	} D;
 	struct E {	/* -E<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} E;
 	struct G {	/* -G<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} G;
	struct L {	/* -L<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct R2 {	/* -r<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} R2;
	struct S {	/* -S<format><scale>[/fontsize[/justify/offset/angle/form]] */
		GMT_LONG active;
		GMT_LONG readmode;
		GMT_LONG plotmode;
		GMT_LONG justify;
		GMT_LONG no_label;
		char type;
		double scale;
		double fontsize, offset;
		struct GMT_FILL fill;
	} S;
	struct T {	/* -Tnplane[/<pen>] */
		GMT_LONG active;
		GMT_LONG n_plane;
		struct GMT_PEN pen;
	} T;
	struct Z2 {	/* -z<pen>] */
		GMT_LONG active;
		struct GMT_PEN pen;
	} Z2;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<cptfile> */
		GMT_LONG active;
		char *file;
	} Z;
	struct a2 {	/* -r<fill> */
		GMT_LONG active;
		char P_sym_type, T_sym_type;
		char P_symbol, T_symbol;
		double size;
	} a2;
 	struct E2 {	/* -e<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} E2;
 	struct G2 {	/* -g<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} G2;
 	struct P2 {	/* -p<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} P2;
 	struct T2 {	/* -t<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} T2;
 	struct O2 {	/* -o */
		GMT_LONG active;
		GMT_LONG mode;
	} O2;
};

void *New_psmeca_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSMECA_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSMECA_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->C.size = DEFAULT_POINTSIZE;
	C->C.pen = C->L.pen = C->T.pen = C->Z2.pen = C->P2.pen = GMT->current.setting.map_default_pen;
	C->D.depmax = 900.0;
	GMT_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	GMT_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	GMT_init_fill (GMT, &C->R2.fill, 1.0, 1.0, 1.0);
	GMT_init_fill (GMT, &C->E2.fill, 1.0, 1.0, 1.0);
	GMT_init_fill (GMT, &C->G2.fill, 0.0, 0.0, 0.0);
	C->S.fontsize = DEFAULT_FONTSIZE;
	C->S.offset = DEFAULT_OFFSET;
	C->a2.size = GMT->session.d_NaN;
	C->O2.mode = 1;
	return (C);
}

void Free_psmeca_Ctrl (struct GMT_CTRL *GMT, struct PSMECA_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Z.file) free (C->Z.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_psmeca_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the psmeca synopsis and optionally full usage information */

	GMT_message (GMT, "psmeca %s - Plot focal mechanisms on maps\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: psmeca [<table>] %s %s\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t-S<format><scale>[/<fontsize>[/<justify>/<offset>/<angle>/<form>]]\n");
	GMT_message (GMT, "\t[%s] [-C[<pen>][P<pointsize>]]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[-D<depmin>/<depmax>] [-E<fill>] [-G<fill>]\n");
	GMT_message (GMT, "\t[-K] [-L<pen>] [-M] [-N] [-O] [-P] [-r]\n");
	GMT_message (GMT, "\t[-T<nplane>[/<pen>]] [%s]\n", GMT_U_OPT);
	GMT_message (GMT, "\t[-V] [-W<pen>] [%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[-Z<cpt>] [-z] [-a[<size>[/<Psymbol>[<Tsymbol>]]]\n\n");
	GMT_message (GMT, "\t[-p<pen>] [-t<pen>] [-e<fill>] -g<fill>]\n\n");
	GMT_message (GMT, "\t[%s] [%s] [%s] [-o] [%s]\n", GMT_c_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_explain_options (GMT, "jR");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<b");
	GMT_message (GMT, "\t-C<pen attributes>\n");
	GMT_message (GMT, "\t  offset focal mechanisms to the latitude and longitude specified in the last two columns of the input file before label.\n");
	GMT_message (GMT, "\t  Default pen attributes is default pen.\n");
	GMT_message (GMT, "\t  A line is plotted between both positions.\n");
	GMT_message (GMT, "\t  A small circle is plotted at the initial location. Add P<pointsize value> to change the size of the circle.\n");
	GMT_message (GMT, "\t-D Plot events between <depmin> and <depmax> deep.\n");
	GMT_message (GMT, "\t-E Set color used for extensive parts [default is white].\n");
	GMT_message (GMT, "\t-G Set color used for compressive parts [default is black].\n");
	GMT_message (GMT, "\t   <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-L Draw line or symbol outline using the default pen (see -W) or sets pen attribute for outline.\n");
	GMT_message (GMT, "\t-M Same size for any magnitude. Size is given with -S.\n");
	GMT_message (GMT, "\t-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-r Draw a box around text.\n");
	GMT_message (GMT, "\t-S Select format type and symbol size (in %s).\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t   Choose format between:\n");
	GMT_message (GMT, "\t (c) Focal mechanisms in Harvard CMT convention\n");
	GMT_message (GMT, "\t     X, Y, depth, strike1, dip1, rake1, strike2, dip2, rake2, moment, newX, newY, event_title\n");
	GMT_message (GMT, "\t     with moment in 2 columns : mantiss and exponent corresponding to seismic moment in dynes-cm\n");
	GMT_message (GMT, "\t (a) Focal mechanism in Aki & Richard's convention:\n");
	GMT_message (GMT, "\t     X, Y, depth, strike, dip, rake, mag, newX, newY, event_title\n");
	GMT_message (GMT, "\t (p) Focal mechanism defined with:\n");
	GMT_message (GMT, "\t     X, Y, depth, strike1, dip1, strike2, fault, mag, newX, newY, event_title\n");
	GMT_message (GMT, "\t     fault = -1/+1 for a normal/inverse fault\n");
	GMT_message (GMT, "\t (m) Sesmic moment tensor (Harvard CMT, with zero trace):\n");
	GMT_message (GMT, "\t     X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, newX, newY, event_title\n");
	GMT_message (GMT, "\t (z) Anisotropic part of seismic moment tensor (Harvard CMT, with zero trace):\n");
	GMT_message (GMT, "\t     X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, event_title\n");
	GMT_message (GMT, "\t (d) Best double couple defined from seismic moment tensor (Harvard CMT, with zero trace):\n");
	GMT_message (GMT, "\t     X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, newX, newY, event_title\n");
	GMT_message (GMT, "\t (x) Principal axis:\n");
	GMT_message (GMT, "\t     X, Y, depth, T_value, T_azim, T_plunge, N_value, N_azim, N_plunge\n");
	GMT_message (GMT, "\t     P_value, P_azim, P_plunge, exp, newX, newY, event_title\n");
	GMT_message (GMT, "\t (t) Zero trace moment tensor defined from principal axis:\n");
	GMT_message (GMT, "\t     X, Y, depth, T_value, T_azim, T_plunge, N_value, N_azim, N_plunge\n");
	GMT_message (GMT, "\t     P_value, P_azim, P_plunge, exp, newX, newY, event_title\n");
	GMT_message (GMT, "\t (y) Best double couple defined from principal axis:\n");
	GMT_message (GMT, "\t     X, Y, depth, T_value, T_azim, T_plunge, N_value, N_azim, N_plunge\n");
	GMT_message (GMT, "\t     P_value, P_azim, P_plunge, exp, newX, newY, event_title\n");
	GMT_message (GMT, "\t Use -o option for old (psvelomeca) format (not depth in third column).\n");
	GMT_message (GMT, "\t Optionally add /fontsize[/offset][u]\n");
	GMT_message (GMT, "\t   Default values are /%g/%fp\n", DEFAULT_FONTSIZE, DEFAULT_OFFSET);
	GMT_message (GMT, "\t   fontsize < 0 : no label written;\n");
	GMT_message (GMT, "\t   offset is from the limit of the beach ball.\n");
	GMT_message (GMT, "\t   By default label is above the beach ball. Add u to plot it under.\n");
	GMT_message (GMT, "\t-Tn[/<pen>] Draw nodal planes and circumference only to provide a transparent\n");
	GMT_message (GMT, "\t   beach ball using the default pen (see -W) or sets pen attribute. \n");
	GMT_message (GMT, "\t   n = 1 the only first nodal plane is plotted.\n");
	GMT_message (GMT, "\t   n = 2 the only second nodal plane is plotted.\n");
	GMT_message (GMT, "\t   n = 0 both nodal planes are plotted.\n");
	GMT_message (GMT, "\t If moment tensor is required, nodal planes overlay moment tensor.\n");
	GMT_message (GMT, "\t-z Overlay zero trace moment tensor.\n");
	GMT_explain_options (GMT, "UV");
	GMT_message (GMT, "\t-W Set pen attributes [%s].\n", GMT_putpen (GMT, GMT->current.setting.map_default_pen));
	GMT_message (GMT, "\t-Z Use cpt-file to assign colors based on depth-value in 3rd column.\n");
	GMT_message (GMT, "\t-a Plot axis. Default symbols are circles.\n");
	GMT_message (GMT, "\t-p Draw P_symbol outline using the default pen (see -W) or sets pen attribute for outline.\n");
	GMT_message (GMT, "\t-t Draw T_symbol outline using the default pen (see -W) or sets pen attribute for outline.\n");
	GMT_message (GMT, "\t-g Set color used for P_symbol [default is compressive parts color].\n");
	GMT_message (GMT, "\t-e Set color used for T_symbol [default is extensive parts color].\n");
	GMT_message (GMT, "\t-o Use psvelomeca format (Without depth in third column).\n");
	GMT_explain_options (GMT, "Xchi:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_psmeca_parse (struct GMTAPI_CTRL *C, struct PSMECA_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psmeca and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, no_size_needed;
	char txt[GMT_TEXT_LEN256], *p = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Change position [set line attributes] */
				Ctrl->C.active = TRUE;
				if (!opt->arg[0]) break;
				p = NULL;	strcpy (txt, opt->arg);
				if ((p = strchr (txt, 'P'))) Ctrl->C.size = GMT_to_inch (GMT, (p+1));
				if (txt[0] != 'P') {	/* Have a pen up front */
					if (p) p[0] = '\0';
					if (GMT_getpen (GMT, txt, &Ctrl->C.pen)) {
						GMT_pen_syntax (GMT, 'C', " ");
						n_errors++;
					}
				}
				break;
			case 'D':	/* Plot events between depmin and depmax deep */
				Ctrl->D.active = TRUE;
				sscanf (opt->arg, "%lf/%lf", &Ctrl->D.depmin, &Ctrl->D.depmax);
				break;
			case 'E':	/* Set color for extensive parts  */
				Ctrl->E.active = TRUE;
				if (!opt->arg[0] || (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->E.fill))) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'G':	/* Set color for compressive parts */
				Ctrl->G.active = TRUE;
				if (!opt->arg[0] || (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->G.fill))) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'L':	/* Draw outline [set outline attributes] */
				Ctrl->L.active = TRUE;
				if (opt->arg[0] && GMT_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					GMT_pen_syntax (GMT, 'L', " ");
					n_errors++;
				}
				break;
			case 'M':	/* Same size for any magnitude */
				Ctrl->M.active = TRUE;
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = TRUE;
				break;
			case 'r':	/* draw box around text */
				Ctrl->R2.active = TRUE;
				if (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->R2.fill)) {
					GMT_fill_syntax (GMT, 'r', " ");
					n_errors++;
				}
				break;
			case 'S':	/* Get symbol [and size] */
				Ctrl->S.active = TRUE;
				Ctrl->S.type = opt->arg[0];
				p = NULL;	strcpy (txt, &opt->arg[1]);
				if ((p = strchr (txt, '/'))) p[0] = '\0';
				Ctrl->S.scale = GMT_to_inch (GMT, txt);
				if ((p = strstr (opt->arg, "/"))) {
					sscanf (p, "/%lf/%lf", &Ctrl->S.fontsize, &Ctrl->S.offset);
					if (GMT_IS_ZERO (Ctrl->S.fontsize)) Ctrl->S.fontsize = DEFAULT_FONTSIZE;
					if (Ctrl->S.fontsize < 0.0) Ctrl->S.no_label = TRUE;
					if (GMT_IS_ZERO (Ctrl->S.offset)) Ctrl->S.offset = DEFAULT_OFFSET;
					if (opt->arg[strlen(opt->arg)-1] == 'u') Ctrl->S.justify = 10;
				}
				switch (Ctrl->S.type) {
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
				Ctrl->T.active = TRUE;
				sscanf (opt->arg, "%ld", &Ctrl->T.n_plane);
				if (strlen (opt->arg) > 2 && GMT_getpen (GMT, &opt->arg[2], &Ctrl->T.pen)) {	/* Set transparent attributes */
					GMT_pen_syntax (GMT, 'T', " ");
					n_errors++;
				}
				break;
			case 'z':	/* overlay zerotrace moment tensor */
				Ctrl->Z2.active = TRUE;
				if (opt->arg && GMT_getpen (GMT, opt->arg, &Ctrl->Z2.pen)) { /* Set pen attributes */
					GMT_pen_syntax (GMT, 'z', " ");
					n_errors++;
				}
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = TRUE;
				if (opt->arg && GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				break;
			case 'Z':	/* Vary symbol color with z */
				Ctrl->Z.active = TRUE;
				Ctrl->Z.file = strdup (opt->arg);
				break;
			case 'a':	/* plot axis */
				Ctrl->a2.active = TRUE;
				if (!opt->arg[0]) {	/* Set defaults */
					strcpy (txt,"0.2c");
					Ctrl->a2.size = GMT_to_inch (GMT, txt);
					Ctrl->a2.P_symbol = GMT_SYMBOL_CIRCLE;
					Ctrl->a2.T_symbol = GMT_SYMBOL_CIRCLE;
					break;
				}
				else {
					strcpy (txt, &opt->arg[1]);
					p = NULL;	strcpy (txt, &opt->arg[1]);
					if ((p = strchr (txt, '/'))) p[0] = '\0';
					Ctrl->a2.size = GMT_to_inch (GMT, txt);
					if ((p = strstr (opt->arg, "/"))) {
						strcpy (txt, p);
						switch (strlen (txt)) {
							case 1:
								Ctrl->a2.P_sym_type = 'c';
								Ctrl->a2.T_sym_type = 'c';
								break;
							case 2:
								Ctrl->a2.P_sym_type = txt[1];
								Ctrl->a2.T_sym_type = txt[1];
								break;
							case 3:
								Ctrl->a2.P_sym_type = txt[1];
								Ctrl->a2.T_sym_type = txt[2];
								break;
						}
					}
				}
				switch (Ctrl->a2.P_sym_type) {
					case 'a':
						Ctrl->a2.P_symbol = GMT_SYMBOL_STAR;
						break;
					case 'c':
						Ctrl->a2.P_symbol = GMT_SYMBOL_CIRCLE;
						break;
					case 'd':
						Ctrl->a2.P_symbol = GMT_SYMBOL_DIAMOND;
						break;
					case 'h':
						Ctrl->a2.P_symbol = GMT_SYMBOL_HEXAGON;
						break;
					case 'i':
						Ctrl->a2.P_symbol = GMT_SYMBOL_INVTRIANGLE;
						break;
					case 's':
						Ctrl->a2.P_symbol = GMT_SYMBOL_SQUARE;
						break;
					case 't':
						Ctrl->a2.P_symbol = GMT_SYMBOL_TRIANGLE;
						break;
					case 'x':
						Ctrl->a2.P_symbol = GMT_SYMBOL_CROSS;
						break;
				}
				switch (Ctrl->a2.T_sym_type) {
					case 'a':
						Ctrl->a2.T_symbol = GMT_SYMBOL_STAR;
						break;
					case 'c':
						Ctrl->a2.T_symbol = GMT_SYMBOL_CIRCLE;
						break;
					case 'd':
						Ctrl->a2.T_symbol = GMT_SYMBOL_DIAMOND;
						break;
					case 'h':
						Ctrl->a2.T_symbol = GMT_SYMBOL_HEXAGON;
						break;
					case 'i':
						Ctrl->a2.T_symbol = GMT_SYMBOL_INVTRIANGLE;
						break;
					case 's':
						Ctrl->a2.T_symbol = GMT_SYMBOL_SQUARE;
						break;
					case 't':
						Ctrl->a2.T_symbol = GMT_SYMBOL_TRIANGLE;
						break;
					case 'x':
						Ctrl->a2.T_symbol = GMT_SYMBOL_CROSS;
						break;
				}
				break;
			case 'e':	/* Set color for T axis symbol */
				Ctrl->E2.active = TRUE;
				if (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->E2.fill)) {
					GMT_fill_syntax (GMT, 'e', " ");
					n_errors++;
				}
				break;
			case 'g':	/* Set color for P axis symbol */
				Ctrl->E2.active = TRUE;
				if (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->G2.fill)) {
					GMT_fill_syntax (GMT, 'g', " ");
					n_errors++;
				}
				break;
			case 'p':	/* Draw outline of P axis symbol [set outline attributes] */
				Ctrl->P2.active = TRUE;
				if (opt->arg[0] && GMT_getpen (GMT, opt->arg, &Ctrl->P2.pen)) {
					GMT_pen_syntax (GMT, 'p', " ");
					n_errors++;
				}
				break;
			case 't':	/* Draw outline of T axis symbol [set outline attributes] */
				Ctrl->T2.active = TRUE;
				if (opt->arg[0] && GMT_getpen (GMT, opt->arg, &Ctrl->T2.pen)) {
					GMT_pen_syntax (GMT, 't', " ");
					n_errors++;
				}
				break;
			case 'o':	/* use psvelomeca format (without depth in 3rd column) */
				Ctrl->O2.active = TRUE;
				Ctrl->O2.mode = 0;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	no_size_needed = (Ctrl->S.readmode == READ_CMT || Ctrl->S.readmode == READ_PLANES || Ctrl->S.readmode == READ_AKI || Ctrl->S.readmode == READ_TENSOR || Ctrl->S.readmode == READ_AXIS);
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !no_size_needed && (Ctrl->S.active > 1 && Ctrl->S.scale <= 0.0), "Syntax error: -S must specify scale\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.active && !Ctrl->O2.mode, "Syntax error: -Z cannot be combined with -o\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psmeca_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_psmeca (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{	/* High-level function that implements the psmeca task */
	GMT_LONG i, n, ix = 0, iy = 1, last = 0, form = 0;
	GMT_LONG n_rec = 0, n_plane_old = 0, n_fields, error;
	GMT_LONG no_size_needed, transparence_old = FALSE, not_defined = FALSE;

	double plot_x, plot_y, plot_xnew, plot_ynew, delaz;
	double t11 = 1.0, t12 = 0.0, t21 = 0.0, t22 = 1.0, xy[2], xynew[2];
	double angle = 0.0, fault, depth, size, P_x, P_y, T_x, T_y;

	char string[GMT_BUFSIZ], event_title[GMT_BUFSIZ], *line, col[15][GMT_TEXT_LEN64];

	st_me meca;
	struct MOMENT moment;
	struct M_TENSOR mt;
	struct AXIS T, N, P;

	struct GMT_PALETTE *CPT = NULL;
	struct PSMECA_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_psmeca_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_psmeca_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_psmeca", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJR:", "BHKOPUVXhixYyc>", options))) Return (error);
	Ctrl = New_psmeca_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psmeca_parse (API, Ctrl, options))) Return (error);
 	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the psmeca main code ----------------------------*/

	GMT_memset (event_title, GMT_BUFSIZ, char);
	GMT_memset (&meca, 1, st_me);

	no_size_needed = (Ctrl->S.readmode == READ_CMT || Ctrl->S.readmode == READ_PLANES || \
		Ctrl->S.readmode == READ_AKI || Ctrl->S.readmode == READ_TENSOR || Ctrl->S.readmode == READ_AXIS);

	if (Ctrl->Z.active) {
		if ((error = GMT_Begin_IO (API, GMT_IS_CPT, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->Z.file, &CPT)) Return (GMT_DATA_READ_ERROR);
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
	}

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	GMT_plotinit (GMT, options);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	GMT_setpen (GMT, &Ctrl->W.pen);
	PSL_setfont (PSL, GMT->current.setting.font_annot[0].id);

	if (!Ctrl->N.active) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);

	ix = (GMT->current.setting.io_lonlat_toggle[0]);	iy = 1 - ix;

	if ((error = GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */

	while ((n_fields = GMT_Get_Record (API, GMT_READ_TEXT, &line)) != EOF) {	/* Keep returning records until we have no more files */

 		if (GMT_REC_IS_ERROR (GMT)) Return (EXIT_FAILURE);
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */

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

		/* Immediately skip locations outside of the map area */

		if ((GMT_scanf (GMT, col[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X], &xy[ix]) == GMT_IS_NAN) || (GMT_scanf (GMT, col[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y], &xy[iy]) == GMT_IS_NAN)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad x and/or y coordinates, must exit)\n", n_rec);
			GMT_exit (EXIT_FAILURE);
		}

		if (!Ctrl->N.active) {
			GMT_map_outside (GMT, xy[GMT_X], xy[GMT_Y]);
			if (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1) continue;
		}

		/* In new (psmeca) input format, third column is depth.
		   Skip record when depth is out of range. Also read an extra column. */

		if (Ctrl->O2.mode) {
			depth = atof (col[GMT_Z]);
			if (depth < Ctrl->D.depmin || depth > Ctrl->D.depmax) continue;
			if (Ctrl->Z.active) GMT_get_rgb_from_z (GMT, CPT, depth, Ctrl->G.fill.rgb);
			sscanf (string, "%s %[^\n]\n", col[last+1], event_title);
		}
		else
			strcpy (event_title, string);
		if (strlen (event_title) <= 0) sprintf (event_title, "\n");

		/* Gather and transform the input records, depending on type */

		if (Ctrl->S.readmode == READ_CMT) {
			meca.NP1.str = atof (col[2+Ctrl->O2.mode]);
			meca.NP1.dip = atof (col[3+Ctrl->O2.mode]);
			meca.NP1.rake = atof (col[4+Ctrl->O2.mode]);
			meca.NP2.str = atof (col[5+Ctrl->O2.mode]);
			meca.NP2.dip = atof (col[6+Ctrl->O2.mode]);
			meca.NP2.rake = atof (col[7+Ctrl->O2.mode]);
			meca.moment.mant = atof (col[8+Ctrl->O2.mode]);
			meca.moment.exponent = atoi(col[9+Ctrl->O2.mode]);
			if (meca.moment.exponent == 0) meca.magms = atof (col[8+Ctrl->O2.mode]);
		}
		else if (Ctrl->S.readmode == READ_AKI) {
			meca.NP1.str = atof (col[2+Ctrl->O2.mode]);
			meca.NP1.dip = atof (col[3+Ctrl->O2.mode]);
			meca.NP1.rake = atof (col[4+Ctrl->O2.mode]);
			meca.magms = atof (col[5+Ctrl->O2.mode]);
			meca.moment.exponent = 0;
			define_second_plane (meca.NP1, &meca.NP2);
		}
		else if (Ctrl->S.readmode == READ_PLANES) {
			meca.NP1.str = atof (col[2+Ctrl->O2.mode]);
			meca.NP1.dip = atof (col[3+Ctrl->O2.mode]);
			meca.NP2.str = atof (col[4+Ctrl->O2.mode]);
			fault = atof (col[5+Ctrl->O2.mode]);
			meca.magms = atof (col[6+Ctrl->O2.mode]);
			meca.moment.exponent = 0;
			meca.NP2.dip = computed_dip2(meca.NP1.str, meca.NP1.dip, meca.NP2.str);
			if (meca.NP2.dip == 1000.0) {
				not_defined = TRUE;
				transparence_old = Ctrl->T.active;
				n_plane_old = Ctrl->T.n_plane;
				Ctrl->T.active = TRUE;
				Ctrl->T.n_plane = 1;
				meca.NP1.rake = 1000.;
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: second plane is not defined for event %s only first plane is plotted.\n", line);
			}
			else
				meca.NP1.rake = computed_rake2(meca.NP2.str, meca.NP2.dip, meca.NP1.str, meca.NP1.dip, fault);
			meca.NP2.rake = computed_rake2(meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip, fault);
		}
		else if (Ctrl->S.readmode == READ_AXIS) {
			T.val = atof (col[2+Ctrl->O2.mode]);
			T.str = atof (col[3+Ctrl->O2.mode]);
			T.dip = atof (col[4+Ctrl->O2.mode]);
			T.e = atoi(col[11+Ctrl->O2.mode]);

			N.val = atof (col[5+Ctrl->O2.mode]);
			N.str = atof (col[6+Ctrl->O2.mode]);
			N.dip = atof (col[7+Ctrl->O2.mode]);
			N.e = atoi(col[11+Ctrl->O2.mode]);

			P.val = atof (col[8+Ctrl->O2.mode]);
			P.str = atof (col[9+Ctrl->O2.mode]);
			P.dip = atof (col[10+Ctrl->O2.mode]);
			P.e = atoi(col[11+Ctrl->O2.mode]);
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

			if (Ctrl->T.active || Ctrl->S.plotmode == PLOT_DC) axe2dc(T, P, &meca.NP1, &meca.NP2);
		}
		else if (Ctrl->S.readmode == READ_TENSOR) {
			for (i=2+Ctrl->O2.mode, n=0; i<8+Ctrl->O2.mode; i++, n++) mt.f[n] = atof (col[i]);
			mt.expo = atoi(col[i]);
			/*
			F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998, p.167.
			Definition of scalar moment.
			*/
			meca.moment.mant = sqrt(squared(mt.f[0]) + squared(mt.f[1]) + squared(mt.f[2]) + 2.*(squared(mt.f[3]) + squared(mt.f[4]) + squared(mt.f[5]))) / M_SQRT2;
			meca.moment.exponent = mt.expo;
			meca.magms = 0.;

			/* normalization by M0 */
			for(i=0;i<=5;i++) mt.f[i] /= meca.moment.mant;

			GMT_momten2axe (GMT, mt, &T, &N, &P);

			if (Ctrl->T.active || Ctrl->S.plotmode == PLOT_DC) axe2dc (T, P, &meca.NP1, &meca.NP2);
		}

		/* Common to all input types ... */

		GMT_geo_to_xy (GMT, xy[GMT_X], xy[GMT_Y], &plot_x, &plot_y);

		/* If option -C is used, read the new position */

		if (Ctrl->C.active) {
			xynew[ix] = atof (col[last-1+Ctrl->O2.mode]);
			xynew[iy] = atof (col[last+Ctrl->O2.mode]);
			if (fabs (xynew[ix]) > EPSIL || fabs (xynew[iy]) > EPSIL) {
				GMT_setpen (GMT, &Ctrl->C.pen);
				GMT_geo_to_xy (GMT, xynew[0], xynew[1], &plot_xnew, &plot_ynew);
				GMT_setfill (GMT, &Ctrl->G.fill, 1);
				PSL_plotsymbol (PSL, plot_x, plot_y, &Ctrl->C.size, GMT_SYMBOL_CIRCLE);
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
		size = (computed_mw(moment, meca.magms) / 5.0) * Ctrl->S.scale;

		get_trans (GMT, xy[GMT_X], xy[GMT_Y], &t11, &t12, &t21, &t22);
		delaz = atan2d(t12,t11);

		if ((Ctrl->S.readmode == READ_AXIS || Ctrl->S.readmode == READ_TENSOR) && Ctrl->S.plotmode != PLOT_DC) {

			T.str = zero_360(T.str + delaz);
			N.str = zero_360(N.str + delaz);
			P.str = zero_360(P.str + delaz);

			GMT_setpen (GMT, &Ctrl->L.pen);
			if (fabs (N.val) < EPSIL && fabs (T.val + P.val) < EPSIL) {
				axe2dc (T, P, &meca.NP1, &meca.NP2);
				ps_mechanism (GMT, PSL, plot_x, plot_y, meca, size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
			}
			else
				ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active, Ctrl->S.plotmode == PLOT_TRACE);
		}

		if (Ctrl->Z2.active) {
			GMT_setpen (GMT, &Ctrl->Z2.pen);
			ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, NULL, NULL, TRUE, TRUE);
		}

		if (Ctrl->T.active) {
			meca.NP1.str = zero_360(meca.NP1.str + delaz);
			meca.NP2.str = zero_360(meca.NP2.str + delaz);
			GMT_setpen (GMT, &Ctrl->T.pen);
			switch (Ctrl->T.n_plane) {
				case 0:
					ps_meca (GMT, PSL, plot_x, plot_y, meca, size);
					break;
				default:
					ps_plan (GMT, PSL, plot_x, plot_y, meca, size, Ctrl->T.n_plane);
					break;
			}
			if (not_defined) {
				not_defined = FALSE;
				Ctrl->T.active = transparence_old;
				Ctrl->T.n_plane = n_plane_old;
			}
		}
		else if (Ctrl->S.readmode == READ_AKI || Ctrl->S.readmode == READ_CMT || Ctrl->S.readmode == READ_PLANES || Ctrl->S.plotmode == PLOT_DC) {
			meca.NP1.str = zero_360(meca.NP1.str + delaz);
			meca.NP2.str = zero_360(meca.NP2.str + delaz);
			GMT_setpen (GMT, &Ctrl->L.pen);
			ps_mechanism (GMT, PSL, plot_x, plot_y, meca, size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
		}

		if (!Ctrl->S.no_label) {
			GMT_setpen (GMT, &Ctrl->W.pen);
			switch (Ctrl->S.justify) {
				case 2 :
					PSL_setfill (PSL, Ctrl->R2.fill.rgb, FALSE);
					if (Ctrl->R2.active) PSL_plotbox (PSL, plot_x - size * 0.5, plot_y + size * 0.5 + Ctrl->S.offset + (Ctrl->S.fontsize / PSL_POINTS_PER_INCH), plot_x + size * 0.5, plot_y + size * 0.5 + Ctrl->S.offset);
					PSL_plottext (PSL, plot_x, plot_y + size * 0.5 + Ctrl->S.offset, Ctrl->S.fontsize, event_title, angle, Ctrl->S.justify, form);
					break;
				case 10 :
					PSL_setfill (PSL, Ctrl->R2.fill.rgb, FALSE);
					if (Ctrl->R2.active) PSL_plotbox (PSL, plot_x - size * 0.5, plot_y - size * 0.5 - Ctrl->S.offset - (Ctrl->S.fontsize / PSL_POINTS_PER_INCH), plot_x + size * 0.5, plot_y - size * 0.5 - Ctrl->S.offset);
					PSL_plottext (PSL, plot_x, plot_y - size * 0.5 - Ctrl->S.offset, Ctrl->S.fontsize, event_title, angle, Ctrl->S.justify, form);
					break;
			}

			if (Ctrl->a2.active) {
				if (Ctrl->S.readmode == READ_AXIS || Ctrl->S.readmode == READ_TENSOR)
					axis2xy(plot_x, plot_y, size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
				else
					ps_pt_axis(plot_x, plot_y, meca, size, &P.str, &P.dip, &T.str, &T.dip, &P_x, &P_y, &T_x, &T_y);
				GMT_setpen (GMT, &Ctrl->P2.pen);
				GMT_setfill (GMT, &Ctrl->G2.fill, Ctrl->P2.active);
				PSL_plotsymbol (PSL, P_x, P_y, &Ctrl->a2.size, Ctrl->a2.P_symbol);
				GMT_setpen (GMT, &Ctrl->T2.pen);
				GMT_setfill (GMT, &Ctrl->E2.fill, Ctrl->T2.active);
				PSL_plotsymbol (PSL, T_x, T_y, &Ctrl->a2.size, Ctrl->a2.P_symbol);
			}
		}
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

	GMT_report (GMT, GMT_MSG_NORMAL, "Number of records read: %li\n", n_rec);

	if (!Ctrl->N.active) GMT_map_clip_off (GMT);

	PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);

	if (Ctrl->W.pen.style) PSL_setdash (PSL, CNULL, 0);

	GMT_map_basemap (GMT);

	GMT_plotend (GMT);

	Return (GMT_OK);
}
