/*--------------------------------------------------------------------
 *
 *    Copyright (c) 1999-2020 by T. Henstock
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* pssegyzz program to plot segy files in 3d in postscript with variable trace spacing option
 * uses routines from the GMT postscriptlight (the postscript imagemask for plotting a
 * 1-bit depth bitmap which will not obliterate stuff underneath!
 *
 * Author:	Tim Henstock (then@noc.soton.ac.uk)
 * Date:	17-Nov-97
 * Version:	1.0
 *
 * heavily modified from pssegyz version 1.2
 *
 * Bug fixes:	1.1, 11-6-96 remove undesirable normalization introduced by -Z option. Add -U option for reduction.
 *
 * add option for colored bitmap (modified psimagemask as well..., old version will segfault probably)
 *
 * enhancements: 1.2 , 1/7/99 check number of samples trace by trace to cope with SEGY with variable trace length
 * NB that -T option from pssegyz is _not_ transferred
 *
 *		2.0, 6/7/99 update for GMT v 3.3.1
 *
 *              2.1 10/4/2001 fix unreduce bug, modify to byte-swap integers in the headers, make 64-bit safe
 *
 *              2.2 25/2/2002 fix bug with reduced data plotting, improve error checking on parameters
 *
 *		2.2 7/30/2010 Ported to GMT 5 P. Wessel (global variables removed)
 *
 * This program is free software and may be copied, modified or redistributed
 * under the terms of the GNU LGPL license, see http://www.gnu.org
 */

#include "gmt_dev.h"
#include "segy_io.h"

#define THIS_MODULE_CLASSIC_NAME	"pssegyz"
#define THIS_MODULE_MODERN_NAME	"segyz"
#define THIS_MODULE_LIB		"segy"
#define THIS_MODULE_PURPOSE	"Plot a SEGY file in 3-D"
#define THIS_MODULE_KEYS	">X}"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYpt" GMT_OPT("c")

#define B_ID	0	/* Indices into Q values */
#define I_ID	1
#define U_ID	2
#define X_ID	3
#define Y_ID	4

#define PLOT_CDP	1
#define PLOT_OFFSET	2

