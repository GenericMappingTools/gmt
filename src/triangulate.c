/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: triangulate reads one or more files (or stdin) with x,y[,whatever] and
 * outputs the indices of the vertices of the optimal Delaunay triangulation
 * using the method by Watson, D. F., ACORD: Automatic contouring of raw data,
 * Computers & Geosciences, 8, 97-101, 1982.  Optionally, the output may take
 * the form of (1) a multi-segment file with the vertex coordinates needed to
 * draw the triangles, or (2) a grid file based on gridding the plane estimates.
 * PS. Instead of Watson's method you may choose to link with the triangulate
 * routine written by Jonathan Shewchuk.  See the file TRIANGLE.HOWTO for
 * details.  That function is far faster than Watson's method and also allows
 * for Voronoi polygon output.
 *
 * Author:	Paul Wessel w/ CURVE addition by Samantha Zambo (-C)
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "longopt/triangulate_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"triangulate"
#define THIS_MODULE_MODERN_NAME	"triangulate"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Delaunay triangulation or Voronoi partitioning and gridding of Cartesian data"
#define THIS_MODULE_KEYS	"<D{,CG(,>D},GG)"
#define THIS_MODULE_NEEDS	"r"
#define THIS_MODULE_OPTIONS "-:>JRVbdefhiqrsw" GMT_OPT("Hm")

enum triangulate_enum_stat {TRI_NONE = 0,	/* No +z or +g given */
	TRI_POLY,			/* Use median instead */
	TRI_MEAN,			/* Use median instead */
	TRI_MEDIAN,			/* Use median instead */
	TRI_MODE,			/* Use mode (LMS) instead */
	TRI_LOWER,			/* Use lowest value encountered instead */
	TRI_UPPER};			/* Use highest value encountered instead */

struct TRIANGULATE_CTRL {
	struct TRIANGULATE_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct TRIANGULATE_A {	/* -A */
		bool active;
	} A;
	struct TRIANGULATE_C {	/* -C<input_slope_grid> */
		bool active;
		char *file;
	} C;
	struct TRIANGULATE_D {	/* -Dx|y */
		bool active;
		unsigned int dir;
	} D;
	struct TRIANGULATE_E {	/* -E<value> */
		bool active;
		double value;
	} E;
	struct TRIANGULATE_F {	/* -F<pregrid>[+d] */
		bool active;
		char *file;
		unsigned int mode;
	} F;
	struct TRIANGULATE_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct TRIANGULATE_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct TRIANGULATE_L {	/* -L<indexfile> */
		bool active;
		bool binary;
		char *file;
	} L;
	struct TRIANGULATE_M {	/* -M */
		bool active;
	} M;
	struct TRIANGULATE_N {	/* -N */
		bool active;
	} N;
	struct TRIANGULATE_Q {	/* -Q[n] */
		bool active;
		unsigned int mode;
	} Q;
	struct TRIANGULATE_S {	/* -S[<start>][+z[a|l|m|p|u]][+g] */
		bool active;
		bool color;	/* If +g given */
		int64_t firstpol;		/* Number of first polygon [0] */
		enum triangulate_enum_stat mode;	/* Compute a <value> in header depending on mode */
	} S;
	struct TRIANGULATE_T {	/* -T */
		bool active;
	} T;
	struct TRIANGULATE_Z {	/* -Z */
		bool active;
	} Z;
};

struct TRIANGULATE_EDGE {
	unsigned int begin, end;
};

#define CUBE_ALPHA 2.0	/* Factor from original CUBE algorithm and it adjusts for relative distances; here set to 2.0 */

enum curve_enum {	/* Indices for coeff array for normalization */
	GMT_H = GMT_Z + 1	,	/* Index into input/output rows */
	GMT_V,
	GMT_U = GMT_H
};

GMT_LOCAL double triangulate_median3 (double three[]) {
	/* Fast median of three values instead of calling gmt_median().
	 * https://stackoverflow.com/questions/17158667/minimum-no-of-comparisons-to-find-median-of-3-numbers
	 */
	double x = three[0] - three[1];
	double y = three[1] - three[2], z;
	if ((x * y) > 0.0) return three[1];
	z = three[0] - three[2];
	if ((x * z) > 0.0) return three[2];
	return three[0];
}

GMT_LOCAL double triangulate_mode3 (double three[]) {
	/* Fast mode of three values instead of calling gmt_mode(). */
	if (three[0] == three[1] || three[0] == three[2]) return (three[0]);
	if (three[1] == three[2]) return (three[1]);
	return triangulate_median3 (three);	/* All three are different so return median instead */
}

GMT_LOCAL int triangulate_compare_edge (const void *p1, const void *p2) {
	const struct TRIANGULATE_EDGE *a = p1, *b = p2;

	if (a->begin < b->begin) return (-1);
	if (a->begin > b->begin) return (+1);
	if (a->end < b->end) return (-1);
	if (a->end > b->end) return (+1);
	return (0);
}

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TRIANGULATE_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct TRIANGULATE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->D.dir = 2;	/* No derivatives */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct TRIANGULATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->F.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
#ifdef NNN_MODE
	GMT_Usage (API, 0, "usage: %s [<table>] [-A] [-C<slopegrid>] [-Dx|y] [-E<empty>] [-G%s] [%s] [%s] [-L<indextable>[+b]] [-M] [-N] "
		"[-Q[n]] [%s] [-S[<first>][+z[a|l|m|p|u]]] [-T] [%s] [-Z] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n", name, GMT_OUTGRID,
		GMT_I_OPT, GMT_J_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT,
		GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);
#else
	GMT_Usage (API, 0, "usage: %s [<table>] [-A] [-C<slopegrid>] [-Dx|y] [-E<empty>] [-G%s] [%s] [%s] [-L<indextable>[+b]] [-M] [-N] "
		"[-Q] [%s] [-S[<first>][+z[a|l|m|p|u]]] [-T] [%s] [-Z] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n", name, GMT_OUTGRID, GMT_I_OPT, 
		GMT_J_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT,
		GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);
