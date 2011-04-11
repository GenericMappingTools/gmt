/*--------------------------------------------------------------------
 *	$Id: pssegyz_func.c,v 1.3 2011-04-11 21:15:32 remko Exp $
 *
 *    Copyright (c) 1999-2011 by T. Henstock
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* pssegyzz program to plot segy files in 3d in postscript with variable trace spacing option
 * uses routines from the GMT pslib (the postscript imagemask for plotting a
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
 * under the terms of the GNU public license, see http://www.gnu.org
 */

#include "pslib.h"
#include "gmt_segy.h"
#include "segy_io.h"

#define B_ID	0	/* Indices into Q values */
#define U_ID	1
#define X_ID	2
#define Z_ID	3

#define PLOT_CDP	1
#define PLOT_OFFSET	2

struct PSSEGYZ_CTRL {
	struct In {	/* -In */
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		double value;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
		double value[2];
	} D;
	struct E {	/* -E */
		GMT_LONG active;
		double value;
	} E;
	struct F {	/* -F<fill> */
		GMT_LONG active;
		double rgb[4];
	} F;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG value;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
		GMT_LONG value;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Qb|u|x|y */
		GMT_LONG active[4];
		double value[4];	/* b is bias, u is redval, x/y are trace and sample interval */
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
		GMT_LONG fixed[2];
		GMT_LONG mode[2];
		GMT_LONG value[2];
		double orig[2];
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		char *file;
	} T;
	struct W {	/* -W */
		GMT_LONG active;
	} W;
	struct Z {	/* -Z */
		GMT_LONG active;
	} Z;
};

void *New_pssegyz_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSEGYZ_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSSEGYZ_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->A.active = !WORDS_BIGENDIAN;
	C->M.value = 10000;
	C->Q.value[X_ID] = 1.0; /* Ctrl->Q.value[X_ID], Ctrl->Q.value[Z_ID] are trace and sample interval */
	return ((void *)C);
}

