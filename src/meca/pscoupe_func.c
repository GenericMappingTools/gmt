/*--------------------------------------------------------------------
 *    $Id: pscoupe_func.c,v 1.3 2011-04-11 21:15:32 remko Exp $
 *
 *    Copyright (c) 1996-2011 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*

pscoupe will read <x,y> pairs (or <lon,lat>) from inputfile and
plot symbols on a cross-section. Focal mechanisms may be specified
(double couple or moment tensor) and require additional columns of data.
PostScript code is written to stdout.


 Author:       Genevieve Patau
 Date:         9 September 1992
 Last change : 02 April 2001
 Version:      4
 Roots:        based on psxy.c version 3.0

 */

#include "pslib.h"	/* to have pslib environment */
#include "gmt_meca.h"	/* to have gmt_meca supplements */

#include "meca.h"
#include "utilmeca.h"
#include "submeca.h"

#define DEFAULT_POINTSIZE	0.005
#define DEFAULT_FONTSIZE	9.0
#define DEFAULT_OFFSET		3.0	/* In points */
#define DEFAULT_JUSTIFY		2

#define READ_CMT	0
#define READ_AKI	1
#define READ_PLANES	2
#define READ_AXIS	4
#define READ_TENSOR	8

#define PLOT_DC		1
#define PLOT_AXIS	2
#define PLOT_TRACE	4
#define PLOT_TENSOR	8

/* Control structure for pscoupe */

struct PSCOUPE_CTRL {
	struct A {	/* -A[<params>] */
		GMT_LONG active;
		GMT_LONG frame, fuseau, polygon;
		char proj_type;
		double size, p_width, p_length, dmin, dmax;
		double xlonref, ylatref;
		struct GMT_PEN pen;
		struct nodal_plane PREF;
		FILE *pnew, *pextract;
	} A;
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
	struct S {	/* -S */
		GMT_LONG active, active2;
		GMT_LONG readmode;
		GMT_LONG plotmode;
		GMT_LONG justify;
		GMT_LONG no_label;
		GMT_LONG symbol;
		GMT_LONG read_size;
		GMT_LONG zerotrace;
		char P_sym_type, T_sym_type;
		char P_symbol, T_symbol;
		char type;
		double scale, size;
		double fontsize, offset;
		struct GMT_FILL fill;
	} S;
	struct T {	/* -Tnplane[/<pen>] */
		GMT_LONG active;
		GMT_LONG n_plane;
		struct GMT_PEN pen;
	} T;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<Ctrl->Z.filefile> */
		GMT_LONG active;
		char *file;
	} Z;
	struct a2 {	/* -a[size[Psymbol[Tsymbol]] */
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
};

void *New_pscoupe_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSCOUPE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSCOUPE_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->L.pen = C->T.pen = C->P2.pen = GMT->current.setting.map_default_pen;
	GMT_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	GMT_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	GMT_init_fill (GMT, &C->E2.fill, 1.0, 1.0, 1.0);
	GMT_init_fill (GMT, &C->G2.fill, 1.0, 1.0, 1.0);
	C->S.fontsize = DEFAULT_FONTSIZE;
	C->S.offset = DEFAULT_OFFSET;
	C->A.size = GMT->session.d_NaN;
	return ((void *)C);
}

