/*--------------------------------------------------------------------
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*

psmeca will read focal mechanisms from input file and plot beachballs on a map.
Focal mechanisms are specified in double couple, moment tensor, or principal axis.
PostScript code is written to stdout.

 Author:	Genevieve Patau
 Date:		7 July 1998
 Version:	5
 Roots:		based on psxy.c, ported to GMT 5 by P. Wessel
 */

#include "gmt_dev.h"
#include "meca.h"
#include "utilmeca.h"

#define THIS_MODULE_CLASSIC_NAME	"psmeca"
#define THIS_MODULE_MODERN_NAME	"meca"
#define THIS_MODULE_LIB		"seis"
#define THIS_MODULE_PURPOSE	"Plot focal mechanisms"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYdehipqt" GMT_OPT("Hc")

#define DEFAULT_FONTSIZE		9.0	/* In points */
#define DEFAULT_OFFSET			3.0	/* In points */
#define DEFAULT_SYMBOL_SIZE		6.0 /* In points */

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
	struct C {	/* -C[<pen>][+s<size>] */
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
	struct S {	/* -S<format><scale>[+a<angle>][+f<font>][+j<justify>][+o<dx>[/<dy>]] */
		bool active;
		bool no_label;
		unsigned int readmode;
		unsigned int plotmode;
		unsigned int n_cols;
		double scale;
		double angle;
		int justify;
		double offset[2];
		struct GMT_FONT font;
	} S;
	struct T {	/* -Tnplane[/<pen>] */
		bool active;
		unsigned int n_plane;
		struct GMT_PEN pen;
	} T;
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<cpt> */
		bool active;
		char *file;
	} Z;
	struct A2 {	/* -Fa[size[/Psymbol[Tsymbol]]] */
		bool active;
		char P_symbol, T_symbol;
		double size;
	} A2;
	struct E2 {	/* -Fe<fill> */
		bool active;
		struct GMT_FILL fill;
	} E2;
	struct G2 {	/* -Fg<fill> */
		bool active;
		struct GMT_FILL fill;
	} G2;
	struct P2 {	/* -Fp[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} P2;
	struct R2 {	/* -Fr[<fill>] */
		bool active;
		struct GMT_FILL fill;
	} R2;
	struct T2 {	/* -Ft[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} T2;
	struct O2 {	/* -Fo */
		bool active;
	} O2;
	struct Z2 {	/* -Fz[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} Z2;
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
	C->L.active = false;
	gmt_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	gmt_init_fill (GMT, &C->R2.fill, 1.0, 1.0, 1.0);
	C->S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	C->S.font.size = DEFAULT_FONTSIZE;
	C->S.justify = PSL_TC;
	C->A2.size = DEFAULT_SYMBOL_SIZE * GMT->session.u2u[GMT_PT][GMT_INCH];
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

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s\n", name, GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t-S<format><scale>[+a<angle>][+f<font>][+j<justify>][+o<dx>[/<dy>]] [%s]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C[<pen>][+s<size>]] [-D<depmin>/<depmax>] [-E<fill>] [-G<fill>] %s[-L<pen>] [-M]\n", API->K_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fa[<size>[/<Psymbol>[<Tsymbol>]]] [-Fe<fill>] [-Fg<fill>] [-Fo] [-Fr<fill>] [-Fp[<pen>]] [-Ft[<pen>]] [-Fz[<pen>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N] %s%s[-T<nplane>[/<pen>]] [%s] [%s] [-W<pen>]\n", API->O_OPT, API->P_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-Z<cpt>]\n", GMT_X_OPT, GMT_Y_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s[%s] [%s] [%s]\n\t[%s]\n\t[%s]\n\t[%s] [%s] [%s] [%s]\n\n", API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select format type and symbol size.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append the format code for your input file:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a  Focal mechanism in Aki & Richard's convention:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth strike dip rake mag [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c  Focal mechanism in Global CMT convention\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth strike1 dip1 rake1 strike2 dip2 rake2 moment [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      with moment in 2 columns : mantissa and exponent corresponding to seismic moment in dynes-cm\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d  Closest double couple defined from seismic moment tensor (zero trace and zero determinant):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p  Focal mechanism defined with:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth strike1 dip1 strike2 fault mag [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      fault = -1/+1 for a normal/inverse fault\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m  Seismic (full) moment tensor:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t  Zero trace moment tensor defined from principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x  Principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   y  Best double couple defined from principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z  Deviatoric part of the moment tensor (zero trace):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Fo option for old (psvelomeca) format (no depth in third column).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally add +a<angle>+f<font>+j<justify>+o<dx>[/<dy>] to change the label angle, font (size,fontname,color), justification and offset.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   fontsize < 0 : no label written; offset is from the limit of the beach ball.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Offset focal mechanisms to the latitude and longitude specified in the last two columns of the input file before label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default pen attributes are set by -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A line is plotted between both positions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A small circle is plotted at the initial location. Append +s<size> to change the size of the circle.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Plot events between <depmin> and <depmax> deep.\n");
	gmt_fill_syntax (API->GMT, 'E', NULL, "Set filling of extensive quadrants [Default is white].");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Sets various attributes of symbols depending on <mode>:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a Plot axis. Default symbols are circles; otherwise append <size>[/<Psymbol>[<Tsymbol>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e Append filling for the T axis symbol [default as set by -E].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g Append filling for the P axis symbol [default as set by -G].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   o Use psvelomeca format (Without depth in third column).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p Draw P_symbol outline using the default pen (see -W) or append pen attribute for outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t Draw T_symbol outline using the default pen (see -W) or append pen attribute for outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r Draw box behind labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z Overlay zero trace moment tensor using default pen (see -W) or append outline pen.\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Set filling of compressive quadrants [Default is black].");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Sets pen attribute for outline other than the default set by -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Same size for any magnitude. Size is given with -S.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not skip/clip beach balls that fall outside map border [Default will ignore those outside].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Tn[/<pen>] Draw nodal planes and circumference only to provide a transparent\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   beach ball using the default pen (see -W) or sets pen attribute. \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 1 the only first nodal plane is plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 2 the only second nodal plane is plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 0 both nodal planes are plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If moment tensor is required, nodal planes overlay moment tensor.\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set pen attributes [%s].\n", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Use CPT to assign colors based on depth-value in 3rd column.\n");
	GMT_Option (API, "X,c,di,e,h,i,p,qi,t,:,.");

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
				if ((p = strstr (txt, "+s")) != NULL) Ctrl->C.size = gmt_M_to_inch (GMT, (p+2));
				else if ((p = strchr (txt, 'P')) != NULL) Ctrl->C.size = gmt_M_to_inch (GMT, (p+1));
				if (txt[0] != 'P' && strncmp (txt, "+s", 2U)) {	/* Have a pen up front */
					if (p) p[0] = '\0';
					if (gmt_getpen (GMT, txt, &Ctrl->C.pen)) {
						gmt_pen_syntax (GMT, 'C', NULL, " ", 0);
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
					gmt_fill_syntax (GMT, 'E', NULL, " ");
					n_errors++;
				}
				break;
			case 'F':	/* Repeatable; Controls various symbol attributes  */
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'a':	/* plot axis */
						Ctrl->A2.active = true;
						strncpy (txt, &opt->arg[1], GMT_LEN256-1);
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
							gmt_fill_syntax (GMT, ' ', "Fe", " ");
							n_errors++;
						}
						break;
					case 'g':	/* Set color for P axis symbol */
						Ctrl->G2.active = true;
						if (gmt_getfill (GMT, &opt->arg[1], &Ctrl->G2.fill)) {
							gmt_fill_syntax (GMT, ' ', "Fg", " ");
							n_errors++;
						}
						break;
					case 'p':	/* Draw outline of P axis symbol [set outline attributes] */
						Ctrl->P2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->P2.pen)) {
							gmt_pen_syntax (GMT, ' ', "Fp", " ", 0);
							n_errors++;
						}
						break;
					case 'r':	/* draw box around text */
						Ctrl->R2.active = true;
						if (opt->arg[1] && gmt_getfill (GMT, &opt->arg[1], &Ctrl->R2.fill)) {
							gmt_fill_syntax (GMT, ' ', "Fr", " ");
							n_errors++;
						}
						break;
					case 't':	/* Draw outline of T axis symbol [set outline attributes] */
						Ctrl->T2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->T2.pen)) {
							gmt_pen_syntax (GMT, ' ', "Ft", " ", 0);
							n_errors++;
						}
						break;
					case 'o':	/* use psvelomeca format (without depth in 3rd column) */
						Ctrl->O2.active = true;
						break;
					case 'z':	/* overlay zerotrace moment tensor */
						Ctrl->Z2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->Z2.pen)) { /* Set pen attributes */
							gmt_pen_syntax (GMT, ' ', "Fz", " ", 0);
							n_errors++;
						}
						break;
				}
				break;
			case 'G':	/* Set color for compressive parts */
				Ctrl->G.active = true;
				if (!opt->arg[0] || (opt->arg[0] && gmt_getfill (GMT, opt->arg, &Ctrl->G.fill))) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'L':	/* Draw outline [set outline attributes] */
				Ctrl->L.active = true;
				if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					gmt_pen_syntax (GMT, 'L', NULL, " ", 0);
					n_errors++;
				}
				break;
			case 'M':	/* Same size for any magnitude */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get format and size */
				Ctrl->S.active = true;
				switch (opt->arg[0]) {	/* parse format */
					case 'c':
						Ctrl->S.readmode = READ_CMT;	Ctrl->S.n_cols = 11;
						break;
					case 'a':
						Ctrl->S.readmode = READ_AKI;	Ctrl->S.n_cols = 7;
						break;
					case 'p':
						Ctrl->S.readmode = READ_PLANES;	Ctrl->S.n_cols = 8;
						break;
					case 'x':
						Ctrl->S.readmode = READ_AXIS;	Ctrl->S.n_cols = 13;
						break;
					case 'y':
						Ctrl->S.readmode = READ_AXIS;	Ctrl->S.n_cols = 13;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 't':
						Ctrl->S.readmode = READ_AXIS;	Ctrl->S.n_cols = 13;
						Ctrl->S.plotmode = PLOT_TRACE;
						break;
					case 'm':
						Ctrl->S.readmode = READ_TENSOR;	Ctrl->S.n_cols = 10;
						break;
					case 'd':
						Ctrl->S.readmode = READ_TENSOR;	Ctrl->S.n_cols = 10;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 'z':
						Ctrl->S.readmode = READ_TENSOR;	Ctrl->S.n_cols = 10;
						Ctrl->S.plotmode = PLOT_TRACE;
						break;
					default:
						n_errors++;
						break;
				}

				if ((strstr (opt->arg, "+a")) || (strstr (opt->arg, "+f")) || strstr (opt->arg, "+o") || strstr (opt->arg, "+j")) {
					/* New syntax: -S<format><scale>+a<angle>+f<font>+o<dx>/<dy>+j<justify> */
					char word[GMT_LEN256] = {""}, *c = NULL;

					/* parse beachball size */
					if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
					Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[1]);
					if (c) c[0] = '+';	/* Restore modifiers */

					if (gmt_get_modifier (opt->arg, 'a', word))
						Ctrl->S.angle = atof(word);
					if (gmt_get_modifier (opt->arg, 'j', word) && strchr ("LCRBMT", word[0]) && strchr ("LCRBMT", word[1]))
						Ctrl->S.justify = gmt_just_decode (GMT, word, Ctrl->S.justify);
					if (gmt_get_modifier (opt->arg, 'f', word))
						n_errors += gmt_getfont (GMT, word, &(Ctrl->S.font));
					if (gmt_get_modifier (opt->arg, 'o', word)) {
						if (gmt_get_pair (GMT, word, GMT_PAIR_DIM_DUP, Ctrl->S.offset) < 0) n_errors++;
					} else {	/* Set default offset */
						if (Ctrl->S.justify%4 != 2) /* Not center aligned */
							Ctrl->S.offset[0] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
						if (Ctrl->S.justify/4 != 1) /* Not middle aligned */
							Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
					}
					if (Ctrl->S.font.size <= 0.0) Ctrl->S.no_label = true;
				} else {	/* Old syntax: -S<format><scale>[/fontsize[/offset]][+u] */
					Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];	/* Set default offset */
					if ((p = strstr (opt->arg, "+u"))) {
						Ctrl->S.justify = PSL_BC;
						p[0] = '\0';	/* Chop off modifier */
					} else if (opt->arg[strlen(opt->arg)-1] == 'u') {
						Ctrl->S.justify = PSL_BC;
						opt->arg[strlen(opt->arg)-1] = '\0';
					}
					txt[0] = txt_b[0] = txt_c[0] = '\0';
					sscanf (&opt->arg[1], "%[^/]/%[^/]/%s", txt, txt_b, txt_c);
					if (txt[0]) Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
					if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
					if (txt_c[0]) Ctrl->S.offset[1] = gmt_convert_units (GMT, txt_c, GMT_PT, GMT_INCH);
					if (Ctrl->S.font.size < 0.0) Ctrl->S.no_label = true;
					if (p) p[0] = '+';	/* Restore modifier */
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				sscanf (opt->arg, "%d", &Ctrl->T.n_plane);
				if (strlen (opt->arg) > 2 && gmt_getpen (GMT, &opt->arg[2], &Ctrl->T.pen)) {	/* Set transparent attributes */
					gmt_pen_syntax (GMT, 'T', NULL, " ", 0);
					n_errors++;
				}
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				break;
			case 'Z':	/* Vary symbol color with z */
				Ctrl->Z.active = true;
				if (opt->arg[0]) Ctrl->Z.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (GMT->parent, &Ctrl->Z.active, &(Ctrl->Z.file));

	/* Check that the options selected are mutually consistent */
	n_errors += gmt_M_check_condition(GMT, !Ctrl->S.active, "Syntax error: Must specify -S option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->S.scale <= 0.0, "Syntax error: -S must specify scale\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->O2.active, "Syntax error: -Z cannot be combined with -Fo\n");

	/* Set to default pen where needed */

	if (Ctrl->C.pen.width  < 0.0) Ctrl->C.pen  = Ctrl->W.pen;
	if (Ctrl->L.pen.width  < 0.0) Ctrl->L.pen  = Ctrl->W.pen;
	if (Ctrl->T.pen.width  < 0.0) Ctrl->T.pen  = Ctrl->W.pen;
	if (Ctrl->T2.pen.width < 0.0) Ctrl->T2.pen = Ctrl->W.pen;
	if (Ctrl->P2.pen.width < 0.0) Ctrl->P2.pen = Ctrl->W.pen;
	if (Ctrl->Z2.pen.width < 0.0) Ctrl->Z2.pen = Ctrl->W.pen;

	/* Default -Fe<fill> and -Fg<fill> to -E<fill> and -G<fill> */

	if (!Ctrl->E2.active) Ctrl->E2.fill = Ctrl->E.fill;
	if (!Ctrl->G2.active) Ctrl->G2.fill = Ctrl->G.fill;

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_meca (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: meca\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psmeca (V_API, mode, args);
}