void Free_pssegyz_Ctrl (struct GMT_CTRL *GMT, struct PSSEGYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C && C->In.file) free ((void *)C->In.file);
	if (C && C->T.file) free ((void *)C->T.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_pssegyz_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "pssegyzz %s - Plot a segy file in PostScript\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: pssegyz [<segyfile>] -D<dev> -F<color> | -W %s %s \n", GMT_Jx_OPT, GMT_Rx_OPT);
	GMT_message (GMT, "\t[-A] [-C<clip>] [-E<slop>] [-I] [-K] [-L<nsamp>] [-M<ntraces>] [-N]\n");
	GMT_message (GMT, "\t[-O] [-P] [-Q<mode><value>] [-S<header>] [-T<tracefile>] [%s]\n", GMT_U_OPT);
	GMT_message (GMT, "\t[%s] [-W] [%s] [%s] [-Z] [%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<segyfile> is an IEEE SEGY file [or standard input] \n\n");
	GMT_message (GMT, "\t-D<dev> to give deviation in X units of plot for 1.0 on scaled trace.\n");
	GMT_message (GMT, "\t<dev> is single number (applied equally in X and Y directions) or <devX>/<devY>.\n");
	GMT_message (GMT, "\n\t-Jx for projection.  Scale in INCH/units.  Specify one:\n\t -Jx<x-scale>              Linear projection\n\t-Jx<x-scale>l             Log10 projection\n\t  -Jx<x-scale>p<power>      x^power projection\n\tUse / to specify separate x/y scaling.\n\t If -JX is used then give axes lengths rather than scales\n\t regular map projections may not work!\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\tNB units for y are s or km\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A flips the default byte-swap state (default assumes data have a bigendian byte-order)\n");
	GMT_message (GMT, "\t-C<clip> to clip scaled trace excursions at <clip>, applied after bias\n");
	GMT_message (GMT, "\t-E<error> slop to allow for -T. recommended in case of arithmetic errors!\n");
	GMT_message (GMT, "\t-I to fill negative rather than positive excursions\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-L<nsamp> to override number of samples\n");
	GMT_message (GMT, "\t-M<ntraces> to fix number of traces. Default reads all traces.\n");
	GMT_message (GMT, "\t  -M0 will read number in binary header, -Mn will attempt to read only n traces.\n");
	GMT_message (GMT, "\t-N to trace normalize the plot\n");
	GMT_message (GMT, "\t	order of operations: [normalize][bias][clip](deviation)\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q<mode><value> can be used to change 4 different settings:\n");
	GMT_message (GMT, "\t  -Qb<bias> to bias scaled traces (-B-0.1 subtracts 0.1 from values)\n");
	GMT_message (GMT, "\t  -Qu<redvel> to apply reduction velocity (-ve removes reduction already present)\n");
	GMT_message (GMT, "\t  -Qx<mult> to multiply trace locations by <mult>\n");
	GMT_message (GMT, "\t  -Qy<dy> to override sample interval\n");
	GMT_message (GMT,"\t-S<x/y> to set variable spacing\n");
	GMT_message (GMT,"\t	x,y are (number) for fixed location, c for cdp, o for offset, b<n> for long int at byte n\n");
	GMT_explain_options (GMT, "UV");
	GMT_message (GMT, "\t-W to plot wiggle trace (must specify either -W or -F)\n");
	GMT_explain_options (GMT, "X");
	GMT_message (GMT, "\t-Z to suppress plotting traces whose rms amplitude is 0.\n");
	GMT_explain_options (GMT, "cpt.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_pssegyz_parse (struct GMTAPI_CTRL *C, struct PSSEGYZ_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pssegyz and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG k, n_errors = 0, n_files = 0;
	char *txt[2], txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Swap data */
				Ctrl->A.active = !Ctrl->A.active;
				break;
			case 'C':	/* trace clip */
				Ctrl->C.active = TRUE;
				Ctrl->C.value = (float) atof (opt->arg);
				break;
			case 'D':	/* trace scaling */
				Ctrl->D.active = TRUE;
				if (strchr (opt->arg, '/')) {
					sscanf (opt->arg, "%[^/]/%lf", txt_a, &Ctrl->D.value[GMT_Y]);
					Ctrl->D.value[GMT_X] = atof (txt_a);
				}
				else
					Ctrl->D.value[GMT_X] = Ctrl->D.value[GMT_Y] = atof (opt->arg);
				break;
			case 'E':
				Ctrl->E.active = TRUE;
				Ctrl->E.value = atof (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				if (GMT_getrgb (GMT, opt->arg, Ctrl->F.rgb)) {
					n_errors++;
					GMT_rgb_syntax (GMT, 'F', " ");
				}
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				Ctrl->L.value = atoi (opt->arg);
				break;
			case 'M':
				Ctrl->M.active = TRUE;
				Ctrl->M.value = atoi (opt->arg);
				break;
			case 'N':	/* trace norm. */
				Ctrl->N.active = TRUE;
				break;
			case 'Q':
				switch (opt->arg[0]) {
					case 'b':	/* Trace bias */
						Ctrl->Q.active[B_ID] = TRUE;
						Ctrl->Q.value[B_ID] = atof (opt->arg);
						break;
					case 'u':	/* reduction velocity application */
						Ctrl->Q.active[U_ID] = TRUE;
						Ctrl->Q.value[U_ID] = atof (opt->arg);
						break;
					case 'x': /* over-rides of header info */
						Ctrl->Q.active[X_ID] = TRUE;
						Ctrl->Q.value[X_ID] = atof (opt->arg);
						break;
					case 'y': /* over-rides of header info */
						Ctrl->Q.active[Z_ID] = TRUE;
						Ctrl->Q.value[Z_ID] = atof (opt->arg);
						break;
				}
				break;
			case 'S':
				if (Ctrl->S.active) {
					GMT_message (GMT, "Syntax error: Can't specify more than one trace location key\n");
					n_errors++;
					continue;
				}
				Ctrl->S.active = TRUE;
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
								Ctrl->S.fixed[k] = TRUE;
								Ctrl->S.mode[k] = PLOT_CDP;
								break;
						}
					}
				}
				else
					n_errors++;
				break;
			case 'T':	/* plot traces only at listed locations */
				Ctrl->T.active = TRUE;
				Ctrl->T.file = strdup (opt->arg);
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error.  Must specify the -R option\n");
	n_errors += GMT_check_condition (GMT, GMT->common.R.wesn[ZLO]  == GMT->common.R.wesn[ZHI], "Syntax error.  Must specify z range in -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !Ctrl->T.file, "Syntax error.  Option -T requires a file name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.file && !access (Ctrl->T.file, R_OK), "Syntax error.  Cannot file file %s\n", Ctrl->T.file);
	n_errors += GMT_check_condition (GMT, Ctrl->E.value < 0.0, "Syntax error -E.  slop cannot be negative\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->F.active, "Syntax error.  Must specify -F with -I\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->F.active && !Ctrl->W.active, "Syntax error.  Must specify -F or -W\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.value[GMT_X] < 0.0 || Ctrl->D.value[GMT_Y] < 0.0, "Syntax error.  Must specify a positive deviation\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

double segyz_rms (float *data, GMT_LONG n_samp)
{	/* function to return rms amplitude of n_samp values from the array data */
	GMT_LONG ix;
	double sumsq = 0.0;

	for (ix = 0; ix < n_samp; ix++) sumsq += ((double) data[ix]) * ((double) data[ix]);
	sumsq /= ((double)n_samp);
	sumsq = sqrt (sumsq);
	return (sumsq);
}

GMT_LONG segyz_paint (GMT_LONG ix, GMT_LONG iy, unsigned char *bitmap, GMT_LONG bm_nx, GMT_LONG bm_ny)
{	/* ix iy is pixel to paint */
	static unsigned char bmask[8]={128, 64, 32, 16, 8, 4, 2, 1};
	GMT_LONG byte, quot, rem;

	quot = ix / 8;
	rem = ix - quot * 8;

	if ((quot >= bm_nx-1) || (iy >= bm_ny-1) || (ix < 0) || (iy < 0)) return (-1); /* outside bounds of plot array */

	byte = (bm_ny - iy - 1) * bm_nx + quot; /* find byte to paint - flip vertical! */
	bitmap[byte] = bitmap[byte] | bmask[rem];
	return (0);
}

void wig_bmap (struct GMT_CTRL *GMT, double x0, double y0, float data0, float data1, double z0, double z1, double dev_x, double dev_y, unsigned char *bitmap, GMT_LONG bm_nx, GMT_LONG bm_ny) /* apply current sample with all options to bitmap */
{
	GMT_LONG px0, px1, py0, py1, ix, iy;
	double xp0, xp1, yp0, yp1, slope;

	GMT_geoz_to_xy (GMT, x0+(double)data0*dev_x, y0+(double)data0*dev_y, z0, &xp0, &yp0); /* returns 2 ends of line segment in plot coords */
	GMT_geoz_to_xy (GMT, x0+(double)data1*dev_x, y0+(double)data1*dev_y, z1, &xp1, &yp1);
	slope = (yp1 - yp0) / (xp1 - xp0);

	px0 = (GMT_LONG) ((xp0 - GMT->current.proj.z_project.xmin) * GMT->current.setting.ps_dpi);
	px1 = (GMT_LONG) ((xp1 - GMT->current.proj.z_project.xmin) * GMT->current.setting.ps_dpi);
	py0 = (GMT_LONG) ((yp0 - GMT->current.proj.z_project.ymin) * GMT->current.setting.ps_dpi);
	py1 = (GMT_LONG) ((yp1 - GMT->current.proj.z_project.ymin) * GMT->current.setting.ps_dpi);

	/* now have the pixel locations for the two samples - join with a line..... */
	if (fabs (slope) <= 1.0) { /* more pixels needed in x direction */
		if (px0 < px1) {
			for (ix = px0; ix <= px1; ix++) {
				iy = py0 + (GMT_LONG) (slope * (float) (ix - px0));
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}
		}
		else {
			for (ix = px1; ix <= px0; ix++) {
				iy = py0 + (GMT_LONG) (slope * (float) (ix - px0));
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}

		}
	}
	else { /* more pixels needed in y direction */
		if (py0 < py1) {
			for (iy = py0; iy <= py1; iy++) {
				ix = px0 + (GMT_LONG) (((float) (iy - py0)) / slope);
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}
		}
		else {
			for (iy = py1; iy <= py0; iy++) {
				ix = px0 + (GMT_LONG) (((float) (iy - py0)) / slope);
				segyz_paint (ix, iy, bitmap, bm_nx, bm_ny);
			}
		}
	}
}

void shade_quad (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y_edge, double slope1, double slope0, unsigned char *bitmap, GMT_LONG bm_nx, GMT_LONG bm_ny)
/* shade a quadrilateral with two sides parallel to x axis, one side at y=y0 with ends at x0 and x1,
with lines with gradients slope0 and slope1 respectively */
{
	GMT_LONG pedge_y, py0, iy, ix1, ix2, ix;

	if (y0 == y_edge) return;

	pedge_y = irint ((y_edge-GMT->current.proj.z_project.ymin) * GMT->current.setting.ps_dpi);
	py0 = irint ((y0 - GMT->current.proj.z_project.ymin) * GMT->current.setting.ps_dpi);
	if (y0 < y_edge) {
		for (iy = py0; iy < pedge_y; iy++) {
			ix1 = irint ((x0-GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi) + GMT->current.proj.z_project.ymin - y0) * slope0) * GMT->current.setting.ps_dpi);
			ix2 = irint ((x1-GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi) + GMT->current.proj.z_project.ymin - y0) * slope1) * GMT->current.setting.ps_dpi);
			if (ix1 < ix2) {
				for (ix = ix1; ix < ix2; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			} else {
				for (ix = ix2; ix < ix1; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			}
		}
	} else {
		for (iy = pedge_y; iy < py0; iy++) {
			ix1 = irint ((x0 - GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi) +  GMT->current.proj.z_project.ymin - y0) * slope0) * GMT->current.setting.ps_dpi);
			ix2 = irint ((x1 - GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi) +GMT->current.proj.z_project.ymin - y0) * slope1) * GMT->current.setting.ps_dpi);
			if (ix1 < ix2) {
				for (ix = ix1; ix < ix2; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			} else {
				for (ix = ix2; ix < ix1; ix++) segyz_paint (ix,iy, bitmap, bm_nx, bm_ny);
			}
		}
	}
}