void Free_pscoupe_Ctrl (struct GMT_CTRL *GMT, struct PSCOUPE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Z.file) free ((void *)C->Z.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_pscoupe_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the pscoupe synopsis and optionally full usage information */

	GMT_message (GMT,"pscoupe %s - Plot seismological symbols on cross-sections\n\n", GMT_VERSION);
	GMT_message (GMT,"usage: pscoupe <infiles> -A<params> %s %s\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[%s] [-E<fill>] [-G<fill>]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[-K] [-L<pen>] [-M] [-N] [-O] [-P]\n");
	GMT_message (GMT, "\t[-S<format><scale>[/fontsize[/justify/offset/angle/form]]]\n");
	GMT_message (GMT, "\t[-s<symbol><scale>[/fontsize[/justify/offset/angle/form]]]\n");
	GMT_message (GMT, "\t[-Tnplane[/<pen>]] [%s]\n", GMT_U_OPT);
	GMT_message (GMT, "\t[-V] [-W<pen>] [%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[-Z<cpt>] [-a[size[Psymbol[Tsymbol]]]\n\n");
	GMT_message (GMT, "\t[-p<pen>] [-t<pen>] [-e<fill>] -g<fill>]\n\n");
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s]\n", GMT_c_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<infiles> is one or more files. If nofiles are given, read standard input\n");
	GMT_message (GMT, "\t-A Specify cross-section parameters. Choose between\n");
	GMT_message (GMT, "\t   -Aa<lon1/lat1/lon2/lat2/dip/p_width/dmin/dmax>[f]\n");
	GMT_message (GMT, "\t   -Ab<lon1/lat1/strike/p_length/dip/p_width/dmin/dmax>[f]\n");
	GMT_message (GMT, "\t   -Ac<x1/y1/x2/y2/dip/p_width/dmin/dmax>[f]\n");
	GMT_message (GMT, "\t   -Ad<x1/y1/strike/p_length/dip/p_width/dmin/max>[f]\n");
	GMT_message (GMT, "\t   Add f to get the frame from the cross-section parameters.\n");
	GMT_explain_options (GMT, "jRb");
	GMT_message (GMT, "\t-E Set color used for extensive parts. [default is white]\n");
	GMT_message (GMT, "\t-G Set color used for compressive parts. [default is black]\n");
	GMT_message (GMT, "\t   <r/g/b> (each 0-255) for color or <gray> (0-255) for gray-shade [0].\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-L draw line or symbol outline using the current pen (see -W) or sets pen attribute for outline.\n");
	GMT_message (GMT, "\t-M Same size for any magnitude. Size is given with -S.\n");
	GMT_message (GMT, "\t-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside]\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-S Select format type and symbol size (in measure_unit).\n");
	GMT_message (GMT, "\t   Choose format between\n");
	GMT_message (GMT, "\t (c) Focal mechanisms in Harvard CMT convention\n");
	GMT_message (GMT, "\t     X, Y, depth, strike1, dip1, rake1, strike2, dip2, rake2, moment, event_title\n");
	GMT_message (GMT, "\t     with moment in 2 columns : mantiss and exponent corresponding to seismic moment in dynes-cm\n");
	GMT_message (GMT, "\t (a) Focal mechanism in Aki & Richard's convention:\n");
	GMT_message (GMT, "\t     X, Y, depth, strike, dip, rake, mag, event_title\n");
	GMT_message (GMT, "\t (p) Focal mechanism defined with\n");
	GMT_message (GMT, "\t     X, Y, depth, strike1, dip1, strike2, fault, mag, event_title\n");
	GMT_message (GMT, "\t     fault = -1/+1 for a normal/inverse fault\n");
	GMT_message (GMT, "\t (m) Sesmic moment tensor (Harvard CMT, with zero trace)\n");
	GMT_message (GMT, "\t     X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, event_title\n");
	GMT_message (GMT, "\t (z) Anisotropic part of seismic moment tensor (Harvard CMT, with zero trace)\n");
	GMT_message (GMT, "\t     X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, event_title\n");
	GMT_message (GMT, "\t (d) Best double couple defined from seismic moment tensor (Harvard CMT, with zero trace)\n");
	GMT_message (GMT, "\t     X, Y, depth, mrr, mtt, mff, mrt, mrf, mtf, exp, event_title\n");
	GMT_message (GMT, "\t (x) Principal axis\n");
	GMT_message (GMT, "\t     X,Y,depth,T_value,T_azimuth,T_plunge,N_value,N_azimuth,N_plunge,\n");
	GMT_message (GMT, "\t     P_value,P_azimuth,P_plunge,exp,event_title\n");
	GMT_message (GMT, "\t (t) Zero trace moment tensor defined from principal axis\n");
	GMT_message (GMT, "\t     X, Y, depth, T_value, T_azim, T_plunge, N_value, N_azim, N_plunge\n");
	GMT_message (GMT, "\t     P_value, P_azim, P_plunge, exp, newX, newY, event_title\n");
	GMT_message (GMT, "\t (y) Best double couple defined from principal axis\n");
	GMT_message (GMT, "\t     X,Y,depth,T_value,T_azimuth,T_plunge,N_value,N_azimuth,N_plunge,\n");
	GMT_message (GMT, "\t     P_value,P_azimuth,P_plunge,exp,event_title\n");
	GMT_message (GMT, "\t Optionally add /fontsize[/offset][u]\n");
	GMT_message (GMT, "\t   Default values are /%g/%f\n", DEFAULT_FONTSIZE, DEFAULT_OFFSET);
	GMT_message (GMT, "\t   fontsize < 0 : no label written;\n");
	GMT_message (GMT, "\t   offset is from the limit of the beach ball.\n");
	GMT_message (GMT, "\t   By default label is above the beach ball. Add u to plot it under.\n");
	GMT_message (GMT, "\t-s to select symbol type and symbol size (in user_unit)\n");
	GMT_message (GMT, "\t  Choose between\n");
	GMT_message (GMT, "\t    st(a)r, (c)ircle, (d)iamond, (h)exagon, (i)nvtriangle, (s)quare, (t)riangle.\n");
	GMT_message (GMT, "\t-Tn[/<pen>] draw nodal planes and circumference only to provide a transparent beach ball using the current pen (see -W) or sets pen attribute. \n");
	GMT_message (GMT, "\t n = 1 the only first nodal plane is plotted\n");
	GMT_message (GMT, "\t n = 2 the only second nodal plane is plotted\n");
	GMT_message (GMT, "\t n = 0 both nodal planes are plotted.\n");
	GMT_message (GMT, "\t If moment tensor is required, nodal planes overlay moment tensor.\n");
	GMT_explain_options (GMT, "UV");
	GMT_message (GMT,  "\t-W sets default pen attributes [%s]\n", GMT_putpen (GMT, GMT->current.setting.map_default_pen));
	GMT_message (GMT, "\t-Z Use cpt-file to assign colors based on depth-value in 3rd column\n");
	GMT_message (GMT, "\t-a plots axis. Default symbols are circles.\n");
	GMT_message (GMT, "\t-p draws P_symbol outline using the current pen (see -W) or sets pen attribute for outline.\n");
	GMT_message (GMT, "\t-t draws T_symbol outline using the current pen (see -W) or sets pen attribute for outline.\n");
	GMT_message (GMT, "\t-g Sets color used for P_symbol. [default is compressive parts color]\n");
	GMT_message (GMT, "\t-e Sets color used for T_symbol. [default is extensive parts color]\n");
	GMT_explain_options (GMT, "Xchi:.");

	return (EXIT_FAILURE);
}