#endif

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A Compute and print triangle areas in header records (requires -S).");
	GMT_Usage (API, 1, "\n-C<slopegrid>");
	GMT_Usage (API, -2, "Compute propagated uncertainty via CURVE algorithm. Give name of input slope grid. The slope "
		"grid (in degrees) also sets -R -I [-r]. Expects (x,y,h,v) or (x,y,z,h,v) on input. Requires -G and cannot be "
		"used with -D, -M, -N, -Q, -S, or -T.");
	GMT_Usage (API, 1, "\n-Dx|y");
	GMT_Usage (API, -2, "Take derivative in the x- or y-direction (only with -G) [Default is z value].");
	GMT_Usage (API, 1, "\n-E<empty>");
	GMT_Usage (API, -2, "Value to use for empty nodes [Default is NaN].");
	gmt_outgrid_syntax (API, 'G', "Grid data. Give name of output grid file and specify -R -I [-r]. If -C is used then output "
		"grids will hold propagated uncertainties and no -R -I [-r] is required. Cannot be combined with -N.");
#ifdef NNN_MODE
	GMT_Usage (API, -2, "Use -Qn for natural nearest neighbors [Default is linear triangulation]");
#endif
	GMT_Option (API, "I,J-");
	GMT_Usage (API, 1, "\n-L<indextable>[+b]");
	GMT_Usage (API, -2, "File with triplets of point indices for each triangle "
		"[Default performs the Delaunay triangulation on <table>]. Append +b to read this file using "
		"the same binary settings as for the primary input file [Read as ASCII].");
	GMT_Usage (API, 1, "\n-M Output triangle edges as multiple segments separated by segment headers. [Default is to "
		"output the indices of vertices for each Delaunay triangle].");
	GMT_Usage (API, 1, "\n-N Write indices of vertices to standard output when -G is used [only write the grid].");
	GMT_Usage (API, 1, "\n-Q Compute Voronoi polygon edges instead (requires -R and Shewchuk algorithm) [Delaunay triangulation].");
#ifdef NNN_MODE
	GMT_Usage (API, -2, "Append n to produce closed Voronoi polygons.");
