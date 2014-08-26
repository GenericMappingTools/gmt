/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
/*
 * Brief synopsis: grdview will read a topofile and produce a 3-D perspective plot
 * of the surface z = f(x,y) using PostScript. The surface can
 * be represented as:
 *	1) A Mesh plot
 *	2) A shaded (or colored) surface w/wo contourlines and w/wo
 *	   illumination by artificial sun(s).
 *
 * grdview calls contours to find the line segments that make up the
 * contour lines. This allows the user to specify that the contours
 * should be smoothed before plotting. This will make the resulting
 * image smoother, especially if nx and ny are relatively small.
 * As an option, a drape grid file can be specified.  Then, the colors
 * are calculated from that file while the topo file is used for shape.
 * Alternatively, give three drape files (red, green, blue components)
 * to bypass the z -> rgb via the cpt file.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#define THIS_MODULE_NAME	"grdview"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create 3-D perspective image or surface mesh from a grid"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYcfnptxy" GMT_OPT("E")

/* grdview needs to work on "tiles" which is defined as the "square" regions made
 * up by 4 nodes.  Since painting takes place within the tiles, any pixel-registered
 * grids will loose 1/2 a grid-spacing in extend in all directions.  We simplify this
 * difference by converting the headers of all pixel-registered grids to become grid-
 * line-registered grids thus: (1) shrink w/e/s/n by 1/2 grid spacing, and (2) set
 * the registration parameter to 0.
 * We can then proceed with algorithms that require gridline registration. */

/* Declarations needed for binning of smooth contours */

#define GRDVIEW_MESH		0	/* Default */
#define GRDVIEW_SURF		1
#define GRDVIEW_IMAGE		2
#define GRDVIEW_WATERFALL_X	3
#define GRDVIEW_WATERFALL_Y	4

struct GRDVIEW_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct G {	/* -G<drapefile> */
		bool active;
		bool image;
		char *file[3];
	} G;
	struct I {	/* -I<intensfile>|<value> */
		bool active;
		bool constant;
		double value;
		char *file;
	} I;
	struct N {	/* -N<level>[+g<fill>] */
		bool active;
		bool facade;
		struct GMT_FILL fill;
		double level;
	} N;
	struct Q {	/* -Q<type>[g] */
		bool active, special;
		bool outline;
		bool mask;
		bool monochrome;
		unsigned int mode;	/* GRDVIEW_MESH, GRDVIEW_SURF, GRDVIEW_IMAGE */
		unsigned int dpi;
		struct GMT_FILL fill;
	} Q;
	struct S {	/* -S<smooth> */
		bool active;
		unsigned int value;
	} S;
	struct T {	/* -T[s][o[<pen>]] */
		bool active;
		bool skip;
		bool outline;
		struct GMT_PEN pen;
	} T;
	struct W {	/* -W[+]<type><pen> */
		bool active;
		bool contour;
		struct GMT_PEN pen[3];
	} W;
};

struct GRDVIEW_BIN {
	struct GRDVIEW_CONT *first_cont;
};

struct GRDVIEW_CONT {
	struct GRDVIEW_POINT *first_point;
	struct GRDVIEW_CONT *next_cont;
	double value;
};

struct GRDVIEW_POINT {
	double x, y;
	struct GRDVIEW_POINT *next_point;
};

struct GRDVIEW_CONT *get_cont_struct (struct GMT_CTRL *GMT, uint64_t bin, struct GRDVIEW_BIN *binij, double value)
{
	struct GRDVIEW_CONT *cont, *new_cont;

	if (!binij[bin].first_cont) binij[bin].first_cont = GMT_memory (GMT, NULL, 1, struct GRDVIEW_CONT);

	for (cont = binij[bin].first_cont; cont->next_cont && cont->next_cont->value <= value; cont = cont->next_cont);

	new_cont = GMT_memory (GMT, NULL, 1, struct GRDVIEW_CONT);
	if (cont->next_cont) {	/* Put it in the link */
		new_cont->next_cont = cont->next_cont;
		cont->next_cont = new_cont;
	}
	else	/* End of list */
		cont->next_cont = new_cont;
	return (new_cont);
}

struct GRDVIEW_POINT *get_point (struct GMT_CTRL *GMT, double x, double y)
{
	struct GRDVIEW_POINT *point = GMT_memory (GMT, NULL, 1, struct GRDVIEW_POINT);
	point->x = x;
	point->y = y;
	return (point);
}

#if 0
/* RS: Removed this because it yields unpredictable results, making it impossible to line up different 3D plots */

void grdview_init_setup (struct GMT_CTRL *GMT, struct GMT_GRID *Topo, int draw_plane, double plane_level)
{
	int row, col, ij;
	double xtmp, ytmp, tmp;

	/* We must find the projected min/max in y-direction */

	/* Reset from whatever they were */
	GMT->current.proj.z_project.ymin = +DBL_MAX;
	GMT->current.proj.z_project.ymax = -DBL_MAX;

	GMT_grd_loop (Topo, row, col, ij) {	/* First loop over all the grid-nodes */
		if (GMT_is_fnan (Topo->data[ij])) continue;
		GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (col, Topo->header), GMT_grd_row_to_y (row, Topo->header), (double)Topo->data[ij], &xtmp, &ytmp);
		GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
		GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
	}
	if (draw_plane) {	/* The plane or facade may exceed the current min/max so loop over these as well */
		for (col = 0; col < Topo->header->nx; col++) {
			tmp = GMT_grd_col_to_x (col, Topo->header);
			GMT_geoz_to_xy (GMT, tmp, Topo->header->wesn[YLO], plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
			GMT_geoz_to_xy (GMT, tmp, Topo->header->wesn[YHI], plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
		}
		for (row = 0; row < Topo->header->ny; row++) {
			tmp = GMT_grd_row_to_y (row, Topo->header);
			GMT_geoz_to_xy (GMT, Topo->header->wesn[XLO], tmp, plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
			GMT_geoz_to_xy (GMT, Topo->header->wesn[XHI], tmp, plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
		}
	}
}
#endif

double get_intensity (struct GMT_GRID *I, uint64_t k)
{
	/* Returns the average intensity for this tile */
	return (0.25 * (I->data[k] + I->data[k+1] + I->data[k-I->header->mx] + I->data[k-I->header->mx+1]));
}

unsigned int pixel_inside (struct GMT_CTRL *GMT, int ip, int jp, int *ix, int *iy, uint64_t bin, int bin_inc[])
{	/* Returns true of the ip,jp point is inside the polygon defined by the tile */
	unsigned int i, what;
	double x[6], y[6];

	for (i = 0; i < 4; i++) {
		x[i] = (double)ix[bin+bin_inc[i]];
		y[i] = (double)iy[bin+bin_inc[i]];
	}
	x[4] = x[0];	y[4] = y[0];
	what = GMT_non_zero_winding (GMT, (double)ip, (double)jp, x, y, 5);
	return (what);
}

int quick_idist (int x1, int y1, int x2, int y2)
{	/* Fast integer distance calculation */
	if ((x2 -= x1) < 0) x2 = -x2;
	if ((y2 -= y1) < 0) y2 = -y2;
	return (x2 + y2 - (((x2 > y2) ? y2 : x2) >> 1));
}

unsigned int get_side (double x, double y, double x_left, double y_bottom, double inc[], double inc2[]) {
	/* Figure out on what side this point sits on */

	double del_x, del_y;
	unsigned int side;

	del_x = x - x_left;
	if (del_x > inc2[GMT_X]) del_x = inc[GMT_X] - del_x;
	del_y = y - y_bottom;
	if (del_y > inc2[GMT_Y]) del_y = inc[GMT_Y] - del_y;
	if (del_x < del_y) /* Cutting N-S gridlines */
		side = ((x-x_left) > inc2[GMT_X]) ? 1 : 3;
	else /* Cutting E-W gridlines */
		side = ((y-y_bottom) > inc2[GMT_Y]) ? 2 : 0;
	return (side);
}

void copy_points_fw (double x[], double y[], double z[], double v[], double xcont[], double ycont[], double zcont[], double vcont[], unsigned int ncont, uint64_t *n) {
	unsigned int k;
	for (k = 0; k < ncont; k++, (*n)++) {
		x[*n] = xcont[k];
		y[*n] = ycont[k];
		z[*n] = zcont[k];
		v[*n] = vcont[k];
	}
}

void copy_points_bw (double x[], double y[], double z[], double v[], double xcont[], double ycont[], double zcont[], double vcont[], unsigned int ncont, uint64_t *n) {
	unsigned int k, k2;
	for (k2 = 0, k = ncont - 1; k2 < ncont; k2++, k--, (*n)++) {
		x[*n] = xcont[k];
		y[*n] = ycont[k];
		z[*n] = zcont[k];
		v[*n] = vcont[k];
	}
}

double get_z_ave (double v[], double next_up, uint64_t n) {
	uint64_t k;
	double z_ave;

	for (k = 0, z_ave = 0.0; k < n; k++) z_ave += MIN (v[k], next_up);
	return (z_ave / n);
}

void add_node (double x[], double y[], double z[], double v[], uint64_t *k, unsigned int node, double X_vert[], double Y_vert[], float topo[], float zgrd[], uint64_t ij) {
	/* Adds a corner node to list of points and increments *k */
	x[*k] = X_vert[node];
	y[*k] = Y_vert[node];
	z[*k] = topo[ij];
	v[*k] = zgrd[node];
	(*k)++;
}

void paint_it_grdview (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, double *x, double *y, int n, double z, bool intens, bool monochrome, double intensity, bool outline) {
	int index;
	double rgb[4];
	struct GMT_FILL *f = NULL;

	if (n < 3) return;	/* Need at least 3 points to make a polygon */

	index = GMT_get_rgb_from_z (GMT, P, z, rgb);
	if (P && P->skip) return;	/* Skip this z-slice */

	/* Now we must paint, with colors or patterns */

	if ((index >= 0 && (f = P->range[index].fill)) || (index < 0 && (f = P->patch[index+3].fill)))	/* Pattern */
		GMT_setfill (GMT, f, outline);
	else {	/* Solid color/gray */
		if (intens) GMT_illuminate (GMT, intensity, rgb);
		if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb); /* GMT_YIQ transformation */
		PSL_setfill (PSL, rgb, outline);
	}
	PSL_plotpolygon (PSL, x, y, n);
}

void *New_grdview_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDVIEW_CTRL *C = GMT_memory (GMT, NULL, 1, struct GRDVIEW_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	GMT_init_fill (GMT, &C->N.fill, -1.0, -1.0, -1.0);	/* Default is no fill of facade */
	C->T.pen = C->W.pen[0] = C->W.pen[1] = C->W.pen[2] = GMT->current.setting.map_default_pen;	/* Tile and mesh pens */
	C->W.pen[0].width *= 3.0;	/* Contour pen */
	C->W.pen[2].width *= 3.0;	/* Facade pen */
	C->Q.dpi = 100;
	GMT_init_fill (GMT, &C->Q.fill, GMT->current.setting.ps_page_rgb[0], GMT->current.setting.ps_page_rgb[1], GMT->current.setting.ps_page_rgb[2]);
	return (C);
}

void Free_grdview_Ctrl (struct GMT_CTRL *GMT, struct GRDVIEW_CTRL *C) {	/* Deallocate control structure */
	unsigned int i;
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->C.file) free (C->C.file);	
	for (i = 0; i < 3; i++) if (C->G.file[i]) free (C->G.file[i]);	
	if (C->I.file) free (C->I.file);	
	GMT_free (GMT, C);	
}

int GMT_grdview_usage (struct GMTAPI_CTRL *API, int level)
{
	struct GMT_PEN P;

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdview <topogrid> %s [%s] [-C[<cpt>]] [-G<drapegrid> | -G<grd_r>,<grd_g>,<grd_b>]\n",
	             GMT_J_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<intensgrid>|<value>] [%s] [-K] [-N<level>[+g<fill>]] [-O] [-P] [-Q<args>[+m]]\n", GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-S<smooth>] [-T[s][o[<pen>]]]\n", GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W<type><pen>] [%s]\n\t[%s] [%s] [%s]\n",
	             GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_f_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s]\n\t[%s] [%s]\n\n", GMT_n_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<topogrid> is data set to be plotted.\n");
	GMT_Option (API, "J-Z");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file.  Optionally, instead give name of a master cpt\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Use <drapegrid> rather than <topogrid> for color-coding.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use <topogrid> as the relief and drape the image on top.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that -Jz and -N always refers to the <topogrid>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give three grid files with the red, green, and blue components in 0-255 range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If so, you must also choose -Qi.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Use illumination. Append name of intensity grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a constant intensity, just give the value instead.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Draw a horizontal plane at z = <level>. Append +g<fill> to paint\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the facade between the plane and the data perimeter.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Set plot request. Choose one of the following:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qm for Mesh plot [Default]. Append <color> for mesh paint [%s].\n",
		GMT_putcolor (API->GMT, API->GMT->PSL->init.page_rgb));
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qmx or -Qmy do waterfall type plots (row or column profiles).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qs[m] for colored or shaded Surface. Append m to draw meshlines on the surface.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qi for scanline converting polygons to rasterimage.  Append effective dpi [100].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qc. As -Qi but use PS Level 3 colormasking for nodes with z = NaN.  Append effective dpi [100].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To force a monochrome image using the GMT_YIQ transformation, append +m.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Smooth contours first (see grdview for <smooth> value info) [no smoothing].\n");
	GMT_pen_syntax (API->GMT, 'T', "Image the data without interpolation by painting polygonal tiles.\n"
	                "\t   Append s to skip tiles for nodes with z = NaN [Default paints all tiles].\n"
	                "\t   Append o[<pen>] to draw tile outline [Default uses no outline].");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with -Jz|Z as it produces a flat image.\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Set pen attributes for various features in form <type><pen>.");
	GMT_Message (API, GMT_TIME_NONE, "\t   <type> can be c for contours, m for mesh, and f for facade.\n");
	P = API->GMT->current.setting.map_default_pen;
	GMT_Message (API, GMT_TIME_NONE, "\t   m sets attributes for mesh lines [%s].\n", GMT_putpen (API->GMT, P));
	GMT_Message (API, GMT_TIME_NONE, "\t     Requires -Qm or -Qsm to take effect.\n");
	P.width *= 3.0;
	GMT_Message (API, GMT_TIME_NONE, "\t   c draw scontours on top of surface or mesh [Default is no contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Optionally append pen attributes [%s].\n", GMT_putpen (API->GMT, P));
	GMT_Message (API, GMT_TIME_NONE, "\t   f sets attributes for facade outline [%s].\n", GMT_putpen (API->GMT, P));
	GMT_Message (API, GMT_TIME_NONE, "\t     Requires -N to take effect.\n");
	GMT_Option (API, "X,c,f,n,p,t,.");
	
	return (EXIT_FAILURE);
}