void shade_tri (struct GMT_CTRL *GMT, double apex_x, double apex_y, double edge_y, double slope, double slope0, unsigned char *bitmap, GMT_LONG bm_nx, GMT_LONG bm_ny)
/* shade a triangle specified by apex coordinates, y coordinate of an edge (parallel to x-axis)
and slopes of the two other sides */
{
	GMT_LONG papex_y, pedge_y, iy, ix, x1, x2;

#ifdef DEBUG
	GMT_report (GMT, GMT_MSG_DEBUG, "in shade_tri apex_x %f apex_y %f edge_y %f slope %f slope0 %f\n",apex_x, apex_y, edge_y, slope, slope0);
#endif

	if (apex_y == edge_y) return;

	papex_y = irint ((apex_y - GMT->current.proj.z_project.ymin) * GMT->current.setting.ps_dpi); /* location in pixels in y of apex and edge */
	pedge_y = irint ((edge_y - GMT->current.proj.z_project.ymin) * GMT->current.setting.ps_dpi);
	if (apex_y < edge_y) {
		for (iy = papex_y; iy < pedge_y; iy++) {
			x1 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi) + GMT->current.proj.z_project.ymin - apex_y) * slope) * GMT->current.setting.ps_dpi);
			x2 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi) + GMT->current.proj.z_project.ymin - apex_y) * slope0) * GMT->current.setting.ps_dpi);