void distaz (double lat1,double lon1,double lat2,double lon2,double *distrad,double *distdeg,double *distkm,double *az12rad,double *az12deg,double *az21rad,double *az21deg,GMT_LONG syscoord);

GMT_LONG GMT_pscoupe_parse (struct GMTAPI_CTRL *C, struct PSCOUPE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pscoupe and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, no_size_needed, syscoord;
	char txt[GMT_LONG_TEXT], *p = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;
	double lon1, lat1, lon2, lat2, tmp1, tmp2, tmp3, tmp4, tmp5;
	char newfile[GMT_LONG_TEXT], extracted_file[GMT_LONG_TEXT];

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':   /* Cross-section definition */
				Ctrl->A.active = TRUE;
				Ctrl->A.proj_type = opt->arg[0];
				if (opt->arg[strlen (opt->arg)-1] == 'f') Ctrl->A.frame = TRUE;
				if (Ctrl->A.proj_type == 'a' || Ctrl->A.proj_type == 'c') {
					sscanf (&opt->arg[1], "%lf/%lf/%lf/%lf/%lf/%lf/%lf/%lf",
						&lon1, &lat1, &lon2, &lat2, &Ctrl->A.PREF.dip, &Ctrl->A.p_width, &Ctrl->A.dmin, &Ctrl->A.dmax);
					syscoord = Ctrl->A.proj_type == 'a' ? 0 : 2;
					distaz (lat1, lon1, lat2, lon2, &tmp1, &tmp2, &Ctrl->A.p_length, &tmp3, &Ctrl->A.PREF.str,&tmp4, &tmp5, syscoord);
					sprintf (newfile, "A%c%.1f_%.1f_%.1f_%.1f_%.0f_%.0f_%.0f_%.0f",
						Ctrl->A.proj_type, lon1, lat1, lon2, lat2, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
					sprintf (extracted_file,"A%c%.1f_%.1f_%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_map",
						Ctrl->A.proj_type, lon1, lat1, lon2, lat2, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
				} else if (Ctrl->A.proj_type == 'b' || Ctrl->A.proj_type == 'd') {
					sscanf (&opt->arg[1], "%lf/%lf/%lf/%lf/%lf/%lf/%lf/%lf",
						&lon1, &lat1, &Ctrl->A.PREF.str, &Ctrl->A.p_length, &Ctrl->A.PREF.dip, &Ctrl->A.p_width, &Ctrl->A.dmin, &Ctrl->A.dmax);
					sprintf (newfile, "A%c%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_%.0f_%.0f",
						Ctrl->A.proj_type, lon1, lat1, Ctrl->A.PREF.str, Ctrl->A.p_length, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
					sprintf (extracted_file,"A%c%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_%.0f_%.0f_map",
						Ctrl->A.proj_type, lon1, lat1, Ctrl->A.PREF.str, Ctrl->A.p_length, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
				}
				Ctrl->A.PREF.rake = 0.;
				Ctrl->A.pnew = fopen (newfile, "w");
				Ctrl->A.pextract = fopen (extracted_file, "w");
				if (Ctrl->A.proj_type == 'a' || Ctrl->A.proj_type == 'b')
					Ctrl->A.fuseau = gutm (lon1, lat1, &Ctrl->A.xlonref, &Ctrl->A.ylatref, Ctrl->A.fuseau);
				else {
					Ctrl->A.fuseau = - 1;
					Ctrl->A.xlonref = lon1;
					Ctrl->A.ylatref = lat1;
				}
				Ctrl->A.polygon = TRUE;
				break;

			case 'E':    /* Set color for extensive parts  */
				if (!opt->arg[0] || (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->E2.fill))) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				Ctrl->A.polygon = TRUE;
				break;
			case 'G':    /* Set color for compressive parts */
				Ctrl->G.active = TRUE;
				if (!opt->arg[0] || (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->G.fill))) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				Ctrl->A.polygon = TRUE;
				break;
			case 'L':    /* Draw outline [set outline attributes] */
				Ctrl->L.active = TRUE;
				if (opt->arg[0] && GMT_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					GMT_pen_syntax (GMT, 'L', " ");
					n_errors++;
				}
				break;
			case 'M':    /* Same size for any magnitude */
				Ctrl->M.active = TRUE;
				break;
			case 'N':    /* Do not skip points outside border */
				Ctrl->N.active = TRUE;
				break;
			case 'S':    /* Mechanisms : get format [and size] */
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
					if (opt->arg[strlen (opt->arg)-1] == 'u') Ctrl->S.justify = 10;
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
						Ctrl->S.plotmode = PLOT_TENSOR;
						Ctrl->S.zerotrace = TRUE;
						break;
					case 'd':
						Ctrl->S.readmode = READ_TENSOR;
						Ctrl->S.plotmode = PLOT_DC;
						Ctrl->S.zerotrace = TRUE;
						break;
					case 'z':
						Ctrl->S.readmode = READ_TENSOR;
						Ctrl->S.plotmode = PLOT_TRACE;
						Ctrl->S.zerotrace = TRUE;
						break;
					default:
						n_errors++;
						break;
				}
				break;

			case 's':    /* Only points : get symbol [and size] */
				Ctrl->S.active2 = TRUE;
				Ctrl->S.type = opt->arg[0];
				p = NULL;	strcpy (txt, &opt->arg[1]);
				if ((p = strchr (txt, '/'))) p[0] = '\0';
				Ctrl->S.scale = GMT_to_inch (GMT, txt);
				if ((p = strstr (opt->arg, "/"))) {
					sscanf (p, "/%lf/%lf", &Ctrl->S.fontsize, &Ctrl->S.offset);
					if (GMT_IS_ZERO (Ctrl->S.fontsize)) Ctrl->S.fontsize = DEFAULT_FONTSIZE;
					if (Ctrl->S.fontsize < 0.0) Ctrl->S.no_label = TRUE;
					if (GMT_IS_ZERO (Ctrl->S.offset)) Ctrl->S.offset = DEFAULT_OFFSET;
					if (opt->arg[strlen (opt->arg)-1] == 'u') Ctrl->S.justify = 10;
				}

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
						GMT_message (GMT, "Syntax error -s option:  Unrecognized symbol type %c\n", Ctrl->S.type);
						break;
				}
				if (GMT_IS_ZERO (Ctrl->S.scale)) Ctrl->S.read_size = TRUE;
				Ctrl->S.size = Ctrl->S.scale;
				break;

			case 'T':
				Ctrl->T.active = TRUE;
				sscanf (opt->arg, "%ld", &Ctrl->T.n_plane);
				if (strlen (opt->arg) > 2 && GMT_getpen (GMT, &opt->arg[2], &Ctrl->T.pen)) {	/* Set transparent attributes */
					GMT_pen_syntax (GMT, 'T', " ");
					n_errors++;
				}
				break;
			case 'W':    /* Set line attributes */
				Ctrl->W.active = TRUE;
				if (opt->arg && GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				break;
			case 'Z':    /* Vary symbol color with z */
				Ctrl->Z.active = TRUE;
				Ctrl->Z.file = strdup (opt->arg);
				break;
			case 'a':    /* plot axis */
				Ctrl->a2.active = TRUE;
				if (!opt->arg[0]) {	/* Set defaults */
					strcpy (txt,"0.08i");
					Ctrl->a2.P_sym_type = 'c';
					Ctrl->a2.T_sym_type = 'c';
				}
				else {
					strcpy (txt, &opt->arg[1]);
					p = NULL;	strcpy (txt, &opt->arg[1]);
					if ((p = strchr (txt, '/'))) p[0] = '\0';
					Ctrl->A.size = GMT_to_inch (GMT, txt);
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
			case 'e':    /* Set color for T axis symbol */
				Ctrl->E2.active = TRUE;
				if (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->E2.fill)) {
					GMT_fill_syntax (GMT, 'e', " ");
					n_errors++;
				}
				break;
			case 'g':    /* Set color for P axis symbol */
				Ctrl->E2.active = TRUE;
				if (opt->arg[0] && GMT_getfill (GMT, opt->arg, &Ctrl->G2.fill)) {
					GMT_fill_syntax (GMT, 'g', " ");
					n_errors++;
				}
				break;
			case 'p':    /* Draw outline of P axis symbol [set outline attributes] */
				Ctrl->P2.active = TRUE;
				if (opt->arg[0] && GMT_getpen (GMT, opt->arg, &Ctrl->P2.pen)) {
					GMT_pen_syntax (GMT, 'p', " ");
					n_errors++;
				}
				break;
			case 't':    /* Draw outline of T axis symbol [set outline attributes] */
				Ctrl->T2.active = TRUE;
				if (opt->arg[0] && GMT_getpen (GMT, opt->arg, &Ctrl->T2.pen)) {
					GMT_pen_syntax (GMT, 't', " ");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	no_size_needed = (Ctrl->S.readmode == READ_CMT || Ctrl->S.readmode == READ_PLANES || Ctrl->S.readmode == READ_AKI || Ctrl->S.readmode == READ_TENSOR || Ctrl->S.readmode == READ_AXIS);
	n_errors += GMT_check_condition (GMT, !Ctrl->A.active, "Syntax error:  Must specify -A option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error:  Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, no_size_needed && Ctrl->S.symbol > 0, "Syntax error:  -S must specify scale\n");
	n_errors += GMT_check_condition (GMT, !no_size_needed && (Ctrl->S.active > 1 && Ctrl->S.scale <= 0.0), "Syntax error:  -S must specify scale\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_pscoupe_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_pscoupe (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG ix, iy, n_fields, n_rec = 0, n_plane_old = 0, form = 0, error;
	GMT_LONG P_sym = 0, T_sym = 0, no_size_needed, greenwich, old_is_world;
	GMT_LONG i, transparence_old = 0, not_defined = 0;

	double xy[2], plot_x, plot_y, angle = 0.0, n_dep, distance, fault, depth;
	double P_x, P_y, T_x, T_y;

	char event_title[BUFSIZ], *line, col[15][GMT_TEXT_LEN];

	struct nodal_plane NP1, NP2;
	st_me meca, mecar;
	struct MOMENT moment;
	struct M_TENSOR mt, mtr;
	struct AXIS T, N, P, Tr, Nr, Pr;
	struct GMT_PALETTE *CPT = NULL;

	struct PSCOUPE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_pscoupe_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_pscoupe_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pscoupe", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJR:", "BHKOPUVXhixYyc>", options))) Return (error);
	Ctrl = (struct PSCOUPE_CTRL *)New_pscoupe_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pscoupe_parse (API, Ctrl, options))) Return (error);
 	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the pscoupe main code ----------------------------*/

	event_title[0] = 0;
	GMT_memset (&meca, 1, meca);
	GMT_memset (&moment, 1, moment);

	no_size_needed = (Ctrl->S.readmode == READ_CMT || Ctrl->S.readmode == READ_PLANES || Ctrl->S.readmode == READ_AKI || Ctrl->S.readmode == READ_TENSOR || Ctrl->S.readmode == READ_AXIS);

	if (Ctrl->Z.active) {
		if ((error = GMT_Begin_IO (API, GMT_IS_CPT, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->Z.file, (void **)&CPT)) Return (GMT_DATA_READ_ERROR);
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
	}

	if (Ctrl->A.frame) {
		GMT->common.R.wesn[XLO] = 0.0;
		GMT->common.R.wesn[XHI] = Ctrl->A.p_length;
		GMT->common.R.wesn[YLO] = Ctrl->A.dmin;
		GMT->common.R.wesn[YHI] = Ctrl->A.dmax;
		if (GMT_IS_ZERO (Ctrl->A.PREF.dip)) Ctrl->A.PREF.dip = 1.0;
	}

	greenwich = (GMT->common.R.wesn[XLO] < 0.0 || GMT->common.R.wesn[XHI] <= 0.0);

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	GMT_plotinit (API, PSL, options);

	PSL_setfont (PSL, GMT->current.setting.font_annot[0].id);

	GMT_setpen (GMT, PSL, &Ctrl->W.pen);
	if (!Ctrl->N.active) GMT_map_clip_on (GMT, PSL, GMT->session.no_rgb, 3);

	old_is_world = GMT->current.map.is_world;

	ix = (GMT->current.setting.io_lonlat_toggle[0]);    iy = 1 - ix;

	if ((error = GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */

	while ((n_fields = GMT_Get_Record (API, GMT_READ_TEXT, (void **)&line)) != EOF) {	/* Keep returning records until we have no more files */

 		if (GMT_REC_IS_ERROR (GMT)) Return (EXIT_FAILURE);
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */

		n_rec++;
		if (Ctrl->S.readmode == READ_CMT) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6],
				col[7], col[8], col[9], col[10], col[11], col[12], event_title);
			if (strlen (event_title) <= 0) sprintf (event_title,"\n");
		}
		else if (Ctrl->S.readmode == READ_AKI) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6],
				col[7], col[8], event_title);
			if (strlen (event_title) <= 0) sprintf (event_title,"\n");
		}
		else if (Ctrl->S.readmode == READ_PLANES) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6],
				col[7], col[8], col[9], event_title);
			if (strlen (event_title) <= 0) sprintf (event_title,"\n");
		}
		else if (Ctrl->S.readmode == READ_AXIS) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7],
				col[8], col[9], col[10], col[11], col[12], col[13], col[14], event_title);
			if (strlen (event_title) <= 0) sprintf (event_title,"\n");
		}
		else if (Ctrl->S.readmode == READ_TENSOR) {
			sscanf (line, "%s %s %s %s %s %s %s %s %s %s %s %s %[^\n]\n",
				col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7],
				col[8], col[9], col[10], col[11], event_title);
			if (strlen (event_title) <= 0) sprintf (event_title,"\n");
		}
		else if (Ctrl->S.symbol > 0 && Ctrl->S.read_size) {
			sscanf (line, "%s %s %s %s %[^\n]\n", col[0], col[1], col[2], col[3], event_title);
		}
		else
			sscanf (line, "%s %s %s %[^\n]\n", col[0], col[1], col[2], event_title);

 		if ((GMT_scanf (GMT, col[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X], &xy[ix]) == GMT_IS_NAN) || (GMT_scanf (GMT, col[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y], &xy[iy]) == GMT_IS_NAN)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad x and/or y coordinates, skip)\n", n_rec);
			continue;
		}
		depth = atof (col[2]);

		if (dans_coupe (xy[0], xy[1], depth, Ctrl->A.xlonref, Ctrl->A.ylatref, Ctrl->A.fuseau, Ctrl->A.PREF.str,
			Ctrl->A.PREF.dip, Ctrl->A.p_length, Ctrl->A.p_width, &distance, &n_dep)) {
			xy[0] = distance;
			xy[1] = n_dep;
		}
		else
			xy[0] = xy[1] = -1.0;

		if (!Ctrl->N.active) {
			GMT_map_outside (GMT, xy[GMT_X], xy[GMT_Y]);
			if (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1) continue;
		}

		if (Ctrl->Z.active) GMT_get_rgb_from_z (GMT, CPT, depth, Ctrl->G.fill.rgb);

		GMT_geo_to_xy (GMT, xy[0], xy[1], &plot_x, &plot_y);

		if (Ctrl->S.symbol > 0) {
			if (Ctrl->S.read_size) Ctrl->S.size = GMT_to_inch (GMT, col[3]);
			fprintf (Ctrl->A.pnew, "%f %f %f %s\n", distance, n_dep, depth, col[3]);
			fprintf (Ctrl->A.pextract, "%s", line);

			GMT_setfill (GMT, PSL, &Ctrl->G.fill, Ctrl->L.active);
			PSL_plotsymbol (PSL, plot_x, plot_y, &Ctrl->S.size, Ctrl->S.symbol);
		} else if (Ctrl->S.readmode == READ_CMT) {
			meca.NP1.str = atof (col[3]);
			meca.NP1.dip = atof (col[4]);
			meca.NP1.rake = atof (col[5]);
			meca.NP2.str = atof (col[6]);
			meca.NP2.dip = atof (col[7]);
			meca.NP2.rake = atof (col[8]);
			moment.mant = atof (col[9]);
			moment.exponent = atoi (col[10]);
			if (moment.exponent == 0) meca.magms = atof (col[9]);
			rot_meca (meca, Ctrl->A.PREF, &mecar);
		} else if (Ctrl->S.readmode == READ_AKI) {
			NP1.str = atof (col[3]);
			NP1.dip = atof (col[4]);
			NP1.rake = atof (col[5]);
			meca.magms = atof (col[6]);

			moment.exponent = 0;
			moment.mant = meca.magms;
			define_second_plane (NP1, &NP2);

			meca.NP1.str = NP1.str;
			meca.NP1.dip = NP1.dip;
			meca.NP1.rake = NP1.rake;
			meca.NP2.str = NP2.str;
			meca.NP2.dip = NP2.dip;
			meca.NP2.rake = NP2.rake;

			rot_meca (meca, Ctrl->A.PREF, &mecar);

		} else if (Ctrl->S.readmode == READ_PLANES) {
			meca.NP1.str = atof (col[3]);
			meca.NP1.dip = atof (col[4]);
			meca.NP2.str = atof (col[5]);
			fault = atof (col[6]);
			meca.magms = atof (col[7]);

			moment.exponent = 0;
			moment.mant = meca.magms;
			meca.NP2.dip = computed_dip2 (meca.NP1.str, meca.NP1.dip, meca.NP2.str);
			if (meca.NP2.dip == 1000.0) {
				not_defined = TRUE;
				transparence_old = Ctrl->T.active;
				n_plane_old = Ctrl->T.n_plane;
				Ctrl->T.active = TRUE;
				Ctrl->T.n_plane = 1;
				meca.NP1.rake = 1000.0;
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: second plane is not defined for event %s only first plane is plotted.\n", line);
			}
			else {
				meca.NP1.rake = computed_rake2 (meca.NP2.str, meca.NP2.dip, meca.NP1.str, meca.NP1.dip, fault);
			}
			meca.NP2.rake = computed_rake2 (meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip, fault);

			rot_meca (meca, Ctrl->A.PREF, &mecar);

		} else if (Ctrl->S.readmode == READ_AXIS) {
			T.val = atof (col[3]);
			T.str = atof (col[4]);
			T.dip = atof (col[5]);
			T.e = atoi (col[12]);

			N.val = atof (col[6]);
			N.str = atof (col[7]);
			N.dip = atof (col[8]);
			N.e = atoi (col[12]);

			P.val = atof (col[9]);
			P.str = atof (col[10]);
			P.dip = atof (col[11]);
			P.e = atoi (col[12]);

/*
F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998, p.167.
Definition of scalar moment.
*/
			meca.moment.exponent = T.e;
			meca.moment.mant = sqrt (squared(T.val) + squared (N.val) + squared (P.val)) / M_SQRT2;
			meca.magms = 0.;

/* normalization by M0 */
			T.val /= meca.moment.mant;
			N.val /= meca.moment.mant;
			P.val /= meca.moment.mant;

			rot_axis(T, Ctrl->A.PREF, &Tr);
			rot_axis(N, Ctrl->A.PREF, &Nr);
			rot_axis(P, Ctrl->A.PREF, &Pr);
			Tr.val = T.val;
			Nr.val = N.val;
			Pr.val = P.val;
			Tr.e = T.e;
			Nr.e = N.e;
			Pr.e = P.e;

			if (Ctrl->S.plotmode == PLOT_DC || Ctrl->T.active) {
				axe2dc(Tr, Pr, &NP1, &NP2);
				meca.NP1.str = NP1.str;
				meca.NP1.dip = NP1.dip;
				meca.NP1.rake = NP1.rake;
				meca.NP2.str = NP2.str;
				meca.NP2.dip = NP2.dip;
				meca.NP2.rake = NP2.rake;
			}

		} else if (Ctrl->S.readmode == READ_TENSOR) {
			for (i = 3; i < 9; i++) mt.f[i-3] = atof (col[i]);
			mt.expo = atoi (col[i]);

			moment.exponent = mt.expo;
/*
F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998, p.167.
Definition of scalar moment.
*/
			moment.mant = sqrt (squared (mt.f[0]) + squared (mt.f[1]) + squared (mt.f[2]) + 2.0 * (squared (mt.f[3]) + squared (mt.f[4]) + squared (mt.f[5]))) / M_SQRT2;
			meca.magms = 0.0;

/* normalization by M0 */
			for(i = 0; i <= 5; i++) mt.f[i] /= moment.mant;

			rot_tensor (mt, Ctrl->A.PREF, &mtr);
			GMT_momten2axe (GMT, mtr, &T, &N, &P);

			if (Ctrl->S.plotmode == PLOT_DC || Ctrl->T.active) {
				axe2dc (T, P, &NP1, &NP2);
				meca.NP1.str = NP1.str;
				meca.NP1.dip = NP1.dip;
				meca.NP1.rake = NP1.rake;
				meca.NP2.str = NP2.str;
				meca.NP2.dip = NP2.dip;
				meca.NP2.rake = NP2.rake;
			}
		}
		if (no_size_needed) {
			if (Ctrl->M.active) {
				moment.mant = 4.0;
				moment.exponent = 23;
			}

			Ctrl->S.size = (computed_mw (moment, meca.magms) / 5.0) * Ctrl->S.scale;

			fprintf (Ctrl->A.pextract, "%s", line);
			if (Ctrl->S.readmode == READ_AXIS) {
				fprintf (Ctrl->A.pnew, "%f %f %f %f %f %f %f %f %f %f %f %f %ld 0 0 %s\n",
					xy[0], xy[1], depth, Tr.val, Tr.str, Tr.dip, Nr.val, Nr.str, Nr.dip,
					Pr.val, Pr.str, Pr.dip, moment.exponent, event_title);
				T = Tr;
				N = Nr;
				P = Pr;
			}
			else if (Ctrl->S.readmode == READ_TENSOR) {
				fprintf (Ctrl->A.pnew, "%f %f %f %f %f %f %f %f %f %ld 0 0 %s\n",
					xy[0], xy[1], depth, mtr.f[0], mtr.f[1], mtr.f[2], mtr.f[3], mtr.f[4], mtr.f[5],
					moment.exponent, event_title);
				mt = mtr;
			}
			else {
				fprintf (Ctrl->A.pnew, "%f %f %f %f %f %f %f %f %f %f %ld 0 0 %s\n",
					xy[0], xy[1], depth, mecar.NP1.str, mecar.NP1.dip, mecar.NP1.rake,
					mecar.NP2.str, mecar.NP2.dip, mecar.NP2.rake,
					moment.mant, moment.exponent, event_title);
				meca = mecar;
			}

			if (Ctrl->S.plotmode == PLOT_TENSOR) {
				GMT_setpen (GMT, PSL, &Ctrl->L.pen);
				ps_tensor (GMT, PSL, plot_x, plot_y, Ctrl->S.size, T, N, P, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active, Ctrl->S.zerotrace);
			}

			if (Ctrl->S.zerotrace) {
				GMT_setpen (GMT, PSL, &Ctrl->W.pen);
				ps_tensor (GMT, PSL, plot_x, plot_y, Ctrl->S.size, T, N, P, NULL, NULL, TRUE, TRUE);
			}

			if (Ctrl->T.active) {
				GMT_setpen (GMT, PSL, &Ctrl->T.pen);
				if (Ctrl->T.n_plane == 0) ps_meca (GMT, PSL, plot_x, plot_y, meca, Ctrl->S.size);
				else ps_plan (GMT, PSL, plot_x, plot_y, meca, Ctrl->S.size, Ctrl->T.n_plane);
				if (not_defined) {
					not_defined = FALSE;
					Ctrl->T.active = transparence_old;
					Ctrl->T.n_plane = n_plane_old;
				}
			} else if (Ctrl->S.plotmode == PLOT_DC) {
				GMT_setpen (GMT, PSL, &Ctrl->L.pen);
				ps_mechanism (GMT, PSL, plot_x, plot_y, meca, Ctrl->S.size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
			}

			if (Ctrl->a2.active) {
				if (Ctrl->S.readmode == READ_TENSOR || Ctrl->S.readmode == READ_AXIS)
					axis2xy (plot_x, plot_y, Ctrl->S.size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
				else {
					dc_to_axe (meca, &T, &N, &P);
					axis2xy (plot_x, plot_y, Ctrl->S.size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
				}
				GMT_setpen (GMT, PSL, &Ctrl->P2.pen);
				GMT_setfill (GMT, PSL, &Ctrl->G2.fill, Ctrl->P2.active);
				PSL_plotsymbol (PSL, P_x, P_y, &Ctrl->A.size, P_sym);
				GMT_setpen (GMT, PSL, &Ctrl->T2.pen);
				GMT_setfill (GMT, PSL, &Ctrl->E2.fill, Ctrl->E2.active);
				PSL_plotsymbol (PSL, T_x, T_y, &Ctrl->A.size, T_sym);
			}
		}

		if (!Ctrl->S.no_label) {
			GMT_setpen (GMT, PSL, &Ctrl->W.pen);
			switch (Ctrl->S.justify) {
				case 2 :
					PSL_plottext (PSL, plot_x, plot_y + Ctrl->S.size * 0.5 + Ctrl->S.offset, Ctrl->S.fontsize, event_title, angle, Ctrl->S.justify, form);
					PSL_plotpoint (PSL, plot_x, plot_y + Ctrl->S.size * 0.5 + Ctrl->S.offset, PSL_DRAW + PSL_STROKE);
					break;
				case 10 :
					PSL_plottext (PSL, plot_x, plot_y - Ctrl->S.size * 0.5 - Ctrl->S.offset, Ctrl->S.fontsize, event_title, angle, Ctrl->S.justify, form);
					PSL_plotpoint (PSL, plot_x, plot_y - Ctrl->S.size * 0.5 - Ctrl->S.offset, PSL_DRAW + PSL_STROKE);
					break;
			}
		}
	}

	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	GMT_report (GMT, GMT_MSG_NORMAL, "Number of records read: %li\n", n_rec);

	if (!Ctrl->N.active) GMT_map_clip_off (GMT, PSL);

	PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);

	if (Ctrl->W.pen.style) PSL_setdash (PSL, CNULL, 0);

 	if (Ctrl->Z.active) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&CPT);

	GMT_map_basemap (GMT, PSL);

	GMT_plotend (GMT, PSL);

	Return (GMT_OK);
}
