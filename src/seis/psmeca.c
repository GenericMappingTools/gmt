/*--------------------------------------------------------------------
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYdehipqt" GMT_OPT("c")

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
	struct PSMECA_A {	/* -A[+p<pen>][+s<size>] */
		bool active;
		double size;
		struct GMT_PEN pen;
	} A;
	struct PSMECA_C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct PSMECA_D {	/* -D<min/max> */
		bool active;
		double depmin, depmax;
	} D;
	struct PSMECA_E {	/* -E<fill> */
		bool active;
		struct GMT_FILL fill;
	} E;
	struct PSMECA_F {	/* Repeatable -F<mode>[<args>] */
		bool active;
	} F;
	struct PSMECA_G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct PSMECA_H {	/* -H read overall scaling factor for symbol size and pen width */
		bool active;
		unsigned int mode;
		double value;
	} H;
	struct PSMECA_I {	/* -I[<intensity>] */
		bool active;
		unsigned int mode;	/* 0 if constant, 1 if read from file */
		double value;
	} I;
	struct PSMECA_L {	/* -L<pen> */
		bool active;
		struct GMT_PEN pen;
	} L;
	struct PSMECA_N {	/* -N */
		bool active;
	} N;
	struct PSMECA_S {	/* -S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>] */
#include "meca_symbol.h"
	} S;
	struct PSMECA_T {	/* -T<nplane>[/<pen>] */
		bool active;
		unsigned int n_plane;
		struct GMT_PEN pen;
	} T;
	struct PSMECA_W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct PSMECA_A2 {	/* -Fa[<size>[/<Psymbol>[<Tsymbol>]]] */
		bool active;
		char P_symbol, T_symbol;
		double size;
	} A2;
	struct PSMECA_E2 {	/* -Fe<fill> */
		bool active;
		struct GMT_FILL fill;
	} E2;
	struct PSMECA_G2 {	/* -Fg<fill> */
		bool active;
		struct GMT_FILL fill;
	} G2;
	struct PSMECA_P2 {	/* -Fp[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} P2;
	struct PSMECA_R2 {	/* -Fr[<fill>] */
		bool active;
		struct GMT_FILL fill;
	} R2;
	struct PSMECA_T2 {	/* -Ft[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} T2;
	struct PSMECA_O2 {	/* -Fo */
		bool active;
	} O2;
	struct PSMECA_Z2 {	/* -Fz[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} Z2;
};

enum Psmeca_scaletype {
	PSMECA_READ_SCALE	= 0,
	PSMECA_CONST_SCALE	= 1};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSMECA_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSMECA_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.size = 0.0;	/* No circle will be plotted */
	C->A.pen = C->L.pen = C->T.pen = C->T2.pen = C->P2.pen = C->Z2.pen = C->W.pen = GMT->current.setting.map_default_pen;
	/* Set width temporarily to -1. This will indicate later that we need to replace by W.pen */
	C->A.pen.width = C->L.pen.width = C->T.pen.width = C->T2.pen.width = C->P2.pen.width = C->Z2.pen.width = -1.0;
	C->D.depmin = -FLT_MAX;
	C->D.depmax = FLT_MAX;
	C->L.active = false;
	gmt_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	gmt_init_fill (GMT, &C->R2.fill, 1.0, 1.0, 1.0);
	C->S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	C->S.font.size = DEFAULT_FONTSIZE;
	C->S.justify = PSL_TC;
	C->S.reference = SEIS_MAG_REFERENCE;
	C->A2.size = DEFAULT_SYMBOL_SIZE * GMT->session.u2u[GMT_PT][GMT_INCH];
	C->A2.P_symbol = C->A2.T_symbol = PSL_CIRCLE;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSMECA_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psmeca synopsis and optionally full usage information */

	struct GMT_FONT font;
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] %s %s "
		"-S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>] [-A[+p<pen>][+s<size>]] [%s] "
		"[-C<cpt>] [-D<depmin>/<depmax>] [-E<fill>] [-Fa[<size>[/<Psymbol>[<Tsymbol>]]]] [-Fe<fill>] [-Fg<fill>] "
		"[-Fr<fill>] [-Fp[<pen>]] [-Ft[<pen>]] [-Fz[<pen>]] [-G<fill>] [-H[<scale>]] [-I[<intens>]] %s[-L<pen>] "
		"[-N] %s%s[-T<nplane>[/<pen>]] [%s] [%s] [-W<pen>] [%s] [%s] %s[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_U_OPT, GMT_V_OPT, GMT_X_OPT,
		GMT_Y_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_tv_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	font = API->GMT->current.setting.font_annot[GMT_PRIMARY];
	font.size = DEFAULT_FONTSIZE;

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Option (API, "J-,R");
	GMT_Usage (API, 1, "\n-S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>]");
	GMT_Usage (API, -2, "Select format directive and optional symbol modifiers:");
	GMT_Usage (API, 3, "a: Focal mechanism in Aki & Richard's convention:");
	GMT_Usage (API, 4, "X Y depth strike dip rake mag [newX newY] [event_title].");
	GMT_Usage (API, 3, "c: Focal mechanism in Global CMT convention:");
	GMT_Usage (API, 4, "X Y depth strike1 dip1 rake1 strike2 dip2 rake2 moment [newX newY] [event_title], "
		"with moment in 2 columns : mantissa and exponent corresponding to seismic moment in dynes-cm.");
	GMT_Usage (API, 3, "d: Closest double couple defined from seismic moment tensor (zero trace and zero determinant):");
	GMT_Usage (API, 4, "X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "p: Focal mechanism defined with:");
	GMT_Usage (API, 4, "X Y depth strike1 dip1 strike2 fault mag [newX newY] [event_title]. "
		"Note: fault = -1/+1 for a normal/inverse fault.");
	GMT_Usage (API, 3, "m: Seismic (full) moment tensor:");
	GMT_Usage (API, 4, "X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "t: Zero trace moment tensor defined from principal axis:");
	GMT_Usage (API, 4, "X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "x: Principal axis:");
	GMT_Usage (API, 4, "X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "y: Best double couple defined from principal axis:");
	GMT_Usage (API, 4, "X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "z: Deviatoric part of the moment tensor (zero trace):");
	GMT_Usage (API, 4, "X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title].");
	GMT_Usage (API, -2, "If <scale> is not given then it is read from the first column after the required columns. Optional modifiers for the label:");
	GMT_Usage (API, 3, "+a Set the label angle [0].");
	GMT_Usage (API, 3, "+f Set font attributes for the label [%s].", gmt_putfont (API->GMT, &font));
	GMT_Usage (API, 3, "+j Set the label <justification> [TC].");
	GMT_Usage (API, 3, "+l Use linear symbol scaling based on moment [magnitude].");
	GMT_Usage (API, 3, "+m Use <scale> as fixed size for any magnitude or moment.");
	GMT_Usage (API, 3, "+o Set the label offset <dx>[/<dy>] [0/0].");
	GMT_Usage (API, 3, "+s Set reference magnitude [%g] or moment [%ge%d] (if +l) for symbol size.", SEIS_MAG_REFERENCE, SEIS_MOMENT_MANT_REFERENCE, SEIS_MOMENT_EXP_REFERENCE);
	GMT_Usage (API, -2, "Note: If fontsize < 0 then no label written; offset is from the limit of the beach ball.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-A[+p<pen>][+s<size>]");
	GMT_Usage (API, -2, "Offset focal mechanisms to the alternate positions given in the last two columns of the input file before label. "
		"A line is drawn between both positions; see -W for pen used or specify it separately via +p [0.25p]. "
		"Optionally, a small circle is plotted at the original location. Append +s<size> to set its diameter [no circle].");
	GMT_Usage (API, 1, "\n-C<cpt>");
	GMT_Usage (API, -2, "Use CPT to assign colors based on depth-value in 3rd column.");
	GMT_Usage (API, 1, "\n-D<depmin>/<depmax>");
	GMT_Usage (API, -2, "Plot events between <depmin> and <depmax> deep.");
	gmt_fill_syntax (API->GMT, 'E', NULL, "Set filling of extensive quadrants [Default is white].");
	GMT_Usage (API, 1, "\n-F<directive><parameters> (repeatable)");
	GMT_Usage (API, -2, "Set various attributes of symbols depending on directive:");
	GMT_Usage (API, 3, "a: Plot axis. Default symbols are circles; otherwise append <size>[/<Psymbol>[<Tsymbol>].");
	GMT_Usage (API, 3, "e: Append filling for the T axis symbol [default as set by -E].");
	GMT_Usage (API, 3, "g: Append filling for the P axis symbol [default as set by -G].");
	GMT_Usage (API, 3, "p: Draw P_symbol outline using the default pen (see -W; or append alternative pen).");
	GMT_Usage (API, 3, "t: Draw T_symbol outline using the default pen (see -W; or append alternative pen).");
	GMT_Usage (API, 3, "r: Draw box behind labels.");
	GMT_Usage (API, 3, "z: Overlay zero trace moment tensor using default pen (see -W; or append alternative pen).");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Set filling of compressive quadrants [Default is black].");
	GMT_Usage (API, 1, "\n-H[<scale>]");
	GMT_Usage (API, -2, "Scale symbol sizes (set via -S or input column) and pen attributes by factors read from scale column. "
		"The scale column follows the symbol size column.  Alternatively, append a fixed <scale>.");
	GMT_Usage (API, 1, "\n-I[<intens>]");
	GMT_Usage (API, -2, "Use the intensity to modulate the compressive fill color (requires -C or -G). "
		"If no intensity is given we expect it to follow the required columns in the data record.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-L<pen>");
	GMT_Usage (API, -2, "Sets pen attribute for outline other than the default set by -W.");
	GMT_Usage (API, 1, "\n-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-T<plane>[/<pen>]");
	GMT_Usage (API, -2, "Draw specified nodal <plane>(s) and circumference only to provide a transparent beach ball "
		"using the current pen (see -W; or append alternative pen):");
	GMT_Usage (API, 3, "1: Only the first nodal plane is plotted.");
	GMT_Usage (API, 3, "2: Only the second nodal plane is plotted.");
	GMT_Usage (API, 3, "0: Both nodal planes are plotted.");
	GMT_Usage (API, -2, "Note: If moment tensor is required, nodal planes overlay moment tensor.");
	GMT_Option (API, "U,V");
	GMT_Usage (API, 1, "\n-W<pen>");
	GMT_Usage (API, -2, "Set pen attributes [%s].", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X,c,di,e,h,i,p,qi,T,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL bool psmeca_is_old_C_option (struct GMT_CTRL *GMT, char *arg) {
	if (strstr (arg, ".cpt")) return false;	/* Clearly a CPT file given */
	if (strstr (arg, "+s") || strchr (arg, 'P')) return true;	/* Clearly setting the circle diameter in old -C */
	if (GMT->current.setting.run_mode == GMT_CLASSIC && arg[0] == '\0') return true;	/* A blank -C in classic mode is clearly the old -C with no settings */
	if (arg[0]) return true;	/* Whatever this is, it is for -A to deal with */
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Option -C: Must assume under modern mode that -C here means use current CPT\n");
	return false;	/* This assumes nobody would use just -C in modern mode but actually mean the old -C */
}

GMT_LOCAL unsigned int psmeca_A_parse (struct GMT_CTRL *GMT, struct PSMECA_CTRL *Ctrl, char *arg) {
	unsigned int n_errors = 0;
	char txt[GMT_LEN256] = {""}, *c = NULL, *q = NULL;
	strncpy (txt, arg, GMT_LEN256-1);

	/* Deal with these possible variations of old -C and new -A options:
	 * 1. -A[+p<pen>][+s<size>]	which is the current syntax
	 * 2. -C[<pen>][+s<size>]	which was the GMT5-6.1.1 syntax
	 * 3. -C[<pen>][P<size>]	which was the GMT4 syntax */

	if ((c = gmt_first_modifier (GMT, txt, "ps"))) {	/* Found at least one valid modifier */
		unsigned int pos = 0;
		char p[GMT_LEN256] = {""};
		while (gmt_getmodopt (GMT, 'A', c, "ps", &pos, p, &n_errors) && n_errors == 0) {
			switch (p[0]) {
				case 'p':	/* Line and circle pen */
					if (p[1] == '\0' || gmt_getpen (GMT, &p[1], &Ctrl->A.pen)) {
						gmt_pen_syntax (GMT, 'A', NULL, " ", NULL, 0);
						n_errors++;
					}
					break;
				case 's':	/* Circle diameter */
					if (p[1] == '\0' || (Ctrl->A.size = gmt_M_to_inch (GMT, (p+2))) < 0.0) {
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -A: Circle diameter cannot be negative or not given!\n");
						n_errors++;
					}
					break;
				default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
			}
		}
		c[0] = '\0';	/* Chop off the modifiers */
	}
	/* If the user used modern modifiers only as case 1 above then we might be done here */
	if (arg[0] == '\0') return n_errors;

	/* Here we got older syntax: -C<pen>[+s<size>] or -C[<pen>][P<size>] (but the +s<size> would have been stripped off
	 * so here we must either have -C<pen> or -C[<pen>][P<size>] */

	if ((q = strchr (txt, 'P')) != NULL) {	/* Case 3 way of changing the diameter */
		if (q[1] == '\0' || (Ctrl->A.size = gmt_M_to_inch (GMT, &q[1])) < 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -A: Circle diameter cannot be negative or not given!\n");
			n_errors++;
		}
		q[0] = '\0';	/* Chop off the Psize setting; if txt is not empty we also have an optional pen */
		if (arg[0] && gmt_getpen (GMT, txt, &Ctrl->A.pen)) {
			gmt_pen_syntax (GMT, 'A', NULL, " ", NULL, 0);
			n_errors++;
		}
	}
	else if (gmt_getpen (GMT, txt, &Ctrl->A.pen)) {	/* Here we just have -C<pen> to deal with */
		gmt_pen_syntax (GMT, 'A', NULL, " ", NULL, 0);
		n_errors++;
	}
	return n_errors;
}

static int parse (struct GMT_CTRL *GMT, struct PSMECA_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				n_errors += psmeca_A_parse (GMT, Ctrl, opt->arg);
				break;
			case 'C':	/* Either modern -Ccpt option of a deprecated -C now served by -A */
				/* Change position [set line attributes] */
				if (psmeca_is_old_C_option (GMT, opt->arg)) {	/* Need the -A parser for obsolete -C syntax */
					Ctrl->A.active = true;
					n_errors += psmeca_A_parse (GMT, Ctrl, opt->arg);
				}
				else {	/* Here we have the modern -C<cpt> parsing */
					Ctrl->C.active = true;
					if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
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
							gmt_pen_syntax (GMT, ' ', "Fp", " ", NULL, 0);
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
							gmt_pen_syntax (GMT, ' ', "Ft", " ", NULL, 0);
							n_errors++;
						}
						break;
					case 'o':	/* use psvelomeca format (without depth in 3rd column) */
						Ctrl->O2.active = true;
						break;
					case 'z':	/* overlay zerotrace moment tensor */
						Ctrl->Z2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->Z2.pen)) { /* Set pen attributes */
							gmt_pen_syntax (GMT, ' ', "Fz", " ", NULL, 0);
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
			case 'H':		/* Overall symbol/pen scale column provided */
				Ctrl->H.active = true;
				if (opt->arg[0]) {	/* Gave a fixed scale - no reading from file */
					Ctrl->H.value = atof (opt->arg);
					Ctrl->H.mode = PSMECA_CONST_SCALE;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				Ctrl->I.active = true;
				if (opt->arg[0])
					Ctrl->I.value = atof (opt->arg);
				else
					Ctrl->I.mode = 1;
				break;
			case 'L':	/* Draw outline [set outline attributes] */
				Ctrl->L.active = true;
				if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					gmt_pen_syntax (GMT, 'L', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			case 'M':	/* Same size for any magnitude [Deprecated 8/14/2021 6.3.0 - use -S+m instead] */
				if (gmt_M_compat_check (GMT, 6)) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-M is deprecated from 6.3.0; use -S modifier +m instead.\n");
					Ctrl->S.fixed = true;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
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

				if ((strstr (opt->arg, "+a")) || (strstr (opt->arg, "+f")) || strstr (opt->arg, "+j") || strstr (opt->arg, "+l") || strstr (opt->arg, "+m") || strstr (opt->arg, "+o") || strstr (opt->arg, "+s")) {
					/* New syntax: -S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>] */
					char word[GMT_LEN256] = {""}, *c = NULL;

					/* Parse beachball size */
					if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
					if (opt->arg[1]) Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[1]);
					if (c) c[0] = '+';	/* Restore modifiers */

					if (gmt_get_modifier (opt->arg, 'a', word))
						Ctrl->S.angle = atof(word);
					if (gmt_get_modifier (opt->arg, 'j', word) && strchr ("LCRBMT", word[0]) && strchr ("LCRBMT", word[1]))
						Ctrl->S.justify = gmt_just_decode (GMT, word, Ctrl->S.justify);
					if (gmt_get_modifier (opt->arg, 'f', word)) {
						if (word[0] == '-' || (word[0] == '0' && (word[1] == '\0' || word[1] == 'p')))
							Ctrl->S.font.size = 0.0;
						else
							n_errors += gmt_getfont (GMT, word, &(Ctrl->S.font));
					}
					if (gmt_get_modifier (opt->arg, 'o', word)) {
						if (gmt_get_pair (GMT, word, GMT_PAIR_DIM_DUP, Ctrl->S.offset) < 0) n_errors++;
					} else {	/* Set default offset */
						if (Ctrl->S.justify%4 != 2) /* Not center aligned */
							Ctrl->S.offset[0] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
						if (Ctrl->S.justify/4 != 1) /* Not middle aligned */
							Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
					}
					if (gmt_get_modifier (opt->arg, 'l', word)) {
						Ctrl->S.linear = true;
						Ctrl->S.reference = SEIS_MOMENT_MANT_REFERENCE * pow (10.0, SEIS_MOMENT_EXP_REFERENCE);	/* May change if +s is given */
					}
					if (gmt_get_modifier (opt->arg, 'm', word))
						Ctrl->S.fixed = true;
					if (gmt_get_modifier (opt->arg, 's', word))
						Ctrl->S.reference = atof (word);
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
					if (p) p[0] = '+';	/* Restore modifier */
				}
				if (Ctrl->S.font.size <= 0.0) Ctrl->S.no_label = true;
				if (gmt_M_is_zero (Ctrl->S.scale)) Ctrl->S.read = true;	/* Must get size from input file */
				break;
			case 'T':
				Ctrl->T.active = true;
				sscanf (opt->arg, "%d", &Ctrl->T.n_plane);
				if (strlen (opt->arg) > 2 && gmt_getpen (GMT, &opt->arg[2], &Ctrl->T.pen)) {	/* Set transparent attributes */
					gmt_pen_syntax (GMT, 'T', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (GMT->parent, &Ctrl->C.active, &(Ctrl->C.file));

	/* Check that the options selected are mutually consistent */
	n_errors += gmt_M_check_condition(GMT, !Ctrl->S.active, "Must specify -S option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	//n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->S.scale <= 0.0, "Option -S: must specify scale\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->O2.active, "Option -Z cannot be combined with -Fo\n");

	/* Set to default pen where needed */

	if (Ctrl->A.pen.width  < 0.0) Ctrl->A.pen  = Ctrl->W.pen;
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

EXTERN_MSC int GMT_psmeca (void *V_API, int mode, void *args) {
	/* High-level function that implements the psmeca task */
	bool transparence_old = false, not_defined = false, has_text;

	int i, n, form = 0, new_fmt;
	int n_rec = 0, n_plane_old = 0, error;
	int n_scanned = 0;
	unsigned int xcol = 0, scol = 0, icol = 0, tcol_f = 0, tcol_s = 0;
	uint64_t tbl, seg, row, col;

	double plot_x, plot_y, plot_xnew, plot_ynew, delaz, in[GMT_LEN16];
	double t11 = 1.0, t12 = 0.0, t21 = 0.0, t22 = 1.0, xynew[2] = {0.0};
	double scale, fault, depth, size, P_x, P_y, T_x, T_y, nominal_size;

	char string[GMT_BUFSIZ] = {""}, Xstring[GMT_BUFSIZ] = {""}, Ystring[GMT_BUFSIZ] = {""}, event_title[GMT_BUFSIZ] = {""};
	char *no_name = "<unnamed>", *event_name = NULL;

	st_me meca;
	struct MOMENT moment;
	struct M_TENSOR mt;
	struct AXIS T, N, P;

	struct GMT_PALETTE *CPT = NULL;
	struct GMT_DATASET *D = NULL;	/* Pointer to GMT multisegment input tables */
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_PEN current_pen;
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
	gmt_M_memset (in, GMT_LEN16, double);
	nominal_size = Ctrl->S.scale;

	if (Ctrl->C.active) {
		if ((CPT = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
	}
	else if (Ctrl->I.active && Ctrl->I.mode == 0) {	/* No CPT and fixed intensity means we can do the constant change once */
		gmt_illuminate (GMT, Ctrl->I.value, Ctrl->G.fill.rgb);
		Ctrl->I.active = false;	/* So we don't do this again */
	}

	if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_set_basemap_orders (GMT, Ctrl->N.active ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_BEFORE);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
 	gmt_map_basemap (GMT);	/* Lay down gridlines */

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	if (Ctrl->O2.active) Ctrl->S.n_cols--;	/* No depth */

	if (Ctrl->S.read) {	/* Read symbol size from file */
		scol = Ctrl->S.n_cols;
		Ctrl->S.n_cols++;
		gmt_set_column_type (GMT, GMT_IN, scol, GMT_IS_DIMENSION);
	}
	else	/* Fixed scale */
		scale = Ctrl->S.scale;
	if (Ctrl->H.active && Ctrl->H.mode == PSMECA_READ_SCALE) {
		xcol = Ctrl->S.n_cols;
		Ctrl->S.n_cols++;	/* Read scaling from data file */
		gmt_set_column_type (GMT, GMT_IN, xcol, GMT_IS_FLOAT);
	}
	if (Ctrl->I.mode) {	/* Read intensity from data file */
		icol = Ctrl->S.n_cols;
		Ctrl->S.n_cols++;
		gmt_set_column_type (GMT, GMT_IN, icol, GMT_IS_FLOAT);
	}
	if (GMT->common.t.variable) {	/* Need one or two transparencies from file */
		if (GMT->common.t.mode & GMT_SET_FILL_TRANSP) {
			tcol_f = Ctrl->S.n_cols;
			Ctrl->S.n_cols++;	/* Read fill transparencies from data file */
			gmt_set_column_type (GMT, GMT_IN, tcol_f, GMT_IS_FLOAT);
		}
		if (GMT->common.t.mode & GMT_SET_PEN_TRANSP) {
			tcol_s = Ctrl->S.n_cols;
			Ctrl->S.n_cols++;	/* Read stroke transparencies from data file */
			gmt_set_column_type (GMT, GMT_IN, tcol_s, GMT_IS_FLOAT);
		}
	}

	if (Ctrl->S.linear)
		GMT_Report (API, GMT_MSG_INFORMATION, "Linear moment scaling selected, normalizing by %e.\n", Ctrl->S.reference);
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "Linear magnitude scaling selected, normalizing by %g.\n", Ctrl->S.reference);

	GMT_Set_Columns (API, GMT_IN, Ctrl->S.n_cols, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}

	/* Read the entire input data set */
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	if (D->n_records == 0)
		GMT_Report (API, GMT_MSG_WARNING, "No data records provided\n");

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			S = D->table[tbl]->segment[seg];	/* Shorthand */
			for (row = 0; row < S->n_rows; row++) {
				for (col = 0; col < S->n_columns; col++) in[col] = S->data[col][row];	/* Make a local copy */

				if (gmt_M_is_dnan (in[GMT_X]) || gmt_M_is_dnan (in[GMT_Y]))	/* Probably a non-recognized header since we got NaNs */
					continue;

				n_rec++;

				/* Immediately skip locations outside of the map area */
				if (!Ctrl->N.active) {
					gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
					if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
				}
				has_text = S->text && S->text[row];
				/* In new (psmeca) input format, third column is depth.
				   Skip record when depth is out of range. Also read an extra column. */
				new_fmt = Ctrl->O2.active ? 0 : 1;
				if (new_fmt) {	/* Not -Fo so we have a depth column */
					depth = in[GMT_Z];
					if (depth < Ctrl->D.depmin || depth > Ctrl->D.depmax) continue;
					if (Ctrl->C.active)	/* Update color based on depth */
						gmt_get_fill_from_z (GMT, CPT, depth, &Ctrl->G.fill);
				}
				if (Ctrl->I.active) {	/* Modify color based on intensity */
					if (Ctrl->I.mode == 0)
						gmt_illuminate (GMT, Ctrl->I.value, Ctrl->G.fill.rgb);
					else
						gmt_illuminate (GMT, in[icol], Ctrl->G.fill.rgb);
				}
				if (GMT->common.t.variable) {	/* Update the transparency for current symbol (or -t was given) */
					double transp[2] = {0.0, 0.0};	/* None selected */
					if (GMT->common.t.n_transparencies == 2) {	/* Requested two separate values to be read from file */
						transp[GMT_FILL_TRANSP] = 0.01 * in[tcol_f];
						transp[GMT_PEN_TRANSP]  = 0.01 * in[tcol_s];
					}
					else if (GMT->common.t.mode & GMT_SET_FILL_TRANSP) {	/* Gave fill transparency */
						transp[GMT_FILL_TRANSP] = 0.01 * in[tcol_f];
						if (GMT->common.t.n_transparencies == 0) transp[GMT_PEN_TRANSP] = transp[GMT_FILL_TRANSP];	/* Implied to be used for stroke also */
					}
					else {	/* Gave stroke transparency */
						transp[GMT_PEN_TRANSP] = 0.01 * in[tcol_s];
						if (GMT->common.t.n_transparencies == 0) transp[GMT_FILL_TRANSP] = transp[GMT_PEN_TRANSP];	/* Implied to be used for fill also */
					}
					PSL_settransparencies (PSL, transp);
				}

				/* Must examine the trailing text for optional columns: newX, newY and title */
				if (S->text && S->text[row]) {
					n_scanned = sscanf (S->text[row], "%s %s %[^\n]s\n", Xstring, Ystring, event_title);
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
								strncpy (event_title, S->text[row], GMT_BUFSIZ-1);
						}
						else if (n_scanned == 2)	/* Got no title */
							event_title[0] = '\0';
					}
					else if (n_scanned == 1)	/* Only got event title */
						strncpy (event_title, S->text[row], GMT_BUFSIZ-1);
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
						event_name = (has_text) ? S->text[row] : no_name;
						GMT_Report (API, GMT_MSG_WARNING, "Second plane is not defined for event %s only first plane is plotted.\n", event_name);
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

				if (Ctrl->A.active) {
					if (fabs (xynew[GMT_X]) > EPSIL || fabs (xynew[GMT_Y]) > EPSIL) {
						current_pen = Ctrl->A.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
						gmt_geo_to_xy (GMT, xynew[GMT_X], xynew[GMT_Y], &plot_xnew, &plot_ynew);
						gmt_setfill (GMT, &Ctrl->G.fill, 1);
						if (Ctrl->A.size > 0.0) PSL_plotsymbol (PSL, plot_x, plot_y, &(Ctrl->A.size), PSL_CIRCLE);
						PSL_plotsegment (PSL, plot_x, plot_y, plot_xnew, plot_ynew);
						plot_x = plot_xnew;
						plot_y = plot_ynew;
					}
				}

				if (Ctrl->S.fixed) {
					meca.moment.mant     = SEIS_MOMENT_MANT_REFERENCE;
					meca.moment.exponent = SEIS_MOMENT_EXP_REFERENCE;
				}

				if (Ctrl->S.read) nominal_size = scale = in[scol];
				moment.mant = meca.moment.mant;
				moment.exponent = meca.moment.exponent;

				size = (scale / Ctrl->S.reference) * ((Ctrl->S.linear) ? moment.mant * pow (10.0, moment.exponent) : meca_computed_mw (moment, meca.magms));

				if (Ctrl->H.active) {	/* Variable scaling of symbol size and pen width */
					double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
					size *= scl;
				}

				if (size < 0.0) {	/* Addressing Bug #1171 */
					GMT_Report (API, GMT_MSG_WARNING, "Skipping negative symbol size %g for record # %d.\n", size, n_rec);
					continue;
				}
				else if (Ctrl->S.linear && size > 50.0) { /* Check for huge symbols */
					double plot_size = size * GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
					GMT_Report (API, GMT_MSG_WARNING, "Linear moment scaling leads to a symbol size of %g %s for record # %d, use -S+s<ref> to set appropriate reference moment.\n",
						plot_size, API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit], n_rec);
				}

				meca_get_trans (GMT, in[GMT_X], in[GMT_Y], &t11, &t12, &t21, &t22);
				delaz = atan2d(t12,t11);

				if ((Ctrl->S.readmode == READ_AXIS || Ctrl->S.readmode == READ_TENSOR) && Ctrl->S.plotmode != PLOT_DC) {

					T.str = meca_zero_360(T.str + delaz);
					N.str = meca_zero_360(N.str + delaz);
					P.str = meca_zero_360(P.str + delaz);

					current_pen = Ctrl->L.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
					if (fabs (N.val) < EPSIL && fabs (T.val + P.val) < EPSIL) {
						meca_axe2dc (T, P, &meca.NP1, &meca.NP2);
						meca_ps_mechanism (GMT, PSL, plot_x, plot_y, meca, size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
					}
					else
						meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active, Ctrl->S.plotmode == PLOT_TRACE, n_rec);
				}

				if (Ctrl->Z2.active) {
					current_pen = Ctrl->Z2.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
					meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, NULL, NULL, true, true, n_rec);
				}

				if (Ctrl->T.active) {
					meca.NP1.str = meca_zero_360(meca.NP1.str + delaz);
					meca.NP2.str = meca_zero_360(meca.NP2.str + delaz);
					current_pen = Ctrl->T.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
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
					current_pen = Ctrl->L.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
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

					current_pen = Ctrl->W.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
					PSL_setfill (PSL, Ctrl->R2.fill.rgb, 0);
					if (Ctrl->R2.active) PSL_plottextbox (PSL, label_x, label_y, Ctrl->S.font.size, event_title, Ctrl->S.angle, label_justify, label_offset, 0);
					form = gmt_setfont(GMT, &Ctrl->S.font);
					PSL_plottext (PSL, label_x, label_y, Ctrl->S.font.size, event_title, Ctrl->S.angle, label_justify, form);
				}

				if (Ctrl->A2.active) {
					if (Ctrl->S.readmode != READ_TENSOR && Ctrl->S.readmode != READ_AXIS) meca_dc2axe (meca, &T, &N, &P);
					meca_axis2xy (plot_x, plot_y, size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
					current_pen = Ctrl->P2.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
					gmt_setfill (GMT, &Ctrl->G2.fill, Ctrl->P2.active ? 1 : 0);
					PSL_plotsymbol (PSL, P_x, P_y, &Ctrl->A2.size, Ctrl->A2.P_symbol);
					current_pen = Ctrl->T2.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSMECA_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
					gmt_setfill (GMT, &Ctrl->E2.fill, Ctrl->T2.active ? 1 : 0);
					PSL_plotsymbol (PSL, T_x, T_y, &Ctrl->A2.size, Ctrl->A2.T_symbol);
				}
				event_title[0] = string[0] = '\0';		/* Reset these two in case next record misses "string" */
				}
			}
	}

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (GMT->common.t.variable) {	/* Reset the transparencies */
		double transp[2] = {0.0, 0.0};	/* None selected */
		PSL_settransparencies (PSL, transp);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of records read: %li\n", n_rec);

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
	PSL_setdash (PSL, NULL, 0);
	gmt_map_basemap (GMT);
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}

EXTERN_MSC int GMT_meca (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: meca\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psmeca (V_API, mode, args);
}