#ifdef DEBUG
			GMT_report (GMT, GMT_MSG_DEBUG, "apex_y<edge_y iy %ld x1 %ld x2 %ld\n",iy,x1,x2);
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
			x1 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi) + GMT->current.proj.z_project.ymin - apex_y) * slope) * GMT->current.setting.ps_dpi);
			x2 = irint ((apex_x - GMT->current.proj.z_project.xmin + (((double)iy / GMT->current.setting.ps_dpi )+ GMT->current.proj.z_project.ymin - apex_y) * slope0) * GMT->current.setting.ps_dpi);
#ifdef DEBUG
				GMT_report (GMT, GMT_MSG_DEBUG, "apex_y>edge_y iy %ld x1 %ld x2 %ld\n",iy,x1,x2);
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
void segyz_shade_bmap (struct GMT_CTRL *GMT, double x0, double y0, float data0, float data1, double z0, double z1, GMT_LONG negative, double dev_x, double dev_y, unsigned char *bitmap, GMT_LONG bm_nx, GMT_LONG bm_ny) /* apply current samples with all options to bitmap */
{
	GMT_LONG ix, iy;
	double xp[NPTS], yp[NPTS], interp, slope01, slope02, slope12, slope13, slope23, slope03;
	double slope0, slope1, slope2, slope3;

	if (data0 == 0.0 && data1 == 0.0) return; /* probably shouldn't strictly, but pathological enough I dont really want to deal with it! */

	interp = 0.0;
	if ((data0 * data1) < 0.0) {
		/* points to plot are on different sides of zero - interpolate to find out where zero is */
		interp = z0 + data0 * ((z0 - z1) / (data1 - data0));
		if (((data0 < 0.0) && negative) || ((data0 > 0.0)&& !negative)) {
			/* plot from top to zero */
			z1 = interp;
			data1 = 0.0;
		}
		else {
			z0 = interp;
			data0 = 0.0;
		}
	}

	GMT_geoz_to_xy (GMT, x0+(double)data0*dev_x, y0+(double)data0*dev_y, z0, &xp[0], &yp[0]); /* returns 2 ends of line segment in plot coords */
	GMT_geoz_to_xy (GMT, x0+(double)data1*dev_x, y0+(double)data1*dev_y, z1, &xp[1], &yp[1]);
	GMT_geoz_to_xy (GMT, x0, y0, z0, &xp[2], &yp[2]); /* to get position of zero at each point*/
	GMT_geoz_to_xy (GMT, x0, y0, z1, &xp[3], &yp[3]); /* to get position of zero at each point*/

	/* now have four corner coordinates - need to sort them */
	for (ix = 0; ix < NPTS-1; ix++)
		for (iy = ix + 1; iy < NPTS; iy++)
			if (yp[ix] > yp[iy]) {
				d_swap (yp[iy], yp[ix]);
				d_swap (xp[iy], xp[ix]);
			}


	/* have to fill the quadrilateral defined by 4 points (now ordered, but care with degenerate cases)*/

	slope01 = (xp[1] - xp[0]) / (yp[1] - yp[0]);
	slope02 = (xp[2] - xp[0]) / (yp[2] - yp[0]);
	slope12 = (xp[2] - xp[1]) / (yp[2] - yp[1]);
	slope13 = (xp[3] - xp[1]) / (yp[3] - yp[1]);
	slope23 = (xp[3] - xp[2]) / (yp[3] - yp[2]);
	slope03 = (xp[3] - xp[0]) / (yp[3] - yp[0]);
	if ((yp[0] != yp[1]) && (yp[2] != yp[3])) {	/* simple case: tri-quad-tri */
		shade_tri (GMT, xp[0], yp[0], yp[1], slope01, slope02, bitmap, bm_nx, bm_ny);
		shade_quad (GMT, xp[1], yp[1],xp[0]+slope02*(yp[1]-yp[0]), yp[2], slope02, slope13, bitmap, bm_nx, bm_ny);
		shade_tri (GMT, xp[3], yp[3], yp[2], slope13, slope23, bitmap, bm_nx, bm_ny);
	}
	if ((yp[0] == yp[1]) && (yp[2] != yp[3])) {
		if (xp[0] == xp[1]) { /* two triangles based on yp[1],yp[2]. yp[3] */
			shade_tri (GMT, xp[1], yp[1], yp[2], slope12, slope13, bitmap, bm_nx, bm_ny);
			shade_tri (GMT, xp[3], yp[3], yp[2], slope23, slope13, bitmap, bm_nx, bm_ny);
		} else { /* quad based on first 3 points, then tri */
			slope0 = (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1])&&(xp[3]>xp[2])))*slope03 + (((xp[0]<xp[1])&&(xp[2]<xp[3])) || ((xp[0]>xp[1])&&(xp[2]>xp[3])))*slope02;
			slope1 = (((xp[1]<xp[0]) && (xp[3]<xp[2])) || ((xp[1]>xp[0]) && (xp[3]>xp[2])))*slope13 + (((xp[1]<xp[0])&&(xp[2]<xp[3])) || ((xp[1]>xp[0])&&(xp[2]>xp[3])))*slope12;
			slope3 = (((xp[1]<xp[0]) && (xp[3]<xp[2])) || ((xp[1]>xp[0]) && (xp[3]>xp[2])))*slope13 + (((xp[0]<xp[1])&&(xp[3]<xp[2])) || ((xp[0]>xp[1])&&(xp[3]>xp[2])))*slope03;
			shade_quad (GMT, xp[0], yp[0], xp[1], yp[2], slope0, slope1, bitmap, bm_nx, bm_ny);
			shade_tri (GMT, xp[3], yp[3], yp[2], slope23, slope3, bitmap, bm_nx, bm_ny);
		}
	}
	if ((yp[0] != yp[1]) && (yp[2] == yp[3])) {
		if (xp[2] == xp[3]) {/* two triangles based on yp[0],yp[1]. yp[2] */
		shade_tri (GMT, xp[0], yp[0], yp[1], slope01, slope02, bitmap, bm_nx, bm_ny);
		shade_tri (GMT, xp[2], yp[2], yp[1], slope12, slope02, bitmap, bm_nx, bm_ny);
		} else { /* triangle based on yp[0], yp[1], then quad based on last 3 points */
			slope0 = (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1]) && (xp[3]>xp[2])))*slope03 + (((xp[0]<xp[1])&&(xp[2]<xp[3])) || ((xp[0]>xp[1])&&(xp[2]>xp[3])))*slope02;
			shade_tri (GMT, xp[0], yp[0], yp[1], slope01, slope0, bitmap, bm_nx, bm_ny);
			slope2 = (((xp[0]<xp[1]) && (xp[2]<xp[3])) || ((xp[0]>xp[1]) && (xp[2]>xp[3])))*slope02 + (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1]) && (xp[3]>xp[2])))*slope12;
			slope3 = (((xp[0]<xp[1]) && (xp[3]<xp[2])) || ((xp[0]>xp[1]) && (xp[3]>xp[2])))*slope03 + (((xp[0]<xp[1]) && (xp[2]<xp[3])) || ((xp[0]>xp[1]) && (xp[2]>xp[3])))*slope13;
			shade_quad (GMT, xp[2], yp[2], xp[3], yp[1], slope2, slope3, bitmap, bm_nx, bm_ny);
		}
	}
}