struct PSSEGYZ_CTRL {
	struct In {	/* -In */
		bool active;
		char *file;
	} In;
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C<cpt> */
		bool active;
		double value;
	} C;
	struct D {	/* -D */
		bool active;
		double value[2];
	} D;
	struct E {	/* -E */
		bool active;
		double value;
	} E;
	struct F {	/* -F<fill> */
		bool active;
		double rgb[4];
	} F;
	struct I {	/* -I */
		bool active;
	} I;
	struct L {	/* -L */
		bool active;
		int value;
	} L;
	struct M {	/* -M */
		bool active;
		int value;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Qb|i|u|x|y */
		bool active[5];
		double value[5];	/* b is bias, i is dpi, u is redval, x/y are trace and sample interval */
	} Q;
	struct S {	/* -S */
		bool active;
		bool fixed[2];
		unsigned int mode[2];
		int value[2];
		double orig[2];
	} S;
	struct T {	/* -T */
		bool active;
		char *file;
	} T;
	struct W {	/* -W */
		bool active;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSEGYZ_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSSEGYZ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.active = !GMT_BIGENDIAN;
	C->M.value = 10000;
	C->Q.value[I_ID] = 300.0; /* Effective dots-per-inch of image */
	C->Q.value[X_ID] = 1.0; /* Ctrl->Q.value[X_ID], Ctrl->Q.value[Y_ID] are trace and sample interval */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSSEGYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->T.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<segyfile>] -D<dev> -F<color> | -W %s\n", name, GMT_Jx_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A] [-C<clip>] [-E<slop>] [-I] %s[-L<nsamp>]\n", GMT_Rx_OPT, API->K_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-M<ntraces>] [-N] %s%s[-Q<mode><value>] [-S<header>] [-T<tracefile>]\n", API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W] [%s]\n\t[%s] [-Z]\n\t%s[%s] [%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<segyfile> is an IEEE SEGY file [or standard input].\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set <dev> to give deviation in X units of plot for 1.0 on scaled trace.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <dev> is single number (applied equally in X and Y directions) or <devX>/<devY>.\n");
	GMT_Option (API, "JX,R");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\tNB units for y are s or km\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Flip the default byte-swap state (default assumes data have a bigendian byte-order).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Clip scaled trace excursions at <clip>, applied after bias.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set <error> slop to allow for -T. Recommended in case of arithmetic errors!\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Fill negative rather than positive excursions.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Let <nsamp> override number of samples.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Fix number of traces. Default reads all traces.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -M0 will read number in binary header, -M<ntraces> will attempt to read only <ntraces> traces.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Trace normalize the plot, with order of operations: [normalize][bias][clip](deviation).\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Append <mode><value> to change any of 5 different settings:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qb<bias> to bias scaled traces (-Qb-0.1 subtracts 0.1 from values).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qi<dpi> to change image dots-per-inch [300].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qu<redvel> to apply reduction velocity (-ve removes reduction already present).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qx<mult> to multiply trace locations by <mult>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qy<dy> to override sample interval.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Specify <x/y> to set variable spacing.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x,y are (number) for fixed location, c for cdp, o for offset, b<n> for long int at byte n.\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Plot wiggle trace (must specify either -W or -F).\n");
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Suppress plotting traces whose rms amplitude is 0.\n");
	GMT_Option (API, "c,p,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSSEGYZ_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pssegyz and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int k, n_errors = 0, n_files = 0;
	char *txt[2] = {NULL, NULL}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Swap data */
				Ctrl->A.active = !Ctrl->A.active;
				break;
			case 'C':	/* trace clip */
				Ctrl->C.active = true;
				Ctrl->C.value = (float) atof (opt->arg);
				break;
			case 'D':	/* trace scaling */
				Ctrl->D.active = true;
				if (strchr (opt->arg, '/')) {
					sscanf (opt->arg, "%[^/]/%lf", txt_a, &Ctrl->D.value[GMT_Y]);
					Ctrl->D.value[GMT_X] = atof (txt_a);
				}
				else
					Ctrl->D.value[GMT_X] = Ctrl->D.value[GMT_Y] = atof (opt->arg);
				break;
			case 'E':
				Ctrl->E.active = true;
				Ctrl->E.value = atof (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = true;
				if (gmt_getrgb (GMT, opt->arg, Ctrl->F.rgb)) {
					n_errors++;
					gmt_rgb_syntax (GMT, 'F', " ");
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				break;
			case 'L':
				Ctrl->L.active = true;
				Ctrl->L.value = atoi (opt->arg);
				break;
			case 'M':
				Ctrl->M.active = true;
				Ctrl->M.value = atoi (opt->arg);
				break;
			case 'N':	/* trace norm. */
				Ctrl->N.active = true;
				break;
			case 'Q':
				switch (opt->arg[0]) {
					case 'b':	/* Trace bias */
						Ctrl->Q.active[B_ID] = true;
						Ctrl->Q.value[B_ID] = atof (&opt->arg[1]);
						break;
					case 'i':	/* Image dpi */
						Ctrl->Q.active[I_ID] = true;
						Ctrl->Q.value[I_ID] = atof (&opt->arg[1]);
						break;
					case 'u':	/* reduction velocity application */
						Ctrl->Q.active[U_ID] = true;
						Ctrl->Q.value[U_ID] = atof (&opt->arg[1]);
						break;
					case 'x': /* over-rides of header info */
						Ctrl->Q.active[X_ID] = true;
						Ctrl->Q.value[X_ID] = atof (&opt->arg[1]);
						break;
					case 'y': /* over-rides of header info */
						Ctrl->Q.active[Y_ID] = true;
						Ctrl->Q.value[Y_ID] = atof (&opt->arg[1]);
						break;
				}
				break;
			case 'S':
				if (Ctrl->S.active) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -S: Can't specify more than one trace location key\n");
					n_errors++;
					continue;
				}
				Ctrl->S.active = true;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) == 2) {
					txt[0] = txt_a;	txt[1] = txt_b;
					for (k = 0; k < 2; k++) {
						switch (txt[k][0]) {
							case 'o':
								Ctrl->S.mode[k] = PLOT_OFFSET;
								break;
							case 'c':
								Ctrl->S.mode[k] = PLOT_CDP;
								break;
							case 'b':
								Ctrl->S.value[k] = atoi (&txt[k][1]);
								break;
							case '0' : case '1': case '2': case '3': case '4': case '5':
							case '6': case '7': case '8': case '9': case '-': case '+': case '.':
								Ctrl->S.fixed[k] = true;
								Ctrl->S.orig[k] = (double) atof(txt[k]);
								break;
						}
					}
				}
				else
					n_errors++;
				break;
			case 'T':	/* plot traces only at listed locations */
				Ctrl->T.active = true;
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				break;
			case 'W':
				Ctrl->W.active = true;
				break;
			case 'Z':
				Ctrl->Z.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify the -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.wesn[ZLO]  == GMT->common.R.wesn[ZHI], "Must specify z range in -R option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !Ctrl->T.file, "Option -T requires a file name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.file && access (Ctrl->T.file, R_OK), "SCannot file file %s\n", Ctrl->T.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.value < 0.0, "Option -E: Slop cannot be negative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && !Ctrl->F.active, "Must specify -F with -I\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->F.active && !Ctrl->W.active, "Must specify -F or -W\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.value[GMT_X] < 0.0 || Ctrl->D.value[GMT_Y] < 0.0, "Option -D: Must specify a positive deviation\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL double segyz_rms (float *data, int n_samp) {
	/* function to return rms amplitude of n_samp values from the array data */
	int ix;
	double sumsq = 0.0;

	for (ix = 0; ix < n_samp; ix++) sumsq += ((double) data[ix]) * ((double) data[ix]);
	sumsq /= ((double)n_samp);
	sumsq = sqrt (sumsq);
	return (sumsq);
}