#endif
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-S[<first>][+z[a|l|m|p|u]]");
	GMT_Usage (API, -2, "Output triangle polygons as multiple segments separated by segment headers. Append <first>, "
		"an integer, to report polygon numbers counting from <first> [Default counts from zero]. Cannot be used with -Q. "
		"Alternatively, compute representative value for the triplet z-values at triangle nodes via modifier +z (implies -Z).  Modes are");
	GMT_Usage (API, 3, "a: The mean of triplet [Default].");
	GMT_Usage (API, 3, "l: The lower value of triplet.");
	GMT_Usage (API, 3, "m: The median of triplet.");
	GMT_Usage (API, 3, "p: The mode of triplet.");
	GMT_Usage (API, 3, "u: The upper value of triplet.");
	GMT_Usage (API, 1, "\n-T Output triangles or polygons even if gridding has been selected with -G. Default behavior "
		"is to produce a grid based on the triangles or polygons only.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-Z Expect (x,y,z) data on input (and output); automatically set if -G or -S+z is used [Expect (x,y) data].");
	GMT_Option (API, "bi2");
	if (gmt_M_showusage (API))  GMT_Usage (API, 1, "\n-bo Write binary (double) index table [Default is ASCII i/o].");
	GMT_Option (API, "d,e,f,h,i,qi,r,s,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct TRIANGULATE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to triangulate and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files after checking they exist */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;
			case '>':	/* Got named output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Out.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->Out.file));
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Calculate triangle areas and add to header in -S */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				break;
			case 'C':	/* CURVE input slope grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->C.file));
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				switch (opt->arg[0]) {
					case 'x': case 'X':
						Ctrl->D.dir = GMT_X; break;
					case 'y': case 'Y':
						Ctrl->D.dir = GMT_Y; break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -D: Give -Dx or -Dy\n");
						n_errors++; break;
				}
				break;
			case 'E':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				break;
			case 'F':	/* Previous grid input values used */
				if (gmt_M_compat_check (GMT, 4) && opt->arg[0] == 0) {	/* Old -F instead of -r */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -F is deprecated. Use -r instead.\n" );
					n_errors += gmt_parse_common_options (GMT, "r", 'r', "");
					break;
				}
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				GMT_Report (API, GMT_MSG_WARNING, "-F is experimental and unstable.\n");
				if ((c = strstr (opt->arg, "+d"))) {	/* Got modifier to also use input data */
					c[0] = '\0';	/* Temporarily chop off modifier */
					Ctrl->F.mode = 1;
				}
				if (opt->arg[0]) Ctrl->F.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->F.file))) n_errors++;
				if (c) c[0] = '+';	/* Restore chopped off modifier */
				break;
			case 'G':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file));
				break;
			case 'I':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* Triplet file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				if ((c = strstr (opt->arg, "+b"))) {
					Ctrl->L.binary = true;
					c[0] = '\0';
				}
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->L.file));
				if (c) c[0] = '+';	/* Restore chopped off modifier */
				break;
			case 'm':
				if (gmt_M_compat_check (GMT, 4)) /* Warn and fall through on purpose */
					GMT_Report (API, GMT_MSG_COMPAT, "-m option is deprecated and reverted back to -M.\n");
				else {
					n_errors += gmt_default_option_error (GMT, opt);
					break;
				}
				/* Intentionally fall through */
			case 'M':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'N':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'Q':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				if (strchr (opt->arg, 'n')) {
					GMT_Report (API, GMT_MSG_INFORMATION, "-Qn is experimental and may be unstable.\n");
					Ctrl->Q.mode |= 1;
				}
				break;
			case 'S':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				if ((c = strstr (opt->arg, "+z"))) {
					switch (c[2]) {
						case 'a': case '\0': Ctrl->S.mode = TRI_MEAN; break;	/* Use mean of three node z-values [Default] */
						case 'l': Ctrl->S.mode = TRI_LOWER; break;	/* Use mean of three node z-values */
						case 'm': Ctrl->S.mode = TRI_MEDIAN; break;	/* Use mean of three node z-values */
						case 'p': Ctrl->S.mode = TRI_MODE; break;	/* Use mean of three node z-values */
						case 'u': Ctrl->S.mode = TRI_UPPER; break;	/* Use mean of three node z-values */
						default:
							GMT_Report (API, GMT_MSG_ERROR, "Option -S: Modifier +z given unavailable statistic %s.\n", &c[2]);
							n_errors++;
							break;
					}
					c[0] = '\0';	/* Truncate the modifier */
				}
				else
					Ctrl->S.mode = TRI_POLY;
				if (opt->arg[0])
					Ctrl->S.firstpol = atol (opt->arg);
				else
					Ctrl->S.firstpol = 0;
				if (c) c[0] = '+';	/* Restore chopped off modifier */
				break;
			case 'T':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_add_R_if_modern_and_true (GMT, THIS_MODULE_NEEDS, (Ctrl->G.active && !Ctrl->C.active) || Ctrl->Q.active);

	n_errors += gmt_check_binary_io (GMT, 2);
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.binary && !GMT->common.b.active[GMT_IN], "Option -L: Cannot imply binary node input if main input is not also binary (see -bi)\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 ||
	                                   GMT->common.R.inc[GMT_Y] <= 0.0), "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && !Ctrl->S.active, "Option -A: Requires -S\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->Q.active, "Option -A: Not compatible with -Q\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !Ctrl->C.file, "Option -C: Must append slope grid file name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && (GMT->common.R.active[ISET] + GMT->common.R.active[RSET]) != 2,
	                                   "Must specify -R, -I, -G for gridding\n");
	(void)gmt_M_check_condition (GMT, !Ctrl->G.active && GMT->common.R.active[ISET], "Option -I: not needed when -G is not set\n");
	(void)gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->Q.active) && GMT->common.R.active[RSET],
	                             "Option -R not needed when -G or -Q are not set\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->Q.active, "Option -S: Cannot be used with -Q\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->G.active, "Option -N: Only required with -G\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && !GMT->common.R.active[RSET], "Option -Q: Requires -R\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->L.active, "Option -L: Cannot be used with -Q\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->M.active + Ctrl->N.active + Ctrl->S.active) > 1, "Can only use one of -M, -N, -S at the same time since all write to stdout\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && GMT->current.setting.triangulate == GMT_TRIANGLE_WATSON,
	                                   "Option -Q: Requires Shewchuk triangulation algorithm\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && (GMT->common.R.active[RSET] || GMT->common.R.active[ISET] ||
	                                   GMT->common.R.active[GSET]),
	                                   "Option -C: No -R -I [-r] allowed, domain given by slope grid\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && (Ctrl->D.active || Ctrl->F.active || Ctrl->M.active ||
	                                   Ctrl->N.active || Ctrl->Q.active || Ctrl->S.active || Ctrl->T.active),
	                                   "Option -C: Cannot use -D, -F, -M, -N, -Q, -S, T\n");
	if (!(Ctrl->M.active || Ctrl->Q.active || Ctrl->S.active || Ctrl->N.active)) Ctrl->N.active = !Ctrl->G.active;	/* The default action */

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_triangulate (void *V_API, int mode, void *args) {
	int *link = NULL;	/* Must remain int and not int due to triangle function */

	uint64_t ij, ij1, ij2, ij3, np = 0, i, j, k, n_edge, p, node = 0, seg, n = 0;
	unsigned int n_input, n_output, side;
	int error = GMT_NOERROR, col_min, col_max, row_min, row_max;
	openmp_int row, col;
	bool triplets[2] = {false, false}, map_them = false, do_output = true, get_input = false;

	size_t n_alloc;

	double hj, hk, hl, vj, vk, vl, uv1, uv2, uv3, dv1, dv2, dv3, distv1, distv2, distv3;
	double zj, zk, zl, zlj, zkj, xp, yp, a, b, c, f;
	double xkj, xlj, ykj, ylj, out[3], vx[4], vy[4];
	double *xx = NULL, *yy = NULL, *zz = NULL, *in = NULL, *zpol = NULL;
	double *xf = NULL, *yf = NULL, *hh = NULL, *vv = NULL;

	char *tri_algorithm[2] = {"Watson", "Shewchuk"};
	char record[GMT_BUFSIZ];

	struct GMT_GRID *Grid = NULL, *F = NULL, *Slopes = NULL;
	struct GMT_DATASET *V = NULL;
	struct GMT_DATASEGMENT *P = NULL;
	struct GMT_RECORD *In = NULL;

	struct TRIANGULATE_EDGE *edge = NULL;
	struct TRIANGULATE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the triangulate main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "%s triangulation algorithm selected\n", tri_algorithm[GMT->current.setting.triangulate]);
	get_input = (!(Ctrl->F.active && Ctrl->F.mode == 0));

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	if (Ctrl->G.active) {	/* Need to build an output grid */
		if (Ctrl->C.active) {	/* Read slope grid and use its domain to set -R -I [-r] for output grid */
			if ((Slopes = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->C.file, NULL)) == NULL)
				Return (API->error);
			if ((Grid = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Slopes)) == NULL)
				Return (API->error);
		}
		else if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	}
	if (Ctrl->Q.active && Ctrl->Z.active)
		GMT_Report (API, GMT_MSG_INFORMATION, "We will read (x,y,z), but only (x,y) will be output when -Q is used\n");
	if (Ctrl->M.active && Ctrl->S.active)
		GMT_Report (API, GMT_MSG_ERROR, "-M and -S cannot be used together, -S will be ignored.\n");
	n_output = (Ctrl->N.active || Ctrl->Z.active) ? 3 : 2;
	if (Ctrl->M.active && Ctrl->Z.active) n_output = 3;
	triplets[GMT_OUT] = (n_output == 3);
	if (Ctrl->G.active && !Ctrl->S.active && !Ctrl->T.active && !Ctrl->N.active) do_output = false;	/* If gridding then we require S, -T or -N to do output */
	if ((error = GMT_Set_Columns (API, GMT_OUT, n_output, GMT_COL_FIX_NO_TEXT)) != 0) Return (error);
	n_input = (Ctrl->G.active || Ctrl->S.mode > TRI_POLY || Ctrl->Z.active) ? 3 : 2;
	if (n_output > n_input) triplets[GMT_OUT] = false;	/* No can do. */
	if (Ctrl->C.active) n_input += 2;	/* Curve requires the horizontal and vertical uncertainties */
	triplets[GMT_IN] = (n_input == 3 || n_input == 5);	/* Either x,y,z or x,y,z,h,v input */
	if (Ctrl->S.mode > TRI_POLY && n_input < 3) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -S: Your modifier(s) require (x, y, z) records.\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->G.active && GMT->common.R.active[RSET] && GMT->common.J.active) { /* Gave -R -J */
		map_them = true;
		if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, Grid->header->wesn), "")) Return (GMT_PROJECTION_ERROR);
	}

	/* Now we are ready to take on some input values */

	n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	xx = gmt_M_memory (GMT, NULL, n_alloc, double);
	yy = gmt_M_memory (GMT, NULL, n_alloc, double);
	if (triplets[GMT_IN]) zz = gmt_M_memory (GMT, NULL, n_alloc, double);
	if (Ctrl->C.active) {
		hh = gmt_M_memory (GMT, NULL, n_alloc, double);
		vv = gmt_M_memory (GMT, NULL, n_alloc, double);
	}
	n = 0;

	if (get_input) {	/* Read primary input */
		if ((error = GMT_Set_Columns (API, GMT_IN, n_input, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
			goto time_to_let_go;
	}

	/* Initialize the i/o since we are doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		error = API->error;	goto time_to_let_go;
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		error = API->error;	goto time_to_let_go;
	}

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				error = GMT_RUNTIME_ERROR;	goto time_to_let_go;
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}

		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			error = API->error;	goto time_to_let_go;
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		xx[n] = in[GMT_X];	yy[n] = in[GMT_Y];
		if (triplets[GMT_IN]) zz[n] = in[GMT_Z];
		if (Ctrl->C.active) {
			hh[n] = fabs(in[GMT_H]);
			vv[n] = fabs(in[GMT_V]);
		}
		n++;

		if (n == n_alloc) {	/* Get more memory */
			n_alloc <<= 1;
			xx = gmt_M_memory (GMT, xx, n_alloc, double);
			yy = gmt_M_memory (GMT, yy, n_alloc, double);
			if (triplets[GMT_IN]) zz = gmt_M_memory (GMT, zz, n_alloc, double);
			if (Ctrl->C.active) {
				hh = gmt_M_memory (GMT, hh, n_alloc, double);
				vv = gmt_M_memory (GMT, vv, n_alloc, double);
			}
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		error = API->error;	goto time_to_let_go;
	}

	if (Ctrl->F.active) {	/* Use non-NaN nodes in a previous grid as input data, possibly in addition to input records */
		double *wesn = (Ctrl->G.active) ? Grid->header->wesn : NULL;
		double xnoise, ynoise, rx, ry, percent = 5.0, fraction = 0.01 * percent;
		if ((F = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, wesn, Ctrl->F.file, NULL)) == NULL) {
			error = API->error;	goto time_to_let_go;
		}
		xf = F->x;
		yf = F->y;
		xnoise = fraction * F->header->inc[GMT_X];
		ynoise = fraction * F->header->inc[GMT_Y];
		gmt_M_grd_loop (GMT, F, row, col, ij) {
			if (gmt_M_is_fnan (F->data[ij])) continue;	/* Only add real data to the input list */
			rx = xnoise * gmt_nrand (GMT);
			ry = ynoise * gmt_nrand (GMT);
			xx[n] = xf[col] + rx;	yy[n] = yf[row] + ry;
			GMT_Report (API, GMT_MSG_DEBUG, "Adding grid point with %g noise: %d\t	%g\t%g\n", percent, (int)n, xx[n], yy[n]);
			if (triplets[GMT_IN]) zz[n] = F->data[ij];
			n++;
			if (n == n_alloc) {	/* Get more memory */
				n_alloc <<= 1;
				xx = gmt_M_memory (GMT, xx, n_alloc, double);
				yy = gmt_M_memory (GMT, yy, n_alloc, double);
				zz = gmt_M_memory (GMT, zz, n_alloc, double);
			}
		}
	}

	if (n >= INT_MAX) {
		GMT_Report (API, GMT_MSG_ERROR, "Cannot triangulate more than %d points\n", INT_MAX);
		error = GMT_RUNTIME_ERROR;	goto time_to_let_go;
	}

	xx = gmt_M_memory (GMT, xx, n, double);
	yy = gmt_M_memory (GMT, yy, n, double);
	if (triplets[GMT_IN]) zz = gmt_M_memory (GMT, zz, n, double);
	if (Ctrl->C.active) {
		hh = gmt_M_memory (GMT, hh, n, double);
		vv = gmt_M_memory (GMT, vv, n, double);
	}

	if (n == 0) {
		GMT_Report (API, GMT_MSG_ERROR, "No data points given - so no triangulation can take effect\n");
		error = GMT_RUNTIME_ERROR;	goto time_to_let_go;
	}

	if (map_them) {	/* Must make parallel arrays for projected x/y */
		double *xxp = NULL, *yyp = NULL;

		xxp = gmt_M_memory (GMT, NULL, n, double);
		yyp = gmt_M_memory (GMT, NULL, n, double);
		for (i = 0; i < n; i++) gmt_geo_to_xy (GMT, xx[i], yy[i], &xxp[i], &yyp[i]);

		GMT_Report (API, GMT_MSG_INFORMATION, "Do Delaunay optimal triangulation on projected coordinates\n");

		if (Ctrl->Q.active) {
			if ((V = gmt_voronoi (GMT, xxp, yyp, n, GMT->current.proj.rect, Ctrl->Q.mode)) == NULL) {
				gmt_M_free (GMT, xxp);	gmt_M_free (GMT, yyp);
				error = GMT_RUNTIME_ERROR;	goto time_to_let_go;
			}
		}
		else if (!Ctrl->L.active)
			np = gmt_delaunay (GMT, xxp, yyp, n, &link);

		gmt_M_free (GMT, xxp);	/* Cannot do this if we are doing NN gridding since we need xxp, yyp */
		gmt_M_free (GMT, yyp);
	}
	else {
		GMT_Report (API, GMT_MSG_INFORMATION, "Do Delaunay optimal triangulation on given coordinates\n");

		if (Ctrl->Q.active) {
			if ((V = gmt_voronoi (GMT, xx, yy, n, GMT->common.R.wesn, Ctrl->Q.mode)) == NULL) {
				error = GMT_RUNTIME_ERROR;	goto time_to_let_go;
			}
		}
		else if (!Ctrl->L.active)
			np = gmt_delaunay (GMT, xx, yy, n, &link);
	}
	if (Ctrl->L.active) {	/* Read precalculated triangulation indices instead */
		int64_t s_np;	/* Need a signed 64-bit integer here */
		if ((s_np = gmt_read_triangulation (GMT, 'L', Ctrl->L.file, Ctrl->L.binary, n, &link)) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_ERROR, "Error reading triangulation indices from file %s\n", Ctrl->L.file);
			Return (GMT_RUNTIME_ERROR);
		}
		np = (uint64_t) s_np;	/* Set number of points as unsigned 64-bit int */	
	}

	if (Ctrl->Q.active) {
		char header[GMT_LEN64] = {""};
		char *feature[2] = {"edges", "polygons"};
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " Voronoi %s found\n", V->n_segments, feature[Ctrl->Q.mode]);
		zpol = gmt_M_memory (GMT, NULL, V->n_segments, double);
		gmt_set_inside_mode (GMT, V, GMT_IOO_UNKNOWN);
		if (triplets[GMT_IN] && Ctrl->Q.mode) {
			for (seg = 0; seg < V->n_segments; seg++) {
				P = V->table[0]->segment[seg];
				/* Annoyingly, must first identify the input data point that is inside this polygon */
				for (k = side = 0; !side && k < n; k++) {
					if (yy[k] < P->min[GMT_Y] || yy[k] > P->max[GMT_Y]) continue;
					if (xx[k] < P->min[GMT_X] || xx[k] > P->max[GMT_X]) continue;
					side = gmt_inonout (GMT, xx[k], yy[k], P);
					if (side != GMT_OUTSIDE) node = k;	/* Found the data node */
				}
				zpol[seg] = zz[node];
				sprintf (header, "%s -Z%g", P->header, zpol[seg]);
				gmt_M_str_free (P->header);
				P->header = strdup (header);
			}
		}
	}
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " Delaunay triangles found\n", np);

	if (Ctrl->G.active) {	/* Need to set up an output grid  */
		if (Ctrl->F.active && gmt_M_grd_same_shape (GMT, Grid, F) && gmt_M_grd_same_region (GMT, Grid, F) && !gmt_M_file_is_memory (Ctrl->F.file)) {
			/* F and G are the same region and F is not a memory grid via API.  Reuse F->data for G */
			Grid->data = F->data;	F->data = NULL;
		}
		else if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_GRID, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid) == NULL) {
			error = API->error;	goto time_to_let_go;
		}
		if (Ctrl->F.active && F->data) {	/* Not the same area, must copy over pregrid node values */
			openmp_int frow, fcol;
			gmt_M_grd_loop (GMT, F, frow, fcol, ij) {
				row = gmt_M_grd_y_to_row (GMT, yf[frow], Grid->header);
				if (row < 0 || row >= (openmp_int)Grid->header->n_columns) continue;
				col = gmt_M_grd_x_to_col (GMT, xf[fcol], Grid->header);
				if (col < 0 || col >= (openmp_int)Grid->header->n_rows) continue;
				p = gmt_M_ijp (Grid->header, row, col);
				Grid->data[p] = F->data[ij];	/* This also copies the NaNs from F to Grid */
			}
		}
	}

	if (Ctrl->G.active && Ctrl->Q.active) {	/* Grid via natural nearest neighbor using Voronoi polygons */
		bool periodic, duplicate_col;
		int s_row, south_row, north_row, w_col, e_col;
		unsigned int row, col, p_col, west_col, east_col, nx1;
		uint64_t n_set = 0;
		double *grid_lon = NULL, *grid_lat = NULL;
		GMT_Report (API, GMT_MSG_INFORMATION, "Perform natural nearest neighbor gridding\n");

		nx1 = (Grid->header->registration == GMT_GRID_PIXEL_REG) ? Grid->header->n_columns : Grid->header->n_columns - 1;
		periodic = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
		duplicate_col = (periodic && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
		grid_lon = Grid->x;
		grid_lat = Grid->y;
		gmt_set_inside_mode (GMT, V, GMT_IOO_UNKNOWN);

		for (seg = 0; seg < V->n_segments; seg++) {
			P = V->table[0]->segment[seg];
			/* Find bounding row/cols over which to loop for this polygon */
			south_row = (int)gmt_M_grd_y_to_row (GMT, P->min[GMT_Y], Grid->header);
			north_row = (int)gmt_M_grd_y_to_row (GMT, P->max[GMT_Y], Grid->header);
			w_col  = (int)gmt_M_grd_x_to_col (GMT, P->min[GMT_X], Grid->header);
			while (w_col < 0) w_col += nx1;
			west_col = w_col;
			e_col = (int)gmt_M_grd_x_to_col (GMT, P->max[GMT_X], Grid->header);
			while (e_col < w_col) e_col += nx1;
			east_col = e_col;
			/* So here, any polygon will have a positive (or 0) west_col with an east_col >= west_col */
			for (s_row = north_row; s_row <= south_row; s_row++) {	/* For each scanline intersecting this polygon */
				if (s_row < 0) continue;	/* North of region */
				row = s_row; if (row >= Grid->header->n_rows) continue;	/* South of region */
				for (p_col = west_col; p_col <= east_col; p_col++) {	/* March along the scanline using col >= 0 */
					if (p_col >= Grid->header->n_columns) {	/* Off the east end of the grid */
						if (periodic)	/* Just shuffle to the corresponding point inside the global grid */
							col = p_col - nx1;
						else		/* Sorry, really outside the region */
							continue;
					}
					else
						col = p_col;
					if (Ctrl->F.active) {	/* Only do interpolation at this point if grid == NaN so check before doing slow gmt_inonout */
						p = gmt_M_ijp (Grid->header, row, col);
						if (!gmt_M_is_fnan (Grid->data[p])) continue;
					}
					side = gmt_inonout (GMT, grid_lon[col], grid_lat[row], P);
					if (side == GMT_OUTSIDE) continue;	/* Outside polygon */
					p = gmt_M_ijp (Grid->header, row, col);
					Grid->data[p] = (gmt_grdfloat)zpol[seg];
					n_set++;
					if (duplicate_col) {	/* Duplicate the repeating column on the other side of this one */
						if (col == 0) Grid->data[p+nx1] = Grid->data[p], n_set++;
						else if (col == nx1) Grid->data[p-nx1] = Grid->data[p], n_set++;
					}
				}
			}
		}
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
			error = API->error;	goto time_to_let_go;
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			error = API->error;	goto time_to_let_go;
		}
	}
	else if (Ctrl->G.active && !Ctrl->Q.active) {	/* Grid via planar triangle segments */
		int n_columns = Grid->header->n_columns, n_rows = Grid->header->n_rows;	/* Signed versions */
		double inv_delta_min = 1.0 / MIN (GMT->common.R.inc[GMT_X], GMT->common.R.inc[GMT_Y]);	/* Inverse minimum spacing */
		double s_H = 1.0, distSum = 0.0, sigma = 0.0;
		double *CoordsX = NULL, *CoordsY = NULL;
		GMT_Report (API, GMT_MSG_INFORMATION, "Perform Delaunay triangle gridding\n");
		if (!Ctrl->F.active) {
			if (!Ctrl->E.active) Ctrl->E.value = GMT->session.d_NaN;
			for (p = 0; p < Grid->header->size; p++) Grid->data[p] = (gmt_grdfloat)Ctrl->E.value;	/* initialize grid */
		}

		if (Ctrl->C.active) {	/* CURVE needs grid coordinates */
			CoordsX = Grid->x;
			CoordsY = Grid->y;
			for (p = 0; p < Grid->header->size; p++) Slopes->data[p] = (gmt_grdfloat)tan (D2R * Slopes->data[p]);	/* Take tan or slopes here instead of later */
		}

		for (k = ij = 0; k < np; k++) {

			/* Find equation for the plane as z = ax + by + c */

			vx[0] = vx[3] = xx[link[ij]];	vy[0] = vy[3] = yy[link[ij]];	zj = zz[link[ij++]];
			vx[1] = xx[link[ij]];		vy[1] = yy[link[ij]];		zk = zz[link[ij++]];
			vx[2] = xx[link[ij]];		vy[2] = yy[link[ij]];		zl = zz[link[ij++]];

			xkj = vx[1] - vx[0];	ykj = vy[1] - vy[0];	zkj = zk - zj;
			xlj = vx[2] - vx[0];	ylj = vy[2] - vy[0];	zlj = zl - zj;

			f = 1.0 / (xkj * ylj - ykj * xlj);
			a = -f * (ykj * zlj - zkj * ylj);
			b = -f * (zkj * xlj - xkj * zlj);
			c = -a * vx[1] - b * vy[1] + zk;

			/* Compute grid indices the current triangle may cover, assuming all triangles are
			   in the -R region (Grid->header->wesn[XLO]/x_max etc.)  Always, col_min <= col_max, row_min <= row_max.
			 */

			xp = MIN (MIN (vx[0], vx[1]), vx[2]);	col_min = (int)gmt_M_grd_x_to_col (GMT, xp, Grid->header);
			xp = MAX (MAX (vx[0], vx[1]), vx[2]);	col_max = (int)gmt_M_grd_x_to_col (GMT, xp, Grid->header);
			yp = MAX (MAX (vy[0], vy[1]), vy[2]);	row_min = (int)gmt_M_grd_y_to_row (GMT, yp, Grid->header);
			yp = MIN (MIN (vy[0], vy[1]), vy[2]);	row_max = (int)gmt_M_grd_y_to_row (GMT, yp, Grid->header);

			/* Adjustments for triangles outside -R region. */
			/* Triangle to the left or right. */
			if ((col_max < 0) || (col_min >= n_columns)) continue;
			/* Triangle Above or below */
			if ((row_max < 0) || (row_min >= n_rows)) continue;

			/* Triangle covers boundary, left or right. */
			if (col_min < 0) col_min = 0;
			if (col_max >= n_columns) col_max = Grid->header->n_columns - 1;
			/* Triangle covers boundary, top or bottom. */
			if (row_min < 0) row_min = 0;
			if (row_max >= n_rows) row_max = Grid->header->n_rows - 1;

			for (row = (openmp_int)row_min; row <= (openmp_int)row_max; row++) {
				yp = gmt_M_grd_row_to_y (GMT, row, Grid->header);
				p = gmt_M_ijp (Grid->header, row, col_min);
				for (col = (openmp_int)col_min; col <= (openmp_int)col_max; col++, p++) {
					if (Ctrl->F.active && !gmt_M_is_fnan (Grid->data[p])) continue;	/* Only do interpolation at this point if grid == NaN so check before doing gmt_non_zero_winding */

					xp = gmt_M_grd_col_to_x (GMT, col, Grid->header);
					if (!gmt_non_zero_winding (GMT, xp, yp, vx, vy, 4)) continue;	/* Outside */

					if (Ctrl->D.dir == GMT_X)	/* d/dx of solution */
						Grid->data[p] = (gmt_grdfloat)a;
					else if (Ctrl->D.dir == GMT_Y)	/* d/dy of solution */
						Grid->data[p] = (gmt_grdfloat)b;
					else if (Ctrl->C.active) {	/* CURVE propagated uncertainty prediction */
						hj = hh[link[ij - 1]];	vj = vv[link[ij - 1]];
						hk = hh[link[ij - 1]];	vk = vv[link[ij - 1]];
						hl = hh[link[ij - 1]];	vl = vv[link[ij - 1]];
						distv1 = sqrt (pow (CoordsX[col] - vx[0], 2.0) + pow (CoordsY[row] - vy[0], 2.0));
						distv2 = sqrt (pow (CoordsX[col] - vx[1], 2.0) + pow (CoordsY[row] - vy[1], 2.0));
						distv3 = sqrt (pow (CoordsX[col] - vx[2], 2.0) + pow (CoordsY[row] - vy[2], 2.0));
						uv1 = pow (vj, 2.0) * (1.0 + pow ((distv1 + s_H * hj) * inv_delta_min, CUBE_ALPHA)) + pow (Slopes->data[p] * hj, 2.0);
						uv2 = pow (vk, 2.0) * (1.0 + pow ((distv2 + s_H * hk) * inv_delta_min, CUBE_ALPHA)) + pow (Slopes->data[p] * hk, 2.0);
						uv3 = pow (vl, 2.0) * (1.0 + pow ((distv3 + s_H * hl) * inv_delta_min, CUBE_ALPHA)) + pow (Slopes->data[p] * hl, 2.0);
						if (fabs (distv1) < DBL_EPSILON)
							sigma = sqrt (uv1);
						else if (fabs (distv2) < DBL_EPSILON)
							sigma = sqrt (uv2);
						else if (fabs (distv3) < DBL_EPSILON)
							sigma = sqrt (uv3);
						else {
							dv1 = uv1 / distv1;
							dv2 = uv2 / distv2;
							dv3 = uv3 / distv3;
							distSum = 1.0 / distv1 + 1.0 / distv2 + 1.0 / distv3;
							sigma = sqrt ((dv1 + dv2 + dv3) / distSum);
						}
						Grid->data[p] = (gmt_grdfloat)(sigma);
					}
					else	/* Planar prediction */
						Grid->data[p] = (gmt_grdfloat)(a * xp + b * yp + c);
				}
			}
		}

		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
			error = API->error;	goto time_to_let_go;
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			error = API->error;	goto time_to_let_go;
		}
	}

	if (do_output && (Ctrl->M.active || Ctrl->Q.active || Ctrl->S.active || Ctrl->N.active)) {	/* Requires output to stdout */

		struct GMT_RECORD *Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
		if (!Ctrl->Q.active) {	/* Still record-by-record output */
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
				error = API->error;	goto time_to_let_go;
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
				error = API->error;	goto time_to_let_go;
			}
		}
		if (Ctrl->M.active || Ctrl->Q.active) {	/* Must find unique edges to output only once */
			gmt_set_segmentheader (GMT, GMT_OUT, true);
			if (Ctrl->Q.active) {	/* Voronoi edges */
				struct GMT_DATASET_HIDDEN *VH = gmt_get_DD_hidden (V);
				if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, V->geometry, VH->io_mode, NULL, Ctrl->Out.file, V) != GMT_NOERROR) {
					error = API->error;	goto time_to_let_go;
				}
			}
			else {	/* Triangle edges */
				if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_LINE) != GMT_NOERROR) {	/* Sets output geometry */
					error = API->error;	goto time_to_let_go;
				}
				n_edge = 3 * np;
				edge = gmt_M_memory (GMT, NULL, n_edge, struct TRIANGULATE_EDGE);
				for (i = ij1 = 0, ij2 = 1, ij3 = 2; i < np; i++, ij1 += 3, ij2 += 3, ij3 += 3) {
					edge[ij1].begin = link[ij1];	edge[ij1].end = link[ij2];
					edge[ij2].begin = link[ij2];	edge[ij2].end = link[ij3];
					edge[ij3].begin = link[ij1];	edge[ij3].end = link[ij3];
				}
				for (i = 0; i < n_edge; i++)
					if (edge[i].begin > edge[i].end) gmt_M_int_swap (edge[i].begin, edge[i].end);

				qsort (edge, n_edge, sizeof (struct TRIANGULATE_EDGE), triangulate_compare_edge);
				for (i = 1, j = 0; i < n_edge; i++) {
					if (edge[i].begin != edge[j].begin || edge[i].end != edge[j].end) j++;
					edge[j] = edge[i];
				}
				n_edge = j + 1;

				GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " unique triangle edges\n", n_edge);

				for (i = 0; i < n_edge; i++) {
					sprintf (record, "Edge %d-%d", edge[i].begin, edge[i].end);
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);
					out[GMT_X] = xx[edge[i].begin];	out[GMT_Y] = yy[edge[i].begin];
					if (triplets[GMT_OUT]) out[GMT_Z] = zz[edge[i].begin];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					out[GMT_X] = xx[edge[i].end];	out[GMT_Y] = yy[edge[i].end];
					if (triplets[GMT_OUT]) out[GMT_Z] = zz[edge[i].end];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
				gmt_M_free (GMT, edge);
			}
		}
		else if (Ctrl->S.active)  {	/* Write triangle polygons */
			char area_txt[GMT_LEN64] = {""}, a_format[GMT_LEN128] = {""};
			int64_t first = Ctrl->S.firstpol;
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POLY) != GMT_NOERROR) {	/* Sets output geometry */
				error = API->error;	goto time_to_let_go;
			}
			gmt_set_segmentheader (GMT, GMT_OUT, true);
			if (Ctrl->A.active)	/* Initialize the area format string */
				sprintf (a_format, " Area: %s", GMT->current.setting.format_float_out);

			if (triplets[GMT_OUT] && Ctrl->S.mode == TRI_NONE) {
				double z_mean;
				for (i = ij = 0; i < np; i++, ij += 3) {
					z_mean = (zz[link[ij]] + zz[link[ij+1]] + zz[link[ij+2]]) / 3;
					sprintf (record, "Polygon %d-%d-%d -Z%.8g", link[ij], link[ij+1], link[ij+2], z_mean);
					if (Ctrl->A.active) {	/* Compute and report area */
						double area = 0.5 * ((xx[link[ij]] - xx[link[ij+2]]) * (yy[link[ij+1]] - yy[link[ij]]) - (xx[link[ij]] - xx[link[ij+1]]) * (yy[link[ij+2]] - yy[link[ij]]));
						sprintf (area_txt, a_format, area);
						strcat (record, area_txt);
					}
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);
					for (k = 0; k < 3; k++) {	/* Three vertices */
						out[GMT_X] = xx[link[ij+k]];	out[GMT_Y] = yy[link[ij+k]];	out[GMT_Z] = zz[link[ij+k]];
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
					}
					/* Explicitly close the polygon */
					out[GMT_X] = xx[link[ij]];	out[GMT_Y] = yy[link[ij]];	out[GMT_Z] = zz[link[ij]];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
			else {	/* Write polygons with various segment header information */
				for (i = ij = 0; i < np; i++, ij += 3) {
					sprintf (record, "Polygon %d-%d-%d ", link[ij], link[ij+1], link[ij+2]);
					if (Ctrl->S.mode > TRI_POLY) {
						double z_triangle, z_node[3];
						for (k = 0; k < 3; k++) z_node[k] = zz[link[ij+k]];	/* Get the three vertices' z-values */
						switch (Ctrl->S.mode) {
							case TRI_LOWER:	/* Set z to the lowest of the three nodes */
								z_triangle = MIN (z_node[0], MIN (z_node[1], z_node[2]));
								break;
							case TRI_MEAN:	/* Set z to the mean of the three nodes */
								z_triangle = (z_node[0] + z_node[1] + z_node[2]) / 3.0;
								break;
							case TRI_MEDIAN:	/* Set z to the median of the three nodes */
								z_triangle = triangulate_median3 (z_node);
								break;
							case TRI_MODE:	/* Set z to the mode of the three nodes */
								z_triangle = triangulate_mode3 (z_node);
								break;
							case TRI_UPPER:	/* Set z to the highest of the three nodes */
								z_triangle = MAX (z_node[0], MAX (z_node[1], z_node[2]));
								break;
							default:	/* Just for Coverity */ 
								break;
						}
						sprintf (record, "Polygon %d-%d-%d -Z%g", link[ij], link[ij+1], link[ij+2], z_triangle);
					}
					else
						sprintf (record, "Polygon %d-%d-%d -Z%" PRIu64, link[ij], link[ij+1], link[ij+2], i+first);
					if (Ctrl->A.active) {	/* Compute and report area */
						double area = 0.5 * ((xx[link[ij]] - xx[link[ij+2]]) * (yy[link[ij+1]] - yy[link[ij]]) - (xx[link[ij]] - xx[link[ij+1]]) * (yy[link[ij+2]] - yy[link[ij]]));
						sprintf (area_txt, a_format, area);
						strcat (record, area_txt);
					}
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);
					for (k = 0; k < 3; k++) {	/* Three vertices */
						out[GMT_X] = xx[link[ij+k]];	out[GMT_Y] = yy[link[ij+k]];
						if (n_output == 3) out[GMT_Z] = zz[link[ij+k]];
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
					}
					/* Explicitly close the polygon */
					out[GMT_X] = xx[link[ij]];	out[GMT_Y] = yy[link[ij]];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
				}
			}
		}
		else if (Ctrl->N.active) {	/* Write table of indices */
			/* Set output format to regular float */
			gmt_set_cartesian (GMT, GMT_OUT);	/* Since output is no longer lon/lat */
			gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);
			for (i = ij = 0; i < np; i++, ij += 3) {
				for (k = 0; k < 3; k++) out[k] = (double)link[ij+k];
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			}
		}
		gmt_M_free (GMT, Out);
		if (!Ctrl->Q.active && GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			error = API->error;	goto time_to_let_go;
		}
	}

time_to_let_go:

	gmt_M_free (GMT, xx);
	gmt_M_free (GMT, yy);
	if (zpol) gmt_M_free (GMT, zpol);
	if (triplets[GMT_IN]) gmt_M_free (GMT, zz);
	if (Ctrl->C.active) {
		gmt_M_free (GMT, hh);
		gmt_M_free (GMT, vv);
	}
	if (Ctrl->L.active) gmt_M_free (GMT, link);
	else if (!Ctrl->Q.active) gmt_delaunay_free (GMT, &link);

	Return (error);
}