void segyz_plot_trace (struct GMT_CTRL *GMT, float *data, double dz, double x0, double y0, GMT_LONG n_samp, GMT_LONG do_fill, GMT_LONG negative, GMT_LONG plot_wig, float toffset, double dev_x, double dev_y, unsigned char *bitmap, GMT_LONG  bm_nx, GMT_LONG bm_ny)
	/* shell function to loop over all samples in the current trace, determine plot options
	 * and call the appropriate bitmap routine */
{
	GMT_LONG iz, paint_wiggle;
	float z0 = (float)GMT->common.R.wesn[ZLO], z1;

	for (iz = 1; iz < n_samp; iz++) {	/* loop over samples on trace - refer to pairs iz-1, iz */
		z1 = (float )(dz * (float) iz + toffset);
		if (z1 >= GMT->common.R.wesn[ZLO] && z1 <= GMT->common.R.wesn[ZHI]) {	/* check within z bounds specified */
#ifdef DEBUG
			GMT_report (GMT, GMT_MSG_DEBUG, "x0, %f\t y0, %f\t,z1, %f\t data[iz], %f\t iz, %ld\n", x0, y0, z1, data[iz], iz);
#endif
			if (plot_wig) wig_bmap (GMT, x0, y0, data[iz-1],data[iz], z0, z1, dev_x, dev_y,bitmap, bm_nx, bm_ny);	/* plotting wiggle */
			if (do_fill) {	/* plotting VA -- check data points first */
				paint_wiggle = ((!negative && ((data[iz-1] >= 0.0) || (data[iz] >= 0.0))) || (negative && ((data[iz-1] <= 0.0) || (data[iz] <= 0.0))));
				if (paint_wiggle) segyz_shade_bmap (GMT, x0, y0, data[iz-1], data[iz], z0, z1, negative, dev_x, dev_y, bitmap, bm_nx, bm_ny);
			}
			z0 = z1;
		}
	}
}

