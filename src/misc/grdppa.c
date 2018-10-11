/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/* ********************************************************************* */
/* Program for automatic extraction of ridge and valley axes from the */
/* digital elevation data set. The main steps are: */
/* step 1: data and parameters input by sub._input */
/* step 2: TarGet ReCognition and CONnection by sub._TGRCON */
/* step 3: SEGment checK-Out by sub._SEGKO */
/* step 4: LiNe SMooth and OutPut by sub._LNSMOP
 *	 
/* ********************************************************************* */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grdppa"
#define THIS_MODULE_LIB		"misc"
#define THIS_MODULE_PURPOSE	"Automatic extraction of ridge or valley axes"
#define THIS_MODULE_KEYS	"<G{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-RVbh"

#define ij(h,i,j) ((i) + ((j)-1)*(h->mx) -1)
#define ijk(h,i,j,k) ((i) + ((j)-1)*(h->mx) + ((k)-1)*(h->mx)*(h->my) -1)

/* Control structure */

struct GRDPPA_CTRL {
	struct GRDPPA_In {
		bool active;
		char *file;
	} In;
	struct GRDPPA_A {	/* -A valleys instead of ridges */
		bool active;
		bool ridges;
	} A;
	struct GRDPPA_L {	/* -L<> */
		bool active;
		int n_pts;
	} L;
	struct GRDPPA_S {	/* -S single-segment (ascii) file */
		bool active;
	} S;
	char  *v;
	int	neigh_x[8], neigh_y[8];
	float *w;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDPPA_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDPPA_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.ridges = true;
	C->L.n_pts = 3;
	//neigh_x[8] = {0, 1, 1, 1, 0, -1, -1, -1};
	C->neigh_x[0] = 0;	C->neigh_x[1] = 1;	C->neigh_x[2] = 1;	C->neigh_x[3] = 1;	
	C->neigh_x[4] = 0;	C->neigh_x[5] = -1;	C->neigh_x[6] = -1;	C->neigh_x[7] = -1;	
	//neigh_y[8] = {1, 1, 0, -1, -1, -1, 0, 1};
	C->neigh_y[0] = 1;	C->neigh_y[1] = 1;	C->neigh_y[2] = 0;	C->neigh_y[3] = -1;	
	C->neigh_y[4] = -1;	C->neigh_y[5] = -1;	C->neigh_y[6] = 0;	C->neigh_y[7] = 1;	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDPPA_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C->w);
	gmt_M_free (GMT, C->v);
	gmt_M_free(GMT, C);
}


GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);

	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmt %s <grid> [-A] [-L<n_pts>] [-M] [%s] [%s] [%s]\n",
	             name, GMT_Rgeoz_OPT, GMT_V_OPT, GMT_bo_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is the grid file to be 'ridged'.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A detect valleys [default is ridges]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L <npoints> for polygon recognition [default = 3]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S write single-segment file separated with NaNs [Default is multi-seg].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note, however, that this matters only to external interfaces.\n");
	GMT_Option (API, "bo");

	return (GMT_MODULE_USAGE);
}


GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDPPA_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options) {
	/* This parses the options provided to grdppa and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	gmt_M_memset (io, 1, struct GMT_Z_IO);

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/*  */
				Ctrl->A.active = true;
				Ctrl->A.ridges = false;
				break;
			case 'L':	/*  */
				Ctrl->L.active = true;
				Ctrl->L.n_pts = atoi (opt->arg);
				break;
			case 'S':	/*  */
				Ctrl->S.active = true;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Table of constant values */
static unsigned int c__0 = 0;
static unsigned int c__1 = 1;
static unsigned int c__2 = 2;

/* --------------------------------------------------------------------------------- */
GMT_LOCAL void con(struct GRDPPA_CTRL *Ctrl, struct GMT_GRID *G, unsigned int i,
                   unsigned int j, unsigned int k, unsigned int *n, unsigned int jc, float *s) {
	/* ********************************************************************* */
	/* check or change the status of connection or route */
	/* ********************************************************************* */
	unsigned int ii, jj, kk;

	if (k <= 0) return;
	ii = i;		jj = j;		kk = k;
	if (k > 4) {
		ii = i + Ctrl->neigh_x[k-1];
		jj = j + Ctrl->neigh_y[k-1];
		kk += -4;
	}
	if (jc == 1) {
		*n = Ctrl->v[ijk(G->header, ii, jj, kk)];
		*s = Ctrl->w[ijk(G->header, ii, jj, kk)];
		return;
	}
	Ctrl->v[ijk(G->header, ii, jj, kk)] = (char)(*n);
	Ctrl->w[ijk(G->header, ii, jj, kk)] = 2e3;
	if (*n == 2)
		Ctrl->w[ijk(G->header, ii,jj,kk)] = G->data[ij(G->header, i,j)] + G->data[ij(G->header, i + Ctrl->neigh_x[k-1], (j + Ctrl->neigh_y[k-1]))];
	return;
}

/* --------------------------------------------------------------------------------- */
GMT_LOCAL int neb(unsigned int i) {
	/* ********************************************************************* */
	/* function to scale the cycled neighbor order */
	/* ********************************************************************* */
	int ret_val;
	ret_val = i;
	if (ret_val > 8)
		ret_val += -8;
	else if (ret_val < 1)
		ret_val += 8;
	return ret_val;
}

/* --------------------------------------------------------------------------------- */
GMT_LOCAL int kst(struct GRDPPA_CTRL *Ctrl, struct GMT_GRID *G, unsigned int i,
                  unsigned int j, unsigned int k, int jc) {
	/* ********************************************************************* */
	/* function to handle connection status and tracing route tables */
	/* jc=1: check the connection status; jc=-1: check the route status */
	/* jc=2: target connection, crossed segment will be checked out */
	/* jc=4: break the connection,        jc=-4: break the route */
	/* jc=8: check number of connections, jc=-8: check number of routes */
	/* ********************************************************************* */
	int ret_val = 0, m, n;
	static float v, w1, w2;

	if (jc == 2) {
		con(Ctrl, G, i, j, k, &c__2, 2, &v);
		if (k % 2 == 1) return ret_val;
		con(Ctrl, G, i + Ctrl->neigh_x[k-2], j + Ctrl->neigh_y[k-2], neb(k+2), &n, 1, &w2);
		if (n == 0) return ret_val;
		con(Ctrl, G, i, j, k, &n, 1, &w1);

		if (w2 > w1)
			con(Ctrl, G, i, j, k, &c__0, 2, &v);
		else
			con(Ctrl, G, i + Ctrl->neigh_x[k-2], j + Ctrl->neigh_y[k-2], neb(k+2), &c__0, 2, &v);
		return ret_val;
	}
	ret_val = 0;
	if (abs(jc) == 8) {
		for (m = 1; m <= 8; ++m) {
			con(Ctrl, G, i, j, m, &n, 1, &v);
			if      (jc == 8 && n != 0)  ret_val++;
			else if (jc == -8 && n == 2) ret_val++;
		}
		return ret_val;
	}
	con(Ctrl, G, i, j, k, &n, 1, &v);
	if (jc == 1 && n != 0)
		ret_val = 1;
	else if (jc == -1 && n == 2)
		ret_val = 1;
	else if (jc == 4)
		con(Ctrl, G, i, j, k, &c__0, 2, &v);
	else if (jc == -4)
		 con(Ctrl, G, i, j, k, &c__1, 2, &v);

	return ret_val;
}

/* --------------------------------------------------------------------------------- */
GMT_LOCAL int tgrcon(struct GMTAPI_CTRL *API, struct GRDPPA_CTRL *Ctrl, struct GMT_GRID *G) {
	/* ********************************************************************* */
	/* Subroutine for target recognition and connection */
	/* ********************************************************************* */
	unsigned int *b, i, j, k, n[8], mc, ii, jj, ml, mm, itg;

	b = gmt_M_memory (API->GMT, NULL, ((uint64_t)G->header->mx * G->header->my), unsigned int);

	/* # recognize the targets along profiles in four direction */
	itg = 0;
	for (j = 2; j < G->header->my - 1; j++) {
		for (i = 3; i < G->header->mx - 1; i++) {
			for (k = 1; k <= 8; ++k) {
				n[k-1] = 0;
				for (ml = 1; ml <= (unsigned int)Ctrl->L.n_pts / 2; ml++) {
					ii = i + ml * Ctrl->neigh_x[k-1];
					jj = j + ml * Ctrl->neigh_y[k-1];
					if (ii < 2 || ii > G->header->mx - 1 || jj < 2 || jj > G->header->my - 1) continue;
					if (G->data[ij(G->header, ii,jj)] < G->data[ij(G->header, i,j)]) n[k-1] = 1;
				}
			}
			for (k = 1; k <= 4; k++) {
				if (n[k-1] + n[k+3] > 1) b[ij(G->header, i,j)] = 1;
			}
			if (b[ij(G->header, i,j)] == 1) itg++;
		}
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, " %d\ttargets found\n", itg);
	/* # target connection */
	for (j = 2; j < G->header->my; j++) {
		for (i = 2; i < G->header->mx; i++) {
			for (k = 1; k <= 4; k++) {
				mm = kst(Ctrl, G, i, j, k, 4);
				if (b[ij(G->header, i,j)] + b[ij(G->header, i + Ctrl->neigh_x[k-1],(j + Ctrl->neigh_y[k-1]))] == 2) {
					mc = kst(Ctrl, G, i, j, k, 2);
				}
			}
		}
	}
	gmt_M_free (API->GMT, b);
	return 0;
}

/* --------------------------------------------------------------------------------- */
GMT_LOCAL int segko(struct GMTAPI_CTRL *API, struct GRDPPA_CTRL *Ctrl, struct GMT_GRID *G, float z_scale) {
	/* ********************************************************************* */
	/* check-out improper segments by polygon breaking and branch reduction */
	/* ********************************************************************* */
	float v, z, wn;
	unsigned int id, ii, jj, kk, in, jn, mm, nv, i, j, k, l, m, n, *b;

	b = gmt_M_memory (API->GMT, NULL, ((uint64_t)G->header->mx * G->header->my), unsigned int);

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, " polygon breaking ...\n");

	m = 0;
	/* # pick the weakest segment */
L1:
	m++;
	wn = 1001.;
	for (j = 2; j < G->header->my; j++) {
		for (i = 2; i < G->header->mx; i++) {
			if (b[ij(G->header, i,j)] == 1)  continue;
			nv = 0;
			for (k = 1; k <= 4; k++) {
				con(Ctrl, G, i, j, k, &n, 1, &v);
				if (v == 2e3) nv++;
				if (v >= wn) continue;
				/* # skip the end-segment */
				if (kst(Ctrl, G, i,j,k,-8) == 1 || kst(Ctrl, G, i + Ctrl->neigh_x[k-1], j + Ctrl->neigh_y[k-1], k, -8) == 1) {
					mm = kst(Ctrl, G, i, j, k, -4);
					continue;
				}
				wn = v;
				ii = i;
				jj = j;
				kk = k;
			}
			if (nv == 4) b[ij(G->header, i,j)] = 1;
		}
	}
	if (wn == 1001.) goto L4;
	if ((MAX(API->verbose, API->GMT->current.setting.verbose) <= GMT_MSG_VERBOSE) && m % 100 == 1) {
		if (Ctrl->A.ridges) {
			z = G->header->z_min + (wn / 2 - 1) / z_scale;
			GMT_Report (API, GMT_MSG_VERBOSE, " segments below z = %.3f\tchecked %f\t%d\r", z,wn,m);
		}
		else {
			z = G->header->z_max - (wn / 2 - 1) / z_scale;
			GMT_Report (API, GMT_MSG_VERBOSE, " segments above z = %.3f\tchecked\r", z);
		}
	}
	/* # polygon tracing */
	mm = kst(Ctrl, G, ii, jj, kk, -4);
	if (kst(Ctrl, G, ii, jj, kk, -8) == 0) goto L1;
	in = ii + Ctrl->neigh_x[kk-1];
	jn = jj + Ctrl->neigh_y[kk-1];
	if (kst(Ctrl, G, in, jn, kk, -8) == 0) goto L1;
	id = 1;
L6:
	i = in;		j = jn;		k = kk;
L8:
	k = neb(k+4);
L11:
	k = neb(k+id);
	if (kst(Ctrl, G, i, j, k, -1) == 0) goto L11;
	i += Ctrl->neigh_x[k-1];
	j += Ctrl->neigh_y[k-1];
	if (i == ii && j == jj) {
		mm = kst(Ctrl, G, ii, jj, kk, 4);
		goto L1;
	}
	if (i == in && j == jn) {
		if (id == -1) goto L1;
		id = -1;
		goto L6;
	}
	goto L8;
	/* # branch reduction */
L4:
	for (l = 1; l <= (unsigned int)Ctrl->L.n_pts / 2; l++) {
		for (j = 2; j < G->header->my; j++) {
			for (i = 2; i < G->header->mx; i++) {
				b[ij(G->header, i,j)] = 0;
				if (kst(Ctrl, G, i, j, k, 8) != 1) continue;
				for (k = 1; k <= 8; k++) {
					if (kst(Ctrl, G, i, j, k, 1) == 1) b[ij(G->header, i,j)] = k;
				}
			}
		}
		for (j = 2; j < G->header->my; j++) {
			for (i = 2; i < G->header->mx; i++) {
				mm = kst(Ctrl, G, i, j, b[ij(G->header, i,j)], 4);
			}
		}
	}
	gmt_M_free (API->GMT, b);
	return 0;
}