GMT_LOCAL int segyz_paint (int ix, int iy, unsigned char *bitmap, int bm_nx, int bm_ny) {
	/* ix iy is pixel to paint */
	static unsigned char bmask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	int byte, quot, rem;

	quot = ix / 8;
	rem = ix - quot * 8;

	if ((quot >= bm_nx-1) || (iy >= bm_ny-1) || (ix < 0) || (iy < 0)) return (-1); /* outside bounds of plot array */

	byte = (bm_ny - iy - 1) * bm_nx + quot; /* find byte to paint - flip vertical! */
	bitmap[byte] = bitmap[byte] | bmask[rem];
	return (0);
}

GMT_LOCAL void wig_bmap (struct GMT_CTRL *GMT, double x0, double y0, float data0, float data1, double z0, double z1, double dev_x, double dev_y, double dpi, unsigned char *bitmap, int bm_nx, int bm_ny) /* apply current sample with all options to bitmap */ {
	int px0, px1, py0, py1, ix, iy;
	double xp0, xp1, yp0, yp1, slope;

	gmt_geoz_to_xy (GMT, x0+(double)data0*dev_x, y0+(double)data0*dev_y, z0, &xp0, &yp0); /* returns 2 ends of line segment in plot coords */
	gmt_geoz_to_xy (GMT, x0+(double)data1*dev_x, y0+(double)data1*dev_y, z1, &xp1, &yp1);
	slope = (yp1 - yp0) / (xp1 - xp0);

	px0 = irint ((xp0 - GMT->current.proj.z_project.xmin) * dpi);
	px1 = irint ((xp1 - GMT->current.proj.z_project.xmin) * dpi);
	py0 = irint ((yp0 - GMT->current.proj.z_project.ymin) * dpi);
	py1 = irint ((yp1 - GMT->current.proj.z_project.ymin) * dpi);

	/* now have the pixel locations for the two samples - join with a line..... */
	if (fabs (slope) <= 1.0) { /* more pixels needed in x direction */
		if (px0 < px1) {
			for (ix = px0; ix <= px1; ix++) {
				iy = py0 + irint (slope * (float) (ix - px0));
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}
		}
		else {
			for (ix = px1; ix <= px0; ix++) {
				iy = py0 + irint (slope * (float) (ix - px0));
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}

		}
	}
	else { /* more pixels needed in y direction */
		if (py0 < py1) {
			for (iy = py0; iy <= py1; iy++) {
				ix = px0 + irint (((float) (iy - py0)) / slope);
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}
		}
		else {
			for (iy = py1; iy <= py0; iy++) {
				ix = px0 + irint (((float) (iy - py0)) / slope);
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}
		}
	}
}