int GMT_grdview_parse (struct GMT_CTRL *GMT, struct GRDVIEW_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdview and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, q_set = 0, n_commas, j, k, n, id, n_drape;
	int sval;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Cpt file */
				Ctrl->C.active = true;
				if (Ctrl->C.file) free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'G':	/* One or Three grid files */
				Ctrl->G.active = true;
				for (k = 0, n_commas = 0; opt->arg[k]; k++) if (opt->arg[k] == ',') n_commas++;
				if (n_commas == 2) {	/* Three r,g,b grids for draping */
					char A[GMT_LEN256] = {""}, B[GMT_LEN256] = {""}, C[GMT_LEN256] = {""};
					sscanf (opt->arg, "%[^,],%[^,],%s", A, B, C);
					if (GMT_check_filearg (GMT, '<', A, GMT_IN))
						Ctrl->G.file[0] = strdup (A);
					else
						n_errors++;
					if (GMT_check_filearg (GMT, '<', B, GMT_IN))
						Ctrl->G.file[1] = strdup (B);
					else
						n_errors++;
					if (GMT_check_filearg (GMT, '<', C, GMT_IN))
						Ctrl->G.file[2] = strdup (C);
					else
						n_errors++;
					Ctrl->G.image = true;
				}
				else if (n_commas == 0) {	/* Just got a single drape file */
					if (GMT_check_filearg (GMT, '<', opt->arg, GMT_IN))
						Ctrl->G.file[0] = strdup (opt->arg);
					else
						n_errors++;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error option -G: Usage is -G<z.grd> | -G<r.grd>,<g.grd>,<b.grd>\n");
					n_errors++;
				}
				break;
			case 'I':	/* Use intensity from grid or constant */
				Ctrl->I.active = true;
				if (!GMT_access (GMT, opt->arg, R_OK))	/* Got a file */
					Ctrl->I.file = strdup (opt->arg);
				else if (opt->arg[0] && !GMT_not_numeric (GMT, opt->arg)) {	/* Looks like a constant value */
					Ctrl->I.value = atof (opt->arg);
					Ctrl->I.constant = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I: Requires a valid grid file or a constant\n");
					n_errors++;
				}
				break;
			case 'L':	/* GMT4 BCs */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
					strncpy (GMT->common.n.BC, opt->arg, 4U);
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'N':	/* Facade */
				if (opt->arg[0]) {
					char colors[GMT_LEN64] = {""};
					Ctrl->N.active = true;
					if ((c = strstr (opt->arg, "+g"))) {	/* Gave modifier +g<fill> */
						c[0] = '\0';	/* Truncate string temporarily */
						Ctrl->N.level = atof (opt->arg);
						c[0] = '+';	/* Restore the + */
						n_errors += GMT_check_condition (GMT, GMT_getfill (GMT, &c[2], &Ctrl->N.fill), "Syntax error option -N: Usage is -N<level>[+g<fill>]\n");
						Ctrl->N.facade = true;
					}
					else if (GMT_compat_check (GMT, 4) && (c = strchr (opt->arg, '/'))) {	/* Deprecated <level>/<fill> */
						GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -N<level>[/<fill>] is deprecated; use -N<level>[+g<fill>] in the future.\n");
						c[0] = ' ';	/* Take out the slash for now */
						n = sscanf (opt->arg, "%lf %s", &Ctrl->N.level, colors);
						n_errors += GMT_check_condition (GMT, GMT_getfill (GMT, colors, &Ctrl->N.fill), "Syntax error option -N: Usage is -N<level>[+g<fill>]\n");
						Ctrl->N.facade = true;
						c[0] = '/';	/* Restore the slash */
					}
					else	/* Just got the level */
						Ctrl->N.level = atof (opt->arg);
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error option -N: Usage is -N<level>[+g<fill>]\n");
					n_errors++;
				}
				break;
			case 'Q':	/* Plot mode */
				Ctrl->Q.active = true;
				q_set++;
				if ((c = strstr (opt->arg, "+m")) != NULL) {
					Ctrl->Q.monochrome = true;
					c[0] = '\0';	/* Chop off +m */
				}
				switch (opt->arg[0]) {
					case 'c':	/* Image with colormask */
						if (opt->arg[1] && isdigit ((int)opt->arg[1])) Ctrl->Q.dpi = atoi (&opt->arg[1]);
						Ctrl->Q.mode = GRDVIEW_IMAGE;
						Ctrl->Q.mask = true;
						break;
					case 't':	/* Image without color interpolation */
						Ctrl->Q.special = true;
						/* Deliberate fall-through */
					case 'i':	/* Image with clipmask */
						Ctrl->Q.mode = GRDVIEW_IMAGE;
						if (opt->arg[1] && isdigit ((int)opt->arg[1])) Ctrl->Q.dpi = atoi (&opt->arg[1]);
						break;
					case 'm':	/* Mesh plot */
						n = 0;
						if (opt->arg[1] && opt->arg[1] == 'x') {
							Ctrl->Q.mode = GRDVIEW_WATERFALL_X;
							Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = 1;	/* Default to white */
							n = 1;
						}
						else if (opt->arg[1] && opt->arg[1] == 'y') {
							Ctrl->Q.mode = GRDVIEW_WATERFALL_Y;
							Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = 1;	/* Default to white */
							n = 1;
						}
						else
							Ctrl->Q.mode = GRDVIEW_MESH;

						if (opt->arg[n+1]) {	/* Appended /<color> or just <color> */
							k = ((opt->arg[n+1] == '/') ? 2 : 1) + n;
							n_errors += GMT_check_condition (GMT, GMT_getfill (GMT, &opt->arg[k], &Ctrl->Q.fill),
							                                 "Syntax error -Qm option: To give mesh color, use -Qm[x|y]<color>\n");
						}
						break;
					case 's':	/* Color without contours */
						Ctrl->Q.mode = GRDVIEW_SURF;
						if (opt->arg[1] == 'm') Ctrl->Q.outline = true;
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error option -Q: Unrecognized qualifier (%c)\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (Ctrl->Q.monochrome)
					c[0] = '+';	/* Restore the chopped off +m */
				else if (GMT_compat_check (GMT, 4) && opt->arg[strlen(opt->arg)-1] == 'g') {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -Q<args>[g] is deprecated; use -Q<args>[+m] in the future.\n");
					Ctrl->Q.monochrome = true;
				}
				break;
			case 'S':	/* Smoothing of contours */
				Ctrl->S.active = true;
				sval = atoi (opt->arg);
				n_errors += GMT_check_condition (GMT, sval, "Syntax error -S option: smooth value must be positive\n");
				Ctrl->S.value = sval;
				break;
			case 'T':	/* Tile plot */
				Ctrl->T.active = true;
				k = 0;
				if (opt->arg[0] == 's') Ctrl->T.skip = true, k = 1;
				if (opt->arg[k] == 'o') {	/* Want tile outline also */
					Ctrl->T.outline = true;
					k++;
					if (opt->arg[k] && GMT_getpen (GMT, &opt->arg[k], &Ctrl->T.pen)) {
						GMT_pen_syntax (GMT, 'T', " ");
						n_errors++;
					}
				}
				break;
			case 'W':	/* Contour, mesh, or facade pens */
				Ctrl->W.active = true;
				j = (opt->arg[0] == 'm' || opt->arg[0] == 'c' || opt->arg[0] == 'f');
				id = 0;
				if (j == 1) {	/* First check that the m or c is not part of a color name instead */
					char txt_a[GMT_LEN256] = {""};
					n = j+1;
					while (opt->arg[n] && opt->arg[n] != ',' && opt->arg[n] != '/') n++;	/* Wind until end or , or / */
					strncpy (txt_a, opt->arg, n);	txt_a[n] = '\0';
					if (GMT_colorname2index (GMT, txt_a) >= 0)	/* Found a colorname: reset j to 0 */
						j = id = 0;
					else
						id = (opt->arg[0] == 'f') ? 2 : ((opt->arg[0] == 'm') ? 1 : 0);
				}
				if (GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[id])) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				if (j == 0)	/* Copy pen when using just -W */
					Ctrl->W.pen[2] = Ctrl->W.pen[1] = Ctrl->W.pen[0];
				if (id == 0) Ctrl->W.contour = true;	/* Switch contouring ON for -Wc or -W */
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !strcmp (Ctrl->In.file, "="), "Error: Piping of topofile not supported!\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");

	/* Gave more than one -Q setting */
	n_errors += GMT_check_condition (GMT, q_set > 1, "Error: -Qm, -Qs, -Qc, and -Qi are mutually exclusive options\n");
	/* Gave both -Q and -T */
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->Q.active, "Error: -Q and -T are mutually exclusive options\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_drape = Ctrl->G.image ? 3 : 1;
	if (Ctrl->G.active) {
		unsigned int i;
		for (i = 0; i < n_drape; i++) {
			n_errors += GMT_check_condition (GMT, !Ctrl->G.file[i][0], "Syntax error -G option: Must specify drape file\n");
		}
		n_errors += GMT_check_condition (GMT, n_drape == 3 && Ctrl->Q.mode != GRDVIEW_IMAGE, "R/G/B drape requires -Qi option\n");
	}
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->I.constant && !Ctrl->I.file, "Syntax error -I option: Must specify intensity file or value\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->Q.mode == GRDVIEW_SURF || Ctrl->Q.mode == GRDVIEW_IMAGE || Ctrl->W.contour) && !Ctrl->C.file && !Ctrl->G.image, "Syntax error: Must specify color palette table\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.mode == GRDVIEW_IMAGE && Ctrl->Q.dpi <= 0, "Syntax error -Qi option: Must specify positive dpi\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && GMT->current.proj.JZ_set, "Syntax error -T option: Cannot specify -JZ|z\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdview_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdview (void *V_API, int mode, void *args)
{
	bool get_contours, bad, good, pen_set, begin, saddle, drape_resample = false;
	bool nothing_inside = false, use_intensity_grid;
	unsigned int c, nk, n4, row, col, n_drape = 0, n_edges, d_reg[3], i_reg = 0;
	unsigned int t_reg, n_out, k, k1, ii, jj, PS_colormask_off = 0, *edge = NULL;
	int i, j, i_bin, j_bin, i_bin_old, j_bin_old, i_start, i_stop, j_start, j_stop;
	int i_inc, j_inc, way, bin_inc[4], ij_inc[4], error = 0;
		
	uint64_t ij, sw, se, nw, ne, bin, n, pt;

	size_t max_alloc;

	double cval, x_left, x_right, y_top, y_bottom, small = GMT_CONV4_LIMIT, z_ave;
	double inc2[2], wesn[4], z_val, x_pixel_size, y_pixel_size;
	double this_intensity = 0.0, next_up = 0.0, xmesh[4], ymesh[4], rgb[4];
	double *x_imask = NULL, *y_imask = NULL, x_inc[4], y_inc[4], *x = NULL, *y = NULL;
	double *z = NULL, *v = NULL, *xx = NULL, *yy = NULL, *xval = NULL, *yval = NULL;

	struct GRDVIEW_CONT *start_cont = NULL, *this_cont = NULL, *last_cont = NULL;
	struct GRDVIEW_POINT *this_point = NULL, *last_point = NULL;
	struct GMT_GRID *Drape[3] = {NULL, NULL, NULL}, *Topo = NULL, *Intens = NULL, *Z = NULL;
	struct GRDVIEW_BIN *binij = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GRDVIEW_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdview_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdview_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdview_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdview_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdview_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdview main code ----------------------------*/

	GMT->current.plot.mode_3D = 1;	/* Only do background axis first; do foreground at end */
	use_intensity_grid = (Ctrl->I.active && !Ctrl->I.constant);	/* We want to use the intensity grid */
	
	if ((Topo = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	if (Ctrl->C.active) {
		if ((P = GMT_Get_CPT (GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, Topo->header->z_min, Topo->header->z_max)) == NULL) {
			Return (API->error);
		}
		if (P->is_bw) Ctrl->Q.monochrome = true;
		if (P->categorical && Ctrl->W.active) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Categorical data (as implied by CPT file) do not have contours.  Check plot.\n");
		}
	}
	get_contours = (Ctrl->Q.mode == GRDVIEW_MESH && Ctrl->W.contour) || (Ctrl->Q.mode == GRDVIEW_SURF && P->n_colors > 1);

	n_drape = (Ctrl->G.image) ? 3 : 1;

	if (use_intensity_grid && (Intens = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->I.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (Ctrl->G.active) {
		for (k = 0; k < n_drape; k++) if ((Drape[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->G.file[k], NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
	}

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, Topo->header->wesn, 4, double);	/* No -R, use grid region */
	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);
	
	if (GMT->common.R.wesn[ZLO] == 0.0 && GMT->common.R.wesn[ZHI] == 0.0) {
		GMT->common.R.wesn[ZLO] = Topo->header->z_min;
		GMT->common.R.wesn[ZHI] = Topo->header->z_max;
		if (Ctrl->N.active && Ctrl->N.level < GMT->common.R.wesn[ZLO]) GMT->common.R.wesn[ZLO] = Ctrl->N.level;
		if (Ctrl->N.active && Ctrl->N.level > GMT->common.R.wesn[ZHI]) GMT->common.R.wesn[ZHI] = Ctrl->N.level;
	}

	if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);

	/* Determine the wesn to be used to read the grid file */

	if (!GMT_grd_setregion (GMT, Topo->header, wesn, BCR_BILINEAR))
		nothing_inside = true;
	else if (use_intensity_grid && !GMT_grd_setregion (GMT, Intens->header, wesn, BCR_BILINEAR))
		nothing_inside = true;
	
	if (nothing_inside) {
		/* No grid to plot; just do empty map and bail */
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT->current.plot.mode_3D |= 2;	/* Ensure that foreground axis is drawn */
		GMT_plotcanvas (GMT);	/* Fill canvas if requested */
		GMT_map_basemap (GMT);
		GMT_plane_perspective (GMT, -1, 0.0);
		GMT_plotend (GMT);
		Return (GMT_OK);
	}

	/* Read data */

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing shape grid\n");

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->In.file, Topo) == NULL) {	/* Get topo data */
		Return (API->error);
	}
	t_reg = GMT_change_grdreg (GMT, Topo->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration */

	if (Ctrl->G.active) {	/* Draping wanted */
		for (k = 0; k < n_drape; k++) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Processing drape grid %s\n", Ctrl->G.file[k]);

			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->G.file[k], Drape[k]) == NULL) {	/* Get drape data */
				Return (API->error);
			}
			if (Drape[k]->header->nx != Topo->header->nx || Drape[k]->header->ny != Topo->header->ny) drape_resample = true;
			d_reg[k] = GMT_change_grdreg (GMT, Drape[k]->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration */
		}
		Z = Drape[0];
	}
	else
		Z = Topo;

	xval = GMT_grd_coord (GMT, Topo->header, GMT_X);
	yval = GMT_grd_coord (GMT, Topo->header, GMT_Y);

	if (!GMT->current.proj.xyz_pos[2]) double_swap (GMT->common.R.wesn[ZLO], GMT->common.R.wesn[ZHI]);	/* Negative z-scale, must flip */

	GMT_grd_set_ij_inc (GMT, Z->header->mx, ij_inc);	/* Offsets for ij (with pad) indices */
	nw = GMT_IJP (Topo->header, 0, 0);
	ne = GMT_IJP (Topo->header, 0, Topo->header->nx - 1);
	sw = GMT_IJP (Topo->header, Topo->header->ny - 1, 0);
	se = GMT_IJP (Topo->header, Topo->header->ny - 1, Topo->header->nx - 1);

#if 0
	grdview_init_setup (GMT, Topo, Ctrl->N.active, Ctrl->N.level);	/* Find projected min/max in y-direction */
#endif

	i_start = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? 0 : Z->header->nx - 2;
	i_stop  = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? Z->header->nx : 0;	i_stop--;
	i_inc   = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? 1 : -1;
	j_start = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? Z->header->ny - 1 : 1;
	j_stop  = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? 0 : Z->header->ny;
	j_inc   = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? -1 : 1;
	GMT_grd_set_ij_inc (GMT, Z->header->nx, bin_inc);	/* Offsets for bin (no pad) indices */
	
	x_inc[0] = x_inc[3] = 0.0;	x_inc[1] = x_inc[2] = Z->header->inc[GMT_X];
	y_inc[0] = y_inc[1] = 0.0;	y_inc[2] = y_inc[3] = Z->header->inc[GMT_Y];

	if (get_contours) {	/* Need to find contours */
		struct GMT_GRID *Z_orig = NULL;
		GMT_Report (API, GMT_MSG_VERBOSE, "Find contours\n");
		n_edges = Z->header->ny * (urint (ceil (Z->header->nx / 16.0)));
		edge = GMT_memory (GMT, NULL, n_edges, unsigned int);
		binij = GMT_memory (GMT, NULL, Topo->header->nm, struct GRDVIEW_BIN);
		small = GMT_CONV4_LIMIT * (Z->header->z_max - Z->header->z_min);
		if (small < 1.0e-7) small = 1.0e-7;	/* Make sure it is not smaller than single-precision EPS */
		if ((Z_orig = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, Z)) == NULL) Return (API->error);	/* Original copy of Z grid used for contouring */
		GMT_Report (API, GMT_MSG_VERBOSE, "Trace and bin contours...\n");
		for (c = 0; c < P->n_colors+1; c++) {	/* For each color change */

			/* Reset markers and set up new zero-contour */

			cval = (c == P->n_colors) ? P->range[c-1].z_high : P->range[c].z_low;

			if (cval < Z->header->z_min || cval > Z->header->z_max) continue;

			GMT_Report (API, GMT_MSG_VERBOSE, "Now tracing contour interval %8g\r", cval);
			/* Old version of loop below could give round-off since we kept subtracting the increments between successive contours.
			 * The safer way is to always start with original grid and subtract current contour value instead, as in grdcontour. PW, 11/18/2011 */
			GMT_grd_loop (GMT, Topo, row, col, ij) {
				if (!GMT_is_fnan (Z_orig->data[ij])) Z->data[ij] = Z_orig->data[ij] - (float)cval;
				if (Z->data[ij] == 0.0) Z->data[ij] += (float)small;
			}

			begin = true;
			while ((n = GMT_contours (GMT, Z, Ctrl->S.value, GMT->current.setting.interpolant, 0, edge, &begin, &x, &y)) > 0) {

				i_bin_old = j_bin_old = -1;
				for (pt = 1; pt < n; pt++) {
					/* Compute the lower-left bin i,j of the tile cut by the start of the contour (first 2 points) */
					i_bin = irint (floor (((0.5 * (x[pt-1] + x[pt]) - Z->header->wesn[XLO]) / Z->header->inc[GMT_X])));
					j_bin = irint (floor (((Z->header->wesn[YHI] - 0.5 * (y[pt-1] + y[pt])) / Z->header->inc[GMT_Y]))) + 1;
					if (i_bin != i_bin_old || j_bin != j_bin_old) {	/* Entering new bin */
						bin = j_bin * Z->header->nx + i_bin;
						this_cont = get_cont_struct (GMT, bin, binij, cval);
						this_cont->value = cval;
						this_cont->first_point = get_point (GMT, x[pt-1], y[pt-1]);
						this_point = this_cont->first_point;
						i_bin_old = i_bin;
						j_bin_old = j_bin;
					}
					this_point->next_point = get_point (GMT, x[pt], y[pt]);
					this_point = this_point->next_point;
				}
				GMT_free (GMT, x);
				GMT_free (GMT, y);
			}
		}

		/* Remove temporary variables */

		GMT_free (GMT, edge);

		/* Destroy original grid and use the copy in Z_orig instead */

		if (Ctrl->G.active) {
			if (GMT_Destroy_Data (API, &Drape[0]) != GMT_OK) {
				Return (API->error);
			}
			Drape[0] = Z_orig;
		}
		else {
			if (GMT_Destroy_Data (API, &Topo) != GMT_OK) {
				Return (API->error);
			}
			Topo = Z_orig;
		}
		Z = Z_orig;
		GMT_change_grdreg (GMT, Z->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration, again */
	}

	if (use_intensity_grid) {	/* Illumination wanted */

		GMT_Report (API, GMT_MSG_VERBOSE, "Processing illumination grid\n");

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->I.file, Intens) == NULL) {	/* Get intensity grid */
			Return (API->error);
		}
		if (Intens->header->nx != Topo->header->nx || Intens->header->ny != Topo->header->ny) {
			GMT_Report (API, GMT_MSG_NORMAL, "Intensity grid has improper dimensions!\n");
			Return (EXIT_FAILURE);
		}
		i_reg = GMT_change_grdreg (GMT, Intens->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration */
	}

	inc2[GMT_X] = 0.5 * Z->header->inc[GMT_X];	inc2[GMT_Y] = 0.5 * Z->header->inc[GMT_Y];

	for (row = 0; row < Topo->header->ny - 1; row++) {	/* Nodes part of tiles completely outside -R is set to NaN and thus not considered below */
		ij = GMT_IJP (Topo->header, row, 0);
		for (col = 0; col < Topo->header->nx - 1; col++, ij++) {
			for (jj = n_out = 0; jj < 2; jj++) {	/* Loop over the 4 nodes making up one tile */
				for (ii = 0; ii < 2; ii++) if (GMT_map_outside (GMT, xval[col+ii], yval[row+jj])) n_out++;
			}
			if (n_out == 4) Topo->data[ij] = Topo->data[ij+1] = Topo->data[ij+Topo->header->mx] = Topo->data[ij+Topo->header->mx+1] = GMT->session.f_NaN;	/* Entire tile is outside */
		}
	}

	max_alloc = 2 * (MAX (1,Ctrl->S.value) * (((Z->header->nx > Z->header->ny) ? Z->header->nx : Z->header->ny) + 2)) + 1;
	x = GMT_memory (GMT, NULL, max_alloc, double);
	y = GMT_memory (GMT, NULL, max_alloc, double);
	z = GMT_memory (GMT, NULL, max_alloc, double);
	v = GMT_memory (GMT, NULL, max_alloc, double);

	GMT_Report (API, GMT_MSG_VERBOSE, "Start creating PostScript plot\n");

	PSL = GMT_plotinit (GMT, options);

	PSL_setformat (PSL, 3);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */
	if (GMT->current.proj.three_D) GMT_map_basemap (GMT); /* Plot basemap first if 3-D */
	if (GMT->current.proj.z_pars[0] == 0.0) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
	GMT_plane_perspective (GMT, -1, 0.0);

	xx = GMT_memory (GMT, NULL, max_alloc, double);
	yy = GMT_memory (GMT, NULL, max_alloc, double);
	if (Ctrl->N.active) {
		PSL_comment (PSL, "Plot the plane at desired level\n");
		GMT_setpen (GMT, &Ctrl->W.pen[2]);
		if (!GMT->current.proj.z_project.draw[0]) {	/* Southern side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[0], GMT->current.proj.z_project.corner_y[0], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[1], GMT->current.proj.z_project.corner_y[1], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (col = 0; col < Z->header->nx; col++) GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YLO], Ctrl->N.level, &xx[col], &yy[col]);
				PSL_plotline (PSL, xx, yy, Z->header->nx, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[2]) {	/* Northern side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[3], GMT->current.proj.z_project.corner_y[3], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[2], GMT->current.proj.z_project.corner_y[2], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (col = 0; col < Z->header->nx; col++) GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YHI], Ctrl->N.level, &xx[col], &yy[col]);
				PSL_plotline (PSL, xx, yy, Z->header->nx, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[3]) {	/* Western side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[0], GMT->current.proj.z_project.corner_y[0], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[3], GMT->current.proj.z_project.corner_y[3], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (row = 0; row < Z->header->ny; row++) GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], GMT_grd_row_to_y (GMT, row, Z->header), Ctrl->N.level, &xx[row], &yy[row]);
				PSL_plotline (PSL, xx, yy, Z->header->ny, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[1]) {	/* Eastern side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[1], GMT->current.proj.z_project.corner_y[1], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[2], GMT->current.proj.z_project.corner_y[2], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (row = 0; row < Z->header->ny; row++) GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], GMT_grd_row_to_y (GMT, row, Z->header), Ctrl->N.level, &xx[row], &yy[row]);
				PSL_plotline (PSL, xx, yy, Z->header->ny, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}

		if (!GMT->common.R.oblique) {
			GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YLO], Ctrl->N.level, &xx[0], &yy[0]);
			GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YLO], Ctrl->N.level, &xx[1], &yy[1]);
			GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YHI], Ctrl->N.level, &xx[2], &yy[2]);
			GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YHI], Ctrl->N.level, &xx[3], &yy[3]);
			if (!GMT_is_fnan (Topo->data[nw])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YHI], (double)(Topo->data[nw]), &x_left, &y_top);
				PSL_plotsegment (PSL, x_left, y_top, xx[3], yy[3]);
			}
			if (!GMT_is_fnan (Topo->data[ne])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YHI], (double)(Topo->data[ne]), &x_right, &y_top);
				PSL_plotsegment (PSL, x_right, y_top, xx[2], yy[2]);
			}
			if (!GMT_is_fnan (Topo->data[se])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YLO], (double)(Topo->data[se]), &x_right, &y_bottom);
				PSL_plotsegment (PSL, x_right, y_bottom, xx[1], yy[1]);
			}
			if (!GMT_is_fnan (Topo->data[sw])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YLO], (double)(Topo->data[sw]), &x_left, &y_bottom);
				PSL_plotsegment (PSL, x_left, y_bottom, xx[0], yy[0]);
			}
		}
	}

	if (Ctrl->T.active) {	/* Plot image as polygonal pieces. Here, -JZ is not set */
		double *xx = NULL, *yy = NULL;
		struct GMT_FILL fill;
		struct GMT_DATASEGMENT S;
		GMT_init_fill (GMT, &fill, -1.0, -1.0, -1.0);	/* Initialize fill structure */

		GMT_Report (API, GMT_MSG_VERBOSE, "Tiling without interpolation\n");

		if (Ctrl->T.outline) GMT_setpen (GMT, &Ctrl->T.pen);
		GMT_memset (&S, 1, struct GMT_DATASEGMENT);
		S.coord = GMT_memory (GMT, NULL, 2, double *);
		GMT_grd_loop (GMT, Z, row, col, ij) {	/* Compute rgb for each pixel */
			if (GMT_is_fnan (Topo->data[ij]) && Ctrl->T.skip) continue;
			if (use_intensity_grid && Ctrl->T.skip && GMT_is_fnan (Intens->data[ij])) continue;
			GMT_get_rgb_from_z (GMT, P, Topo->data[ij], fill.rgb);
			if (use_intensity_grid)
				GMT_illuminate (GMT, Intens->data[ij], fill.rgb);
			else
				GMT_illuminate (GMT, Ctrl->I.value, fill.rgb);
			n = GMT_graticule_path (GMT, &xx, &yy, 1, true, xval[col] - inc2[GMT_X], xval[col] + inc2[GMT_X], yval[row] - inc2[GMT_Y], yval[row] + inc2[GMT_Y]);
			GMT_setfill (GMT, &fill, Ctrl->T.outline);
			S.coord[GMT_X] = xx;	S.coord[GMT_Y] = yy;	S.n_rows = n;
			GMT_geo_polygons (GMT, &S);
			GMT_free (GMT, xx);
			GMT_free (GMT, yy);
		}
		GMT_free (GMT, S.coord);
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
	}
	
	else if (Ctrl->Q.mode == GRDVIEW_IMAGE) {	/* Plot image */
		int nx_i, ny_i, ip, jp, min_i, max_i, min_j, max_j, dist;
		int done, layers, last_i, last_j;
		int *top_jp = NULL, *bottom_jp = NULL, *ix = NULL, *iy = NULL;
		uint64_t d_node, nm_i, node, kk, p;
		double xp, yp, sum_w, w, sum_i, x_width, y_width, value;
		double sum_r, sum_g, sum_b, intval = 0.0, *y_drape = NULL, *x_drape = NULL;
		float *int_drape = NULL;
		unsigned char *bitimage_24 = NULL, *bitimage_8 = NULL;

		if (Ctrl->C.active && P->has_pattern) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Patterns in cpt file will not work with -Qi\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "Get and store projected vertices\n");

		PSL_comment (PSL, "Plot 3-D surface using scanline conversion of polygons to raster image\n");

		x_width = GMT->current.proj.z_project.xmax - GMT->current.proj.z_project.xmin;	/* Size of image in inches */
		y_width = GMT->current.proj.z_project.ymax - GMT->current.proj.z_project.ymin;
		nx_i = irint (x_width * Ctrl->Q.dpi);	/* Size of image in pixels */
		ny_i = irint (y_width * Ctrl->Q.dpi);
		last_i = nx_i - 1;	last_j = ny_i - 1;

		if (drape_resample) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Resampling illumination grid to drape grid resolution\n");
			ix = GMT_memory (GMT, NULL, Z->header->nm, int);
			iy = GMT_memory (GMT, NULL, Z->header->nm, int);
			x_drape = GMT_grd_coord (GMT, Z->header, GMT_X);
			y_drape = GMT_grd_coord (GMT, Z->header, GMT_Y);
			if (use_intensity_grid) int_drape = GMT_memory (GMT, NULL, Z->header->mx*Z->header->my, float);
			bin = 0;
			GMT_grd_loop (GMT, Z, row, col, ij) {	/* Get projected coordinates converted to pixel locations */
				value = GMT_get_bcr_z (GMT, Topo, x_drape[col], y_drape[row]);
				if (GMT_is_dnan (value))	/* Outside -R or NaNs not used */
					ix[bin] = iy[bin] = -1;
				else {
					GMT_geoz_to_xy (GMT, x_drape[col], y_drape[row], value, &xp, &yp);
					/* Make sure ix,iy fall in the range (0,nx_i-1), (0,ny_i-1) */
					ix[bin] = MAX(0, MIN(irint (floor((xp - GMT->current.proj.z_project.xmin) * Ctrl->Q.dpi)), last_i));
					iy[bin] = MAX(0, MIN(irint (floor((yp - GMT->current.proj.z_project.ymin) * Ctrl->Q.dpi)), last_j));
				}
				if (use_intensity_grid) int_drape[ij] = (float)GMT_get_bcr_z (GMT, Intens, x_drape[col], y_drape[row]);
				bin++;
			}
			GMT_free (GMT, x_drape);
			GMT_free (GMT, y_drape);
			if (use_intensity_grid) {	/* Reset intensity grid so that we have no boundary row/cols */
				GMT_free_aligned (GMT, Intens->data);
				Intens->data = int_drape;
			}
		}
		else {
			ix = GMT_memory (GMT, NULL, Topo->header->nm, int);
			iy = GMT_memory (GMT, NULL, Topo->header->nm, int);
			bin = 0;
			GMT_grd_loop (GMT, Z, row, col, ij) {	/* Get projected coordinates converted to pixel locations */
				if (GMT_is_fnan (Topo->data[ij]))	/* Outside -R or NaNs not used */
					ix[bin] = iy[bin] = -1;
				else {
					GMT_geoz_to_xy (GMT, xval[col], yval[row], (double)Topo->data[ij], &xp, &yp);
					/* Make sure ix,iy fall in the range (0,nx_i-1), (0,ny_i-1) */
					ix[bin] = MAX(0, MIN(irint (floor((xp - GMT->current.proj.z_project.xmin) * Ctrl->Q.dpi)), last_i));
					iy[bin] = MAX(0, MIN(irint (floor((yp - GMT->current.proj.z_project.ymin) * Ctrl->Q.dpi)), last_j));
				}
				bin++;
			}
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);

		/* Allocate image array and set background to PAGE_COLOR */

		if (Ctrl->Q.monochrome) {
			char gray;

			nm_i = nx_i * ny_i;
			layers = 1;
			bitimage_8 = GMT_memory (GMT, NULL, nm_i, unsigned char);
			gray = GMT_u255 (GMT_YIQ (GMT->current.setting.ps_page_rgb));
			memset (bitimage_8, gray, nm_i * sizeof (unsigned char));
		}
		else {
			nm_i = nx_i * ny_i * 3;
			layers = 3;
			if (Ctrl->Q.mask) PS_colormask_off = 3;
			bitimage_24 = GMT_memory (GMT, NULL, nm_i + PS_colormask_off, unsigned char);
			if (Ctrl->C.active && Ctrl->Q.mask)
				GMT_rgb_copy (rgb, P->patch[GMT_NAN].rgb);
			else
				GMT_rgb_copy (rgb, GMT->current.setting.ps_page_rgb);
			kk = 0;
			while (kk < (nm_i + PS_colormask_off)) {
				for (k = 0; k < 3; k++) bitimage_24[kk++] = GMT_u255 (rgb[k]);
			}
		}

		if (!Ctrl->Q.mask) {	/* Set up arrays for staircase clippath and initialize them */
			top_jp = GMT_memory (GMT, NULL, nx_i, int);
			bottom_jp = GMT_memory (GMT, NULL, nx_i, int);
			for (ip = 0; ip < nx_i; ip++) bottom_jp[ip] = ny_i;
		}

		/* Plot from back to front */

		GMT_memset (rgb, 4, double);
		GMT_Report (API, GMT_MSG_VERBOSE, "Start rasterization\n");
		for (j = j_start; j != j_stop; j += j_inc) {

			GMT_Report (API, GMT_MSG_VERBOSE, "Scan line conversion at j-line %.6ld\r", j);

			for (i = i_start; i != i_stop; i += i_inc) {
				bin = GMT_IJ0 (Z->header, j, i);
				ij = GMT_IJP (Z->header, j, i);
				for (k = bad = 0; !bad && k < 4; k++) bad = (ix[bin+bin_inc[k]] < 0 || iy[bin+bin_inc[k]] < 0);
				if (bad) continue;

				min_i = max_i = ix[bin];
				min_j = max_j = iy[bin];
				for (k = 1; k < 4; k++) {
					p = bin+bin_inc[k];
					if (ix[p] < min_i) min_i = ix[p];
					if (ix[p] > max_i) max_i = ix[p];
					if (iy[p] < min_j) min_j = iy[p];
					if (iy[p] > max_j) max_j = iy[p];
				}
				for (jp = min_j; jp <= max_j; jp++) {	/* Loop over all the pixels that will make up this tile */
					if (jp < 0 || jp >= ny_i) continue;
					for (ip = min_i; ip <= max_i; ip++) {
						if (ip < 0 || ip >= nx_i) continue;
						if (!pixel_inside (GMT, ip, jp, ix, iy, bin, bin_inc)) continue;
						/* These pixels are part of the current tile */
						if (!Ctrl->Q.mask) {	/* Update clip mask */
							if (jp > top_jp[ip]) top_jp[ip] = jp; 
							if (jp < bottom_jp[ip]) bottom_jp[ip] = jp;
						}

						sum_r = sum_g = sum_b = sum_w = sum_i = 0.0;
						done = false;
						for (k = good = 0; !done && k < 4; k++) {	/* Loop over the 4 corners of the present tile */
							node = bin + bin_inc[k];
							d_node = ij + ij_inc[k];
							if (Ctrl->G.image) {	/* Have 3 grids with R,G,B values */
								for (kk = 0; kk < 3; kk++) {
									rgb[kk] = GMT_is255 (Drape[kk]->data[d_node]);
									if (rgb[kk] < 0.0) rgb[kk] = 0; else if (rgb[kk] > 1.0) rgb[kk] = 1.0;
								}
								if (Ctrl->C.active && GMT_same_rgb (rgb, P->patch[GMT_NAN].rgb)) continue;	/* Skip NaN colors */
							}
							else {		/* Use lookup to get color */
								GMT_get_rgb_from_z (GMT, P, Z->data[d_node], rgb);
								if (GMT_is_fnan (Z->data[d_node])) continue;	/* Skip NaNs in the z-data*/
							}
							if (use_intensity_grid && GMT_is_fnan (Intens->data[d_node])) continue;	/* Skip NaNs in the intensity data*/
							/* We don't want to blend in the (typically) gray NaN colors with the others. */
							
							good++;
							dist = quick_idist (ip, jp, ix[node], iy[node]);
							if (dist == 0) {	/* Only need this corner value */
								done = true;
								if (Ctrl->I.active) intval = (use_intensity_grid) ? Intens->data[d_node] : Ctrl->I.value;
							}
							else {	/* Crude weighted average based on 1/distance to the nearest node */
								w = 1.0 / (double)dist;
								sum_r += rgb[0] * w;
								sum_g += rgb[1] * w;
								sum_b += rgb[2] * w;
								if (use_intensity_grid) sum_i += Intens->data[d_node] * w;
								sum_w += w;
							}
						}
						if (!done && good) {	/* Must get weighted value when more than one non-nan value was found */
							sum_w = 1.0 / sum_w;
							rgb[0] = sum_r * sum_w;
							rgb[1] = sum_g * sum_w;
							rgb[2] = sum_b * sum_w;
							if (Ctrl->I.active) intval = (use_intensity_grid) ? sum_i * sum_w : Ctrl->I.value;
						}
						if (Ctrl->Q.special) GMT_get_rgb_from_z (GMT, P, Z->data[ij], rgb);
						if (Ctrl->I.active && good) GMT_illuminate (GMT, intval, rgb);
						kk = layers * ((ny_i-jp-1) * nx_i + ip) + PS_colormask_off;
						if (Ctrl->Q.monochrome) /* GMT_YIQ transformation */
							bitimage_8[kk] = (unsigned char) GMT_YIQ (rgb);
						else {
							for (k = 0; k < 3; k++) bitimage_24[kk++] = GMT_u255 (rgb[k]);
						}
					}
				}
			}
		}

		if (!Ctrl->Q.mask) {	/* Must implement the clip path for the image */

			x_pixel_size = x_width / (double)nx_i;
			y_pixel_size = y_width / (double)ny_i;
			n4 = 4 * nx_i;
			x_imask = GMT_memory (GMT, NULL, n4, double);
			y_imask = GMT_memory (GMT, NULL, n4, double);
			nk = n4 - 1;

			for (ip = k = 0; ip < nx_i; ip++, k += 2) {
				k1 = k + 1;
				x_imask[k]  = x_imask[nk-k]  = GMT->current.proj.z_project.xmin + ip * x_pixel_size;
				x_imask[k1] = x_imask[nk-k1] = x_imask[k] + x_pixel_size;
				if (top_jp[ip] < bottom_jp[ip]) {	/* No pixels set in this column */
					y_imask[k] = y_imask[k1] = y_imask[nk-k] = y_imask[nk-k1] = GMT->current.proj.z_project.ymin;
				}
				else {	/* Set top of upper pixel and bottom of lower pixel */
					y_imask[k] = y_imask[k1] = GMT->current.proj.z_project.ymin + (top_jp[ip] + 1) * y_pixel_size;
					y_imask[nk-k] = y_imask[nk-k1] = GMT->current.proj.z_project.ymin + bottom_jp[ip] * y_pixel_size;
				}
			}
			PSL_beginclipping (PSL, x_imask, y_imask, 4 * nx_i, GMT->session.no_rgb, 3);
			GMT_free (GMT, x_imask);
			GMT_free (GMT, y_imask);
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "Creating PostScript image ");
		if (Ctrl->Q.monochrome) {
			if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "[B/W image]\n");
			PSL_plotcolorimage (PSL, GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin,
			                    x_width, y_width, PSL_BL, bitimage_8, nx_i, ny_i, 8);
			GMT_free (GMT, bitimage_8);
		}
		else {
			if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "[color image]\n");
			PSL_plotcolorimage (PSL, GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin,
			                    x_width, y_width, PSL_BL, bitimage_24, Ctrl->Q.mask ? -nx_i : nx_i, ny_i, 24);
			GMT_free (GMT, bitimage_24);
		}

		if (!Ctrl->Q.mask){
			PSL_endclipping (PSL, 1);	/* Undo mask clipping */
			GMT_free (GMT, top_jp);
			GMT_free (GMT, bottom_jp);
		}

		GMT_free (GMT, ix);
		GMT_free (GMT, iy);
	}

	else if (Ctrl->Q.mode == GRDVIEW_WATERFALL_Y) {	/* Plot Y waterfall */
		double z_base = Ctrl->N.active ? Ctrl->N.level : Z->header->z_min;
		PSL_comment (PSL, "Start of waterfall plot\n");
		GMT_setpen (GMT, &Ctrl->W.pen[1]);
		GMT_setfill (GMT, &Ctrl->Q.fill, true);
		if (Ctrl->Q.monochrome)
			Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = GMT_YIQ (Ctrl->Q.fill.rgb);	/* Do GMT_YIQ transformation */
		for (i = i_start+1; i != i_stop; i += i_inc) {
			for (k = 0, j = j_start-1; j != j_stop; j += j_inc, k++) {
				ij  = GMT_IJP(Topo->header, j, i);
				if (GMT_is_fnan(Topo->data[ij+ij_inc[0]])) continue;
				GMT_geoz_to_xy (GMT, xval[i], yval[j], (double)(Topo->data[ij+ij_inc[1]]), &xx[k], &yy[k]);
			}
			GMT_geoz_to_xy (GMT, xval[i], yval[j_start-1] + (k - 1) * (i_inc * Z->header->inc[GMT_Y]), z_base, &xx[k], &yy[k]);
			GMT_geoz_to_xy (GMT, xval[i], yval[j_start-1], z_base, &xx[k+1], &yy[k+1]);
			PSL_plotpolygon (PSL, xx, yy, k+2);
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
	}

	else if (Ctrl->Q.mode == GRDVIEW_WATERFALL_X) {	/* Plot X waterfall */
		double z_base = Ctrl->N.active ? Ctrl->N.level : Z->header->z_min;
		PSL_comment (PSL, "Start of waterfall plot\n");
		GMT_setpen (GMT, &Ctrl->W.pen[1]);
		GMT_setfill (GMT, &Ctrl->Q.fill, true);
		if (Ctrl->Q.monochrome)
			Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = GMT_YIQ (Ctrl->Q.fill.rgb);	/* Do GMT_YIQ transformation */
		for (j = j_start-1; j != j_stop; j += j_inc) {
			for (k = 0, i = i_start+1; i != i_stop; i += i_inc, k++) {
				ij  = GMT_IJP(Topo->header, j, i);
				if (GMT_is_fnan(Topo->data[ij+ij_inc[0]])) continue;
				GMT_geoz_to_xy (GMT, xval[i_start+1] + k * (i_inc * Z->header->inc[GMT_X]), yval[j],		// i_inc = -1
				                (double)(Topo->data[ij+ij_inc[1]]), &xx[k], &yy[k]);
			}
			GMT_geoz_to_xy (GMT, xval[i_start+1] + (k - 1) * (i_inc * Z->header->inc[GMT_X]), yval[j], z_base, &xx[k], &yy[k]);
			GMT_geoz_to_xy (GMT, xval[i_start+1], yval[j], z_base, &xx[k+1], &yy[k+1]);
			PSL_plotpolygon (PSL, xx, yy, k+2);
			//PSL_plotline (PSL, xx, yy, k+2, PSL_MOVE + PSL_STROKE);
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
	}

	else if (Ctrl->Q.mode == GRDVIEW_MESH) {	/* Plot mesh */
		GMT_Report (API, GMT_MSG_VERBOSE, "Do mesh plot with mesh color %s\n", GMT_putcolor (GMT, Ctrl->Q.fill.rgb));
		PSL_comment (PSL, "Start of mesh plot\n");
		GMT_setpen (GMT, &Ctrl->W.pen[1]);
		if (Ctrl->Q.monochrome)
			Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = GMT_YIQ (Ctrl->Q.fill.rgb);	/* Do GMT_YIQ transformation */
		for (j = j_start; j != j_stop; j += j_inc) {
			y_bottom = yval[j];
			y_top = y_bottom + abs (j_inc) * Z->header->inc[GMT_Y];
			for (i = i_start; i != i_stop; i += i_inc) {
				bin = GMT_IJ0 (Z->header, j, i);
				ij = GMT_IJP (Topo->header, j, i);
				for (k = bad = 0; !bad && k < 4; k++) bad += GMT_is_fnan (Topo->data[ij+ij_inc[k]]);
				if (bad) continue;
				x_left = xval[i];
				x_right = x_left + abs (i_inc) * Z->header->inc[GMT_X];
				GMT_geoz_to_xy (GMT, x_left, y_bottom, (double)(Topo->data[ij+ij_inc[0]]), &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, x_right, y_bottom, (double)(Topo->data[ij+ij_inc[1]]), &xx[1], &yy[1]);
				GMT_geoz_to_xy (GMT, x_right, y_top, (double)(Topo->data[ij+ij_inc[2]]), &xx[2], &yy[2]);
				GMT_geoz_to_xy (GMT, x_left, y_top, (double)(Topo->data[ij+ij_inc[3]]), &xx[3], &yy[3]);
				GMT_setfill (GMT, &Ctrl->Q.fill, true);
				PSL_plotpolygon (PSL, xx, yy, 4);
				if (Ctrl->W.contour) {
					pen_set = false;
					if (binij[bin].first_cont == NULL) continue;
					for (this_cont = binij[bin].first_cont->next_cont; this_cont; this_cont = this_cont->next_cont) {
						for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
							z_val = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, (double)this_point->x, (double)this_point->y) : this_cont->value;
							if (GMT_is_dnan (z_val)) continue;
							GMT_geoz_to_xy (GMT, (double)this_point->x, (double)this_point->y, z_val, &xx[k], &yy[k]);
							k++;
						}
						if (!pen_set) {
							GMT_setpen (GMT, &Ctrl->W.pen[0]);
							pen_set = true;
						}
						PSL_plotline (PSL, xx, yy, k, PSL_MOVE + PSL_STROKE);
					}
					if (pen_set) GMT_setpen (GMT, &Ctrl->W.pen[1]);
				}
			}
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
	}
	
	else if (Ctrl->Q.mode == GRDVIEW_SURF) {	/* Plot surface using closed polygons */
		int start_side, entry_side, exit_side, next_side, low, ncont, nw_se_diagonal, check;
		int corner[2], bad_side[2][2], p, p1, p2, saddle_sign;
		double *xcont = NULL, *ycont = NULL, *zcont = NULL, *vcont = NULL, X_vert[4], Y_vert[4], saddle_small;
		float Z_vert[4];

		/* PW: Bugs fixed in Nov, 2011: Several problems worth remembering:
			1) Earlier [2004] we had fixed grdcontour but not grdview in dealing with the current zero contour.  Because
			   of float precision we cannot take the grid and repeatedly subtract the difference in contour values.
			   Instead for each contour value cval, we must subtract cval from the original grid to get the tmp grid.
			2) To avoid never to have contours go through nodes EXACTLY, we check if there are nodes that equal zero.
			   If so, we add small (a small amount to make it different from zero).  We then get contours.  However,
			   for -Qs we again use the grid node values to determine which way to loop around a polygon piece when
			   stitching it together from pieces of contours and node values. Thus it is important that we use node
			   values that have been adjusted as above and not the original nodes.
			3) We should make sure that the small value exceeeds single-precision EPS (~ 1.2e-7), otherwise adding small
			   will not make the node non-zero.  This does not seem to have been a problem yet but I added a new check
			   just in case and if so set small to 1.0e-7.
			4) Given zgrd is float, there is a difference between zgrd[node] -= (float)cval and zgrd[node] -= cval,
			   since cval is double precision.  We had the cast during the contouring but here under -Qs we had some
			   comparisions without the cast, the result being both sides got promoted to double prior to the test and
			   we can get a different result.
			5) In the case of no contour we paint the entire tile.  However, in the case of discontinuous CPT files we
			   based the color on the lower-left node value (since it should not matter which corner we pick).  But it
			   does: If that node HAPPENS to be one that had small added to it and as a result no contour goes through,
			   we have no way of adjusting the node accordingly.  Consequently, it is safer to always take the average
			   of the 4 nodes as the other 3 will pull the average into the middle somewhere.
		*/
			
		xcont = GMT_memory (GMT, NULL, max_alloc, double);
		ycont = GMT_memory (GMT, NULL, max_alloc, double);
		zcont = GMT_memory (GMT, NULL, max_alloc, double);
		vcont = GMT_memory (GMT, NULL, max_alloc, double);

		PSL_comment (PSL, "Start of filled surface\n");
		if (Ctrl->Q.outline) GMT_setpen (GMT, &Ctrl->W.pen[1]);

		for (j = j_start; j != j_stop; j += j_inc) {
			y_bottom = yval[j];
			y_top = y_bottom + Z->header->inc[GMT_Y];
			for (i = i_start; i != i_stop; i += i_inc) {
				ij = GMT_IJP (Topo->header, j, i);
				bin = GMT_IJ0 (Topo->header, j, i);
				x_left = xval[i];
				x_right = x_left + Z->header->inc[GMT_X];
				for (k = bad = 0; !bad && k < 4; k++) bad += GMT_is_fnan (Topo->data[ij+ij_inc[k]]);
				if (bad) {
					if (P->patch[GMT_NAN].skip || GMT->current.proj.three_D) continue;

					X_vert[0] = X_vert[3] = x_left;	X_vert[1] = X_vert[2] = x_right;
					Y_vert[0] = Y_vert[1] = y_bottom;	Y_vert[2] = Y_vert[3] = y_top;
					for (k = 0; k < 4; k++) GMT_geoz_to_xy (GMT, X_vert[k], Y_vert[k], 0.0, &xmesh[k], &ymesh[k]);
					paint_it_grdview (GMT, PSL, P, xmesh, ymesh, 4, GMT->session.d_NaN, false, Ctrl->Q.monochrome, 0.0, Ctrl->Q.outline);
					continue;
				}

				if (Ctrl->I.active) {
					if (use_intensity_grid) {
						this_intensity = get_intensity (Intens, ij);
						if (GMT_is_dnan (this_intensity)) continue;
					}
					else
						this_intensity = Ctrl->I.value;
				}
	
				/* Get mesh polygon */

				X_vert[0] = X_vert[3] = x_left;		X_vert[1] = X_vert[2] = x_right;
				Y_vert[0] = Y_vert[1] = y_bottom;	Y_vert[2] = Y_vert[3] = y_top;
				
				/* Also get z-values at the 4 nodes.  Because some nodes may have been adjusted by small during
				 * the contouring stage we need to make the same adjustments below */
				
				for (k = 0; k < 4; k++) Z_vert[k] = Z->data[ij+ij_inc[k]];	/* First a straight copy */

				if (get_contours && binij[bin].first_cont) {	/* Contours go thru here */

					/* Determine if this bin will give us saddle trouble */

					start_cont = this_cont = binij[bin].first_cont->next_cont;
					saddle = false;
					while (!saddle && this_cont->next_cont) {
						if (this_cont->next_cont->value == this_cont->value)
							saddle = true;
						else
							this_cont = this_cont->next_cont;
					}
					for (k = 0; k < 4; k++) {	/* Deal with the fact that some nodes may have had small added to them */
						Z_vert[k] -= (float)this_cont->value;	/* Note we cast to float to get the same precision as for contours */
						if (Z_vert[k] == 0.0) Z_vert[k] += (float)small;
						Z_vert[k] += (float)this_cont->value;
					}
					/* Here, Z_vert reflects what the grid was when contouring was determined */

					if (saddle) {	/* Must deal with this separately */

						this_point = this_cont->first_point;
						entry_side = get_side (this_point->x, this_point->y, x_left, y_bottom, Z->header->inc, inc2);
						while (this_point->next_point) this_point = this_point->next_point;	/* Go to end */
						exit_side  = get_side (this_point->x, this_point->y, x_left, y_bottom, Z->header->inc, inc2);


						if (MIN (Z_vert[1], Z_vert[3]) > MAX (Z_vert[0], Z_vert[2])) {
							saddle_sign = +1;
							check = true;
						}
						else if (MAX (Z_vert[1], Z_vert[3]) < MIN (Z_vert[0], Z_vert[2])) {
							saddle_sign = -1;
							check = true;
						}
						else if (MIN (Z_vert[0], Z_vert[2]) > MAX (Z_vert[1], Z_vert[3])) {
							saddle_sign = +1;
							check = false;
						}
						else {
							saddle_sign = -1;
							check = false;
						}
						nw_se_diagonal = ((entry_side + exit_side) == 3);
						if (nw_se_diagonal != check) saddle_sign = -saddle_sign;
						if (nw_se_diagonal) {	/* Diagonal goes NW - SE */
							corner[0] = 0;	bad_side[0][0] = 1;	bad_side[0][1] = 2;
							corner[1] = 2;	bad_side[1][0] = 0;	bad_side[1][1] = 3;
						}
						else {	/* Diagonal goes NE -SW */
							corner[0] = 1;	bad_side[0][0] = 2;	bad_side[0][1] = 3;
							corner[1] = 3;	bad_side[1][0] = 0;	bad_side[1][1] = 1;
						}
						saddle_small = saddle_sign * small;

						for (p = 0; p < 2; p++) {	/* For each triangular half */

							/* Set this points as the start anchor */

							low = corner[p];
							n = 0;
							add_node (x, y, z, v, &n, low, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[low]);
							start_side = next_side = low;
							way = 0;

							for (this_cont = start_cont; this_cont; this_cont = this_cont->next_cont) {

								/* First get all the x/y pairs for this contour */

								for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
									xcont[k] = this_point->x;
									ycont[k] = this_point->y;
									zcont[k] = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, xcont[k], ycont[k]) : this_cont->value;
									if (GMT_is_dnan (zcont[k])) continue;
									vcont[k] = this_cont->value;
									k++;
								}
								ncont = k;

								entry_side = get_side (xcont[0], ycont[0], x_left, y_bottom, Z->header->inc, inc2);
								exit_side  = get_side (xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

								if (entry_side == bad_side[p][0] || entry_side == bad_side[p][1]) continue;
								if (exit_side == bad_side[p][0] || exit_side == bad_side[p][1]) continue;

								/* OK, got the correct contour */

								next_up = (this_cont->next_cont) ? this_cont->next_cont->value : DBL_MAX;

								exit_side  = get_side (xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

								if (way == 0 || next_side == entry_side) {	/* Just hook up */
									copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
									next_side = exit_side;
								}
								else if (next_side == exit_side) {	/* Just hook up but reverse */
									copy_points_bw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
									next_side = entry_side;
								}
								/* Compute the xy from the xyz triplets */

								for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);
								z_ave = (P->is_continuous) ? get_z_ave (v, next_up, n) : this_cont->value;

								/* Now paint the polygon piece */

								paint_it_grdview (GMT, PSL, P, xx, yy, (int)n, z_ave-saddle_small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, false);

								/* Reset the anchor points to previous contour */

								n = 0;
								copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = exit_side;
								start_side = entry_side;
								way = (Z_vert[low] < (float)this_cont->value) ? -1 : 1;
							}

							/* Final contour needs to add diagonal */

							if (corner[p] == 0 || corner[p] == 2) {
								p1 = (next_side < 2) ? 1 : 3;
								p2 = (next_side < 2) ? 3 : 1;
							}
							else {
								p1 = (next_side % 3) ? 2 : 0;
								p2 = (next_side % 3) ? 0 : 2;
							}
							add_node (x, y, z, v, &n, p1, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[p1]);
							add_node (x, y, z, v, &n, p2, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[p2]);

							/* Compute the xy from the xyz triplets */

							for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);

							z_ave = (P->is_continuous) ? get_z_ave (v, next_up, n) : v[0];

							/* Now paint the polygon piece */

							paint_it_grdview (GMT, PSL, P, xx, yy, (int)n, z_ave+saddle_small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, false);

						} /* End triangular piece */

					} /* End Saddle section */
					else {
						/* Ok, here we do not have to worry about saddles */

						/* Find lowest corner (id = low) */

						for (k = 1, low = 0; k < 4; k++) if (Z_vert[k] < Z_vert[low]) low = k;

						/* Set this points as the start anchor */

						n = 0;
						add_node (x, y, z, v, &n, low, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[low]);
						start_side = next_side = low;
						way = 1;

						this_cont = start_cont;
						while (this_cont) {

							next_up = (this_cont->next_cont) ? this_cont->next_cont->value : DBL_MAX;
							/* First get all the x/y pairs for this contour */

							for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
								xcont[k] = this_point->x;
								ycont[k] = this_point->y;
								zcont[k] = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, xcont[k], ycont[k]) : this_cont->value;
								if (GMT_is_dnan (zcont[k])) continue;
								vcont[k] = this_cont->value;
								k++;
							}
							ncont = k;

							entry_side = get_side (xcont[0], ycont[0], x_left, y_bottom, Z->header->inc, inc2);
							exit_side  = get_side (xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

							while (!(next_side == entry_side || next_side == exit_side)) {	/* Must add intervening corner */
								if (way == 1) next_side = (next_side + 1) % 4;
								add_node (x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[next_side]);
								if (way == -1) next_side = (next_side - 1 + 4) % 4;
							}
							if (next_side == entry_side) {	/* Just hook up */
								copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = exit_side;
							}
							else if (next_side == exit_side) {	/* Just hook up but reverse */
								copy_points_bw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = entry_side;
							}
							/* Now we must complete the polygon if necessary */

							while (!(start_side == next_side)) {	/* Must add intervening corner */
								if (way == 1) next_side = (next_side + 1) % 4;
								add_node (x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[next_side]);
								if (way == -1) next_side = (next_side - 1 + 4) % 4;
							}

							/* Compute the xy from the xyz triplets */

							for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);
							z_ave = (P->is_continuous) ? get_z_ave (v, next_up, n) : this_cont->value;

							/* Now paint the polygon piece */

							paint_it_grdview (GMT, PSL, P, xx, yy, (int)n, z_ave-small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, false);

							/* Reset the anchor points to previous contour */

							n = 0;
							copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
							next_side = exit_side;
							start_side = entry_side;
							way = (Z_vert[start_side] < (float)this_cont->value) ? -1 : 1;

							this_cont = this_cont->next_cont;	/* Goto next contour */
 						}

						/* Final contour needs to compete with corners only */

						while (!(start_side == next_side)) {	/* Must add intervening corner */
							if (way == 1) next_side = (next_side +1) % 4;
							add_node (x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[next_side]);
							if (way == -1) next_side = (next_side - 1 + 4) % 4;
						}

						/* Compute the xy from the xyz triplets */

						for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);

						z_ave = (P->is_continuous) ? get_z_ave (v, next_up, n) : v[0];

						/* Now paint the polygon piece */

						paint_it_grdview (GMT, PSL, P, xx, yy, (int)n, z_ave+small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, false);

					} /* End non-saddle case */

					/* Draw contour lines if desired */

					pen_set = false;
					for (this_cont = start_cont; Ctrl->W.contour && this_cont; this_cont = this_cont->next_cont) {
						for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
							z_val = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, (double)this_point->x, (double)this_point->y) : this_cont->value;
							if (GMT_is_dnan (z_val)) continue;

							GMT_geoz_to_xy (GMT, (double)this_point->x, (double)this_point->y, z_val, &xx[k], &yy[k]);
							k++;
						}
						if (!pen_set) {
							GMT_setpen (GMT, &Ctrl->W.pen[0]);
							pen_set = true;
						}
						PSL_plotline (PSL, xx, yy, k, PSL_MOVE + PSL_STROKE);
					}
					if (pen_set) GMT_setpen (GMT, &Ctrl->W.pen[1]);
					if (Ctrl->Q.outline) {
						for (k = 0; k < 4; k++) GMT_geoz_to_xy (GMT, X_vert[k], Y_vert[k], (double)(Topo->data[ij+ij_inc[k]]), &xmesh[k], &ymesh[k]);
						PSL_setfill (PSL, GMT->session.no_rgb, true);
						PSL_plotpolygon (PSL, xmesh, ymesh, 4);
					}
				}
				else {	/* No Contours */

					/* For stability, take the color corresponding to the average value of the four corners */
					z_ave = 0.25 * (Z_vert[0] + Z_vert[1] + Z_vert[2] + Z_vert[3]);

					/* Now paint the polygon piece */

					for (k = 0; k < 4; k++) GMT_geoz_to_xy (GMT, X_vert[k], Y_vert[k], (double)(Topo->data[ij+ij_inc[k]]), &xmesh[k], &ymesh[k]);
					paint_it_grdview (GMT, PSL, P, xmesh, ymesh, 4, z_ave+small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, Ctrl->Q.outline);
				}
			}
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
		GMT_free (GMT, xcont);
		GMT_free (GMT, ycont);
		GMT_free (GMT, zcont);
		GMT_free (GMT, vcont);
	}

	if (Ctrl->W.pen[1].style || Ctrl->W.pen[0].style) PSL_setdash (PSL, NULL, 0);

	if (GMT->current.proj.z_pars[0] == 0.0) GMT_map_clip_off (GMT);

	if (Ctrl->N.facade) {	/* Cover the two front sides */
		PSL_comment (PSL, "Painting the frontal facade\n");
		GMT_setpen (GMT, &Ctrl->W.pen[2]);
		GMT_setfill (GMT, &Ctrl->N.fill, true);
		if (!GMT->current.proj.z_project.draw[0])	{	/* Southern side */
			for (col = 0, n = 0, ij = sw; col < Z->header->nx; col++, ij++) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YLO], (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (col = Z->header->nx ; col > 0; col--, n++)
				GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (GMT, col-1, Z->header), Z->header->wesn[YLO], Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
		if (!GMT->current.proj.z_project.draw[1]) {	/*	Eastern side */
			for (row = 0, n = 0, ij = ne; row < Z->header->ny; row++, ij += Topo->header->mx) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], GMT_grd_row_to_y (GMT, row, Z->header), (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (row = Z->header->ny; row > 0; row--, n++)
				GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], GMT_grd_row_to_y (GMT, row-1, Z->header), Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
		if (!GMT->current.proj.z_project.draw[2])	{	/* Northern side */
			for (col = 0, n = 0, ij = nw; col < Z->header->nx; col++, ij++) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YHI], (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (col = Z->header->nx; col > 0; col--, n++)
				GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (GMT, col-1, Z->header), Z->header->wesn[YHI], Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
		if (!GMT->current.proj.z_project.draw[3]) {	/*	Western side */
			for (row = 0, n = 0, ij = nw; row < Z->header->ny; row++, ij += Topo->header->mx) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], GMT_grd_row_to_y (GMT, row, Z->header), (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (row = Z->header->ny; row > 0; row--, n++)
				GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], GMT_grd_row_to_y (GMT, row-1, Z->header), Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
	}

	if (GMT->current.proj.three_D)
		GMT_vertical_axis (GMT, 2);	/* Draw foreground axis */
	else
		GMT_map_basemap (GMT);	/* Plot basemap last if not 3-D */

	GMT_plotend (GMT);

	/* Free memory */

	if (get_contours) {
		for (ij = 0; ij < Topo->header->nm; ij++) {
			if (!binij[ij].first_cont) continue;
			last_cont = binij[ij].first_cont;
			for (this_cont = binij[ij].first_cont->next_cont; this_cont; this_cont = this_cont->next_cont) {
				if (this_cont->first_point) {
					last_point = this_cont->first_point;
					for (this_point = this_cont->first_point->next_point; this_point; this_point = this_point->next_point) {
						GMT_free (GMT, last_point);
						last_point = this_point;
					}
					GMT_free (GMT, last_point);
				}
				GMT_free (GMT, last_cont);
				last_cont = this_cont;
			}
			GMT_free (GMT, last_cont);
		}
		GMT_free (GMT, binij);
	}

	GMT_change_grdreg (GMT, Topo->header, t_reg);	/* Reset registration, if required */
	if (use_intensity_grid) {
		GMT_change_grdreg (GMT, Intens->header, i_reg);	/* Reset registration, if required */
	}
	GMT_free (GMT, xx);
	GMT_free (GMT, yy);
	GMT_free (GMT, x);
	GMT_free (GMT, y);
	GMT_free (GMT, z);
	GMT_free (GMT, v);
	if (Ctrl->G.active) for (k = 0; k < n_drape; k++) {
		GMT_change_grdreg (GMT, Drape[k]->header, d_reg[k]);	/* Reset registration, if required */
	}
	if (get_contours && GMT_Destroy_Data (API, &Z) != GMT_OK) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free Z\n");
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");

	Return (EXIT_SUCCESS);
}