/* --------------------------------------------------------------------------------- */
GMT_LOCAL int smooth(struct GRDPPA_CTRL *Ctrl, struct GMT_GRID *G, unsigned int i, unsigned int j, float *x, float *y, float *w) {
/* ********************************************************************* */
/* subroutine to smooth target position according to the weights of */
/* connected neighbors */
/* ********************************************************************* */
	static float f;
	static int k;

	*w = G->data[ij(G->header, i,j)];
	*x = 0.;
	*y = 0.;
	for (k = 1; k <= 8; k++) {
		f = kst(Ctrl, G, i, j, k, 1);
		*w += G->data[ij(G->header, i + Ctrl->neigh_x[k-1], j + Ctrl->neigh_y[k-1])] * f;
		*x += Ctrl->neigh_x[k-1] * G->data[ij(G->header, i + Ctrl->neigh_x[k-1], j + Ctrl->neigh_y[k-1])] * f;
		*y += Ctrl->neigh_y[k-1] * G->data[ij(G->header, i + Ctrl->neigh_x[k-1], j + Ctrl->neigh_y[k-1])] * f;
	}
	*x = G->header->wesn[XLO] + ((float)(i - 2) + *x / *w) * G->header->inc[GMT_X];
	*y = G->header->wesn[YLO] + ((float)(j - 2) + *y / *w) * G->header->inc[GMT_Y];
	*w /= (kst(Ctrl, G, i, j, k, 8) + 1);
	return 0;
}