#define Return(code) {Free_pssegyz_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_pssegyz (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, nm, ix, iz, n_samp = 0, check, bm_nx, bm_ny;

	double xlen, ylen, xpix, ypix, x0, y0, trans[3] = {-1.0,-1.0,-1.0};

	float scale = 1.0, toffset = 0.0;

	char *head = NULL;
	unsigned char *bitmap = NULL;
	int head2;

	char reelhead[3200];
	float *data = NULL;
	FILE *fpi = NULL;
	SEGYHEAD *header = NULL;
	SEGYREEL binhead;

	struct PSSEGYZ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;				/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_pssegyz_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_pssegyz_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pssegyz", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJR", "BKOPUXYcpt>", options))) Return (error);
	Ctrl = (struct PSSEGYZ_CTRL *)New_pssegyz_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pssegyz_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the pssegyz main code ----------------------------*/

	if (!GMT_IS_LINEAR (GMT)) GMT_message (GMT, "Warning: you asked for a non-rectangular projection. \n It will probably still work, but be prepared for problems\n");

	if (Ctrl->In.active) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Will read segy file %s\n", Ctrl->In.file);
		if ((fpi = fopen (Ctrl->In.file, "rb")) == NULL) {
			GMT_message (GMT, "Cannot find segy file %s\n", Ctrl->In.file);
			Return (EXIT_FAILURE);
		}
	}
	else {
		GMT_report (GMT, GMT_MSG_NORMAL, "Will read segy file from standard input\n");
		if (fpi == NULL) fpi = stdin;
	}

	/* set up map projection and PS plotting */
	if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);
	GMT_plotinit (API, PSL, options);

	/* define area for plotting and size of array for bitmap */
	xlen = GMT->current.proj.rect[XHI] - GMT->current.proj.rect[XLO];
	xpix = xlen * GMT->current.setting.ps_dpi; /* pixels in x direction */
	bm_nx = (GMT_LONG) ceil (xpix / 8.0); /* store 8 pixels per byte in x direction but must have
				whole number of bytes per scan */
	ylen = GMT->current.proj.rect[YHI] - GMT->current.proj.rect[YLO];
	ypix = ylen * GMT->current.setting.ps_dpi; /* pixels in y direction */
	bm_ny = (GMT_LONG) ypix;
	nm = bm_nx * bm_ny;

	if ((check = get_segy_reelhd (fpi, reelhead)) != TRUE) exit (1);
	if ((check = get_segy_binhd (fpi, &binhead)) != TRUE) exit (1);

	if (Ctrl->A.active) {
/* this is a little-endian system, and we need to byte-swap ints in the reel header - we only
use a few of these*/
		GMT_report (GMT, GMT_MSG_NORMAL, "Swapping bytes for ints in the headers\n");
		binhead.num_traces = GMT_swab2 (binhead.num_traces);
		binhead.nsamp = GMT_swab2 (binhead.nsamp);
		binhead.dsfc = GMT_swab2 (binhead.dsfc);
		binhead.sr = GMT_swab2 (binhead.sr);
	}