int GMT_psmeca (void *V_API, int mode, void *args) {
	/* High-level function that implements the psmeca task */
	int i, n, form = 0, new_fmt;
	int n_rec = 0, n_plane_old = 0, error;
	int n_scanned = 0;
	bool transparence_old = false, not_defined = false;

	double plot_x, plot_y, plot_xnew, plot_ynew, delaz, *in = NULL;
	double t11 = 1.0, t12 = 0.0, t21 = 0.0, t22 = 1.0, xynew[2] = {0.0};
	double fault, depth, size, P_x, P_y, T_x, T_y;

	char string[GMT_BUFSIZ] = {""}, Xstring[GMT_BUFSIZ] = {""}, Ystring[GMT_BUFSIZ] = {""}, event_title[GMT_BUFSIZ] = {""};

	st_me meca;
	struct MOMENT moment;
	struct M_TENSOR mt;
	struct AXIS T, N, P;

	struct GMT_PALETTE *CPT = NULL;
	struct GMT_RECORD *In = NULL;
	struct PSMECA_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the psmeca main code ----------------------------*/

	gmt_M_memset (event_title, GMT_BUFSIZ, char);
	gmt_M_memset (&meca, 1, st_me);
	gmt_M_memset (&T, 1, struct AXIS);
	gmt_M_memset (&N, 1, struct AXIS);
	gmt_M_memset (&P, 1, struct AXIS);

	if (Ctrl->Z.active) {
		if ((CPT = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->Z.file, NULL)) == NULL) {
			Return (API->error);
		}
	}

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	if (Ctrl->O2.active) Ctrl->S.n_cols--;	/* No depth */

	GMT_Set_Columns (API, GMT_IN, Ctrl->S.n_cols, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
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

		/* Data record to process */
		in = In->data;

		n_rec++;

		/* Immediately skip locations outside of the map area */
		if (!Ctrl->N.active) {
			gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
			if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
		}

		/* In new (psmeca) input format, third column is depth.
		   Skip record when depth is out of range. Also read an extra column. */
		new_fmt = Ctrl->O2.active ? 0 : 1;
		if (new_fmt) {
			depth = in[GMT_Z];
			if (depth < Ctrl->D.depmin || depth > Ctrl->D.depmax) continue;
			if (Ctrl->Z.active) gmt_get_fill_from_z (GMT, CPT, depth, &Ctrl->G.fill);
		}

		/* Must examine the trailing text for optional columns: newX, newY and title */
		if (In->text) {
			n_scanned = sscanf (In->text, "%s %s %[^\n]s\n", Xstring, Ystring, event_title);
			if (n_scanned >= 2) { /* Got new x,y coordinates and possibly event title */
				unsigned int type;
				if (GMT->current.setting.io_lonlat_toggle[GMT_IN]) {	/* Expect lat lon but watch for junk */
					if ((type = gmt_scanf_arg (GMT, Ystring, GMT_IS_LON, false, &xynew[GMT_X])) == GMT_IS_NAN) xynew[GMT_X] = GMT->session.d_NaN;
					if ((type = gmt_scanf_arg (GMT, Xstring, GMT_IS_LAT, false, &xynew[GMT_Y])) == GMT_IS_NAN) xynew[GMT_Y] = GMT->session.d_NaN;
				}
				else {	/* Expect lon lat but watch for junk */
					if ((type = gmt_scanf_arg (GMT, Xstring, GMT_IS_LON, false, &xynew[GMT_X])) == GMT_IS_NAN) xynew[GMT_X] = GMT->session.d_NaN;
					if ((type = gmt_scanf_arg (GMT, Ystring, GMT_IS_LAT, false, &xynew[GMT_Y])) == GMT_IS_NAN) xynew[GMT_Y] = GMT->session.d_NaN;
				}
				if (gmt_M_is_dnan (xynew[GMT_X]) || gmt_M_is_dnan (xynew[GMT_Y])) {	/* Got part of a title, presumably */
					xynew[GMT_X] = 0.0;	 /* revert to 0 if newX and newY are not given */
					xynew[GMT_Y] = 0.0;
					if (!(strchr ("XY", Xstring[0]) && strchr ("XY", Ystring[0])))	/* Old meca format with X Y placeholders */
						strncpy (event_title, In->text, GMT_BUFSIZ-1);
				}
				else if (n_scanned == 2)	/* Got no title */
					event_title[0] = '\0';
			}
			else if (n_scanned == 1)	/* Only got event title */
				strncpy (event_title, In->text, GMT_BUFSIZ-1);
			else	/* Got no title */
				event_title[0] = '\0';
		}

		/* Gather and transform the input records, depending on type */
		if (Ctrl->S.readmode == READ_CMT) {
			meca.NP1.str = in[2+new_fmt];
			if (meca.NP1.str > 180.0)			meca.NP1.str -= 360.0;
			else if (meca.NP1.str < -180.0) 	meca.NP1.str += 360.0;	/* Strike must be in -180/+180 range*/
			meca.NP1.dip = in[3+new_fmt];
			meca.NP1.rake = in[4+new_fmt];
			if (meca.NP1.rake > 180.0)			meca.NP1.rake -= 360.0;
			else if (meca.NP1.rake < -180.0) 	meca.NP1.rake += 360.0;	/* Rake must be in -180/+180 range*/
			meca.NP2.str = in[5+new_fmt];
			if (meca.NP2.str > 180.0)			meca.NP2.str -= 360.0;
			else if (meca.NP2.str < -180.0) 	meca.NP2.str += 360.0;	/* Strike must be in -180/+180 range*/
			meca.NP2.dip = in[6+new_fmt];
			meca.NP2.rake = in[7+new_fmt];
			if (meca.NP2.rake > 180.0)			meca.NP2.rake -= 360.0;
			else if (meca.NP2.rake < -180.0) 	meca.NP2.rake += 360.0;	/* Rake must be in -180/+180 range*/
			meca.moment.mant = in[8+new_fmt];
			meca.moment.exponent = irint (in[9+new_fmt]);
			if (meca.moment.exponent == 0) meca.magms = in[8+new_fmt];
		}
		else if (Ctrl->S.readmode == READ_AKI) {
			meca.NP1.str = in[2+new_fmt];
			if (meca.NP1.str > 180.0)			meca.NP1.str -= 360.0;
			else if (meca.NP1.str < -180.0) 	meca.NP1.str += 360.0;	/* Strike must be in -180/+180 range*/
			meca.NP1.dip = in[3+new_fmt];
			meca.NP1.rake = in[4+new_fmt];
			if (meca.NP1.rake > 180.0)			meca.NP1.rake -= 360.0;
			else if (meca.NP1.rake < -180.0) 	meca.NP1.rake += 360.0;	/* Rake must be in -180/+180 range*/
			if (gmt_M_is_zero (meca.NP1.rake)) meca.NP1.rake = 0.00001;	/* Fixing the issue http://gmt.soest.hawaii.edu/issues/894 */
			meca.magms = in[5+new_fmt];
			meca.moment.exponent = 0;
			meca_define_second_plane (meca.NP1, &meca.NP2);
		}
		else if (Ctrl->S.readmode == READ_PLANES) {
			meca.NP1.str = in[2+new_fmt];
			if (meca.NP1.str > 180.0)		meca.NP1.str -= 360.0;
			else if (meca.NP1.str < -180.0) meca.NP1.str += 360.0;		/* Strike must be in -180/+180 range*/
			meca.NP1.dip = in[3+new_fmt];
			meca.NP2.str = in[4+new_fmt];
			if (meca.NP2.str > 180.0)		meca.NP2.str -= 360.0;
			else if (meca.NP2.str < -180.0) meca.NP2.str += 360.0;		/* Strike must be in -180/+180 range*/
			fault = in[5+new_fmt];
			meca.magms = in[6+new_fmt];
			meca.moment.exponent = 0;
			meca.NP2.dip = meca_computed_dip2(meca.NP1.str, meca.NP1.dip, meca.NP2.str);
			if (meca.NP2.dip == 1000.0) {
				not_defined = true;
				transparence_old = Ctrl->T.active;
				n_plane_old = Ctrl->T.n_plane;
				Ctrl->T.active = true;
				Ctrl->T.n_plane = 1;
				meca.NP1.rake = 1000.;
				GMT_Report (API, GMT_MSG_VERBOSE, "Second plane is not defined for event %s only first plane is plotted.\n", In->text);
			}
			else
				meca.NP1.rake = meca_computed_rake2(meca.NP2.str, meca.NP2.dip, meca.NP1.str, meca.NP1.dip, fault);
		}
		else if (Ctrl->S.readmode == READ_AXIS) {
			T.val = in[2+new_fmt];
			T.str = in[3+new_fmt];
			T.dip = in[4+new_fmt];
			T.e = irint (in[11+new_fmt]);

			N.val = in[5+new_fmt];
			N.str = in[6+new_fmt];
			N.dip = in[7+new_fmt];
			N.e = irint (in[11+new_fmt]);

			P.val = in[8+new_fmt];
			P.str = in[9+new_fmt];
			P.dip = in[10+new_fmt];
			P.e = irint (in[11+new_fmt]);
			/*
			F. A. Dahlen and Jeroen Tromp, Theoretical Global Seismology, Princeton, 1998, p.167.
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
			for (i = 2+new_fmt, n = 0; i < 8+new_fmt; i++, n++) mt.f[n] = in[i];
			mt.expo = irint (in[i]);
			/*
			F. A. Dahlen and Jeroen Tromp, Theoretical Global Seismology, Princeton, 1998, p.167.
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

		gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);

		/* If option -C is used, read the new position */

		if (Ctrl->C.active) {
			if (fabs (xynew[GMT_X]) > EPSIL || fabs (xynew[GMT_Y]) > EPSIL) {
				gmt_setpen (GMT, &Ctrl->C.pen);
				gmt_geo_to_xy (GMT, xynew[GMT_X], xynew[GMT_Y], &plot_xnew, &plot_ynew);
				gmt_setfill (GMT, &Ctrl->G.fill, 1);
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

		if (size < 0.0) {	/* Addressing Bug #1171 */
			GMT_Report (API, GMT_MSG_VERBOSE, "Skipping negative symbol size %g for record # %d.\n", size, n_rec);
			continue;
		}

		meca_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
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
			int label_justify = 0;
			double label_x, label_y;
			double label_offset[2];

			label_justify = gmt_flip_justify(GMT, Ctrl->S.justify);
			label_offset[0] = label_offset[1] = GMT_TEXT_CLEARANCE * 0.01 * Ctrl->S.font.size / PSL_POINTS_PER_INCH;

			label_x = plot_x + 0.5 * (Ctrl->S.justify%4 - label_justify%4) * size * 0.5;
			label_y = plot_y + 0.5 * (Ctrl->S.justify/4 - label_justify/4) * size * 0.5;

			/* Also deal with any justified offsets if given */
			if (Ctrl->S.justify%4 == 1) /* Left aligned */
				label_x -= Ctrl->S.offset[0];
			else /* Right or center aligned */
				label_x += Ctrl->S.offset[0];
			if (Ctrl->S.justify/4 == 0) /* Bottom aligned */
				label_y -= Ctrl->S.offset[1];
			else /* Top or middle aligned */
				label_y += Ctrl->S.offset[1];

			gmt_setpen (GMT, &Ctrl->W.pen);
			PSL_setfill (PSL, Ctrl->R2.fill.rgb, 0);
			if (Ctrl->R2.active) PSL_plottextbox (PSL, label_x, label_y, Ctrl->S.font.size, event_title, Ctrl->S.angle, label_justify, label_offset, 0);
			form = gmt_setfont(GMT, &Ctrl->S.font);
			PSL_plottext (PSL, label_x, label_y, Ctrl->S.font.size, event_title, Ctrl->S.angle, label_justify, form);
		}

		if (Ctrl->A2.active) {
			if (Ctrl->S.readmode != READ_TENSOR && Ctrl->S.readmode != READ_AXIS) meca_dc2axe (meca, &T, &N, &P);
			meca_axis2xy (plot_x, plot_y, size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
			gmt_setpen (GMT, &Ctrl->P2.pen);
			gmt_setfill (GMT, &Ctrl->G2.fill, Ctrl->P2.active ? 1 : 0);
			PSL_plotsymbol (PSL, P_x, P_y, &Ctrl->A2.size, Ctrl->A2.P_symbol);
			gmt_setpen (GMT, &Ctrl->T2.pen);
			gmt_setfill (GMT, &Ctrl->E2.fill, Ctrl->T2.active ? 1 : 0);
			PSL_plotsymbol (PSL, T_x, T_y, &Ctrl->A2.size, Ctrl->A2.T_symbol);
		}
		event_title[0] = string[0] = '\0';		/* Reset these two in case next record misses "string" */
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Number of records read: %li\n", n_rec);

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
	PSL_setdash (PSL, NULL, 0);
	gmt_map_basemap (GMT);
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