#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* --------------------------------------------------------------------------------- */
int GMT_grdppa (void *V_API, int mode, void *args) {
	bool first = true, mem_G = false;
	unsigned int i, j, k;
	int error = 0;
	
	float  z_scale, ww, x, y, v1, v2, x1, y1, x_save, y_save;
	double wesn[4], out[2];

	struct GMT_GRID *G = NULL, *W = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_Z_IO io;
	struct GMT_OPTION *opt = NULL;
	struct GRDPPA_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, &io, options)) != 0) Return (error);
	
	/*---------------------------- This is the grd2xyz main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grid(s)\n");
	
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	if (io.binary)
		GMT->common.b.active[GMT_OUT] = true;

	if (!Ctrl->S.active)
		gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

	if ((error = GMT_Set_Columns (API, GMT_OUT, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers stdout, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	gmt_set_pad (GMT, 1);	/* Change the default pad */
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (gmt_M_is_subset (GMT, G->header, wesn))	/* If subset requested make sure wesn matches header spacing */
		gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, G->header), "");

	/* Read data */
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL) {
		Return (API->error);
	}

	if (!Ctrl->A.ridges) {
		mem_G = gmt_M_file_is_memory (Ctrl->In.file);
		for (k = 0; k < G->header->size; k++)
			G->data[k] *= -1;
	}

	Ctrl->w = gmt_M_memory (GMT, NULL, G->header->my*G->header->mx*4, float);
	Ctrl->v = gmt_M_memory (GMT, NULL, G->header->my*G->header->mx*4, char);

	z_scale = 499.0 / (G->header->z_max - G->header->z_min);

	/* Regarding the original fortran algo we need to flipud the grid */
	gmtlib_grd_flip_vertical (G->data, G->header->mx, G->header->my, 0, sizeof(G->data[0]));

	tgrcon(API, Ctrl, G);
	segko(API, Ctrl, G, z_scale);

	/* This was the lnsmop() routine */
	for (j = 2; j < G->header->my; j++) {
		for (i = 2; i < G->header->mx; i++) {
			for (k = 1; k <= 4; k++) {
				if (kst(Ctrl, G, i, j, k, 1) == 0) continue;
				smooth(Ctrl, G, i, j, &x, &y, &v1);
				smooth(Ctrl, G, i + Ctrl->neigh_x[k-1], j + Ctrl->neigh_y[k-1], &x1, &y1, &v2);
				ww = (v1 + v2) / 2;
				/*	Not used!!
				if (Ctrl->A.ridges)
					heig = G->header->z_min + (ww - 1) / z_scale;
				else
					heig = G->header->z_max - (ww - 1) / z_scale;
				*/

				if (first) {
					if (!Ctrl->S.active)
						GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					Out = gmt_new_record (GMT, out, NULL);
					out[0] = x;		out[1] = y;
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					out[0] = x1;	out[1] = y1;
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					x_save = x1;	y_save = y1;
					first = false;
				}
				else {
					/* Atempt to reduce the number of segments */
					if (fabs(x - x_save) < 1e-5 && fabs(y - y_save) < 1e-5) {
						out[0] = x1;	out[1] = y1;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
					else if (fabs(x1 - x_save) < 1e-5 && fabs(y1 - y_save) < 1e-5) {
						out[0] = x;		out[1] = y;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
					else {
						if (!Ctrl->S.active)
							GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
						else {
							out[0] = out[1] = GMT->session.d_NaN;
							GMT_Put_Record (API, GMT_WRITE_DATA, Out);
						}
						out[0] = x;		out[1] = y;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
						out[0] = x1;	out[1] = y1;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
					x_save = x1;	y_save = y1;
				}
			}
		}
	}

	if (!Ctrl->A.ridges && mem_G) {		/* Need to restore the input memory grid */
		for (k = 0; k < G->header->size; k++)
			G->data[k] *= -1;
	}

	gmt_M_free (GMT, Out);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	if (GMT_Destroy_Data (GMT->parent, &G) != GMT_NOERROR)
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free G\n");

	Return (GMT_NOERROR);
}