GMT_LOCAL void shade_quad (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y_edge, double slope1, double slope0, double dpi, unsigned char *bitmap, int bm_nx, int bm_ny) {
	/* shade a quadrilateral with two sides parallel to x axis, one side at y=y0 with ends at x0 and x1,
	   with lines with gradients slope0 and slope1 respectively */

	int pedge_y, py0, iy, ix1, ix2, ix;

	if (y0 == y_edge) return;

	pedge_y = irint ((y_edge-GMT->current.proj.z_project.ymin) * dpi);
	py0 = irint ((y0 - GMT->current.proj.z_project.ymin) * dpi);
	if (y0 < y_edge) {
		for (iy = py0; iy < pedge_y; iy++) {
			ix1 = irint ((x0-GMT->current.proj.z_project.xmin + (((double)iy / dpi) + GMT->current.proj.z_project.ymin - y0) * slope0) * dpi);
			ix2 = irint ((x1-GMT->current.proj.z_project.xmin + (((double)iy / dpi) + GMT->current.proj.z_project.ymin - y0) * slope1) * dpi);
			if (ix1 < ix2) {
				for (ix = ix1; ix < ix2; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			} else {
				for (ix = ix2; ix < ix1; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			}
		}
	} else {
		for (iy = pedge_y; iy < py0; iy++) {
			ix1 = irint ((x0 - GMT->current.proj.z_project.xmin + (((double)iy / dpi) +  GMT->current.proj.z_project.ymin - y0) * slope0) * dpi);
			ix2 = irint ((x1 - GMT->current.proj.z_project.xmin + (((double)iy / dpi) +GMT->current.proj.z_project.ymin - y0) * slope1) * dpi);
			if (ix1 < ix2) {
				for (ix = ix1; ix < ix2; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			} else {
				for (ix = ix2; ix < ix1; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			}
		}
	}
}

GMT_LOCAL void shade_tri (struct GMT_CTRL *GMT, double apex_x, double apex_y, double edge_y, double slope, double slope0, double dpi, unsigned char *bitmap, int bm_nx, int bm_ny) {
	/* shade a triangle specified by apex coordinates, y coordinate of an edge (parallel to x-axis)
	   and slopes of the two other sides */

	int papex_y, pedge_y, iy, ix, x1, x2;

#ifdef DEBUG
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "in shade_tri apex_x %f apex_y %f edge_y %f slope %f slope0 %f\n",apex_x, apex_y, edge_y, slope, slope0);
#endif

	if (apex_y == edge_y) return;

	papex_y = irint ((apex_y - GMT->current.proj.z_project.ymin) * dpi); /* location in pixels in y of apex and edge */
	pedge_y = irint ((edge_y - GMT->current.proj.z_project.ymin) * dpi);
	if (apex_y < edge_y) {
		for (iy = papex_y; iy < pedge_y; iy++) {
			x1 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / dpi) + GMT->current.proj.z_project.ymin - apex_y) * slope) * dpi);
			x2 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / dpi) + GMT->current.proj.z_project.ymin - apex_y) * slope0) * dpi);
#ifdef DEBUG
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "apex_y<edge_y iy %d x1 %d x2 %d\n",iy,x1,x2);
#endif
			/* locations in pixels of two x positions for this scan line */
			if (x1 < x2) {
				for (ix = x1; ix < x2; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			} else {
				for (ix = x2; ix < x1; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			}
		}
	} else {
		for (iy = pedge_y; iy < papex_y; iy++) {
			x1 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / dpi) + GMT->current.proj.z_project.ymin - apex_y) * slope) * dpi);
			x2 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / dpi )+ GMT->current.proj.z_project.ymin - apex_y) * slope0) * dpi);
#ifdef DEBUG
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "apex_y>edge_y iy %d x1 %d x2 %d\n",iy,x1,x2);
#endif
			if (x1 < x2) {
				for (ix = x1; ix < x2; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			}
			else {
				for (ix = x2; ix < x1; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			}
		}
	}
}

