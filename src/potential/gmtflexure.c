/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Author:      Paul Wessel
 * Date:        29-OCT-2015
 *
 *
 * 2-D plate flexure allowing for variable Te, load, and infill
 *
 * */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtflexure"
#define THIS_MODULE_MODERN_NAME	"gmtflexure"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute flexural deformation of 2-D loads, forces, and bending moments"
#define THIS_MODULE_KEYS	"ED(,QD(,TD(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vdefhio"

#define	YOUNGS_MODULUS	7.0e10		/* Pascal = Nt/m**2  */
#define	NORMAL_GRAVITY	9.806199203	/* Moritz's 1980 IGF value for gravity at 45 degrees latitude (m/s) */
#define	POISSONS_RATIO	0.25

/*
 * gmtflexure computes the flexure produced by an arbitrary varying load
 * on a variable rigidity plate. One of four possible boundary contditions
 * can be imposed on each end of the profiles. The user may [optionally] provide 2
 * input files. 1) Load file having x and load, and 2) Rigidity file
 * having x and rigidity. If both files are present, they must list load and
 * rigidity at the same x positions. All units must be in SI. The program writes
 * the deflections to standard output. Z axis is positive DOWN, so
 * positive loads, moments, and forces will all generate positive deflections.
 * The load file is optional, whereas the rigidity file OR a uniform plate
 * thickness (-E) must be supplied. If no inputfiles are given, then the min/max
 * distance and increment must be given on the commandline using the -T option.
 * An arbitrary horizontal stress may be imposed with the -F option. If rho_infill
 * and rho_water is different, a variable restoring force scheme is used (flx1dk)
 * rather than the fixed k(x) solution (flx1d). If there is pre-existing deformation
 * we use another solution (flx1dw0).
 * All profiles must have equidistant sampling!!
 *
 */

struct GMTFLEXURE_CTRL {
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A[l|r]<bc>[<args>] */
		bool active;
		unsigned int bc[2];	/* Left and Right BC code */
		double deflection[2], moment[2], force[2];	/* Left and Right arguments */
	} A;
	struct C {	/* -Cy<E> or -Cp<poisson> */
		bool active[2];
		double E, nu;
	} C;
	struct D {	/* -D<rhom/rhol[/rhoi]/rhow> */
		bool active;
		double rhom, rhol, rhoi, rhow;
	} D;
	struct E {	/* -E<te|D|>file> */
		bool active;
		double te;
		char *file;
	} E;
	struct F {	/* -F<force> */
		bool active;
		double force;
	} F;
	struct L {	/* Use variable restoring force [constant] */
		bool active;
	} L;
	struct M {	/* -Mx|z  */
		bool active[2];	/* True if km, else m */
	} M;
	struct Q {	/* Load specifier -Qn|q|t[/args] */
		bool active;
		bool set_x;
		unsigned int mode;
		struct GMT_ARRAY T;
		char *file;
	} Q;
	struct S {	/* Compute second derivatives (curvatures) */
		bool active;
	} S;
	struct T {	/* Pre-existing deformation */
		bool active;
		char *file;
	} T;
	struct W {	/* Water depth */
		bool active;
		double water_depth;	/* Reference water depth [0] */
	} W;
	struct Z {	/* Moho depth */
		bool active;
		double zm;	/* Reference depth to flexed surface [0] */
	} Z;
};

enum gmtflexure_side {
	LEFT = 0,
	RIGHT};

enum gmtflexure_load {
	NO_LOAD = 0,
	F_LOAD,
	T_LOAD};