/* set parameters from the reel headers */
	if (!Ctrl->M.value) Ctrl->M.value = binhead.num_traces;

	GMT_report (GMT, GMT_MSG_NORMAL, "Number of traces in header is %ld\n", Ctrl->M.value);


	if (!Ctrl->L.value) {/* number of samples not overridden*/
		Ctrl->L.value = binhead.nsamp;
		GMT_message (GMT, "Number of samples per trace is %ld\n", Ctrl->L.value);
	}
	else if ((Ctrl->L.value != binhead.nsamp) && (binhead.nsamp))
		GMT_message (GMT, "warning nsampr input %ld, nsampr in header %d\n", Ctrl->L.value, binhead.nsamp);

	if (!Ctrl->L.value) { /* no number of samples still - a problem! */
		GMT_message (GMT, "Error, number of samples per trace unknown\n");
		exit (EXIT_FAILURE);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Number of samples is %ld\n", n_samp);

	if (binhead.dsfc != 5) GMT_message (GMT, "Warning: data not in IEEE format\n");

	if (!Ctrl->Q.value[Z_ID]) {
		Ctrl->Q.value[Z_ID] = binhead.sr; /* sample interval of data (microseconds) */
		Ctrl->Q.value[Z_ID] /= 1000000.0;
		GMT_message (GMT, "Sample interval is %f s\n", Ctrl->Q.value[Z_ID]);
	}
	else if ((Ctrl->Q.value[Z_ID] != binhead.sr) && (binhead.sr)) /* value in header overridden by input */
		GMT_message (GMT, "Warning dz input %f, dz in header %f\n", Ctrl->Q.value[Z_ID], (float)binhead.sr);

	if (!Ctrl->Q.value[Z_ID]) { /* still no sample interval at this point is a problem! */
		GMT_message (GMT, "Error, no sample interval in reel header\n");
		exit (EXIT_FAILURE);
	}

	bitmap = GMT_memory (GMT, NULL, nm, unsigned char);

	ix = 0;
	while ((ix < Ctrl->M.value) && (header = get_segy_header (fpi))) {   /* read traces one by one */
		/* check true location header for x */
		if (Ctrl->S.mode[GMT_X] == PLOT_OFFSET) { /* plot traces by offset, cdp, or input order */
			int32_t offset = ((Ctrl->A.active)? GMT_swab4 (header->sourceToRecDist) : header->sourceToRecDist);
			x0 = (double) offset;
		}
		else if (Ctrl->S.mode[GMT_X] == PLOT_CDP) {
			int32_t cdpval = ((Ctrl->A.active)? GMT_swab4 (header->cdpEns) : header->cdpEns);
			x0 = (double) cdpval;
		}
		else if (Ctrl->S.value[GMT_X]) { /* ugly code - want to get value starting at Ctrl->S.value[GMT_X] of header into a double... */
			head = (char *) header;
			memcpy (&head2, &head[Ctrl->S.value[GMT_X]], 4); /* edited to fix bug where 8bytes were copied from head.
                                                Caused by casting to a long directly from char array*/
			x0 = (double) ((Ctrl->A.active)? GMT_swab4 (head2) : head2);
		}
		else if (Ctrl->S.fixed[GMT_X])
			x0 = Ctrl->S.orig[GMT_X] / Ctrl->Q.value[X_ID];
		else
			x0 = (1.0 + (double) ix); /* default x to input trace number */

		/* now do same for y */
		if (Ctrl->S.mode[GMT_Y] == PLOT_OFFSET) { /* plot traces by offset, cdp, or input order */
			int32_t offset = ((Ctrl->A.active)? GMT_swab4 (header->sourceToRecDist) : header->sourceToRecDist);
			y0 = (double) offset;
		}
		else if (Ctrl->S.mode[GMT_Y] == PLOT_CDP) {
			int32_t cdpval = ((Ctrl->A.active)? GMT_swab4 (header->cdpEns) : header->cdpEns);
			y0 = (double) cdpval;
		}
		else if (Ctrl->S.value[GMT_Y]) {
			head =  (char *) header;
			memcpy (&head2, &head[Ctrl->S.value[GMT_Y]], 4); /* edited to fix bug where 8bytes were copied from head.
                                                Caused by casting to a long directly from char array*/
			y0 = (double) ((Ctrl->A.active)? GMT_swab4 (head2) : head2);
		}
		else if (Ctrl->S.fixed[GMT_Y])
			y0  = Ctrl->S.orig[GMT_Y] / Ctrl->Q.value[X_ID];
		else
			y0 = GMT->common.R.wesn[YLO] / Ctrl->Q.value[X_ID]; /* default y to s edge of projection */

		x0 *= Ctrl->Q.value[X_ID];
		y0 *= Ctrl->Q.value[X_ID]; /* scale x and y by the input Ctrl->Q.value[X_ID] scalar */

		if (Ctrl->A.active) {
			/* need to permanently byte-swap some things in the trace header
			   do this after getting the location of where traces are plotted in case the general Ctrl->S.value[GMT_X] case
			   overlaps a defined header in a strange way */
			header->sourceToRecDist = GMT_swab4 (header->sourceToRecDist);
			header->sampleLength = GMT_swab2 (header->sampleLength);
			header->num_samps = GMT_swab4 (header->num_samps);
		}

		GMT_report (GMT, GMT_MSG_NORMAL, "trace %ld at x=%f, y=%f \n", ix+1, x0, y0);

		if (Ctrl->Q.value[U_ID]) {
			toffset = (float) -(fabs ((double)(header->sourceToRecDist)) / Ctrl->Q.value[U_ID]);
			GMT_report (GMT, GMT_MSG_NORMAL, "time shifted by %f\n", toffset);
		}

		data = (float *) get_segy_data (fpi, header);	/* read a trace */
		/* get number of samples in _this_ trace (e.g. OMEGA has strange ideas about SEGY standard)
		   or set to number in reel header */
                if (!(n_samp = samp_rd (header))) n_samp = Ctrl->L.value;

		if (Ctrl->A.active) {	/* need to swap the order of the bytes in the data even though assuming IEEE format */
			int *intdata = (int *) data;
			for (iz = 0; iz < n_samp; iz++) intdata[iz] = GMT_swab4 (intdata[iz]);
		}

		if (Ctrl->N.active || Ctrl->Z.active) {
			scale= (float) segyz_rms (data, n_samp);
			GMT_report (GMT, GMT_MSG_NORMAL, "\t\t rms value is %f\n", scale);
		}
		for (iz = 0; iz < n_samp; iz++) { /* scale bias and clip each sample in the trace */
			if (Ctrl->N.active) data[iz] /= scale;
			data[iz] += (float)Ctrl->Q.value[B_ID];
			if (Ctrl->C.active && (fabs (data[iz]) > Ctrl->C.value)) data[iz] = (float)(Ctrl->C.value*data[iz] / fabs (data[iz])); /* apply bias and then clip */
		}

		if (!Ctrl->Z.active || scale) segyz_plot_trace (GMT, data, Ctrl->Q.value[Z_ID], x0, y0, n_samp, Ctrl->F.active, Ctrl->I.active, Ctrl->W.active, toffset, Ctrl->D.value[GMT_X], Ctrl->D.value[GMT_Y], bitmap, bm_nx, bm_ny);
		free (data);
		free (header);
		ix++;
	}

	/* map_clip_on (-1, -1, -1, 3); */
	/* set a clip at the map boundary since the image space overlaps a little */
	PSL_plotbitimage (PSL, 0.0, 0.0, xlen, ylen, 1, bitmap, 8*bm_nx, bm_ny, trans, Ctrl->F.rgb);

	/* map_clip_off ();*/

	if (fpi != stdin) fclose (fpi);

	/*ps_rotatetrans (GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin, 0.0);*/
	GMT_plotend (GMT, PSL);

	GMT_free (GMT, bitmap);

	Return (EXIT_SUCCESS);
}