#define NPTS 4 /* 4 points for the general case here */
GMT_LOCAL void segyz_shade_bmap (struct GMT_CTRL *GMT, double x0, double y0, float data0, float data1, double z0, double z1, int negative, double dev_x, double dev_y, double dpi, unsigned char *bitmap, int bm_nx, int bm_ny) {
	/* apply current samples with all options to bitmap */
	int ix, iy;
	double xp[NPTS], yp[NPTS], interp, slope01, slope02, slope12, slope13, slope23, slope03;
	double slope0, slope1, slope2, slope3;

	if (data0 == 0.0 && data1 == 0.0) return; /* probably shouldn't strictly, but pathological enough I don't really want to deal with it! */

	if ((data0 * data1) < 0.0) {
		/* points to plot are on different sides of zero - interpolate to find out where zero is */
		interp = z0 + data0 * ((z0 - z1) / (data1 - data0));
		if (((data0 < 0.0) && negative) || ((data0 > 0.0)&& !negative)) {
			/* plot from top to zero */
			z1 = interp;
			data1 = 0.0f;
		}
		else {
			z0 = interp;
			data0 = 0.0f;
		}
	}

	gmt_geoz_to_xy (GMT, x0+(double)data0*dev_x, y0+(double)data0*dev_y, z0, &xp[0], &yp[0]); /* returns 2 ends of line segment in plot coords */
	gmt_geoz_to_xy (GMT, x0+(double)data1*dev_x, y0+(double)data1*dev_y, z1, &xp[1], &yp[1]);
	gmt_geoz_to_xy (GMT, x0, y0, z0, &xp[2], &yp[2]); /* to get position of zero at each point*/
	gmt_geoz_to_xy (GMT, x0, y0, z1, &xp[3], &yp[3]); /* to get position of zero at each point*/

	/* now have four corner coordinates - need to sort them */
	for (ix = 0; ix < NPTS-1; ix++)
		for (iy = ix + 1; iy < NPTS; iy++)
			if (yp[ix] > yp[iy]) {
				gmt_M_double_swap (yp[iy], yp[ix]);
				gmt_M_double_swap (xp[iy], xp[ix]);
			}


	/* have to fill the quadrilateral defined by 4 points (now ordered, but care with degenerate cases)*/

	slope01 = (xp[1] - xp[0]) / (yp[1] - yp[0]);
	slope02 = (xp[2] - xp[0]) / (yp[2] - yp[0]);
	slope12 = (xp[2] - xp[1]) / (yp[2] - yp[1]);
	slope13 = (xp[3] - xp[1]) / (yp[3] - yp[1]);
	slope23 = (xp[3] - xp[2]) / (yp[3] - yp[2]);
	slope03 = (xp[3] - xp[0]) / (yp[3] - yp[0]);
	if ((yp[0] != yp[1]) && (yp[2] != yp[3])) {	/* simple case: tri-quad-tri */
		shade_tri (GMT, xp[0], yp[0], yp[1], slope01, slope02, dpi, bitmap, bm_nx, bm_ny);
		shade_quad (GMT, xp[1], yp[1],xp[0]+slope02*(yp[1]-yp[0]), yp[2], slope02, slope13, dpi, bitmap, bm_nx, bm_ny);
		shade_tri (GMT, xp[3], yp[3], yp[2], slope13, slope23, dpi, bitmap, bm_nx, bm_ny);
	}
	if ((yp[0] == yp[1]) && (yp[2] != yp[3])) {
		if (xp[0] == xp[1]) { /* two triangles based on yp[1],yp[2]. yp[3] */
			shade_tri (GMT, xp[1], yp[1], yp[2], slope12, slope13, dpi, bitmap, bm_nx, bm_ny);
			shade_tri (GMT, xp[3], yp[3], yp[2], slope23, slope13, dpi, bitmap, bm_nx, bm_ny);
		} else { /* quad based on first 3 points, then tri */
			slope0 = (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1])&&(xp[3]>xp[2])))*slope03 + (((xp[0]<xp[1])&&(xp[2]<xp[3])) || ((xp[0]>xp[1])&&(xp[2]>xp[3])))*slope02;
			slope1 = (((xp[1]<xp[0]) && (xp[3]<xp[2])) || ((xp[1]>xp[0]) && (xp[3]>xp[2])))*slope13 + (((xp[1]<xp[0])&&(xp[2]<xp[3])) || ((xp[1]>xp[0])&&(xp[2]>xp[3])))*slope12;
			slope3 = (((xp[1]<xp[0]) && (xp[3]<xp[2])) || ((xp[1]>xp[0]) && (xp[3]>xp[2])))*slope13 + (((xp[0]<xp[1])&&(xp[3]<xp[2])) || ((xp[0]>xp[1])&&(xp[3]>xp[2])))*slope03;
			shade_quad (GMT, xp[0], yp[0], xp[1], yp[2], slope0, slope1, dpi, bitmap, bm_nx, bm_ny);
			shade_tri (GMT, xp[3], yp[3], yp[2], slope23, slope3, dpi, bitmap, bm_nx, bm_ny);
		}
	}
	if ((yp[0] != yp[1]) && (yp[2] == yp[3])) {
		if (xp[2] == xp[3]) {/* two triangles based on yp[0],yp[1]. yp[2] */
		shade_tri (GMT, xp[0], yp[0], yp[1], slope01, slope02, dpi, bitmap, bm_nx, bm_ny);
		shade_tri (GMT, xp[2], yp[2], yp[1], slope12, slope02, dpi, bitmap, bm_nx, bm_ny);
		} else { /* triangle based on yp[0], yp[1], then quad based on last 3 points */
			slope0 = (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1]) && (xp[3]>xp[2])))*slope03 + (((xp[0]<xp[1])&&(xp[2]<xp[3])) || ((xp[0]>xp[1])&&(xp[2]>xp[3])))*slope02;
			shade_tri (GMT, xp[0], yp[0], yp[1], slope01, slope0, dpi, bitmap, bm_nx, bm_ny);
			slope2 = (((xp[0]<xp[1]) && (xp[2]<xp[3])) || ((xp[0]>xp[1]) && (xp[2]>xp[3])))*slope02 + (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1]) && (xp[3]>xp[2])))*slope12;
			slope3 = (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1]) && (xp[3]>xp[2])))*slope03 + (((xp[0]<xp[1]) && (xp[2]<xp[3])) || ((xp[0]>xp[1]) && (xp[2]>xp[3])))*slope13;
			shade_quad (GMT, xp[2], yp[2], xp[3], yp[1], slope2, slope3, dpi, bitmap, bm_nx, bm_ny);
		}
	}
}

