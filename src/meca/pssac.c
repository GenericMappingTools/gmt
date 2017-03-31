/*--------------------------------------------------------------------
 *
 *  Copyright (c) 2016-2017 by Dongdong Tian
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 3 or any later version.
 *
 *  This program is distrdeibuted in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  Contact info: seisman.info@gmail.com
 *--------------------------------------------------------------------*/

/*
 * Brief synopsis: pssac will plot seismograms in SAC format on maps
 */

/* 
The SAC I/O functions are initially written by Prof. Lupei Zhu,
and modified by Dongdong Tian.

Original License:

Permission to use, copy, modify, and distribute the package
and supporting documentation for any purpose without fee is hereby granted,
provided that the above copyright notice appear in all copies, that both
that copyright notice and this permission notice appear in supporting documentation.
*/

#include "gmt_dev.h"
#include "sacio.h"

#define THIS_MODULE_NAME	"pssac"
#define THIS_MODULE_LIB		"meca"
#define THIS_MODULE_PURPOSE	"Plot seismograms in SAC format on maps"
#define THIS_MODULE_KEYS	">XO,RG-"
#define THIS_MODULE_NEEDS	"RJ"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYht" GMT_OPT("c")

/* Control structure for pssac */

struct PSSAC_CTRL {
	struct PSSAC_In {   /* Input files */
		bool active;
		char **file;
		unsigned int n;
	} In;
	struct PSSAC_C {    /* -C<t0>/<t1> */
		bool active;
		double t0, t1;
	} C;
	struct PSSAC_D {	/* -D<dx>/<dy> */
		bool active;
		double dx, dy;
	} D;
	struct PSSAC_E {    /* -Ea|b|d|k|n<n>|u<n> */
		bool active;
		char keys[GMT_LEN256];
	} E;
	struct PSSAC_F {    /* -Fiqr */
		bool active;
		char keys[GMT_LEN256];
	} F;
	struct PSSAC_G {    /* -G[p|n]+g<fill>+z<zero>+t<t0>/<t1> */
		bool active[2];
		struct GMT_FILL fill[2];
		float zero[2];
		bool cut[2];
		float t0[2];
		float t1[2];
	} G;
	struct PSSAC_M {    /* -M<size>/<alpha> */
		bool active;
		double size;
		bool norm;      /* true if -M<size> */
		bool scaleALL;  /* true if alpha=0 */
		double alpha;
		bool dist_scaling; /* true if alpha>=0 */
	} M;
	struct PSSAC_Q {	/* -Q */
		bool active;
	} Q;
	struct PSSAC_S {	/* -S<sec_per_measure> */
		bool active;
		double sec_per_measure;
	} S;
	struct PSSAC_T {   /* -T+t<n>+r<reduce_vel>+s<shift> */
		bool active;
		bool align;
		int tmark;
		bool reduce;
		double reduce_vel;
		double shift;
	} T;
	struct PSSAC_W {	/* -W<pen> */
		struct GMT_PEN pen;
	} W;
};

struct SAC_LIST {
	char *file;
	bool position;
	double x;
	double y;
	bool custom_pen;
	struct GMT_PEN pen;
};


GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSAC_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSSAC_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->W.pen = GMT->current.setting.map_default_pen;

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSSAC_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_freepen (GMT, &C->W.pen);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pssac synopsis and optionally full usage information */

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pssac [<saclist>|<SACfiles>] %s %s\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-C[<t0>/<t1>]] [-D<dx>[/<dy>]] [-Ea|b|k|d|n[<n>]|u[<n>]] [-F[i][q][r]]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G[p|n][+g<fill>][+t<t0>/<t1>][+z<zero>]] [-K] [-M<size>[<unit>]/<alpha>] [-O] [-P] [-Q]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<sec_per_measure>[<unit>]] [-T[+t<tmark>][+r<reduce_vel>][+s<shift>]] \n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W<pen>]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_h_OPT, GMT_t_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\n");

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<SACfiles> are the name of SAC files to plot on maps. Only evenly spaced SAC data is supported.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<saclist> is an ASCII file (or stdin) which contains the name of SAC files to plot and controlling parameters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Each record has 1, 3 or 4 items:  <filename> [<X> <Y> [<pen>]]. \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <filename> is the name of SAC file to plot.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <X> and <Y> are the position of seismograms to plot on a map.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      On linear plots, the default <X> is the begin time of SAC file, which will be adjusted if -T option is used, \n");
	GMT_Message (API, GMT_TIME_NONE, "\t      the default <Y> is determined by **-E** option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      On geographic plots, the default <X> and <Y> are station longitude and latitude specified in SAC header.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      The <X> and <Y> given here will override the position determined by command line options.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <pen> is given, it will override the pen from -W option for current SAC file only.\n");
	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Read and plot seismograms in timewindow between <t0> and <t1> only. <t0> and <t1>* are relative to a reference time specified by -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T option is not specified, use the reference time (kzdate and kztime) defined in SAC header instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default to read and plot the whole trace. If only -C is used, t0 and t1 are determined from -R option\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Offset all traces by <dx>/<dy>. If <dy> is not given it is set equal to <dx>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Choose profile type (the type of Y axis). \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a: azimuth profile\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   b: back-azimuth profile\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   k: epicentral distance (in km) profile\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d: epicentral distance (in degree) profile \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n: trace number profile. The <Y> position of first trace is numbered as <n> [Default <n> is 0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   u: user defined profile. The <Y> positions are determined by SAC header variable user<n>, default using user0.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Data preprocessing before plotting.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i: integral\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   q: square\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r: remove mean value\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i|q|r can repeat multiple times. -Frii will convert accerate to displacement.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The order of i|q|r controls the order of the data processing.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Paint positive or negative portion of traces.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If only -G is used, default to fill the positive portion black.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [p|n] controls the painting of positive portion or negative portion. Repeat -G option to specify fills for pos/neg portion, respectively.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +g<fill>: color to fill\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +t<t0>/<t1>: paint traces between t0 and t1 only. The reference time of t0 and t1 is determined by -T option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +z<zero>: define zero line. From <zero> to top is positive portion, from <zero> to bottom is negative portion.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Vertical scaling\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <size>: each trace will scaled to <size>[u]. The default unit is PROJ_LENGTH_UNIT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      The scale factor is defined as yscale = size*(north-south)/(depmax-depmin)/map_height \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <size>/<alpha>: \n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <alpha> < 0, use the same scaling factor for all traces. The scaling factor will scale the first trace to <size>[<u>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <alpha> = 0, multiply all traces by <size>. No unit is allowed. \n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <alpha> > 0, multiply all traces by size*r^alpha, r is the distance range in km.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Plot traces vertically.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Specify the time scale in seconds per <unit> while plotting on geographic plots. Use PROJ_LENGTH_UNIT if <unit> is omitted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Time alignment. \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +t<tmark> align all trace along time mark. Choose <tmark> from -5(b), -4(e), -3(o), -2(a), 0-9(t0-t9).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +r<reduce_vel> reduce velocity in km/s.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +s<shift> shift all traces by <shift> seconds.\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', "Set pen attributes [Default pen is %s]:", 0);
	GMT_Option (API, "X,h,t");
	GMT_Option (API, ".");

	return (EXIT_FAILURE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSSAC_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pssac and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos = 0;
	int j, k;
	size_t n_alloc = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, p[GMT_BUFSIZ] = {""};
	char path[GMT_BUFSIZ] = {""};	/* Full path to sac file */
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Collect input files */
				Ctrl->In.active = true;
				if (n_alloc <= Ctrl->In.n)  Ctrl->In.file = gmt_M_memory (GMT, Ctrl->In.file, n_alloc += GMT_SMALL_CHUNK, char *);
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
					if (gmt_getdatapath (GMT, opt->arg, path, R_OK) == NULL) {
						GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open file %s.\n", opt->arg);
						continue;
					}
					Ctrl->In.file[Ctrl->In.n++] = strdup (path);
				} else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				if ((j = sscanf (opt->arg, "%lf/%lf", &Ctrl->C.t0, &Ctrl->C.t1)) != 2) {
					Ctrl->C.t0 = GMT->common.R.wesn[XLO];
					Ctrl->C.t1 = GMT->common.R.wesn[XHI];
				}
				break;
			case 'D':
				if ((j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b)) < 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: Give x [and y] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.active = true;
					Ctrl->D.dx = gmt_M_to_inch (GMT, txt_a);
					Ctrl->D.dy = (j == 2) ? gmt_M_to_inch (GMT, txt_b) : Ctrl->D.dx;
				}
				break;
			case 'E':
				Ctrl->E.active = true;
				strncpy(Ctrl->E.keys, &opt->arg[0], GMT_LEN256-1);
				break;
			case 'F':
				Ctrl->F.active = true;
				strncpy(Ctrl->F.keys, &opt->arg[0], GMT_LEN256-1);
				break;
			case 'G':      /* phase painting */
				switch (opt->arg[0]) {
					case 'p': j = 1, k = 0; break;
					case 'n': j = 1, k = 1; break;
					default : j = 0, k = 0; break;
				}
				Ctrl->G.active[k] = true;
				pos = j;
				while (gmt_getmodopt (GMT, 'G', opt->arg, "gtz", &pos, p, &n_errors) && n_errors == 0) {
					switch (p[0]) {
						case 'g':  /* fill */
							if (gmt_getfill (GMT, &p[1], &Ctrl->G.fill[k])) {
								GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -G+g<fill> option.\n");
								n_errors++;
							}
							break;
						case 'z':  /* zero */
							Ctrl->G.zero[k] = (float)atof (&p[1]);
							break;
						case 't':  /* +t<t0>/<t1> */
							Ctrl->G.cut[k] = true;
							if (sscanf (&p[1], "%f/%f", &Ctrl->G.t0[k], &Ctrl->G.t1[k]) != 2) {
								GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -G+t<t0>/<t1> option.\n");
								n_errors++;
							}
							break;
						default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
					}
				}
				break;
			case 'M':
				Ctrl->M.active = true;
				j = sscanf(opt->arg, "%[^/]/%s", txt_a, txt_b);
				if (j == 1) { /* -Msize */
					Ctrl->M.norm = true;
					Ctrl->M.size = gmt_M_to_inch (GMT, txt_a);
				}
				else if (j == 2) {
					if (strcmp(txt_b, "s") == 0 ) {   /* -Msize/s */
						// TODO
					} 
					else if (strcmp(txt_b, "b") == 0) {  /* -Msize/b */
						// TODO
					}
					else {  /* -Msize/alpha */
						Ctrl->M.alpha = atof (txt_b);
						if (Ctrl->M.alpha < 0) {
							Ctrl->M.scaleALL = true;
							Ctrl->M.size = gmt_M_to_inch (GMT, txt_a);
						}
						else {
							Ctrl->M.dist_scaling = true;
							Ctrl->M.size = atof (txt_a);
						}
					}
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -M option: -M<size>[/<alpha>]\n");
					n_errors++;
				}

				if (gmt_M_is_linear(GMT) && (Ctrl->M.norm || Ctrl->M.scaleALL))
					Ctrl->M.size *= fabs((GMT->common.R.wesn[YHI]-GMT->common.R.wesn[YLO])/GMT->current.proj.pars[1]);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;
			case 'S':
				Ctrl->S.active = true;
				Ctrl->S.sec_per_measure = gmt_M_to_inch (GMT, opt->arg);
				break;
			case 'T':
				pos = 0;
				Ctrl->T.active = true;
				Ctrl->T.shift = 0.0;  /* default no shift */
				while (gmt_getmodopt (GMT, 'T', opt->arg, "trs", &pos, p, &n_errors) && n_errors == 0) {
					switch (p[0]) {
						case 't':
							Ctrl->T.align = true;
							Ctrl->T.tmark = atoi (&p[1]);
							break;
						case 'r':
							Ctrl->T.reduce = true;
							Ctrl->T.reduce_vel = atof (&p[1]);
							break;
						case 's':
							Ctrl->T.shift = atof (&p[1]);
							break;
						default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
					}
				}
				break;
			case 'W':		/* Set line attributes */
				if (opt->arg[0] && gmt_getpen (GMT, &opt->arg[0], &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:", 3);
					n_errors++;
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && gmt_M_is_zero(Ctrl->S.sec_per_measure), "Syntax error -S option: <sec_per_measure> must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.reduce && gmt_M_is_zero(Ctrl->T.reduce_vel), "Syntax error -T option: <reduce_vel> must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.align && !(Ctrl->T.tmark >= -5 && Ctrl->T.tmark <= 9 && Ctrl->T.tmark != -1), "Syntax error -T option: <tmark> should be chosen from -5, -4, -3, -2, 0-9\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LOCAL double linear_interpolate_x (double x0, double y0, double x1, double y1, double y) {
	if (doubleAlmostEqualZero(y0, y1)) return x0;
	return (x1-x0)/(y1-y0)*(y-y0) + x0;
}

#if 0 /* Not used yet */
GMT_LOCAL double linear_interpolate_y (double x0, double y0, double x1, double y1, double x) {
	if (x<x0 || x>x1) return y1;  // no extrapolation
	if (doubleAlmostEqualZero(x0, x1)) return y0;
	return (y1-y0)/(x1-x0)*(x-x0) + y0;
}
#endif

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL void paint_phase(struct GMT_CTRL *GMT, struct PSSAC_CTRL *Ctrl, struct PSL_CTRL *PSL, double *x, double *y, int n, double zero, double t0, double t1, int mode) {
	/* mode=0: paint positive phase */
	/* mode=1: paint negative phase */
	int i, ii;
	double *xx = NULL, *yy = NULL;

	if (Ctrl->Q.active) {
		double *tmp = x;
		x = y;
		y = tmp;
	}

	xx = gmt_M_memory (GMT, 0, n+2, double);
	yy = gmt_M_memory (GMT, 0, n+2, double);

	for (i = 0; i < n; i++) {
		if ((x[i] >= t0) && ((mode == 0 && y[i] >= zero) || (mode == 1 && y[i] <= zero))) {
			double *xp = NULL, *yp = NULL;
			int npts;

			ii = 0;
			/* first point of polygon */
			yy[ii] = zero;
			if (i == 0)
				xx[ii] = x[i];
			else
				xx[ii] = linear_interpolate_x(x[i-1], y[i-1], x[i], y[i], yy[ii]);
			ii++;

			while((i < n) && (x[i] <= t1) && ((mode == 0 && y[i] >= zero) || (mode == 1 && y[i] <= zero))) {
				xx[ii] = x[i];
				yy[ii] = y[i];
				i++;
				ii++;
			}

			/* last point of polygon */
			yy[ii] = zero;
			if (i == n || x[i] > t1)
				xx[ii] = x[i-1];
			else
				xx[ii] = linear_interpolate_x(x[i], y[i], x[i-1], y[i-1], yy[ii]);
			ii++;

			if (Ctrl->Q.active) {
				double *tmp = xx;
				xx = yy;
				yy = tmp;
			}

			if (gmt_M_is_linear(GMT)) {
				if ((GMT->current.plot.n = gmt_geo_to_xy_line(GMT, xx, yy, ii)) < 3) continue;
				xp = GMT->current.plot.x;
				yp = GMT->current.plot.y;
				npts = (int)GMT->current.plot.n;
			}
			else {
				xp = xx;
				yp = yy;
				npts = ii;
			}
			gmt_setfill(GMT, &Ctrl->G.fill[mode], false);

			PSL_plotpolygon(PSL, xp, yp, npts);
		}
	}
	gmt_M_free (GMT, xx);
	gmt_M_free (GMT, yy);
}

GMT_LOCAL void integral (double *y, double delta, int n) {
	int i;

	y[0] = (y[0] + y[1]) * delta / 2.0;
	for (i=1; i<n-1; i++)
		y[i] = y[i-1] + (y[i] + y[i+1]) * delta / 2.0;
}

GMT_LOCAL void rmean (double *y, int n) {
	int i;
	double depmen = 0.0;
	for (i = 0; i < n; i++) depmen += y[i];
	depmen /= n;

	for (i = 0; i < n; i++) y[i] -= depmen;
}

GMT_LOCAL void sqr (double *y, int n) {
	int i;
	for (i = 0; i < n; i++) y[i] *= y[i];
}

GMT_LOCAL int init_sac_list (struct GMT_CTRL *GMT, char **files, unsigned int n_files, struct SAC_LIST **list) {
	unsigned int n = 0, nr;
	char path[GMT_BUFSIZ] = {""};	/* Full path to sac file */
	struct SAC_LIST *L = NULL;

	/* Got a bunch of SAC files or one file in SAC format */
	if (n_files > 1 || (n_files == 1 && issac(files[0]))) {
		L = gmt_M_memory (GMT, NULL, n_files, struct SAC_LIST) ;
		for (n = 0; n < n_files; n++) {
			L[n].file = strdup (files[n]);
			L[n].position = false;
			L[n].custom_pen = false;
		}
	}
	else {    /* Must read a list file */
		size_t n_alloc = 0;
		char *line = NULL, pen[GMT_LEN256] = {""}, file[GMT_LEN256] = {""};
		double x, y;
		gmt_set_meminc (GMT, GMT_SMALL_CHUNK);
		do {
			if ((line = GMT_Get_Record(GMT->parent, GMT_READ_TEXT, NULL)) == NULL) {
				if (gmt_M_rec_is_error (GMT))   /* Bail if there are any read error */
					return (GMT_RUNTIME_ERROR);
				if (gmt_M_rec_is_any_header (GMT)) /* skip headers */
					continue;
				if (gmt_M_rec_is_eof(GMT))  /* Reached end of file */
					break;
			}
			if (line == NULL) {	/* Crazy safety valve but it should never get here*/
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal error: input pointer is NULL where it should not be, aborting\n");
				return (GMT_PTR_IS_NULL);
			}

			nr = sscanf (line, "%s %lf %lf %s", file, &x, &y, pen);
			if (nr < 1) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Read error for sac list file near row %d\n", n);
				return (EXIT_FAILURE);
			}

			if (n == n_alloc) L = gmt_M_malloc (GMT, L, n, &n_alloc, struct SAC_LIST);

			if (gmt_getdatapath (GMT, file, path, R_OK) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot find/open file %s.\n", file);
				continue;
			}
			L[n].file = strdup (path);
			if (nr >= 3) {
				L[n].position = true;
				L[n].x = x;
				L[n].y = y;
			}
			if (nr == 4) {
				L[n].custom_pen = true;
				if (gmt_getpen (GMT, pen, &L[n].pen)) {
					gmt_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:", 3);
				}
			}
			n++;
		} while(true);
		gmt_reset_meminc (GMT);
		n_files = n;
	}
	*list = L;

	return n_files;
}

int GMT_pssac (void *V_API, int mode, void *args) {	/* High-level function that implements the pssac task */
	bool old_is_world, free_plot_pen = false, read_from_ascii;
	unsigned int n_files, *plot_pen = NULL;
	int error = GMT_NOERROR, n, i, npts;
	double yscale = 1.0, y0 = 0.0, x0, tref, dt, *x = NULL, *y = NULL, *xp = NULL, *yp = NULL;
	float tmark_value, *data = NULL;
	struct SAC_LIST *L = NULL;
	SACHEAD hd;
	struct GMT_PEN current_pen;
	struct PSSAC_CTRL *Ctrl = NULL;
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);

	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pssac main code ----------------------------*/

	current_pen = Ctrl->W.pen;

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	gmt_setpen (GMT, &current_pen);

	if (Ctrl->D.active) PSL_setorigin (PSL, Ctrl->D.dx, Ctrl->D.dy, 0.0, PSL_FWD);	/* Shift plot a bit */

	old_is_world = GMT->current.map.is_world;

	read_from_ascii = (Ctrl->In.n == 0) || (Ctrl->In.n == 1 && !issac(Ctrl->In.file[0]));
	if (read_from_ascii) {      /* Got a ASCII file or read from stdin */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reading from saclist file or stdin\n");
		if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {    /* Register data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {  /* Enables data input and sets access mode */
			Return (API->error);
		}
	}
	n_files = init_sac_list (GMT, Ctrl->In.file, Ctrl->In.n, &L);

	if (read_from_ascii && GMT_End_IO (API, GMT_IN, 0) != GMT_OK) { /* Disables further data input */
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Collecting %ld SAC files to plot.\n", n_files);

	for (n = 0; n < (int)n_files; n++) {  /* Loop over all SAC files */
 		GMT_Report (API, GMT_MSG_VERBOSE, "Plotting SAC file %d: %s\n", n, L[n].file);
		
		/* -T: determine the reference time for all times in pssac */
		tref = 0.0;
		if (Ctrl->T.active) {
			/* read SAC header only to determine the reference time */
 			if ((read_sac_head (L[n].file, &hd))) {  /* skip or not */
 				GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: unable to read, skipped.\n", L[n].file);
				continue;
			}
			/* +t */
			if (Ctrl->T.align) {
				tmark_value = *((float *)&hd + TMARK + Ctrl->T.tmark);
				if (floatAlmostEqualZero(tmark_value, SAC_FLOAT_UNDEF)) {
					GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: tmark %d not defined in SAC header, skipped.\n", L[n].file, Ctrl->T.tmark);
					continue;
				}
				tref += (double)tmark_value;
			}

			/* +r */
			if (Ctrl->T.reduce) {
				if (floatAlmostEqualZero(hd.dist, SAC_FLOAT_UNDEF)) {
					GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: dist not defined in SAC header, skipped.\n", L[n].file);
					continue;
				}
				tref += fabs(hd.dist)/Ctrl->T.reduce_vel;
			}
			/* +s */
			tref -= Ctrl->T.shift;
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "=> %s: reference time is %g\n", L[n].file, tref);

		/* read SAC data */
		if (!Ctrl->C.active) {
			if ((data = read_sac (L[n].file, &hd)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: unable to read, skipped.\n", L[n].file);
				continue;
			}
		}
		else {
			if ((data = read_sac_pdw (L[n].file, &hd, 10, (float)(tref+Ctrl->C.t0), (float)(tref+Ctrl->C.t1) )) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: unable to read, skipped.\n", L[n].file);
				continue;
			}
		}

		/* prepare datas */
		x = gmt_M_memory (GMT, NULL, hd.npts, double);
		y = gmt_M_memory (GMT, NULL, hd.npts, double);

		if (gmt_M_is_linear(GMT)) dt = hd.delta;
		else if (Ctrl->S.active) dt = hd.delta/Ctrl->S.sec_per_measure;
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: -S option is needed in geographic plots.\n");
			gmt_M_free(GMT, x);		gmt_M_free(GMT, y);		gmt_M_free (GMT, L);
			gmt_M_free(GMT, data);
			Return(EXIT_FAILURE);
		}
		for (i = 0; i < hd.npts; i++) {
			x[i] = i * dt;
			y[i] = data[i];
		}
		free (data);

		/* -F: data preprocess */
		for (i = 0; Ctrl->F.keys[i] != '\0'; i++) {
			switch (Ctrl->F.keys[i]) {
				case 'i': integral(y, hd.delta, hd.npts); hd.npts--; break;
				case 'q':   sqr(y, hd.npts); break;
				case 'r': rmean(y, hd.npts); break;
				default: break;
			}
		}

		/* recalculate depmin, depmax, depmen for further use */
		hd.depmax = -FLT_MAX;	hd.depmin = FLT_MAX; hd.depmen = 0;
		for (i = 0; i < hd.npts; i++){
			hd.depmax = hd.depmax > y[i] ? hd.depmax : (float)y[i];
			hd.depmin = hd.depmin < y[i] ? hd.depmin : (float)y[i];
			hd.depmen += (float)y[i];
		}
		hd.depmen = hd.depmen/hd.npts;
		GMT_Report (API, GMT_MSG_VERBOSE, "=> %s: depmax=%g depmin=%g depmen=%g\n", L[n].file, hd.depmax, hd.depmin, hd.depmen);

		/* -M: determine yscale for multiple traces */
		if (Ctrl->M.active) {
			if (Ctrl->M.norm || (Ctrl->M.scaleALL && n == 0))
				yscale = Ctrl->M.size / (hd.depmax - hd.depmin);
			else if (Ctrl->M.dist_scaling)
				yscale = Ctrl->M.size * pow(fabs(hd.dist), Ctrl->M.alpha);
			for (i = 0; i < hd.npts; i++) y[i] *= (float)yscale;
			hd.depmin *= (float)yscale;
			hd.depmax *= (float)yscale;
			hd.depmen *= (float)yscale;
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "=> %s: yscale of trace: %g\n", L[n].file, yscale);

		/* -Q: swap x and y */
		if (Ctrl->Q.active) {
			/* swap arrays */
			double *xp = gmt_M_memory (GMT, NULL, hd.npts, double);
			gmt_M_memcpy (xp, y, hd.npts, double);
			gmt_M_memcpy (y,  x, hd.npts, double);
			gmt_M_memcpy (x, xp, hd.npts, double);
			gmt_M_free (GMT, xp);
		}

		/* Default to plot trace at station locations on geographic maps */
		if (!gmt_M_is_linear(GMT) && L[n].position == false) {
			L[n].position = true;
			gmt_geo_to_xy (GMT, hd.stlo, hd.stla, &L[n].x, &L[n].y);
			GMT_Report (API, GMT_MSG_VERBOSE, "=> %s: Geographic location: (%g, %g)\n", L[n].file, hd.stlo, hd.stla);
		}

		if (L[n].position) {   /* position (X0,Y0) on plots */
			x0 = L[n].x;
			y0 = L[n].y;
		}
		else {
			unsigned int user = 0; /* default using user0 */
			/* determine X0 */
			if (!Ctrl->C.active) x0 = hd.b - tref;
			else                 x0 = Ctrl->C.t0;

			/* determine Y0 */
			if (Ctrl->E.active) {
				switch (Ctrl->E.keys[0]) {
					case 'a':
						if (floatAlmostEqualZero(hd.az, SAC_FLOAT_UNDEF)) {
							GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: az not defined in SAC header, skipped.\n", L[n].file);
							continue;
						}
						y0 = hd.az;
						break;
					case 'b':
						if (floatAlmostEqualZero(hd.baz, SAC_FLOAT_UNDEF)) {
							GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: baz not defined in SAC header, skipped.\n", L[n].file);
							continue;
						}
						y0 = hd.baz;
						break;
					case 'd':
						if (floatAlmostEqualZero(hd.gcarc, SAC_FLOAT_UNDEF)) {
							GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: gcarc not defined in SAC header, skipped.\n", L[n].file);
							continue;
						}
						y0 = hd.gcarc;
						break;
					case 'k':
						if (floatAlmostEqualZero(hd.dist, SAC_FLOAT_UNDEF)) {
							GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: dist not defined in SAC header, skipped.\n", L[n].file);
							continue;
						}
						y0 = hd.dist;
						break;
					case 'n':
						y0 = n;
						if (Ctrl->E.keys[1]!='\0') y0 += atof(&Ctrl->E.keys[1]);
						break;
					case 'u':  /* user0 to user9 */
						if (Ctrl->E.keys[1] != '\0') user = atoi(&Ctrl->E.keys[1]);
						y0 = *((float *) &hd + USERN + user);
						if (floatAlmostEqualZero((float)y0, SAC_FLOAT_UNDEF)) {
							GMT_Report (API, GMT_MSG_NORMAL, "=> %s: Warning: user%d not defined in SAC header, skipped.\n", user, L[n].file);
							continue;
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Wrong choice of profile type (d|k|a|b|n) \n");
						gmt_M_free(GMT, x);		gmt_M_free(GMT, y);		gmt_M_free (GMT, L);
						Return(EXIT_FAILURE);
						break;
				}
			}
			if (Ctrl->Q.active) {	/* swap x0 and y0 */
				double xy;
				xy = x0;  x0 = y0;  y0 = xy;
			}
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "=> %s: location of trace: (%g, %g)\n", L[n].file, x0, y0);
		for (i = 0; i < hd.npts; i++) {
			x[i] += x0;
			y[i] += y0;
		}

		/* report xmin, xmax, ymin and ymax */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "=> %s: after scaling and shifting : xmin=%g xmax=%g ymin=%g ymax=%g\n",
		                                       L[n].file, x[0], x[hd.npts-1], hd.depmin, hd.depmax);

		if (gmt_M_is_linear(GMT)) {
			GMT->current.plot.n = gmt_geo_to_xy_line (GMT, x, y, hd.npts);
			xp = GMT->current.plot.x;
			yp = GMT->current.plot.y;
			npts = (int)GMT->current.plot.n;
			plot_pen = GMT->current.plot.pen;
		}
		else {
			xp = x;
			yp = y;
			npts = hd.npts;
			plot_pen = gmt_M_memory (GMT, NULL, npts, unsigned int);
			plot_pen[0] = PSL_MOVE;
			free_plot_pen = true;
		}

		/* plot trace */
		if (L[n].custom_pen) {
			current_pen = L[n].pen;
			gmt_setpen (GMT, &L[n].pen);
		}
		gmt_plot_line (GMT, xp, yp, plot_pen, npts, current_pen.mode);
		if (L[n].custom_pen) {
			current_pen = Ctrl->W.pen;
			gmt_setpen (GMT, &current_pen);
		}

		/* paint trace */
		for (i = 0; i <= 1; i++) { /* 0=positive; 1=negative */
			if (Ctrl->G.active[i]) {
				double zero = 0.0;
				if (!Ctrl->Q.active) zero = Ctrl->G.zero[i]*yscale + y0;
				else                 zero = Ctrl->G.zero[i]*yscale + x0;

 				if (!Ctrl->G.cut[i]) {
 					if (!Ctrl->Q.active) {
						if (gmt_M_is_linear(GMT)) {
							Ctrl->G.t0[i] = MAX((float)x[0], (float)GMT->common.R.wesn[XLO]);
							Ctrl->G.t1[i] = MIN((float)x[hd.npts-1], (float)GMT->common.R.wesn[XHI]);
						} else {
 							Ctrl->G.t0[i] = (float)x[0];
 							Ctrl->G.t1[i] = (float)x[hd.npts-1];
 						}
					}
 					else {
						if (gmt_M_is_linear(GMT)) {
							Ctrl->G.t0[i] = MAX((float)y[0], (float)GMT->common.R.wesn[YLO]);
							Ctrl->G.t1[i] = MIN((float)y[hd.npts-1], (float)GMT->common.R.wesn[YHI]);
						} else {
 							Ctrl->G.t0[i] = (float)y[0];
 							Ctrl->G.t1[i] = (float)y[hd.npts-1];
 						}
 					}
				}
				GMT_Report (API, GMT_MSG_VERBOSE, "=> %s: Painting traces: zero=%g t0=%g t1=%g\n",
				                                  L[n].file, zero, Ctrl->G.t0[i], Ctrl->G.t1[i]);
				paint_phase(GMT, Ctrl, PSL, x, y, hd.npts, zero, Ctrl->G.t0[i], Ctrl->G.t1[i], i);
			}
		}
		gmt_M_free(GMT, x);
		gmt_M_free(GMT, y);
		if (free_plot_pen) gmt_M_free(GMT, plot_pen);
		free_plot_pen = false;
	}
	gmt_M_free(GMT, x);		gmt_M_free(GMT, y);		/* They might not have been released above due to 'continues' */
	gmt_M_free (GMT, L);
	gmt_M_free (GMT, Ctrl->In.file);

	if (Ctrl->D.active) PSL_setorigin (PSL, -Ctrl->D.dx, -Ctrl->D.dy, 0.0, PSL_FWD);	/* Reset shift */

	PSL_setdash (PSL, NULL, 0);
	GMT->current.map.is_world = old_is_world;

	gmt_map_basemap (GMT);
	gmt_plane_perspective (GMT, -1, 0.0);

	gmt_plotend (GMT);

	Return (GMT_OK);
}