enum gmtflexure_bc {
	BC_INFINITY=0,
	BC_PERIODIC,
	BC_CLAMPED,
	BC_FREE};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTFLEXURE_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTFLEXURE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.E = YOUNGS_MODULUS;
	C->C.nu = POISSONS_RATIO;

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTFLEXURE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->E.file);
	gmt_M_str_free (C->Q.file);
	gmt_M_str_free (C->T.file);
	gmt_free_array (GMT, &(C->Q.T));
	gmt_M_free (GMT, C);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTFLEXURE_CTRL *Ctrl, struct GMT_OPTION *options) {

	unsigned int side, k, n_errors = 0, n_files = 0;
	int n;
	bool both;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
		switch (opt->option) {

			case '>':	/* Got named output file */
				if (n_files++ == 0 && gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'A':	/* Boundary conditions -A[l|r]<bc>[/<w>|<m>/<f>]*/
				Ctrl->A.active = true;
				both = false;	side = 0;
				switch (opt->arg[0]) {
					case 'l': case 'L':	side = LEFT; break;
					case 'r': case 'R':	side = RIGHT; break;
					default:		both = true; break;
				}
				k = (both) ? 0 : 1;	/* Offset to <bc> argument */
				n = atoi (&opt->arg[k]);
				if (n < BC_INFINITY || n > BC_FREE) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: <bc> must be in 1-4 range\n");
					n_errors++;
					break;
				}
				Ctrl->A.bc[side] = (unsigned int)n;
				if (Ctrl->A.bc[side] == BC_CLAMPED)	/* Get clamped deflection */
					Ctrl->A.deflection[side] = (opt->arg[k+2]) ? atof (&opt->arg[k+2]) : 0.0;
				else if (Ctrl->A.bc[side] == BC_FREE) {	/* Get bending moment and shear force */
					if (opt->arg[k+2])
						sscanf (&opt->arg[k+2], "%lf/%lf", &Ctrl->A.moment[side], &Ctrl->A.force[side]);
				}
				if (both) {	/* Copy values over from left to right */
					Ctrl->A.bc[RIGHT] = Ctrl->A.bc[LEFT];
					Ctrl->A.deflection[RIGHT] = Ctrl->A.deflection[LEFT];
					Ctrl->A.moment[RIGHT] = Ctrl->A.moment[LEFT];
					Ctrl->A.force[RIGHT] = Ctrl->A.force[LEFT];
				}
				break;
			case 'C':	/* Rheology constants E and nu */
				switch (opt->arg[0]) {
					case 'p': Ctrl->C.nu = atof (&opt->arg[1]); break;
					case 'y': Ctrl->C.E = atof (&opt->arg[1]); break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Unrecognized modifier %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'D':	/* Set densities */
				Ctrl->D.active = true;
				n = sscanf (opt->arg, "%lf/%lf/%lf/%lf", &Ctrl->D.rhom, &Ctrl->D.rhol, &Ctrl->D.rhoi, &Ctrl->D.rhow);
				if (!(n == 4 || n == 3)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: must give 3-4 density values\n");
					n_errors++;
				}
				if (n == 3) {	/* Assume no rhoi given, shuffle args */
					Ctrl->D.rhow = Ctrl->D.rhoi;
					Ctrl->D.rhoi = Ctrl->D.rhol;
				}
				break;
			case 'E':	/* Set elastic thickness or rigidities */
				Ctrl->E.active = true;
				if (!gmt_access (GMT, opt->arg, F_OK))	/* file exists */
					Ctrl->E.file = strdup (opt->arg);
				else {	/* Got a value */
					GMT_Get_Values (API, opt->arg, &Ctrl->E.te, 1);	/* Returns Te in m if k was appended */
					if (Ctrl->E.te > 1e10) { /* Given flexural rigidity, compute Te in meters */
						Ctrl->E.te = pow ((12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu)) * Ctrl->E.te / Ctrl->C.E, 1.0/3.0);
					}
				}
				break;
			case 'F':	/* Horizontal end load */
				Ctrl->F.active = true;
				Ctrl->F.force = atof (opt->arg);
				break;
			case 'L':	/* Variable restoring force */
				Ctrl->L.active = true;
				break;
			case 'M':	/* Length units */
				both = false;	side = 0;
				switch (opt->arg[0]) {
					case 'x': side = 0; break;
					case 'z': side = 1; break;
					default:  both = true; break;
				}
				Ctrl->M.active[side] = true;
				if (both) Ctrl->M.active[1] = Ctrl->M.active[0];
				break;
			case 'S':	/* Compute curvatures also */
				Ctrl->S.active = true;
				break;
			case 'T':	/* Preexisting deformation */
				Ctrl->T.active = true;
				Ctrl->T.file = strdup (opt->arg);
				break;
			case 'Q':	/* Load setting -Qn|q|t[/args] */
				Ctrl->Q.active = true;
				switch (opt->arg[0]) {
					case 'n':	Ctrl->Q.mode = NO_LOAD;
						if (opt->arg[1]) {	/* Gave domain info */
							Ctrl->Q.set_x = true;
							n_errors += gmt_parse_array (GMT, 'Q', &opt->arg[1], &(Ctrl->Q.T), GMT_ARRAY_DIST, 0);
						}
						break;
					case 'q':	Ctrl->Q.mode = F_LOAD; break;
					case 't':	Ctrl->Q.mode = T_LOAD; break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Unrecognized mode -Q%c\n", opt->arg[0]);
						n_errors++;
					break;
				}
				if (Ctrl->Q.mode != NO_LOAD && opt->arg[1]) Ctrl->Q.file = strdup (&opt->arg[1]);
				break;
			case 'W':	/* Water depth */
				Ctrl->W.active = true;
				GMT_Get_Values (API, opt->arg, &Ctrl->W.water_depth, 1);	/* This yields water depth in meters if k was added */
				break;
			case 'Z':	/* Moho depth */
				Ctrl->Z.active = true;
				GMT_Get_Values (API, opt->arg, &Ctrl->Z.zm, 1);	/* This yields Moho depth in meters if k was added */
				break;
			default:
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += gmt_M_check_condition (GMT, !Ctrl->D.active, "Syntax error -D option: Must set density values\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.active, "Syntax error -E option: Must specify plate thickness or rigidity\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->Q.active, "Syntax error -Q option: Must specify load option\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.file && Ctrl->Q.mode == NO_LOAD && !Ctrl->Q.set_x, "Syntax error -Q option: Must specify equidistant min/max/inc setting\n");
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s -D<rhom>/<rhol>[/<rhoi>]/<rhow> -E<te> -Q<loadinfo> [-A[l|r]<bc>[/<args>]]\n", name);
	GMT_Message (API, GMT_TIME_NONE,"\t[-C[p|y]<value] [-F<force>] [-L] [-S] [-T<wpre>] [%s] [-W<w0>] [-Z<zm>]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_e_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-D Sets density values for mantle, load(crust), optional moat infill [same as load], and water in kg/m^3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Sets elastic plate thickness in m; append k for km.  If Te > 1e10 it will be interpreted\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   as the flexural rigidity [Default computes D from Te, Young's modulus, and Poisson's ratio].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <te> can be opened as a file it is expected to hold elastic thicknesses at each load location.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Input load option. Choose among these options:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qn indicates there is no load (only -A and -L contribute to deformation).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      If no file is given via -E<file> then append <min/max/inc> to set an equidistant profile.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +n to inc to indicate the number of points instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qq[<load>] is a file (or stdin) with (x,load in Pa) for all points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qt[<load>] is a file (or stdin) with (x,load in m or km) for all points (see -M).\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Al and -Ar specify boundary conditions at the left and right end, respectively.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Four types of BC's are recognized (here, w = w(x) = the deflection):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Al0 or -Ar0 :         \"Infinity\" condition, w' = w'' = 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Al1 or -Ar1 :         \"Periodic\" condition, w' = w''' = 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Al2/w0 or -Ar2/w0 :   \"Clamped\", w at end = w0 [0], w' = 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Al3/m/f or -Ar3/m/f : \"Free\" condition, specify (m)oment and (f)orce at end [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default BCs are -Al0 -Ar0.  Use SI units for any optional arguments.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use -Cy<Young> or -Cp<poisson> to change Young's modulus [%g] or Poisson's ratio [%g].\n", YOUNGS_MODULUS, POISSONS_RATIO);
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specifies the uniform, horizontal stress in the plate [Pa m].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Use variable restoring force k(x) that depends on w(x).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Sets units used, as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mx indicates all x-distances are in km [meters]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mz indicates all z-deflections are in km [meters]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Also compute second derivatives (curvatures) on output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T To use file <wpre> with pre-existing deflections [none].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Specify water depth in m; append k for km.  Must be positive.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Subaerial topography will be scaled by -D to account for density differences.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Specify reference depth to flexed surface in m; append k for km.  Must be positive [0].\n");
	GMT_Option (API, "d,e,h,i,o,.");
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL double te_2_d (struct GMTFLEXURE_CTRL *Ctrl, double te) {
	/* Convert elastic thickness to flexural rigidity */
	return (Ctrl->C.E * pow (te, 3.0) / (12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu)));
}

GMT_LOCAL int get_curvature (double flexure[], int n, double dist_increment, double curvature[]) {
	/* Calculate central second differences of flexure = curvature */
	int i;

	dist_increment = -1.0/(dist_increment*dist_increment);	/* - since z points down */
	n--;
	for (i = 1; i < n; i++)
		curvature[i] = (flexure[i-1] - 2.0*flexure[i] + flexure[i+1])*dist_increment;
	curvature[0] = curvature[1];
	curvature[n] = curvature[n-1];
	return (1);
}

GMT_LOCAL int lu_solver (struct GMT_CTRL *GMT, double *a, int n, double *x, double *b) {
	/* A 5-diagonal matrix problem A*w = p solved using a LU-transformation */

	int i, off3, off5;
	double new_max, old_max, *l = NULL, *u = NULL, *z = NULL;

	if (n < 4) {
		fprintf (stderr, "lu_solver: n < 4!\n");
		return (1);
	}

	l = gmt_M_memory (GMT, NULL, n * 5, double);
	u = gmt_M_memory (GMT, NULL, n * 5, double);
	z = gmt_M_memory (GMT, NULL, n, double);

	/* Find largest element in coefficient matrix */

	old_max = 1.0;
	for (i = 0; i < n*5; i++) if ((new_max = fabs(a[i])) > old_max) old_max = new_max;

	/* Normalize by old_max */

	old_max = 1.0 / old_max;
	for (i = 0; i < n*5; i++) a[i] *= old_max;

	for (i = 0; i < n; i++) b[i] *= old_max;

	/* Start decomposition of a to l * u */
	/* Row 1 */

	u[0] = a[2];
	u[1] = a[3];
	u[2] = a[4];

	l[2] = 1.0;

	/* Row 2 */

	l[4] = a[6] / u[0];
	l[5] = 1.0;

	u[3] = a[7] - l[4] * u[1];
	u[4] = a[8] - l[4] * u[2];
	u[5] = a[9];

	/* Row 3 to n-2 */

	for (i = 2, off3 = 6, off5 = 10; i < (n-2); i++, off3 += 3, off5 += 5) {
		l[off3] = a[off5] / u[off3-6];
		l[off3+1] = (a[off5+1] - l[off3] * u[off3-5]) / u[off3-3];
		l[off3+2] = 1.0;

		u[off3] = a[off5+2] - l[off3] * u[off3-4] - l[off3+1] * u[off3-2];
		u[off3+1] = a[off5+3] - l[off3+1] * u[off3-1];
		u[off3+2] = a[off5+4];
	}

	/* Row n-1 */

	l[off3] = a[off5] / u[off3-6];
	l[off3+1] = (a[off5+1] - l[off3] * u[off3-5]) / u[off3-3];
	l[off3+2] = 1.0;
	u[off3] = a[off5+2] - l[off3] * u[off3-4] - l[off3+1] * u[off3-2];
	u[off3+1] = a[off5+3] - l[off3+1] * u[off3-1];

	/* Row n */

	off3 += 3;
	off5 += 5;
	l[off3] = a[off5] / u[off3-6];
	l[off3+1] = (a[off5+1] - l[off3] * u[off3-5]) / u[off3-3];
	l[off3+2] = 1.0;
	u[off3] = a[off5+2] - l[off3] * u[off3-4] - l[off3+1] * u[off3-2];

	/* Then solve l * z = b by forward substitution */

	z[0] = b[0];
	z[1] = b[1] - z[0] * l[4];

	for (i = 2, off3 = 6; i < n; i++, off3 += 3)
		z[i] = b[i] - z[i-1] * l[off3+1] - z[i-2] * l[off3];

	/* Finally solve u * x = z by backward substitution */

	x[n-1] = z[n-1] / u[(n-1)*3];
	x[n-2] = (z[n-2] - x[n-1] * u[(n-2)*3+1]) / u[(n-2)*3];

	for (i = n-3, off3 = i*3; i >= 0; i--, off3 -= 3)
		x[i] = (z[i] - x[i+1] * u[off3+1] - x[i+2] * u[off3+2]) / u[off3];

	gmt_M_free (GMT, u);
	gmt_M_free (GMT, l);
	gmt_M_free (GMT, z);

	return (0);
}

/* flx1d will compute 1-D plate flexure for a variable rigidity case.
 * The equation is
 *	d2/dx2 (D * d2/dx2 w(x)) + T * d2/dx2 w(x) + k(x) * w(x) = p (x)
 * Various boundary conditions may be imposed by setting the
 * variables bc_left and bc_right to on of the permissible values:
 * 	0:	'infinity' condition. w' = w'' = 0.
 *	1:	'periodic'. w' = w''' = 0. (Reflection)
 *	2:	'clamped'. w = const. w' = 0. The value of w must be
 *		passed in w[i] on input, where i is 0/(n-1) for the
 *		left/right edge.
 *	3:	'free'. Moment = const, Force = const. Store M in w[i] and F in
 *		w[i+1], where i is 0/(n-2) for left/right edge.
 * The deflections are solved by forward/backward substitution to solve
 * the 5-diagonal matrix problem A*w = p using a LU-transformation (lu_solver)
 * The parameters passed are:
 *	w	: Name of array holding flexure (output)
 *	d	: Name of array holding rigidities (input)
 *	p	: Name of array holding pressures/loads (input)
 *	n	: Number of points in profile (input)
 *	dx	: Distance between points (input)
 *	k	: Restoring force term k(x) = delta_rho (x) * gravity (input)
 *	k_flag	: 0 means k[0] applies for entire profile, 1 means k[x] is an array
 *	stress	: Horisontal stress T in the plate (input). Positive = compression.
 *	bc_left : value 0 - 3. See above (input)
 *	bc_right: value 0 - 3. See above (input)
 *
 * Author:	Paul Wessel
 * Date:	5-SEP-1988
 * Revised:	5-AUG-1989	Now k is a function of x
 *
 */

GMT_LOCAL int flx1d (struct GMT_CTRL *GMT, double *w, double *d, double *p, int n, double dx, double *k, int k_flag, double stress, int bc_left, int bc_right) {
	int i, row, off, ind, error;
	double dx_4, c_1 = 0.0, c_2 = 0.0, stress2, restore, *work = NULL;

	/* Must allocate memory */

	work = gmt_M_memory (GMT, NULL, 5 * n, double);

	dx_4 = pow (dx, 4.0);
	stress *= (dx * dx);
	stress2 = 2.0 * stress;

	for (i = 0; i < n; i++) p[i] *= dx_4;

	/* Matrix row 0: */

	work[0] = 0.0;
	work[1] = 0.0;
	restore = k[0] * dx_4;
	if (bc_left == 0) {		/* 'infinity' conditions */
		work[2] = 1.0;
		work[3] = work[4] = 0.0;
		p[0] = 0.0;
	}
	else if (bc_left == 1) {	/* Periodic BC */
		work[2] = 10.0 * d[0] - 4.0 * d[1] + restore - stress2;
		work[3] = 4.0 * d[1] - 12.0 * d[0] + stress2;
		work[4] = 2.0 * d[0];
	}
	else if (bc_left == 2) {	/* 'clamped' plate */
		work[2] = 1.0;
		work[3] = work[4] = 0.0;
		p[0] = w[0];
	}
	else {				/* Free end */
		work[2] = 2.0 * d[0] + restore - stress2;
		work[3] = -4.0 * d[0] + stress2;
		work[4] = 2.0 * d[0];
		c_1 = -w[0] * dx * dx / d[0];
		c_2 =  - 2.0 * pow (dx, 3.0) * w[1];
		p[0] -= c_1*(2.0*d[1] - 4.0*d[0]) + c_2;
	}

	/* Matrix row 1: */

	ind = (k_flag) ? 1 : 0;
	restore = k[ind] * dx_4;
	work[5] = 0.0;
	if (bc_left == 0) {	/* 'infinity' */
		work[6] = 1.0;
		work[7] = -1.0;
		work[8] = work[9] = 0.0;
		p[1] = 0.0;
	}
	else if (bc_left == 1) {	/* Periodic */
		work[6] = 2.0 * d[2] - 6.0 * d[1] + stress;
		work[7] = 11.0 * d[1] - 1.5 * d[0] - 2.5 * d[2] + restore - stress2;
		work[8] = 2.0 * d[0] - 6.0 * d[1] + stress;
		work[9] = d[1] + 0.5 * d[2] - 0.5 * d[0];
	}
	else if (bc_left == 2) {	/* 'clamped' */
		work[6] = 2.0 * d[2] - 6.0 * d[1] + stress;
		work[7] = 11.0 * d[1] - 1.5 * d[0] - 2.5 * d[2] + restore - stress2;
		work[8] = 2.0 * d[0] - 6.0 * d[1] + stress;
		work[9] = d[1] + 0.5 * d[2] - 0.5 * d[0];
		w[0] = 0.0;
	}
	else {				/* Free end */
		work[6] = d[0] - 4.0 * d[1] + d[2] + stress;
		work[7] = 9.0 * d[1] - 1.5 * d[2] - 2.5 * d[0] + restore - stress2;
		work[8] = 2.0 * d[0] - 6.0 * d[1] + stress;
		work[9] = d[1] + 0.5 * d[2] - 0.5 * d[0];
		p[1] -= c_1 * (d[1] + 0.5 * d[2] - 0.5 * d[0]);
		w[0] = w[1] = 0.0;
	}

	/* Matrix row 2 -> n - 3: */

	for (row = 2; row < (n-2); row++) {
		ind = (k_flag) ? row : 0;
		restore = k[ind] * dx_4;
		off = row * 5;
		work[off]   = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 10.0 * d[row] - 2.0 * d[row+1] - 2.0 * d[row-1] + restore - stress2;
		work[off+3] = 2.0 * d[row-1] - 6.0 * d[row] + stress;
		work[off+4] = d[row] + 0.5 * d[row+1] - 0.5 * d[row-1];
	}

	/* Matrix row n - 2: */

	row = n - 2;
	off = row * 5;
	ind = (k_flag) ? row : 0;
	restore = k[ind] * dx_4;
	work[off+4] = 0.0;
	if (bc_right == 0) {		/* 'infinity' */
		work[off] = work[off+1] = 0.0;
		work[off+2] = -1.0;
		work[off+3] = 1.0;
		p[row] = 0.0;
	}
	else if (bc_right == 1) {	/* Periodic */
		work[off]   = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 11.0 * d[row] - 2.5 * d[row-1] - 1.5 * d[row+1] + restore - stress2;
		work[off+3] = 2.0 * d[row-1] - 6.0 * d[row] + stress;
	}
	else if (bc_right == 2) {	/* 'clamped' */
		work[off] = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 11.0 * d[row] - 2.5 * d[row-1] - 1.5 * d[row+1] + restore - stress2;
		work[off+3] = 2.0 * d[row-1] - 6.0 * d[row] + stress;
	}
	else {				/* Free end */
		c_1 = -w[row] * dx * dx / d[n-1];
		work[off] = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 9.0 * d[row] - 2.5 * d[row+1] - 1.5 * d[row-1] + restore - stress2;
		work[off+3] = d[row-1] + d[row+1] - 4.0 * d[row] + stress;
		p[row] -= c_1*(0.5*d[row+1] + d[row] - 0.5*d[row-1]);
	}

	/* Matrix row nx - 1: */

	off += 5;
	row++;
	ind = (k_flag) ? row : 0;
	restore = k[ind] * dx_4;
	work[off+3] = work[off+4] = 0.0;
	if (bc_right == 0) {		/* 'infinity' */
		work[off] = work[off+1] = 0.0;
		work[off+2] = 1.0;
		p[row] = 0.0;
	}
	else if (bc_right == 1) {	/* Periodic */
		work[off]   = 2.0 * d[row];
		work[off+1] = 4.0 * d[row-1] - 12.0 * d[row] + stress2;
		work[off+2] = 10.0 * d[row] -4.0 *d[row-1] + restore - stress2;
	}
	else if (bc_right == 2) {	/* 'clamped' */
		work[off] = work[off+1] = 0.0;
		work[off+2] = 1.0;
		p[row] = w[row];
		w[row] = 0.0;
	}
	else {				/* Free end */
		c_2 =  - 2.0 * pow (dx, 3.0) * w[row];
		work[off]   = 2.0 * d[row];
		work[off+1] = -4.0 * d[row] + stress2;
		work[off+2] = 2.0 * d[row] + restore - stress2;
		p[row] -= c_1*(2.0*d[row-1] - 4.0*d[row]) + c_2;
		w[row-1] = w[row] = 0.0;
	}


	/* Solve for w */

	error = lu_solver (GMT, work, n, w, p);
	gmt_M_free (GMT, work);
	if (error == 1) {
		fprintf (stderr, "flx1d: error=1 returned from lu_solver!\n");
		return (error);
	}
	return (0);
}

/* flx1dk will compute 1-D plate flexure for a variable rigidity case with
 * a restoring force that depends on the sign of the deflection.  After each
 * iteration, we recompute k(x) so that k(x) = G * (rho_m - rho_infill (or rho_load)) where
 * deflections are positive (i.e. down), whereas k(x) = G * (rho_m - rho_w)
 * at the bulges.  We iterate until rms of difference is < LIMIT (1 cm)
 * The equation we solve is
 *
 *	d2/dx2 (D * d2/dx2 w(x)) + T * d2/dx2 w(x) + k(x) * w(x) = p (x)
 *
 * Various boundary conditions may be imposed by setting the
 * variables bc_left and bc_right to on of the permissible values:
 * 	0:	'infinity' condition. w' = w'' = 0.
 *	1:	'periodic'. w' = w''' = 0. (Reflection)
 *	2:	'clamped'. w = const. w' = 0. The value of w must be
 *		passed in w[i] on input, where i is 0/(n-1) for the
 *		left/right edge.
 *	3:	'free'. Moment = const, Force = const. Store M in w[i] and F in
 *		w[i+1], where i is 0/(n-2) for left/right edge.
 * The deflections are solved by forward/backward substitution to solve
 * the 5-diagonal matrix problem A*w = p using a LU-transformation (lu_solver)
 * The parameters passed are:
 *	w	: Name of array holding flexure (output)
 *	d	: Name of array holding rigidities (input)
 *	p	: Name of array holding pressures/loads (input)
 *	n	: Number of points in profile (input)
 *	dx	: Distance between points (input)
 *	rho_m	: mantle_density [3300]
 *	rho_l	: load density [2800]
 *	rho_i	: infill density [2300]
 *	rho_w	: water density [1000]
 *	stress	: Horisontal stress T in the plate (input). Positive = compression.
 *	bc_left : value 0 - 3. See above (input)
 *	bc_right: value 0 - 3. See above (input)
 *
 * Author:	Paul Wessel
 * Date:	5-SEP-1988
 * Revised:	5-AUG-1989	Now k is a function of x
 *
 */

#define LIMIT	0.01

GMT_LOCAL int flx1dk (struct GMT_CTRL *GMT, double w[], double d[], double p[], int n, double dx, double rho_m, double rho_l, double rho_i, double rho_w, double stress, int bc_left, int bc_right) {
	int i, error;
	double restore_a, restore_b1, restore_b2, diff = 2 * LIMIT, dw, w0, w1, wn1, wn, max_dw;
	double *w_old = NULL, *k = NULL, *load = NULL;

	/* Allocate memory for load and restore force */

	k = gmt_M_memory (GMT, NULL, n, double);
	load = gmt_M_memory (GMT, NULL, n, double);

	/* Initialize restoring force */

	restore_a  = NORMAL_GRAVITY * (rho_m - rho_w);
	restore_b1 = NORMAL_GRAVITY * (rho_m - rho_i);
	restore_b2 = NORMAL_GRAVITY * (rho_m - rho_l);

	for (i = 0; i < n; i++)	k[i] = (p[i] > 0.0) ? restore_b2 : restore_b1;

	/* Save possible boundary values */

	w0 = w[0];	w1 = w[1];	wn1 = w[n-2];	wn = w[n-1];

	gmt_M_memcpy (load, p, n, double);

	if ((error = flx1d (GMT, w, d, load, n, dx, k, 1, stress, bc_left, bc_right))) {
		gmt_M_free (GMT, load);		gmt_M_free (GMT, k);
		return (error);
	}

	w_old = gmt_M_memory (GMT, NULL, n, double);

	while (!error && diff > LIMIT) {	/* Iterate as long as rms difference is > LIMIT. diff starts at 2*LIMIT. */

		/* Set variable restoring force */

		for (i = 0; i < n; i++)	k[i] = (w[i] > 0.0) ? ((p[i] > 0.0) ? restore_b2 : restore_b1) : restore_a;

		/* Save previous solution */

		gmt_M_memcpy (w_old, w, n, double);

		/* Initialize arrays again */

		gmt_M_memcpy (load, p, n, double);
		gmt_M_memset (w, n, double);
		w[0] = w0;	w[1] = w1;	w[n-2] = wn1;	w[n-1] = wn;	/* Reset BC values */

		error = flx1d (GMT, w, d, load, n, dx, k, 1, stress, bc_left, bc_right);

		for (i = 0, diff = max_dw = 0.0; i < n; i++) {
			dw = fabs (w[i] - w_old[i]);
			if (dw > max_dw) max_dw = dw;
			/* diff += dw * dw; */
		}
		/* diff = sqrt (diff / n); RMS */
		diff = max_dw;
	}

	gmt_M_free (GMT, k);
	gmt_M_free (GMT, load);
	gmt_M_free (GMT, w_old);

	return (error);
}

/* flx1dw0 will compute 1-D plate flexure for a variable rigidity case with
 * a pre-existing deformation.  The equation is
 *
 *	d2/dx2 (D * d2/dx2 w(x)) + T * d2/dx2 [w(x) + w0(x)] + k(x) * w(x) = p (x)
 *
 * Various boundary conditions may be imposed by setting the
 * variables bc_left and bc_right to on of the permissible values:
 * 	0:	'infinity' condition. w' = w'' = 0.
 *	1:	'periodic'. w' = w''' = 0. (Reflection)
 *	2:	'clamped'. w = const. w' = 0. The value of w must be
 *		passed in w[i] on input, where i is 0/(n-1) for the
 *		left/right edge.
 *	3:	'free'. Moment = const, Force = const. Store M in w[i] and F in
 *		w[i+1], where i is 0/(n-2) for left/right edge.
 * The deflections are solved by forward/backward substitution to solve
 * the 5-diagonal matrix problem A*w = p using a LU-transformation (lu_solver)
 * The parameters passed are:
 *	w0	: Name of array holding pre-existing flexure (input)
 *	w	: Name of array holding flexure (output)
 *	d	: Name of array holding rigidities (input)
 *	p	: Name of array holding pressures/loads (input)
 *	n	: Number of points in profile (input)
 *	dx	: Distance between points (input)
 *	k	: Restoring force term k(x) = delta_rho (x) * gravity (input)
 *	k_flag	: 0 means k[0] applies for entire profile, 1 means k[x] is an array
 *	stress	: Horisontal stress T in the plate (input). Positive = compression.
 *	bc_left : value 0 - 3. See above (input)
 *	bc_right: value 0 - 3. See above (input)
 *
 * Author:	Paul Wessel
 * Date:	5-SEP-1988
 * Revised:	5-AUG-1989	Now k is a function of x
 * 		29-OCT-1990	Include pre-existing deformation
 *
 */

GMT_LOCAL int flx1dw0 (struct GMT_CTRL *GMT, double *w, double *w0, double *d, double *p, int n, double dx, double *k, int k_flag, double stress, int bc_left, int bc_right) {
	int i, row, off, ind, error;
	double dx_4, c_1 = 0.0, c_2, stress2, restore, *work = NULL, *squeeze = NULL;

	/* Must allocate memory */

	work = gmt_M_memory (GMT, NULL, 5 * n, double);
	squeeze = gmt_M_memory (GMT, NULL, n, double);

	dx_4 = pow (dx, 4.0);
	stress *= (dx * dx);
	stress2 = 2.0 * stress;

	for (i = 0; i < n; i++) p[i] *= dx_4;	/* Scale load */

	/* Add in buckling force on preexisting topo */

	for (i = 1; i < n-1; i++) squeeze[i] = -stress * (w0[i+1] - 2.0 * w[i] - w[i-1]);
	squeeze[0] = squeeze[1];	squeeze[n-1] = squeeze[n-2];
	for (i = 0; i < n; i++) p[i] -= squeeze[i];

	/* Matrix row 0: */

	work[0] = work[1] = 0.0;
	restore = k[0] * dx_4;
	if (bc_left == 0) {		/* 'infinity' conditions */
		work[2] = 10.0 * d[0] - 4.0 * d[1] + restore - stress2;
		work[3] = 2.0 * d[1] - 6.0 * d[0] + stress2;
		work[4] = d[0];
	}
	else if (bc_left == 1) {	/* Periodic BC */
		work[2] = 10.0 * d[0] - 4.0 * d[1] + restore - stress2;
		work[3] = 4.0 * d[1] - 12.0 * d[0] + stress2;
		work[4] = 2.0 * d[0];
	}
	else if (bc_left == 2) {	/* 'clamped' plate */
		work[2] = 1.0;
		work[3] = work[4] = 0.0;
		p[0] = w[0];
	}
	else {				/* Free end */
		work[2] = 2.0 * d[0] + restore - stress2;
		work[3] = -4.0 * d[0] + stress2;
		work[4] = 2.0 * d[0];
		c_1 = -w[0] * dx * dx / d[0];
		c_2 =  - 2.0 * pow (dx, 3.0) * w[1];
		p[0] -= c_1*(2.0*d[1] - 4.0*d[0]) + c_2;
	}

	/* Matrix row 1: */

	ind = (k_flag) ? 1 : 0;
	restore = k[ind] * dx_4;
	work[5] = 0.0;
	if (bc_left == 0) {	/* 'infinity' */
		work[6] = 2.0 * d[2] - 6.0 * d[1] + stress;
		work[7] = 10.0 * d[1] - 2.0 * d[2] - 2.0 * d[0] + restore - stress2;
		work[8] = 2.0 * d[0] - 6.0 * d[1] + stress;
		work[9] = d[1] + 0.5 * d[2] - 0.5 * d[0];
	}
	else if (bc_left == 1) {	/* Periodic */
		work[6] = 2.0 * d[2] - 6.0 * d[1] + stress;
		work[7] = 11.0 * d[1] - 1.5 * d[0] - 2.5 * d[2] + restore - stress2;
		work[8] = 2.0 * d[0] - 6.0 * d[1] + stress;
		work[9] = d[1] + 0.5 * d[2] - 0.5 * d[0];
	}
	else if (bc_left == 2) {	/* 'clamped' */
		work[6] = 2.0 * d[2] - 6.0 * d[1] + stress;
		work[7] = 11.0 * d[1] - 1.5 * d[0] - 2.5 * d[2] + restore - stress2;
		work[8] = 2.0 * d[0] - 6.0 * d[1] + stress;
		work[9] = d[1] + 0.5 * d[2] - 0.5 * d[0];
		w[0] = 0.0;
	}
	else {				/* Free end */
		work[6] = d[0] - 4.0 * d[1] + d[2] + stress;
		work[7] = 9.0 * d[1] - 1.5 * d[2] - 2.5 * d[0] + restore - stress2;
		work[8] = 2.0 * d[0] - 6.0 * d[1] + stress;
		work[9] = d[1] + 0.5 * d[2] - 0.5 * d[0];
		p[1] -= c_1 * (d[1] + 0.5 * d[2] - 0.5 * d[0]);
		w[0] = w[1] = 0.0;
	}

	/* Matrix row 2 -> n - 3: */

	for (row = 2; row < (n-2); row++) {
		ind = (k_flag) ? row : 0;
		restore = k[ind] * dx_4;
		off = row * 5;
		work[off]   = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 10.0 * d[row] - 2.0 * d[row+1] - 2.0 * d[row-1] + restore - stress2;
		work[off+3] = 2.0 * d[row-1] - 6.0 * d[row] + stress;
		work[off+4] = d[row] + 0.5 * d[row+1] - 0.5 * d[row-1];
	}

	/* Matrix row n - 2: */

	row = n - 2;
	off = row * 5;
	ind = (k_flag) ? row : 0;
	restore = k[ind] * dx_4;
	work[off+4] = 0.0;
	if (bc_right == 0) {		/* 'infinity' */
		work[off] = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 10.0 * d[row] - 2.0 * d[row-1] - 2.0 * d[row+1] + restore - stress2;
		work[off+3] = 2.0 * d[row-1] - 6.0 * d[row] + stress;
	}
	else if (bc_right == 1) {	/* Periodic */
		work[off]   = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 11.0 * d[row] - 2.5 * d[row-1] - 1.5 * d[row+1] + restore - stress2;
		work[off+3] = 2.0 * d[row-1] - 6.0 * d[row] + stress;
	}
	else if (bc_right == 2) {	/* 'clamped' */
		work[off] = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 11.0 * d[row] - 2.5 * d[row-1] - 1.5 * d[row+1] + restore - stress2;
		work[off+3] = 2.0 * d[row-1] - 6.0 * d[row] + stress;
	}
	else {				/* Free end */
		c_1 = -w[row] * dx * dx / d[n-1];
		work[off] = d[row] + 0.5 * d[row-1] - 0.5 * d[row+1];
		work[off+1] = 2.0 * d[row+1] - 6.0 * d[row] + stress;
		work[off+2] = 9.0 * d[row] - 2.5 * d[row+1] - 1.5 * d[row-1] + restore - stress2;
		work[off+3] = d[row-1] + d[row+1] - 4.0 * d[row] + stress;
		p[row] -= c_1*(0.5*d[row+1] + d[row] - 0.5*d[row-1]);
	}

	/* Matrix row nx - 1: */

	off += 5;
	row++;
	ind = (k_flag) ? row : 0;
	restore = k[ind] * dx_4;
	work[off+3] = work[off+4] = 0.0;
	if (bc_right == 0) {		/* 'infinity' */
		work[off] = d[row];
		work[off+1] = 2.0 *d[row-1] - 6.0 * d[row] + stress2;
		work[off+2] = 10.0 * d[row] - 4.0 * d[row-1] + restore - stress2;
	}
	else if (bc_right == 1) {	/* Periodic */
		work[off]   = 2.0 * d[row];
		work[off+1] = 4.0 * d[row-1] - 12.0 * d[row] + stress2;
		work[off+2] = 10.0 * d[row] -4.0 *d[row-1] + restore - stress2;
	}
	else if (bc_right == 2) {	/* 'clamped' */
		work[off] = work[off+1] = 0.0;
		work[off+2] = 1.0;
		p[row] = w[row];
		w[row] = 0.0;
	}
	else {				/* Free end */
		c_2 =  - 2.0 * pow (dx, 3.0) * w[row];
		work[off]   = 2.0 * d[row];
		work[off+1] = -4.0 * d[row] + stress2;
		work[off+2] = 2.0 * d[row] + restore - stress2;
		p[row] -= c_1*(2.0*d[row-1] - 4.0*d[row]) + c_2;
		w[row-1] = w[row] = 0.0;
	}


	/* Solve for w */

	error = lu_solver (GMT, work, n, w, p);
	gmt_M_free (GMT, work);
	gmt_M_free (GMT, squeeze);
	if (error == 1) {
		fprintf (stderr, "flx1d: error=1 returned from lu_solver!\n");
		return (error);
	}
	/* for (i = 0; i < n; i++) w[i] += w0[i]; */
	return (0);
}

/* flxr will compute a cross-section of 3-D plate flexure for a variable rigidity
 * plate with an axially symmetric load applied. In this case we have no phi-
 * dependence, only radial, so a 1-D solution can be used;
 * The equation is
 *	d2/dr2 (Dr * d2/dr2 w(r)) + k * w(r) = p (r)
 * BC's at r = Inf are w(r) = w'(r) = 0
 *
 * Finite difference equation taken from J. Bodine's Technical Report.
 * The deflections are solved by forward/backward substitution to solve
 * the 5-diagonal matrix problem A*w = p using a LU-transformation (lu_solver)
 * The parameters passed are:
 *	w	: Name of array holding flexure (output)
 *	d	: Name of array holding rigidities (input)
 *	p	: Name of array holding pressures/loads (input)
 *	n	: Number of points in profile (input)
 *	dx	: Distance between points (input)
 *	restore	: Restoring force term ( = delta_rho * gravity ) (input)
 *	work	: name of work array. Must have at least n * 12 elements
 *
 * Author:	Paul Wessel
 * Date:	8-OCT-1988
 *
 * ONLY APPLIES IF RHO_L == RHO_I
 */

#if 0
GMT_LOCAL int flxr (struct GMT_CTRL *GMT, double *w, double *d, double *p, int n, double dx, double restore) {
	int i, row, off, error;
	double dx_4, r2m1, r2p1, rp1, rm1, r4 = 0.0, r, *work = NULL;

	work = gmt_M_memory (GMT, NULL, n * 5, double);
	dx_4 = pow (dx, 4.0);
	restore *= dx_4;

	for (i = 0; i < n; i++) p[i] *= dx_4;

	/* Matrix row 0: */

	work[0] = 0.0;
	work[1] = 0.0;
	work[2] = 16.0 * d[0] + 2.0 * d[1] + restore;
	work[3] = -8.0 * d[1] - 16.0 * d[0];
	work[4] = 6.0 * d[1];

	/* Matrix row 1: */

	work[5] = 0.0;
	work[6] = -2.0 * d[0] - d[1];
	work[7] = 4.0 * d[1] + 1.125 * d[2] + 2.0 * d[0] + restore;
	work[8] = -3.0 * (d[2] + d[1]);
	work[9] = 1.875 * d[2];

	/* Matrix row 2 -> n - 3: */

	for (row = 2; row < (n-2); row++) {
		r = row;
		off = row * 5;
		r2m1 = 2.0*r - 1.0;
		r2p1 = 2.0*r + 1.0;
		rm1 = r - 1.0;
		rp1 = r + 1.0;
		r4 = 4.0 * r;
		work[off]   = r2m1 * (2.0*r - 3.0) * d[row-1] / (r4 * rm1);
		work[off+1] = -r2m1 * (d[row-1] + d[row]) / r;
		work[off+2] = r2p1 * r2p1 * d[row+1] / (r4 * rp1) + 4.0 * d[row] + r2m1 * r2m1 * d[row-1] / (r4 * rm1) + restore;
		work[off+3] = -r2p1 * (d[row+1] + d[row]) / r;
		work[off+4] = (2.0*r + 3.0) * r2p1 * d[row+1] / (r4 * rp1);
	}

	/* Matrix row n - 2: */

	row = n - 2;
	off = row * 5;
	r = row;
	r2m1 = 2.0*r - 1.0;
	r2p1 = 2.0*r + 1.0;
	rm1 = r - 1.0;
	rp1 = r + 1.0;
	work[off+4] = 0.0;
	work[off]   = r2m1 * (2.0*r - 3.0) * d[row-1] / (r4 * rm1);
	work[off+1] = -r2m1 * (d[row-1] + d[row]) / r;
	work[off+2] = r2p1 * r2p1 * d[row+1] / (r4 * rp1) + 4.0 * d[row] + r2m1 * r2m1 * d[row-1] / (r4 * rm1) + restore;
	work[off+2] += (2.0*r + 3.0) * r2p1 * d[row+1] / (r4 * rp1);
	work[off+3] = -r2p1 * (d[row+1] + d[row]) / r;
	/* Matrix row nx - 1: */

	off += 5;
	row++;
	work[off] = work[off+1] = work[off+3] = work[off+4] = 0.0;
	work[off+2] = 1.0;
	p[row] = 0.0;


	/* Solve for w */
	off = 5*n;
	error = lu_solver (GMT, work, n, w, p);
	gmt_M_free (GMT, work);
	if (error == 1) {
		fprintf(stderr, "flxr: error=1 returned from lu_solver!\n");
		return (error);
	}
	return (0);
}

GMT_LOCAL int flxr2 (struct GMT_CTRL *GMT, double *w, double *d, double *p, int n, double dx, double *restore) {
	int i, row, off, error;
	double dx_4, r2m1, r2p1, rp1, rm1, r4 = 0.0, r, *work = NULL;

	work = gmt_M_memory (GMT, NULL, n * 5, double);
	dx_4 = pow (dx, 4.0);

	for (i = 0; i < n; i++) p[i] *= dx_4;

	/* Matrix row 0: */

	work[0] = 0.0;
	work[1] = 0.0;
	work[2] = 16.0 * d[0] + 2.0 * d[1] + restore[0] * dx_4;
	work[3] = -8.0 * d[1] - 16.0 * d[0];
	work[4] = 6.0 * d[1];

	/* Matrix row 1: */

	work[5] = 0.0;
	work[6] = -2.0 * d[0] - d[1];
	work[7] = 4.0 * d[1] + 1.125 * d[2] + 2.0 * d[0] + restore[1] * dx_4;
	work[8] = -3.0 * (d[2] + d[1]);
	work[9] = 1.875 * d[2];

	/* Matrix row 2 -> n - 3: */

	for (row = 2; row < (n-2); row++) {
		r = row;
		off = row * 5;
		r2m1 = 2.0*r - 1.0;
		r2p1 = 2.0*r + 1.0;
		rm1 = r - 1.0;
		rp1 = r + 1.0;
		r4 = 4.0 * r;
		work[off]   = r2m1 * (2.0*r - 3.0) * d[row-1] / (r4 * rm1);
		work[off+1] = -r2m1 * (d[row-1] + d[row]) / r;
		work[off+2] = r2p1 * r2p1 * d[row+1] / (r4 * rp1) + 4.0 * d[row] + r2m1 * r2m1 * d[row-1] / (r4 * rm1) + restore[row] * dx_4;
		work[off+3] = -r2p1 * (d[row+1] + d[row]) / r;
		work[off+4] = (2.0*r + 3.0) * r2p1 * d[row+1] / (r4 * rp1);
	}

	/* Matrix row n - 2: */

	row = n - 2;
	off = row * 5;
	r = row;
	r2m1 = 2.0*r - 1.0;
	r2p1 = 2.0*r + 1.0;
	rm1 = r - 1.0;
	rp1 = r + 1.0;
	work[off+4] = 0.0;
	work[off]   = r2m1 * (2.0*r - 3.0) * d[row-1] / (r4 * rm1);
	work[off+1] = -r2m1 * (d[row-1] + d[row]) / r;
	work[off+2] = r2p1 * r2p1 * d[row+1] / (r4 * rp1) + 4.0 * d[row] + r2m1 * r2m1 * d[row-1] / (r4 * rm1) + restore[row] * dx_4;
	work[off+2] += (2.0*r + 3.0) * r2p1 * d[row+1] / (r4 * rp1);
	work[off+3] = -r2p1 * (d[row+1] + d[row]) / r;
	/* Matrix row nx - 1: */

	off += 5;
	row++;
	work[off] = work[off+1] = work[off+3] = work[off+4] = 0.0;
	work[off+2] = 1.0;
	p[row] = 0.0;


	/* Solve for w */
	off = 5*n;
	error = lu_solver (GMT, work, n, w, p);
	gmt_M_free (GMT, work);
	if (error == 1) {
		fprintf(stderr, "flxr2: error=1 returned from lu_solver!\n");
		return (error);
	}
	return (0);
}

GMT_LOCAL int flxrk (struct GMT_CTRL *GMT, double w[], double  d[], double  p[], int n, double dx, double rho_m, double rho_l, double rho_i, double rho_w, double rho_i2, double rx) {
	int i, error, i_rx;
	double restore_a, restore_b1, restore_b2, restore_b3, diff = 2 * LIMIT, dw, max_dw;
	double *w_old = NULL, *k = NULL, *load = NULL;

	/* Allocate memory for load and restore force */

	k = gmt_M_memory (GMT, NULL, n, double);
	w_old = gmt_M_memory (GMT, NULL, n, double);
	load = gmt_M_memory (GMT, NULL, n, double);

	/* Initialize restoring force */

	restore_a  = NORMAL_GRAVITY * (rho_m - rho_w);
	restore_b1 = NORMAL_GRAVITY * (rho_m - rho_i);
	restore_b2 = NORMAL_GRAVITY * (rho_m - rho_l);
	restore_b3 = NORMAL_GRAVITY * (rho_m - rho_i2);

	i_rx = (int) rint (rx / dx);

	for (i = 0; i < n; i++)	k[i] = (p[i] > 0.0) ? ((i <= i_rx) ? restore_b3 : restore_b2) : restore_b1;

	memcpy ((void *)load, (void *)p, n * sizeof (double));

	error = flxr2 (GMT, w, d, p, n, dx, k);

	while (!error && diff > LIMIT) {	/* Iterate as long as rms difference is > LIMIT. diff starts at 2*LIMIT. */

		/* Set variable restoring force */

		for (i = 0; i < n; i++)	{
			if (w[i] > 0.0) {	/* Positive depression is down */
				if (i <= i_rx)		/* We are under the heavier core load */
					k[i] = restore_b3;
				else if (p[i] > 0.0)	/* Under the regular, lighter load */
					k[i] = restore_b2;
				else			/* Just sediment infill in the moat */
					k[i] = restore_b1;
			}
			else				/* At the bulges */
				k[i] = restore_a;
		}

		/* Save previous solution */

		memcpy ((void *)w_old, (void *)w, n * sizeof (double));

		/* Initialize arrays again */

		memcpy ((void *)p, (void *)load, n * sizeof (double));
		memset ((void *)w, 0, n * sizeof (double));

		error = flxr2 (GMT, w, d, p, n, dx, k);

		for (i = 0, diff = max_dw = 0.0; i < n; i++) {
			dw = fabs (w[i] - w_old[i]);
			if (dw > max_dw) max_dw = dw;
			/* diff += dw * dw; */
		}
		/* diff = sqrt (diff / n); RMS */
		diff = max_dw;
	}

	gmt_M_free (GMT, k);
	gmt_M_free (GMT, load);
	gmt_M_free (GMT, w_old);

	return (error);
}
#endif

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtflexure (void *V_API, int mode, void *args) {
	uint64_t tbl, seg, row, n_columns;
	int error;
	bool airy;
	char msg[GMT_LEN256] = {""}, txt[GMT_LEN256] = {""};
	double x_inc, restore;
	double *load = NULL, *deflection = NULL, *rigidity = NULL;

	struct GMTFLEXURE_CTRL *Ctrl = NULL;
	struct GMT_DATASET *Q = NULL, *E = NULL, *T = NULL, *W = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtflexure main code ----------------------------*/


	if (Ctrl->Q.mode == NO_LOAD) {	/* No load file given */
		GMT_Report (API, GMT_MSG_VERBOSE, "No load file given; Flexure only determined by boundary conditions\n");
	}
	else {	/* Load file given */
		char *type = (Ctrl->Q.mode == T_LOAD) ? "topography" : "pressure";
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input %s table data\n", type);
		if ((Q = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->Q.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Q->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->Q.file, (int)Q->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		/* If topography given then scale to load */
		if (Ctrl->Q.mode == T_LOAD) {
			uint64_t n_subaerial = 0;
			double scale = (Ctrl->D.rhol - Ctrl->D.rhow) * NORMAL_GRAVITY;	/* Convert load height to pressure */
			double boost = Ctrl->D.rhol / (Ctrl->D.rhol - Ctrl->D.rhow);	/* Boost factor for subarial load */
			for (tbl = 0; tbl < Q->n_tables; tbl++) {
				for (seg = 0; seg < Q->table[tbl]->n_segments; seg++) {
					S = Q->table[tbl]->segment[seg];	/* Current segment */
					for (row = 0; row < S->n_rows; row++) {	/* Covert to pressure */
						if (Ctrl->M.active[1]) S->data[GMT_Y][row] *= 1000;	/* Got topography in km so scale to meters */
						if (Ctrl->W.active && S->data[GMT_Y][row] > Ctrl->W.water_depth) {
							S->data[GMT_Y][row] = (float)(Ctrl->W.water_depth + (S->data[GMT_Y][row] - Ctrl->W.water_depth) * boost);
							n_subaerial++;
						}
						S->data[GMT_Y][row] *= scale;
					}
				}
			}
			if (n_subaerial) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points were subarial so heights were scaled to the equivalent submerged case\n", n_subaerial);
		}
	}
	if (Ctrl->E.file) {	/* Gave file with elastic thicknesses or rigidities */
		double scale = (Ctrl->M.active[1]) ? 1000.0 : 1.0;	/* Either got Te in km or m */
		double d_min = DBL_MAX, d_max = 0.0;
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input Te or Rigidity table data\n");
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -C,-F,-L files */
		if ((E = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->E.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (E->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->E.file, (int)E->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		for (tbl = 0; tbl < E->n_tables; tbl++) {
			for (seg = 0; seg < E->table[tbl]->n_segments; seg++) {
				S = E->table[tbl]->segment[seg];	/* Current segment */
				for (row = 0; row < S->n_rows; row++) {	/* Covert to pressure */
					if (S->data[GMT_Y][row] < 1e10) /* Got elastic thickness, convert to rigidity */
						S->data[GMT_Y][row] = te_2_d (Ctrl, scale * S->data[GMT_Y][row]);
					if (S->data[GMT_Y][row] < d_min) d_min = S->data[GMT_Y][row];
					if (S->data[GMT_Y][row] > d_max) d_max = S->data[GMT_Y][row];
				}
			}
		}
		if (d_min == d_max)
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Constant rigidity: %g \n", d_min);
		else
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Range of rigidities: %g to %g\n", d_min, d_max);
	}
	if (Ctrl->Q.mode == NO_LOAD)	{	/* No load file given */
		if (Ctrl->E.file) {	/* Use info from elastic thickness file instead */
			Q = GMT_Duplicate_Data (API, GMT_IS_DATASET, GMT_DUPLICATE_DATA, E);
			/* Since no load given make sure the load is set to zero */
			for (tbl = 0; tbl < Q->n_tables; tbl++) {
				for (seg = 0; seg < Q->table[tbl]->n_segments; seg++) {
					S = Q->table[tbl]->segment[seg];	/* Current segment */
					gmt_M_memset (S->data[GMT_Y], S->n_rows, double);
				}
			}
		}
		else {	/* No input files given, create single equidistant profile */
			uint64_t dim[GMT_DIM_SIZE] = {1, 1, 0, 2};
			if (gmt_create_array (GMT, 'Q', &(Ctrl->Q.T), NULL, NULL))
				Return (GMT_RUNTIME_ERROR);
			dim[2] = Ctrl->Q.T.n;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create empty load table data\n");
			if ((Q = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				Return (API->error);	/* An empty table */
			}
			S = Q->table[0]->segment[0];	/* Only a single segment here */
			S->n_rows = dim[GMT_ROW];
			gmt_M_memcpy (S->data[GMT_X], Ctrl->Q.T.array, Ctrl->Q.T.n, double);
		}
	}
	if (!Ctrl->E.file) {	/* Got a constant Te in m instead */
		double d = te_2_d (Ctrl, Ctrl->E.te);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Constant rigidity: %g \n", d);
		E = GMT_Duplicate_Data (API, GMT_IS_DATASET, GMT_DUPLICATE_DATA, Q);
		/* Overwrite 2nd column with constant d below */
		for (tbl = 0; tbl < E->n_tables; tbl++) {
			for (seg = 0; seg < E->table[tbl]->n_segments; seg++) {
				S = E->table[tbl]->segment[seg];	/* Current segment */
				for (row = 0; row < S->n_rows; row++)	/* Set constant rigidity */
					S->data[GMT_Y][row] = d;
			}
		}
	}
	if (Q->n_tables != E->n_tables || Q->n_segments != E->n_segments || Q->n_records != E->n_records) {
		GMT_Report (API, GMT_MSG_NORMAL, "Number of load and rigidity records are not the same!\n");
		Return (API->error);
	}

	if (Ctrl->T.active && Ctrl->T.file)	{	/* Read pre-existing deflections */
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -C,-F,-L files */
		if ((T = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (T->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->T.file, (int)T->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		if (T->n_tables != E->n_tables || T->n_segments != E->n_segments || T->n_records != E->n_records) {
			GMT_Report (API, GMT_MSG_NORMAL, "Number of pre-existing deflection records is not correct!\n");
			Return (API->error);
		}
	}

	restore = (Ctrl->D.rhom - Ctrl->D.rhoi) * NORMAL_GRAVITY;
	n_columns = (Ctrl->S.active) ? 3 : 2;	/* Duplicate Q but posibly add 1 more column */
	W = gmt_alloc_dataset (GMT, Q, 0, n_columns, GMT_ALLOC_NORMAL);

	for (tbl = 0; tbl < W->n_tables; tbl++) {
		for (seg = 0; seg < W->table[tbl]->n_segments; seg++) {
			S = Q->table[tbl]->segment[seg];
			deflection = W->table[tbl]->segment[seg]->data[GMT_Y];	/* Current flexure */
			load = S->data[GMT_Y];	/* Current load */
			rigidity = E->table[tbl]->segment[seg]->data[GMT_Y];	/* Current rigidities */
			gmt_M_memcpy (W->table[tbl]->segment[seg]->data[GMT_X], S->data[GMT_X], S->n_rows, double);
			sprintf (msg, "BCs > ");
			if (Ctrl->A.bc[LEFT] == BC_INFINITY) strcat (msg, "infinity at left edge + ");
			else if (Ctrl->A.bc[LEFT] == BC_PERIODIC) strcat (msg, "periodic at left edge + ");
			else if (Ctrl->A.bc[LEFT] == BC_CLAMPED) {
				deflection[0] = Ctrl->A.deflection[LEFT];
				sprintf (txt, "plate clamped with deflection = %g at left edge + ", Ctrl->A.deflection[LEFT]);
				strcat (msg, txt);
			}
			else if (Ctrl->A.bc[LEFT] == BC_FREE) {
				deflection[0] =  -Ctrl->A.moment[LEFT];	/* Minus-sign because of the +ve gives +ve convention */
				deflection[1] = Ctrl->A.force[LEFT];
				sprintf (txt, "plate free with Moment = %g and Force = %g at left edge + ", Ctrl->A.moment[LEFT], Ctrl->A.force[LEFT]);
				strcat (msg, txt);
			}
			if (Ctrl->A.bc[RIGHT] == BC_INFINITY) strcat (msg, "infinity at right edge.\n");
			else if (Ctrl->A.bc[RIGHT] == BC_PERIODIC) strcat (msg, "periodic at right edge.\n");
			else if (Ctrl->A.bc[RIGHT] == BC_CLAMPED) {
				deflection[S->n_rows-1] = Ctrl->A.deflection[RIGHT];
				sprintf (txt, "plate clamped with deflection = %g at right edge.\n", Ctrl->A.deflection[RIGHT]);
				strcat (msg, txt);
			}
			else if (Ctrl->A.bc[RIGHT] == BC_FREE) {
				deflection[S->n_rows-2] = -Ctrl->A.moment[RIGHT];	/* Minus-sign because of the +ve gives +ve convention */
				deflection[S->n_rows-1] = Ctrl->A.force[RIGHT];
				sprintf (txt, "plate free with Moment = %g and Force = %g at right edge.\n", Ctrl->A.moment[RIGHT], Ctrl->A.force[RIGHT]);
				strcat (msg, txt);
			}
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, msg);
			for (row = 0, airy = true; airy && row < S->n_rows; row++)
				if (rigidity[row] > 0.0) airy = false;

			if (airy) {	/* Airy compensation */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calculate flexure using Airy compensation\n");
				for (row = 0; row < S->n_rows; row++) deflection[row] = load[row] / restore;
			}
			x_inc = S->data[GMT_X][1] - S->data[GMT_X][0];
			if (Ctrl->M.active[0]) x_inc *= 1000.0;	/* Got x in km */

			if (Ctrl->T.active) {	/* Plate has pre-existing deflection */
				double *w0 = T->table[tbl]->segment[seg]->data[GMT_Y];
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calculate flexure of pre-deformed surface\n");
				error = flx1dw0 (GMT, deflection, w0, rigidity, load, (int)S->n_rows, x_inc, &restore, 0, Ctrl->F.force, Ctrl->A.bc[LEFT], Ctrl->A.bc[RIGHT]);
			}
			else if (Ctrl->L.active) {
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calculate flexure with variable restoring force\n");
				error = flx1dk (GMT, deflection, rigidity, load, (int)S->n_rows, x_inc, Ctrl->D.rhom, Ctrl->D.rhol, Ctrl->D.rhoi, Ctrl->D.rhow, Ctrl->F.force, Ctrl->A.bc[LEFT], Ctrl->A.bc[RIGHT]);
			}
			else {	/* Constant restoring force */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calculate flexure for constant restoring force\n");
				error = flx1d (GMT, deflection, rigidity, load, (int)S->n_rows, x_inc, &restore, 0, Ctrl->F.force, Ctrl->A.bc[LEFT], Ctrl->A.bc[RIGHT]);
			}

			if (error) {
				GMT_Report (API, GMT_MSG_NORMAL, "Flexure sub-function returned error = %d!\n", error);
				Return (API->error);
			}

			if (Ctrl->S.active) {	/* Compute curvatures */
				double *curvature = W->table[tbl]->segment[seg]->data[GMT_Z];
				get_curvature (deflection, (int)S->n_rows, x_inc, curvature);
			}

			/* Add in Moho depth, possibly convert back to km, and let z be positive up */
			for (row = 0; row < S->n_rows; row++) {
				deflection[row] = -(deflection[row] + Ctrl->Z.zm);
				if (Ctrl->M.active[1]) deflection[row] /= 1000.0;	/* m -> km */
			}
		}
	}

	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, 0, NULL, Ctrl->Out.file, W) != GMT_NOERROR) {
		Return (API->error);
	}

	gmt_free_dataset (GMT, &W);

	Return (GMT_NOERROR);
}