GMT_LOCAL void segyz_plot_trace (struct GMT_CTRL *GMT, float *data, double dz, double x0, double y0, int n_samp, int do_fill, int negative, int plot_wig, float toffset, double dev_x, double dev_y, double dpi, unsigned char *bitmap, int  bm_nx, int bm_ny) {
	/* shell function to loop over all samples in the current trace, determine plot options
	 * and call the appropriate bitmap routine */

	int iz, paint_wiggle;
	float z0 = (float)GMT->common.R.wesn[ZLO], z1;

	for (iz = 1; iz < n_samp; iz++) {	/* loop over samples on trace - refer to pairs iz-1, iz */
		z1 = (float )(dz * (float) iz + toffset);
		if (z1 >= GMT->common.R.wesn[ZLO] && z1 <= GMT->common.R.wesn[ZHI]) {	/* check within z bounds specified */
#ifdef DEBUG
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "x0, %f\t y0, %f\t,z1, %f\t data[iz], %f\t iz, %d\n", x0, y0, z1, data[iz], iz);
#endif
			if (plot_wig) wig_bmap (GMT, x0, y0, data[iz-1],data[iz], z0, z1, dev_x, dev_y, dpi, bitmap, bm_nx, bm_ny);	/* plotting wiggle */
			if (do_fill) {	/* plotting VA -- check data points first */
				paint_wiggle = ((!negative && ((data[iz-1] >= 0.0) || (data[iz] >= 0.0))) || (negative && ((data[iz-1] <= 0.0) || (data[iz] <= 0.0))));
				if (paint_wiggle) segyz_shade_bmap (GMT, x0, y0, data[iz-1], data[iz], z0, z1, negative, dev_x, dev_y, dpi, bitmap, bm_nx, bm_ny);
			}
			z0 = z1;
		}
	}
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_segyz (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: segyz\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_pssegyz (V_API, mode, args);
}

int GMT_pssegyz (void *V_API, int mode, void *args) {
	int nm, ix, iz, n_samp = 0, check, bm_nx, bm_ny, error;
	double xlen, ylen, xpix, ypix, x0, y0, trans[3] = {-1.0,-1.0,-1.0};
	float scale = 1.0f, toffset = 0.0f;
	unsigned char *bitmap = NULL;
	char reelhead[3200] = {""};
	float *data = NULL;
	FILE *fpi = NULL;
	SEGYHEAD *header = NULL;
	SEGYREEL binhead;

	struct PSSEGYZ_CTRL *Ctrl = NULL;
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pssegyz main code ----------------------------*/

	if (!gmt_M_is_linear (GMT)) GMT_Report (API, GMT_MSG_WARNING, "You asked for a non-rectangular projection. \n It will probably still work, but be prepared for problems\n");

	if (Ctrl->In.active) {
		GMT_Report (API, GMT_MSG_INFORMATION, "Will read segy file %s\n", Ctrl->In.file);
		if ((fpi = gmt_fopen (GMT, Ctrl->In.file, "rb")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot find segy file %s\n", Ctrl->In.file);
			Return (GMT_ERROR_ON_FOPEN);
		}
	}
	else {
		GMT_Report (API, GMT_MSG_INFORMATION, "Will read segy file from standard input\n");
		if (fpi == NULL) fpi = stdin;
	}

	/* set up map projection and PS plotting */
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) {
		if (fpi != stdin) fclose (fpi);
		Return (GMT_PROJECTION_ERROR);
	}
	if ((PSL = gmt_plotinit (GMT, options)) == NULL) {
		if (fpi != stdin) fclose (fpi);
		Return (GMT_RUNTIME_ERROR);
	}
	/* In this program we DO NOT want to call gmt_plane_perspective since that is already in the SEGV projection
	 * Per Tim Henstock, Nov, 2015.
	 * gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	 */
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	/* define area for plotting and size of array for bitmap */
	xlen = GMT->current.proj.rect[XHI] - GMT->current.proj.rect[XLO];
	xpix = xlen * Ctrl->Q.value[I_ID]; /* pixels in x direction */
	bm_nx = irint (ceil (xpix / 8.0)); /* store 8 pixels per byte in x direction but must have
				whole number of bytes per scan */
	ylen = GMT->current.proj.rect[YHI] - GMT->current.proj.rect[YLO];
	ypix = ylen * Ctrl->Q.value[I_ID]; /* pixels in y direction */
	bm_ny = irint (ypix);
	nm = bm_nx * bm_ny;

	if ((check = segy_get_reelhd (fpi, reelhead)) != true) {
		fclose (fpi);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); Return(GMT_RUNTIME_ERROR);
	}
	if ((check = segy_get_binhd (fpi, &binhead)) != true) {
		fclose (fpi);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); Return(GMT_RUNTIME_ERROR);
	}

	if (Ctrl->A.active) {
/* this is a little-endian system, and we need to byte-swap ints in the reel header - we only
use a few of these*/
		GMT_Report (API, GMT_MSG_INFORMATION, "Swapping bytes for ints in the headers\n");
		binhead.num_traces = bswap16 (binhead.num_traces);
		binhead.nsamp = bswap16 (binhead.nsamp);
		binhead.dsfc = bswap16 (binhead.dsfc);
		binhead.sr = bswap16 (binhead.sr);
	}

/* set parameters from the reel headers */
	if (!Ctrl->M.value) Ctrl->M.value = binhead.num_traces;

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of traces in header is %d\n", Ctrl->M.value);


	if (!Ctrl->L.value) {/* number of samples not overridden*/
		Ctrl->L.value = binhead.nsamp;
		GMT_Report (API, GMT_MSG_INFORMATION, "Number of samples per trace is %d\n", Ctrl->L.value);
	}
	else if ((Ctrl->L.value != binhead.nsamp) && (binhead.nsamp))
		GMT_Report (API, GMT_MSG_INFORMATION, "nsampr input %d, nsampr in header %d\n", Ctrl->L.value, binhead.nsamp);

	if (!Ctrl->L.value) { /* no number of samples still - a problem! */
		GMT_Report (API, GMT_MSG_ERROR, "Number of samples per trace unknown\n");
		GMT_exit (GMT, GMT_RUNTIME_ERROR);
		if (fpi != stdin) fclose (fpi);
		Return(GMT_RUNTIME_ERROR);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of samples is %d\n", n_samp);

	if (binhead.dsfc != 5) GMT_Report (API, GMT_MSG_WARNING, "Data not in IEEE format\n");

	if (!Ctrl->Q.value[Y_ID]) {
		Ctrl->Q.value[Y_ID] = binhead.sr; /* sample interval of data (microseconds) */
		Ctrl->Q.value[Y_ID] /= 1000000.0;
		GMT_Report (API, GMT_MSG_INFORMATION, "Sample interval is %f s\n", Ctrl->Q.value[Y_ID]);
	}
	else if ((Ctrl->Q.value[Y_ID] != binhead.sr) && (binhead.sr)) /* value in header overridden by input */
		GMT_Report (API, GMT_MSG_INFORMATION, "dz input %f, dz in header %f\n", Ctrl->Q.value[Y_ID], (float)binhead.sr);

	if (!Ctrl->Q.value[Y_ID]) { /* still no sample interval at this point is a problem! */
		GMT_Report (API, GMT_MSG_ERROR, "No sample interval in reel header\n");
		if (fpi != stdin) fclose (fpi);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); Return(GMT_RUNTIME_ERROR);
	}

	bitmap = gmt_M_memory (GMT, NULL, nm, unsigned char);

	ix = 0;
	while ((ix < Ctrl->M.value) && (header = segy_get_header (fpi)) != 0) {
		/* read traces one by one */
		/* check true location header for x */
		if (Ctrl->S.mode[GMT_X] == PLOT_OFFSET) {
			/* plot traces by offset, cdp, or input order */
			int32_t tmp = header->sourceToRecDist;
			if (Ctrl->A.active) {
				uint32_t *p = (uint32_t *)&tmp;
				*p = bswap32 (*p);
			}
			x0 = (double) tmp;
		}
		else if (Ctrl->S.mode[GMT_X] == PLOT_CDP) {
			int32_t tmp = header->cdpEns;
			if (Ctrl->A.active) {
				uint32_t *p = (uint32_t *)&tmp;
				*p = bswap32 (*p);
			}
			x0 = (double) tmp;
		}
		else if (Ctrl->S.value[GMT_X]) {
			/* get value starting at Ctrl->S.value of header into a double */
			uint32_t tmp;
			memcpy (&tmp, &header[Ctrl->S.value[GMT_X]], sizeof (uint32_t));
			x0 = (Ctrl->A.active) ? (double) bswap32 (tmp) : (double) tmp;
		}
		else if (Ctrl->S.fixed[GMT_X])
			x0 = Ctrl->S.orig[GMT_X] / Ctrl->Q.value[X_ID];
		else
			x0 = (1.0 + (double) ix); /* default x to input trace number */

		/* now do same for y */
		if (Ctrl->S.mode[GMT_Y] == PLOT_OFFSET) {
			/* plot traces by offset, cdp, or input order */
			int32_t tmp = header->sourceToRecDist;
			if (Ctrl->A.active) {
				uint32_t *p = (uint32_t *)&tmp;
				*p = bswap32 (*p);
			}
			y0 = (double) tmp;
		}
		else if (Ctrl->S.mode[GMT_Y] == PLOT_CDP) {
			int32_t tmp = header->cdpEns;
			if (Ctrl->A.active) {
				uint32_t *p = (uint32_t *)&tmp;
				*p = bswap32 (*p);
			}
			y0 = (double) tmp;
		}
		else if (Ctrl->S.value[GMT_Y]) {
			/* get value starting at Ctrl->S.value of header into a double */
			uint32_t tmp;
			memcpy (&tmp, &header[Ctrl->S.value[GMT_Y]], sizeof (uint32_t));
			y0 = (Ctrl->A.active) ? (double) bswap32 (tmp) : (double) tmp;
		}
		else if (Ctrl->S.fixed[GMT_Y])
			y0  = Ctrl->S.orig[GMT_Y] / Ctrl->Q.value[X_ID];
		else
			y0 = GMT->common.R.wesn[YLO] / Ctrl->Q.value[X_ID]; /* default y to s edge of projection */

		x0 *= Ctrl->Q.value[X_ID];
		y0 *= Ctrl->Q.value[X_ID]; /* scale x and y by the input Ctrl->Q.value[X_ID] scalar */

		if (Ctrl->A.active) {
			/* need to permanently byte-swap some things in the trace header
				 do this after getting the location of where traces are plotted
				 in case the general Ctrl->S.value[GMT_X] case overlaps a defined
				 header in a strange way */
			uint32_t *p = (uint32_t *)&header->sourceToRecDist;
			*p = bswap32 (*p);
			header->sampleLength = bswap16 (header->sampleLength);
			header->num_samps = bswap32 (header->num_samps);
		}

		GMT_Report (API, GMT_MSG_INFORMATION, "trace %d at x=%f, y=%f \n", ix+1, x0, y0);

		if (Ctrl->Q.value[U_ID]) {
			toffset = (float) -(fabs ((double)(header->sourceToRecDist)) / Ctrl->Q.value[U_ID]);
			GMT_Report (API, GMT_MSG_INFORMATION, "time shifted by %f\n", toffset);
		}

		data = segy_get_data (fpi, header);	/* read a trace */
		/* get number of samples in _this_ trace (e.g. OMEGA has strange ideas about SEGY standard)
		   or set to number in reel header */
                if ((n_samp = segy_samp_rd (header)) != 0) n_samp = Ctrl->L.value;

		if (Ctrl->A.active) {
			/* need to swap the order of the bytes in the data even though assuming IEEE format */
			uint32_t tmp;
			for (iz = 0; iz < n_samp; ++iz) {
				memcpy (&tmp, &data[iz], sizeof(uint32_t));
				tmp = bswap32 (tmp);
				memcpy (&data[iz], &tmp, sizeof(uint32_t));
			}
		}

		if (Ctrl->N.active || Ctrl->Z.active) {
			scale= (float) segyz_rms (data, n_samp);
			GMT_Report (API, GMT_MSG_INFORMATION, "rms value is %f\n", scale);
		}
		for (iz = 0; iz < n_samp; iz++) { /* scale bias and clip each sample in the trace */
			if (Ctrl->N.active) data[iz] /= scale;
			data[iz] += (float)Ctrl->Q.value[B_ID];
			if (Ctrl->C.active && (fabs (data[iz]) > Ctrl->C.value)) data[iz] = (float)(Ctrl->C.value*data[iz] / fabs (data[iz])); /* apply bias and then clip */
		}

		if (!Ctrl->Z.active || scale) segyz_plot_trace (GMT, data, Ctrl->Q.value[Y_ID], x0, y0, n_samp, Ctrl->F.active, Ctrl->I.active, Ctrl->W.active, toffset, Ctrl->D.value[GMT_X], Ctrl->D.value[GMT_Y], Ctrl->Q.value[I_ID], bitmap, bm_nx, bm_ny);
		gmt_M_str_free (data);
		gmt_M_str_free (header);
		ix++;
	}

	/* set a clip at the map boundary since the image space overlaps a little */
	PSL_plotbitimage (PSL, 0.0, 0.0, xlen, ylen, 1, bitmap, 8*bm_nx, bm_ny, trans, Ctrl->F.rgb);

	if (fpi != stdin) fclose (fpi);

	/* Not needed, see above for why.  gmt_plane_perspective (GMT, -1, 0.0); */

	gmt_plotend (GMT);

	gmt_M_free (GMT, bitmap);

	Return (GMT_NOERROR);
}
