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
 * image smoother, especially if n_columns and n_rows are relatively small.
 * As an option, a drape grid file can be specified.  Then, the colors
 * are calculated from that file while the topo file is used for shape.
 * Alternatively, give three drape files (red, green, blue components)
 * to bypass the z -> rgb via the CPT.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "longopt/grdview_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"grdview"
#define THIS_MODULE_MODERN_NAME	"grdview"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create 3-D perspective image or surface mesh from a grid"
#define THIS_MODULE_KEYS	"<G{,CC(,GG(,zI(,IG(,>X}"
#define THIS_MODULE_NEEDS	"Jg"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYfnptxy" GMT_OPT("Ec")

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
	struct GRDVIEW_In {
		bool active;
		char *file;
	} In;
	struct GRDVIEW_C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...][+i<dz>] */
		bool active;
		double dz;
		char *file;
		char *savecpt;	/* For when we want to save the automatically generated CPT */
	} C;
	struct GRDVIEW_G {	/* -G<drapefile> 1 or 3 times*/
		bool active;
		bool image;
		unsigned int n;
		char *file[3];
	} G;
	struct GRDVIEW_I {	/* -I<intensfile>|<value>|<modifiers> */
		bool active;
		bool constant;
		bool derive;
		double value;
		char *azimuth;	/* Default azimuth(s) for shading */
		char *file;
		char *method;	/* Default scaling method */
		char *ambient;	/* Default ambient offset */
	} I;
	struct GRDVIEW_N {	/* -N<level>[+g<fill>] */
		bool active;
		bool facade;
		bool implicit;
		struct GMT_FILL fill;
		double level;
	} N;
	struct GRDVIEW_Q {	/* -Q<type>[g] */
		bool active, special;
		bool mask;
		bool monochrome;
		bool cpt;
		int outline;
		unsigned int mode;	/* GRDVIEW_MESH, GRDVIEW_SURF, GRDVIEW_IMAGE */
		double dpi;
		struct GMT_FILL fill;
	} Q;
	struct GRDVIEW_S {	/* -S<smooth> */
		bool active;
		unsigned int value;
	} S;
	struct GRDVIEW_T {	/* -T[s][o[<pen>]] */
		bool active;
		bool skip;
		bool outline;
		struct GMT_PEN pen;
	} T;
	struct GRDVIEW_W {	/* -W[+]<type><pen> */
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

GMT_LOCAL struct GRDVIEW_CONT * grdview_get_cont_struct (struct GMT_CTRL *GMT, uint64_t bin, struct GRDVIEW_BIN *binij, double value) {
	struct GRDVIEW_CONT *cont, *new_cont;

	if (!binij[bin].first_cont) binij[bin].first_cont = gmt_M_memory (GMT, NULL, 1, struct GRDVIEW_CONT);

	for (cont = binij[bin].first_cont; cont->next_cont && cont->next_cont->value <= value; cont = cont->next_cont);

	new_cont = gmt_M_memory (GMT, NULL, 1, struct GRDVIEW_CONT);
	if (cont->next_cont) {	/* Put it in the link */
		new_cont->next_cont = cont->next_cont;
		cont->next_cont = new_cont;
	}
	else	/* End of list */
		cont->next_cont = new_cont;
	return (new_cont);
}

GMT_LOCAL struct GRDVIEW_POINT * grdview_get_point (struct GMT_CTRL *GMT, double x, double y) {
	struct GRDVIEW_POINT *point = gmt_M_memory (GMT, NULL, 1, struct GRDVIEW_POINT);
	point->x = x;
	point->y = y;
	return (point);
}

#if 0
/* RS: Removed this because it yields unpredictable results, making it impossible to line up different 3D plots */

GMT_LOCAL void grdview_init_setup (struct GMT_CTRL *GMT, struct GMT_GRID *Topo, int draw_plane, double plane_level) {
	int row, col, ij;
	double xtmp, ytmp, tmp;

	/* We must find the projected min/max in y-direction */

	/* Reset from whatever they were */
	GMT->current.proj.z_project.ymin = +DBL_MAX;
	GMT->current.proj.z_project.ymax = -DBL_MAX;

	gmt_M_grd_loop (Topo, row, col, ij) {	/* First loop over all the grid-nodes */
		if (gmt_M_is_fnan (Topo->data[ij])) continue;
		gmt_geoz_to_xy (GMT, gmt_M_grd_col_to_x (col, Topo->header), gmt_M_grd_row_to_y (row, Topo->header), (double)Topo->data[ij], &xtmp, &ytmp);
		GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
		GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
	}
	if (draw_plane) {	/* The plane or facade may exceed the current min/max so loop over these as well */
		for (col = 0; col < Topo->header->n_columns; col++) {
			tmp = gmt_M_grd_col_to_x (col, Topo->header);
			gmt_geoz_to_xy (GMT, tmp, Topo->header->wesn[YLO], plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
			gmt_geoz_to_xy (GMT, tmp, Topo->header->wesn[YHI], plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
		}
		for (row = 0; row < Topo->header->n_rows; row++) {
			tmp = gmt_M_grd_row_to_y (row, Topo->header);
			gmt_geoz_to_xy (GMT, Topo->header->wesn[XLO], tmp, plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
			gmt_geoz_to_xy (GMT, Topo->header->wesn[XHI], tmp, plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
		}
	}
}
#endif

GMT_LOCAL double grdview_get_intensity (struct GMT_GRID *I, uint64_t k) {
	/* Returns the average intensity for this tile */
	return (0.25 * (I->data[k] + I->data[k+1] + I->data[k-I->header->mx] + I->data[k-I->header->mx+1]));
}

GMT_LOCAL unsigned int grdview_pixel_inside (struct GMT_CTRL *GMT, int ip, int jp, int *ix, int *iy, int64_t bin, int bin_inc[]) {
	/* Returns true of the ip,jp point is inside the polygon defined by the tile */
	unsigned int i, what;
	double x[6], y[6];

	for (i = 0; i < 4; i++) {
		x[i] = (double)ix[bin+bin_inc[i]];
		y[i] = (double)iy[bin+bin_inc[i]];
	}
	x[4] = x[0];	y[4] = y[0];
	what = gmt_non_zero_winding (GMT, (double)ip, (double)jp, x, y, 5);
	return (what);
}

GMT_LOCAL int grdview_quick_idist (int x1, int y1, int x2, int y2) {
	/* Fast integer distance calculation */
	if ((x2 -= x1) < 0) x2 = -x2;
	if ((y2 -= y1) < 0) y2 = -y2;
	return (x2 + y2 - (((x2 > y2) ? y2 : x2) >> 1));
}

GMT_LOCAL unsigned int grdview_get_side (double x, double y, double x_left, double y_bottom, double inc[], double inc2[]) {
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

GMT_LOCAL void grdview_copy_points_fw (double x[], double y[], double z[], double v[], double xcont[], double ycont[], double zcont[], double vcont[], unsigned int ncont, uint64_t *n) {
	unsigned int k;
	for (k = 0; k < ncont; k++, (*n)++) {
		x[*n] = xcont[k];
		y[*n] = ycont[k];
		z[*n] = zcont[k];
		v[*n] = vcont[k];
	}
}

GMT_LOCAL void grdview_copy_points_bw (double x[], double y[], double z[], double v[], double xcont[], double ycont[], double zcont[], double vcont[], unsigned int ncont, uint64_t *n) {
	unsigned int k, k2;
	for (k2 = 0, k = ncont - 1; k2 < ncont; k2++, k--, (*n)++) {
		x[*n] = xcont[k];
		y[*n] = ycont[k];
		z[*n] = zcont[k];
		v[*n] = vcont[k];
	}
}

GMT_LOCAL double grdview_get_z_ave (double v[], double next_up, uint64_t n) {
	uint64_t k;
	double z_ave;

	for (k = 0, z_ave = 0.0; k < n; k++) z_ave += MIN (v[k], next_up);
	return (z_ave / n);
}

GMT_LOCAL void grdview_add_node (bool used[], double x[], double y[], double z[], double v[], uint64_t *k, unsigned int node, double X_vert[], double Y_vert[], gmt_grdfloat topo[], gmt_grdfloat zgrd[], uint64_t ij) {
	/* Adds a corner node to list of points and increments *k */
	if (used) {	/* If we keep track of which nodes we have used then we do not want to use one twice */
		if (used[node]) return;
		used[node] = true;
	}
	x[*k] = X_vert[node];
	y[*k] = Y_vert[node];
	z[*k] = topo[ij];
	v[*k] = zgrd[node];
	(*k)++;
}

GMT_LOCAL void grdview_paint_it (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, double *x, double *y, int n, double z, bool intens, bool monochrome, double intensity, int outline) {
	int index;
	double rgb[4];
	struct GMT_FILL *f = NULL;
	struct GMT_PALETTE_HIDDEN *PH = gmt_get_C_hidden (P);

	if (n < 3) return;	/* Need at least 3 points to make a polygon */
	index = gmt_get_rgb_from_z (GMT, P, z, rgb);	/* This sets P->skip as well */
	if (PH->skip) return;	/* Skip this z-slice */

	PSL_comment (PSL, "Paint polygon z = %g\n", z);
	/* Now we must paint, with colors or patterns */

	if ((index >= 0 && (f = P->data[index].fill) != NULL) || (index < 0 && (f = P->bfn[index+3].fill) != NULL))	/* Pattern */
		gmt_setfill (GMT, f, outline);
	else {	/* Solid color/gray */
		if (intens) gmt_illuminate (GMT, intensity, rgb);
		if (monochrome) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb); /* gmt_M_yiq transformation */
		PSL_setfill (PSL, rgb, outline);
	}
	PSL_plotpolygon (PSL, x, y, n);
}

GMT_LOCAL void grdview_set_loop_order (struct GMT_CTRL *GMT, struct GMT_GRID *Z, int start[], int stop[], int inc[], unsigned int id[]) {
	/* We want to loop over the grid from "back" to "front".  What is back and what is front depends on the
	 * projection (-J) and view angles (-p).  We pick a point in the middle of the grid and the point above it
	 * (i.e., north of it) and compute their projected coordinates, then calculate the angle from center to north.
	 * We use this orientation of the grid to determine which way we loop (rows then columns or columns then rows)
	 * and which direction.
	 * The start and stop items work like this: start is the lower-left row,col values of the first tile,
	 * while stop is 1 beyond the last valid index - we loop as long as current index is NOT equal to stop.
	 */
	unsigned int col, row, oct, one = 1;
	double az, x0, x1, y0, y1;
	char *kind = "xy";

	//if (gmt_M_is_azimuthal (GMT) && gmt_M_360_range (Z->header->wesn[XLO], Z->header->wesn[XHI])) one = 0;	/* Need to connect across the gap */
	col = Z->header->n_columns / 2;
	row = Z->header->n_rows / 2;
	gmt_geoz_to_xy (GMT, Z->x[col], Z->y[row],   GMT->current.proj.z_project.level, &x0, &y0);
	gmt_geoz_to_xy (GMT, Z->x[col], Z->y[row-1], GMT->current.proj.z_project.level, &x1, &y1);
	if (doubleAlmostEqualZero (x0, x1))	/* Vertical orientation */
		az = (y1 > y0) ? 90.0 : -90.0;
	else
		az = d_atan2d (y1 - y0, x1 - x0);
	if (az < 0.0) az += 360.0;
	else if (az >= 360.0) az -= 360.0;
	oct = 1 + urint (floor (az / 45.0));	/* Gives octants 1 - 8 */
	id[0] = id[1] = start[0] = start[1] = stop[0] = stop[1] = inc[0] = inc[1] = 0;	/* Init arrays */
	switch (oct) {
		case 1:	/* 0-45: Outer loop over columns */
			id[0] = GMT_X;	start[0] = 0;	stop[0] = Z->header->n_columns - one;	inc[0] = +1;
			id[1] = GMT_Y;	start[1] = 1;	stop[1] = Z->header->n_rows;		inc[1] = +1;
			break;
		case 2:	/* 45-90: Outer loop over rows */
			id[0] = GMT_Y;	start[0] = 1;	stop[0] = Z->header->n_rows; 		inc[0] = +1;
			id[1] = GMT_X;	start[1] = 0;	stop[1] = Z->header->n_columns - one;	inc[1] = +1;
			break;
		case 3:	/* 90-135: Outer loop over rows */
			id[0] = GMT_Y;	start[0] = 1;	stop[0] = Z->header->n_rows; 		inc[0] = +1;
			id[1] = GMT_X;	start[1] = Z->header->n_columns - 1 - one;	stop[1] = -1;	inc[1] = -1;
			break;
		case 4:	/* 135-180: Outer loop over columns */
			id[0] = GMT_X;	start[0] = Z->header->n_columns - 1 - one;	stop[0] = -1; 	inc[0] = -1;
			id[1] = GMT_Y;	start[1] = 1;	stop[1] = Z->header->n_rows;		inc[1] = +1;
			break;
		case 5:	/* 180-225: Outer loop over columns */
			id[0] = GMT_X;	start[0] = Z->header->n_columns - 1 - one;	stop[0] = -1; 	inc[0] = -1;
			id[1] = GMT_Y;	start[1] = Z->header->n_rows - 1;	stop[1] = 0;	inc[1] = -1;
			break;
		case 6:	/* 225-270: Outer loop over rows */
			id[0] = GMT_Y;	start[0] = Z->header->n_rows - 1;	stop[0] = 0;	inc[0] = -1;
			id[1] = GMT_X;	start[1] = Z->header->n_columns - 1 - one;	stop[1] = -1; 	inc[1] = -1;
			break;
		case 7:	/* 270-315: Outer loop over rows */
			id[0] = GMT_Y;	start[0] = Z->header->n_rows - 1;	stop[0] = 0;	inc[0] = -1;
			id[1] = GMT_X;	start[1] = 0;	stop[1] = Z->header->n_columns - one;	inc[1] = +1;
			break;
		case 8:	/* 315-360: Outer loop over columns */
			id[0] = GMT_X;	start[0] = 0;	stop[0] = Z->header->n_columns - one;	inc[0] = +1;
			id[1] = GMT_Y;	start[1] = Z->header->n_rows - 1;	stop[1] = 0;	inc[1] = -1;
			break;
		default:	/* For Coverity */
			break;
	}
	if (GMT->current.proj.projection == GMT_POLAR) {	/* Polar cylindrical has 0 at center, not pole */
		unsigned int k = (id[0] == GMT_Y) ? 0 : 1, off = 0;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Loop over y flipped for cylindrical/polar projection\n");
		off = (start[k] == 1) ? -1 : +1;
		gmt_M_int_swap (start[k], stop[k]);
		start[k] += off;	stop[k] += off;
		inc[k] = -inc[k];
	}

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Octant %d (az = %g) one = %d\n", oct, az, one);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Outer loop over %c doing %d:%d:%d\n", kind[id[0]], start[0], inc[0], stop[0]);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Inner loop over %c doing %d:%d:%d\n", kind[id[1]], start[1], inc[1], stop[1]);
}

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDVIEW_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct GRDVIEW_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	gmt_init_fill (GMT, &C->N.fill, -1.0, -1.0, -1.0);	/* Default is no fill of facade */
	C->I.azimuth = strdup ("-45.0");		/* Default azimuth for shading when -I is used */
	C->I.method  = strdup ("t1");	/* Default normalization for shading when -I is used */
	C->T.pen = C->W.pen[0] = C->W.pen[1] = C->W.pen[2] = GMT->current.setting.map_default_pen;	/* Tile and mesh pens */
	C->W.pen[0].width *= 3.0;	/* Contour pen */
	C->W.pen[2].width *= 3.0;	/* Facade pen */
	C->Q.dpi = GMT->current.setting.graphics_dpu;	/* Set the default dpu for -Qi */
	if (GMT->current.setting.graphics_dpu_unit == 'c') C->Q.dpi *= 2.54;	/* Convert dpc to dpi */
	gmt_init_fill (GMT, &C->Q.fill, GMT->current.setting.ps_page_rgb[0], GMT->current.setting.ps_page_rgb[1], GMT->current.setting.ps_page_rgb[2]);
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDVIEW_CTRL *C) {	/* Deallocate control structure */
	unsigned int i;
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.file);
	for (i = 0; i < 3; i++) gmt_M_str_free (C->G.file[i]);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->I.azimuth);
	gmt_M_str_free (C->I.method);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	struct GMT_PEN P;

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <topogrid> %s [%s] [-C[<cpt>]] [-G<drapegrid> | <image> | -G<grd_r> -G<grd_g> -G<grd_b>] "
		"[-I[<intensgrid>|<value>|<modifiers>]] [%s] %s[-N[<level>][+g<fill>]] %s%s[-Q<args>[+m]] [%s] [-S<smooth>] "
		"[-T[+o[<pen>]][+s]] [%s] [%s] [-W<type><pen>] [%s] [%s] %s[%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_J_OPT, GMT_B_OPT, GMT_Jz_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_Rgeoz_OPT, GMT_U_OPT, GMT_V_OPT,
		GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_f_OPT, GMT_n_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<topogrid> is the grid surface to be plotted.");
	GMT_Option (API, "J-Z");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-C[<cpt>]");
	GMT_Usage (API, -2, "Color palette file to convert grid values to colors. Optionally, name a master cpt "
		"to automatically assign continuous colors over the data range [%s]; if so, "
		"optionally append +i<dz> to quantize the range [the exact grid range]. "
		"Another option is to specify -C<color1>,<color2>[,<color3>,...] to build a "
		"linear continuous cpt from those colors automatically. Append +s<fname> to save the generated cpt ",
		"in a disk file.", API->GMT->current.setting.cpt);
	GMT_Usage (API, 1, "\n-G<drapegrid> | <image> | -G<grd_r> -G<grd_g> -G<grd_b>");
	GMT_Usage (API, -2, "Specify how to color the 3-D surface defined by <topogrid>:");
	GMT_Usage (API, 3, "%s Provide a grid (<drapegrid>) and colors will be determined from it and the cpt.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Provide an image (<drapeimage>) and it will be draped over the surface.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Repeat -G for three grid files with separate red, green, and blue components in 0-255 range.", GMT_LINE_BULLET);
	GMT_Usage (API, -2, "Notes: 1) -JZ|z and -N always refer to the data in the <topogrid>. "
		"2) The -G option requires the -Qi[<dpi>] option.");
	GMT_Usage (API, 1, "\n-I[<intensgrid>|<value>|<modifiers>]");
	GMT_Usage (API, -2, "Apply directional illumination. Append name of an intensity grid, or "
		"for a constant intensity (i.e., change the ambient light), just give a scalar. "
		"To derive intensities from <topogrid> instead, append desired modifiers:");
	GMT_Usage (API, 3, "+a Specify <azim> of illumination [-45].");
	GMT_Usage (API, 3, "+n Set the <method> and <scale> to use [t1].");
	GMT_Usage (API, 3, "+m Set <ambient> light to add [0].");
	GMT_Usage (API, -2, "Alternatively, use -I+d to accept the default values (see grdgradient for more details). "
		"To derive intensities from another grid than <topogrid>, give the alternative data grid with suitable modifiers.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-N[<level>][+g<fill>]");
	GMT_Usage (API, -2, "Draw a horizontal plane at z = <level> [minimum grid (or -R) value]. For rectangular projections, append +g<fill> "
		"to paint the facade between the plane and the data perimeter.");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-Q<args>[+m]");
	GMT_Usage (API, -2, "Set plot type request. Choose one of the following directives:");
	GMT_Usage (API, 3, "m: Mesh plot [Default]. Append <color> for mesh paint [%s]. "
		"Alternatively, append x or -y for row or column \"waterfall\" profiles.",
		gmt_putcolor (API->GMT, API->GMT->PSL->init.page_rgb));
	GMT_Usage (API, 3, "s: Colored or shaded Surface. Append m to draw mesh-lines on the surface.");
	GMT_Usage (API, 3, "i: Transform polygons to raster-image via scanline conversion.  Append effective dpu [%lg%c].",
		API->GMT->current.setting.graphics_dpu, API->GMT->current.setting.graphics_dpu_unit);
	GMT_Usage (API, 3, "c: As i, but use PS Level 3 color-masking for nodes with z = NaN.  Append effective dpu [%lg%c].",
		API->GMT->current.setting.graphics_dpu, API->GMT->current.setting.graphics_dpu_unit);
	GMT_Usage (API, -2, "To force a monochrome image using the YIQ transformation, append +m.");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-S<smooth>");
	GMT_Usage (API, -2, "Smooth contours first (see grdcontour for <smooth> value info) [no smoothing].");
	gmt_pen_syntax (API->GMT, 'T', NULL, "Image the data without interpolation by painting polygonal tiles in the form [+o[<pen>]][+s].", NULL, 0);
	GMT_Usage (API, 3, "+s Skip tiles for nodes with z = NaN [Default paints all tiles].");
	GMT_Usage (API, 3, "+o to draw tile outline; optionally append <pen> [Default uses no outline].");
	GMT_Usage (API, -2, "Note: Cannot be used with -Jz|Z as it produces a flat image.");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes for various features in form <type><pen>.", NULL, 0);
	GMT_Usage (API, -2, "Note: <type> can be c for contours, m for mesh, and f for facade:");
	P = API->GMT->current.setting.map_default_pen;
	GMT_Usage (API, 3, "m: Sets attributes for mesh lines [%s]. "
		"Requires -Qm or -Qsm to take effect.", gmt_putpen (API->GMT, &P));
	P.width *= 3.0;
	GMT_Usage (API, 3, "c: Draw contours on top of surface or mesh [Default is no contours]. "
		"Optionally append pen attributes [%s].", gmt_putpen (API->GMT, &P));
	GMT_Usage (API, 3, "f: Set attributes for facade outline [%s]. "
		"Requires -N to take effect.", gmt_putpen (API->GMT, &P));
	GMT_Option (API, "X,c,f,n,p,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL double grdview_get_dpi (char *text) {
	double dpi = atoi (text);
	if (text[strlen(text)-1] == 'c') dpi *= 2.54;	/* Got dpc, convert to dpi */
	return (dpi);
}

static int parse (struct GMT_CTRL *GMT, struct GRDVIEW_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdview and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, q_set = 0, n_commas, j, k, n, id;
	int sval;
	bool no_cpt = false;
	char *c = NULL, *f = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */
			case '<':	/* Input file (only one is accepted) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->In.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file));
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Cpt file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				if (opt->arg[0] && (f = gmt_strrstr (Ctrl->C.file, "+s")) != NULL) {	/* Filename has a +s<outname>, extract that part */
					Ctrl->C.savecpt = &f[2];
					f[0] = '\0';		/* Remove the +s<outname> from Ctrl->C.file */
				}
				gmt_cpt_interval_modifier (GMT, &(Ctrl->C.file), &(Ctrl->C.dz));
				break;
			case 'z':	/* One image. This is an undocumented fake option but one that lets externals pass both images and grids for draping. */
			case 'G':	/* One grid or image or three separate r,g,b grids */
				Ctrl->G.active = true;
				for (k = 0, n_commas = 0; opt->arg[k]; k++) if (opt->arg[k] == ',') n_commas++;
				if (gmt_M_compat_check (GMT, 5) && n_commas == 2) {	/* Old-style -Ga,b,c option */
					/* Three r,g,b grids for draping */
					char A[GMT_LEN256] = {""}, B[GMT_LEN256] = {""}, C[GMT_LEN256] = {""};
					sscanf (opt->arg, "%[^,],%[^,],%s", A, B, C);
					n_errors += gmt_get_required_file (GMT, A, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->G.file[0]));
					n_errors += gmt_get_required_file (GMT, B, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->G.file[1]));
					n_errors += gmt_get_required_file (GMT, C, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->G.file[2]));
					Ctrl->G.n = 3;
				}
				else if (n_commas == 0 && Ctrl->G.n < 3) {	/* Just got another drape grid or image */
					Ctrl->G.file[Ctrl->G.n] = strdup (opt->arg);
					if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->G.file[Ctrl->G.n]))) n_errors++;
					Ctrl->G.n++;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -G: Usage is -G<z.grd|image> | -G<r.grd> -G<g.grd> -G<b.grd>\n");
					n_errors++;
				}
				break;
			case 'I':	/* Use intensity from grid or constant or auto-compute it */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				if ((c = strstr (opt->arg, "+d"))) {	/* Gave +d, so derive intensities from the input grid using default settings */
					Ctrl->I.derive = true;
					c[0] = '\0';	/* Chop off modifier */
				}
				else if ((c = gmt_first_modifier (GMT, opt->arg, "amn"))) {	/* Want to control how grdgradient is run */
					unsigned int pos = 0;
					char p[GMT_BUFSIZ] = {""};
					Ctrl->I.derive = true;
					while (gmt_getmodopt (GMT, 'I', c, "amn", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'a': gmt_M_str_free (Ctrl->I.azimuth); Ctrl->I.azimuth  = strdup (&p[1]); break;
							case 'm': gmt_M_str_free (Ctrl->I.ambient);  Ctrl->I.ambient = strdup (&p[1]); break;
							case 'n': gmt_M_str_free (Ctrl->I.method);  Ctrl->I.method   = strdup (&p[1]); break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';	/* Chop off all modifiers so range can be determined */
				}
				else if (!opt->arg[0] || !strcmp (opt->arg, "+")) {	/* Gave deprecated option to derive intensities from the input grid using default settings */
					Ctrl->I.derive = true;
					if (opt->arg[0]) opt->arg[0] = '\0';	/* Remove the single + */
				}
				if (opt->arg[0]) {	/* Gave an argument in addition to or instead of a modifier */
					if (!gmt_access (GMT, opt->arg, R_OK))	/* Got a file */
						Ctrl->I.file = strdup (opt->arg);
					else if (gmt_M_file_is_remote (opt->arg))	/* Got a remote file */
						Ctrl->I.file = strdup (opt->arg);
					else if (opt->arg[0] && gmt_is_float (GMT, opt->arg)) {	/* Looks like a constant value */
						Ctrl->I.value = atof (opt->arg);
						Ctrl->I.constant = true;
					}
				}
				else if (!Ctrl->I.active) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -I: Requires a valid grid file or a constant\n");
					n_errors++;
				}
				if (c) c[0] = '+';	/* Restore the plus */
				break;
			case 'L':	/* GMT4 BCs */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT,
					            "Option -L is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
					gmt_strncpy (GMT->common.n.BC, opt->arg, 4U);
				}
				else
					n_errors += gmt_default_option_error (GMT, opt);
				break;
			case 'N':	/* Facade */
				if (opt->arg[0]) {
					char colors[GMT_LEN64] = {""};
					n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
					if ((c = strstr (opt->arg, "+g")) != NULL) {	/* Gave modifier +g<fill> */
						c[0] = '\0';	/* Truncate string temporarily */
						if (opt->arg[0])
							Ctrl->N.level = atof (opt->arg);
						else
							Ctrl->N.implicit = true;	/* Set level when we know grids zmin */
						c[0] = '+';	/* Restore the + */
						n_errors += gmt_M_check_condition (GMT, gmt_getfill (GMT, &c[2], &Ctrl->N.fill),
						                                 "Option -N: Usage is -N[<level>][+g<fill>]\n");
						Ctrl->N.facade = true;
					}
					else if (gmt_M_compat_check (GMT, 4) && (c = strchr (opt->arg, '/')) != NULL) {	/* Deprecated <level>/<fill> */
						GMT_Report (API, GMT_MSG_COMPAT,
						            "Option -N<level>[/<fill>] is deprecated; use -N[<level>][+g<fill>] in the future.\n");
						c[0] = ' ';	/* Take out the slash for now */
						sscanf (opt->arg, "%lf %s", &Ctrl->N.level, colors);
						n_errors += gmt_M_check_condition (GMT, gmt_getfill (GMT, colors, &Ctrl->N.fill),
						                                   "Option -N: Usage is -N<level>[+g<fill>]\n");
						Ctrl->N.facade = true;
						c[0] = '/';	/* Restore the slash */
					}
					else	/* Just got the level */
						Ctrl->N.level = atof (opt->arg);
				}
				else
					Ctrl->N.implicit = true;	/* Set level when we know grids zmin */
				break;
			case 'Q':	/* Plot mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				q_set++;
				if ((c = strstr (opt->arg, "+m")) != NULL) {
					Ctrl->Q.monochrome = true;
					c[0] = '\0';	/* Chop off +m */
				}
				switch (opt->arg[0]) {
					case 'c':	/* Image with colormask */
						if (opt->arg[1] && isdigit ((int)opt->arg[1])) Ctrl->Q.dpi = grdview_get_dpi (&opt->arg[1]);
						Ctrl->Q.mode = GRDVIEW_IMAGE;
						Ctrl->Q.cpt = true;	/* Will need a CPT */
						Ctrl->Q.mask = true;
						break;
					case 't':	/* Image without color interpolation */
						Ctrl->Q.special = true;
						Ctrl->Q.cpt = true;	/* Will need a CPT */
						/* Intentionally fall through - to 'i' */
					case 'i':	/* Image with clipmask */
						Ctrl->Q.mode = GRDVIEW_IMAGE;
						if (opt->arg[1] && isdigit ((int)opt->arg[1])) Ctrl->Q.dpi = grdview_get_dpi (&opt->arg[1]);
						Ctrl->Q.cpt = true;	/* Will need a CPT */
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
							n_errors += gmt_M_check_condition (GMT, gmt_getfill (GMT, &opt->arg[k], &Ctrl->Q.fill),
							                                 "Option -Qm: To give mesh color, use -Qm[x|y]<color>\n");
						}
						break;
					case 's':	/* Color without contours */
						Ctrl->Q.mode = GRDVIEW_SURF;
						if (opt->arg[1] == 'm') Ctrl->Q.outline = 1;
						Ctrl->Q.cpt = true;	/* Will need a CPT */
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -Q: Unrecognized qualifier (%c)\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (c != NULL && Ctrl->Q.monochrome)
					c[0] = '+';	/* Restore the chopped off +m */
				else if (gmt_M_compat_check (GMT, 4) && opt->arg[strlen(opt->arg)-1] == 'g') {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -Q<args>[g] is deprecated; use -Q<args>[+m] in the future.\n");
					Ctrl->Q.monochrome = true;
				}
				break;
			case 'S':	/* Smoothing of contours */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				n_errors += gmt_get_required_sint (GMT, opt->arg, opt->option, 0, &sval);
				n_errors += gmt_M_check_condition (GMT, sval <= 0, "Option -S: smooth value must be positive\n");
				Ctrl->S.value = sval;
				break;
			case 'T':	/* Tile plot -T[+s][+o<pen>] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				if (opt->arg[0] && (strchr (opt->arg, '+') || gmt_M_compat_check (GMT, 6))) {	/* New syntax */
					if (strstr (opt->arg, "+s"))
						Ctrl->T.skip = true;
					if ((c = strstr (opt->arg, "+o"))) {
						if (gmt_getpen (GMT, &c[2], &Ctrl->T.pen)) {
							gmt_pen_syntax (GMT, 'T', NULL, " ", NULL, 0);
							n_errors++;
						}
						else
							Ctrl->T.outline = true;
					}
				}
				else if (opt->arg[0]) {	/* Old-style syntax -T[s][o<pen>] */
					k = 0;
					if (opt->arg[0] == 's') Ctrl->T.skip = true, k = 1;
					if (opt->arg[k] == 'o') {	/* Want tile outline also */
						Ctrl->T.outline = true;
						k++;
						if (opt->arg[k] && gmt_getpen (GMT, &opt->arg[k], &Ctrl->T.pen)) {
							gmt_pen_syntax (GMT, 'T', NULL, " ", NULL, 0);
							n_errors++;
						}
					}
				}
				break;
			case 'W':	/* Contour, mesh, or facade pens */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				j = (opt->arg[0] == 'm' || opt->arg[0] == 'c' || opt->arg[0] == 'f');
				id = 0;
				if (j == 1) {	/* First check that the c|f|m is not part of a color name instead */
					char txt_a[GMT_LEN256] = {""};
					n = j+1;
					while (opt->arg[n] && opt->arg[n] != ',' && opt->arg[n] != '/') n++;	/* Wind until end or , or / */
					strncpy (txt_a, opt->arg, n);	txt_a[n] = '\0';
					if (gmt_colorname2index (GMT, txt_a) >= 0)	/* Found a colorname: reset j to 0 and parse normally */
						j = id = 0;
					else	/* Got a type, set the id index accordingly */
						id = (opt->arg[0] == 'f') ? 2 : ((opt->arg[0] == 'm') ? 1 : 0);
				}
				if (gmt_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[id])) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
					n_errors++;
				}
				if (j == 0)	/* Copy pen when using just -W */
					Ctrl->W.pen[2] = Ctrl->W.pen[1] = Ctrl->W.pen[0];
				if (id == 0) Ctrl->W.contour = true;	/* Switch contouring ON for -Wc or -W */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	if (Ctrl->G.active) {
		if (gmt_M_file_is_image (Ctrl->G.file[0])) no_cpt = true;
		if (no_cpt) Ctrl->Q.cpt = false;
		if (Ctrl->G.n == 3)
			Ctrl->G.image = true;
	}
	if (Ctrl->N.facade && GMT->common.R.oblique) {	/* Cannot yet do facade with oblique grid */
		GMT_Report (API, GMT_MSG_WARNING, "Cannot paint facade for oblique projection\n");
		Ctrl->N.facade = false;
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Must specify input file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.file && !strcmp (Ctrl->In.file, "="),
	                                   "Piping of topofile not supported!\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active,
		                               "Must specify a map projection with the -J option\n");

	/* Gave more than one -Q setting */
	n_errors += gmt_M_check_condition (GMT, q_set > 1, "Options -Qm, -Qs, -Qc, and -Qi are mutually exclusive.\n");
	/* Gave both -Q and -T */
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->Q.active, "Options -Q and -T are mutually exclusive.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && (Ctrl->G.n < 1 || Ctrl->G.n > 3),
	                                   "Option -G: Requires either 1 or 3 grids\n");
	if (Ctrl->G.active) {	/* Draping was requested */
		for (k = 0; k < Ctrl->G.n; k++)
			n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file[k][0], "Option -G: Must specify drape file\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->G.n == 3 && Ctrl->Q.mode != GRDVIEW_IMAGE,
		                                   "The draping option requires -Qi option\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && !Ctrl->I.constant && !Ctrl->I.file && !Ctrl->I.derive,
	                                   "Option -I: Must specify intensity file, value, or modifiers\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->Q.mode == GRDVIEW_SURF || Ctrl->Q.mode == GRDVIEW_IMAGE || Ctrl->W.contour) &&
	                                   !Ctrl->C.file && Ctrl->G.n != 3 && !no_cpt && GMT->current.setting.run_mode == GMT_CLASSIC, "Must specify color palette table\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.mode == GRDVIEW_IMAGE && Ctrl->Q.dpi <= 0.0,
	                                 "Option -Qi: Must specify positive dpi\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && GMT->current.proj.JZ_set,
	                                 "Option -T: Cannot specify -JZ|z\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdview (void *V_API, int mode, void *args) {
	bool get_contours, bad, pen_set, begin, saddle, drape_resample = false;
	bool nothing_inside = false, use_intensity_grid, do_G_reading = true, read_z = false;

	openmp_int row, col;
	unsigned int c, nk, n4, d_reg[3], i_reg = 0, id[2], good;
	unsigned int t_reg, n_out, k, k1, ii, jj, PS_colormask_off = 0, n_max = 0;

	int i, j, i_bin, j_bin, i_bin_old, j_bin_old, way, bin_inc[4], ij_inc[4], error = 0;
	int start[2], stop[2], inc[2];

	uint64_t sw, se, nw, ne, n, pt, ij, bin;
	int64_t ns;

	size_t max_alloc;

	gmt_grdfloat *saved_data_pointer = NULL;

	double cval, x_left, x_right, y_top, y_bottom, small = FLT_EPSILON, z_ave;
	double inc2[2], wesn[4] = {0.0, 0.0, 0.0, 0.0}, z_val, x_pixel_size, y_pixel_size;
	double this_intensity = 0.0, next_up = 0.0, xmesh[4], ymesh[4], rgb[4];
	double *x_imask = NULL, *y_imask = NULL, x_inc[4], y_inc[4], *x = NULL, *y = NULL;
	double *z = NULL, *v = NULL, *xx = NULL, *yy = NULL, *xval = NULL, *yval = NULL;

	struct GRDVIEW_CONT *start_cont = NULL, *this_cont = NULL, *last_cont = NULL;
	struct GRDVIEW_POINT *this_point = NULL, *last_point = NULL;
	struct GMT_GRID *Drape[3] = {NULL, NULL, NULL}, *Topo = NULL, *Intens = NULL, *Z = NULL;
	struct GRDVIEW_BIN *binij = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GRDVIEW_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL internal parameters */
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

	/*---------------------------- This is the grdview main code ----------------------------*/

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	GMT->current.plot.mode_3D = 1;	/* Only do background axis first; do foreground at end */
	use_intensity_grid = (Ctrl->I.active && !Ctrl->I.constant);	/* We want to use the intensity grid */

	if ((Topo = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|GMT_GRID_NEEDS_PAD2, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	if ((API->error = gmt_img_sanitycheck (GMT, Topo->header))) {	/* Used map projection on a Mercator (cartesian) grid */
		Return (API->error);
	}
	if (use_intensity_grid && !Ctrl->I.derive) {
		GMT_Report (API, GMT_MSG_INFORMATION, "Read intensity grid header from file %s\n", Ctrl->I.file);
		if ((Intens = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|GMT_GRID_NEEDS_PAD2, NULL, Ctrl->I.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
	}

	if (!Ctrl->C.active && (Ctrl->Q.cpt || Ctrl->T.active) && Ctrl->G.n != 3)
		Ctrl->C.active = true;	/* Use default CPT (GMT->current.setting.cpt) and auto-stretch or under modern reuse current CPT */

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active[RSET]) {			/* No -R, use grid region */
		gmt_set_R_from_grd(GMT, Topo->header);
		GMT->common.R.active[RSET] = true;		/* Needed to have gmtapi_import_image pick the right branch when draping image is referenced */
	}

	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);

	if (gmt_file_is_tiled_list (API, Ctrl->In.file, NULL, NULL, NULL)) {	/* Must read it so we can get zmin/zmax set */
		/* PW Note: This step could go away if we pre-saved the z-range of all remote global grids */
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY|GMT_GRID_NEEDS_PAD2, wesn, Ctrl->In.file, Topo) == NULL) {	/* Get topo data */
			Return (API->error);
		}
		read_z = true;	/* So we don't do it again later */
	}
	n_max = MAX (Topo->header->n_columns, Topo->header->n_rows);	/* Get max dimension so far */
	/* Set default z-range for plot to be that of the grid if not specified via -R */
	if (GMT->common.R.wesn[ZLO] == 0.0 && GMT->common.R.wesn[ZHI] == 0.0) {
		GMT->common.R.wesn[ZLO] = Topo->header->z_min;
		GMT->common.R.wesn[ZHI] = Topo->header->z_max;
		if (Ctrl->N.active && Ctrl->N.level < GMT->common.R.wesn[ZLO]) GMT->common.R.wesn[ZLO] = Ctrl->N.level;
		if (Ctrl->N.active && Ctrl->N.level > GMT->common.R.wesn[ZHI]) GMT->common.R.wesn[ZHI] = Ctrl->N.level;
	}

	if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	/* Determine the wesn to be used to read the grid file */

	if (!gmt_grd_setregion (GMT, Topo->header, wesn, BCR_BILINEAR))
		nothing_inside = true;
	else if (use_intensity_grid && !Ctrl->I.derive && !gmt_grd_setregion (GMT, Intens->header, wesn, BCR_BILINEAR))
		nothing_inside = true;

	if (nothing_inside) {
		/* No grid to plot; just do empty map and bail */
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_BEFORE, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_BEFORE);
		GMT->current.plot.mode_3D |= 2;	/* Ensure that foreground axis is drawn */
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		gmt_map_basemap (GMT);
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_plotend (GMT);
		Return (GMT_NOERROR);
	}

	if (Ctrl->I.derive) {	/* Auto-create intensity grid from data grid */
		char int_grd[GMT_VF_LEN] = {""}, data_file[PATH_MAX] = {""}, cmd[GMT_LEN256] = {""};
		struct GMT_GRID *I_data = NULL;

		GMT_Report (API, GMT_MSG_INFORMATION, "Derive intensity grid from data grid\n");
		/* Create a virtual file to hold the intensity grid */
		if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT|GMT_IS_REFERENCE, NULL, int_grd))
			Return (API->error);

		if (Ctrl->I.file) {	/* Gave a file to derive from */
			if (gmt_file_is_tiled_list (API, Ctrl->I.file, NULL, NULL, NULL)) {	/* Must read and stitch the tiles first */
				if ((I_data = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA|GMT_GRID_NEEDS_PAD2, wesn, Ctrl->I.file, NULL)) == NULL)	/* Get srtm grid data */
					Return (API->error);
				if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, I_data, data_file))
					Return (API->error);
			}
			else
				strcpy (data_file, Ctrl->I.file);
		}
		else if (Topo->data) {	/* If not NULL it means we have a tiled and blended grid, use as is */
			if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, Topo, data_file))
				Return (API->error);
		}
		else
				strcpy (data_file, Ctrl->In.file);

		/* Prepare the grdgradient arguments using selected -A -N */
		gmt_filename_set (data_file);	/* Replace any spaces with ASCII 29 */
		sprintf (cmd, "%s -G%s -A%s -N%s+a%s -R%.16g/%.16g/%.16g/%.16g --GMT_HISTORY=readonly",
			data_file, int_grd, Ctrl->I.azimuth, Ctrl->I.method, Ctrl->I.ambient, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
		/* Call the grdgradient module */
		GMT_Report (API, GMT_MSG_INFORMATION, "Calling grdgradient with args %s\n", cmd);
		if (GMT_Call_Module (API, "grdgradient", GMT_MODULE_CMD, cmd))
			Return (API->error);
		/* Obtain the data from the virtual file */
		if ((Intens = GMT_Read_VirtualFile (API, int_grd)) == NULL)
			Return (API->error);
		i_reg = gmt_change_grdreg (GMT, Intens->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration */
	}

	/* Read data */

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing shape grid\n");

	if (!read_z && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY|GMT_GRID_NEEDS_PAD2, wesn, Ctrl->In.file, Topo) == NULL) {	/* Get topo data */
		Return (API->error);
	}
	t_reg = gmt_change_grdreg (GMT, Topo->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration */

	if (Ctrl->C.active) {
		char *dataset_cpt = (Ctrl->G.active && Ctrl->G.n == 1 && !gmt_M_file_is_image (Ctrl->G.file[0])) ? Ctrl->G.file[0] : Ctrl->In.file;
		char *cpt = gmt_cpt_default (API, Ctrl->C.file, dataset_cpt, Topo->header);
		if ((P = gmt_get_palette (GMT, cpt, GMT_CPT_OPTIONAL, Topo->header->z_min, Topo->header->z_max, Ctrl->C.dz)) == NULL) {
			Return (API->error);
		}
		if (P->is_bw) Ctrl->Q.monochrome = true;
		if (P->categorical && !(Ctrl->Q.mode == GRDVIEW_MESH || Ctrl->T.active)) {
			GMT_Report (API, GMT_MSG_ERROR, "Categorical data (as implied by CPT) cannot be interpolated and require -T or just -Qm.\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (cpt) gmt_M_str_free (cpt);
	}
	get_contours = (Ctrl->Q.mode == GRDVIEW_MESH && Ctrl->W.contour) || (Ctrl->Q.mode == GRDVIEW_SURF && P && P->n_colors > 1);

	if (Ctrl->G.active) {	/* Draping wanted */
		if (Ctrl->G.n == 1 && gmt_M_file_is_image (Ctrl->G.file[0])) {
			double inc[2];
			/* Want to drape an image on top of surface.  Do so by converting the image to r, g, b grids */
			struct GMT_IMAGE *I = NULL;
			if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA | GMT_IMAGE_NO_INDEX | GMT_GRID_NEEDS_PAD2, NULL, Ctrl->G.file[0], NULL)) == NULL) {
				Return (API->error);
			}
			/* Compute the x,y increment in the Topo x/y units that the image will have give its dimensions and pixel registration */
			inc[GMT_X] = gmt_M_get_inc (GMT, Topo->header->wesn[XLO], Topo->header->wesn[XHI], I->header->n_columns, I->header->registration);
			inc[GMT_Y] = gmt_M_get_inc (GMT, Topo->header->wesn[YLO], Topo->header->wesn[YHI], I->header->n_rows, I->header->registration);
			for (k = 0; k < 3; k++) {	/* Create 3 grids with the same w/e/s/n as topo but with its own nx/ny and registration */
				if ((Drape[k] = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Topo->header->wesn, inc, I->header->registration, 2, NULL)) == NULL) {
					Return (API->error);
				}
			}
			/* Handle transparent images */
			if (I->colormap != NULL) {	/* Image has a color map */
				/* Convert colormap from integer to unsigned char and count colors */
				unsigned char *colormap = gmt_M_memory (GMT, NULL, 4 * 256, unsigned char);
				int64_t n, j;
				n = gmt_unpack_rgbcolors (GMT, I, colormap);	/* colormap will be RGBARGBA... */
				/* Expand 8-bit indexed image to a 24-bit image */
				I->data = gmt_M_memory (GMT, I->data, 3 * I->header->size, unsigned char);
				n = 3 * I->header->size - 1;	/* Index or last pixel's blue value */
				for (j = (int64_t)I->header->size - 1; j >= 0; j--) {	/* Loop from back to front and turn index into r/g/b */
					k = 4 * I->data[j] + 3;
					I->data[n--] = colormap[--k], I->data[n--] = colormap[--k], I->data[n--] = colormap[--k];
				}
				I->header->n_bands = 3;	/* Update the number of bands from 1 to 3 */
				gmt_M_free (GMT, colormap);
			}
			else if (I->header->n_bands == 4) { /* RGBA image, with a color map, scrap the A part */
				uint64_t n4, j4;
				for (j4 = n4 = 0; j4 < 4 * I->header->size; j4++) { /* Reduce image from 32- to 24-bit */
					I->data[n4++] = I->data[j4++], I->data[n4++] = I->data[j4++], I->data[n4++] = I->data[j4++];
				}
				I->header->n_bands = 3;
			}
			/* Now assign r,g,b pixel values to the three separate drape grids */
			gmt_M_grd_loop (GMT, I, row, col, ij) {
				for (k = 0; k < 3; k++)
					Drape[k]->data[ij] = I->data[3*ij+k];
			}

			if (GMT_Destroy_Data (API, &I) != GMT_NOERROR) {	/* Image no longer needed */
				Return (API->error);
			}
			Ctrl->G.n = 3;	/* Basically pretend we read three drape grids from now on */
			Ctrl->G.image = true;	/* But not forget were we came from */
			do_G_reading = false;	/* For instance, we are not going to read in drape data since all done with that */
		}
		else {	/* Read the single (or triple) drape grids */
			if (Ctrl->G.n == 3 && Ctrl->C.active) {
				GMT_Report (API, GMT_MSG_ERROR, "Cannot specify both a CPT and the three r,g,b grids\n");
				Return (GMT_RUNTIME_ERROR);
			}
			for (k = 0; k < Ctrl->G.n; k++) {
				if ((Drape[k] = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|GMT_GRID_NEEDS_PAD2, NULL, Ctrl->G.file[k], NULL)) == NULL) {	/* Get header only */
					Return(API->error);
				}
			}
		}

		for (k = 0; k < Ctrl->G.n; k++) {	/* Process drape grid, if read */
			if (do_G_reading) {
				GMT_Report (API, GMT_MSG_INFORMATION, "Processing drape grid %s\n", Ctrl->G.file[k]);
				if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY|GMT_GRID_NEEDS_PAD2, wesn, Ctrl->G.file[k], Drape[k]) == NULL) {	/* Get drape data */
					Return (API->error);
				}
			}
			/* If Drape and Topo rids are not identical in dimension then we need to resample the draping */
			if (Drape[k]->header->n_columns != Topo->header->n_columns || Drape[k]->header->n_rows != Topo->header->n_rows) drape_resample = true;
			d_reg[k] = gmt_change_grdreg (GMT, Drape[k]->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration */
		}
		Z = Drape[0];
		n_max = MAX (MAX (Z->header->n_columns, Z->header->n_rows), n_max);	/* Watch for possibly longer dimension */
	}
	else
		Z = Topo;

	xval = Topo->x;
	yval = Topo->y;

	if (!GMT->current.proj.xyz_pos[2]) gmt_M_double_swap (GMT->common.R.wesn[ZLO], GMT->common.R.wesn[ZHI]);	/* Negative z-scale, must flip */

	gmt_grd_set_ij_inc (GMT, Z->header->mx, ij_inc);	/* Offsets for ij (with pad) indices */
	nw = gmt_M_ijp (Topo->header, 0, 0);
	ne = gmt_M_ijp (Topo->header, 0, Topo->header->n_columns - 1);
	sw = gmt_M_ijp (Topo->header, Topo->header->n_rows - 1, 0);
	se = gmt_M_ijp (Topo->header, Topo->header->n_rows - 1, Topo->header->n_columns - 1);

#if 0
	grdview_init_setup (GMT, Topo, Ctrl->N.active, Ctrl->N.level);	/* Find projected min/max in y-direction */
#endif

	grdview_set_loop_order (GMT, Z, start, stop, inc, id);

	gmt_grd_set_ij_inc (GMT, Z->header->n_columns, bin_inc);	/* Offsets for bin (no pad) indices */

	x_inc[0] = x_inc[3] = 0.0;	x_inc[1] = x_inc[2] = Z->header->inc[GMT_X];
	y_inc[0] = y_inc[1] = 0.0;	y_inc[2] = y_inc[3] = Z->header->inc[GMT_Y];

	if (get_contours) {	/* Need to find contours */
		unsigned int n_edges, *edge = NULL;
		struct GMT_GRID *Z_orig = NULL;
		GMT_Report (API, GMT_MSG_INFORMATION, "Find contours\n");

		edge = gmt_contour_edge_init (GMT, Z->header, &n_edges);
		binij = gmt_M_memory (GMT, NULL, Topo->header->nm, struct GRDVIEW_BIN);
		if ((Z_orig = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, Z)) == NULL) {
			gmt_M_free (GMT, edge);		gmt_M_free (GMT, binij);
			Return (API->error);	/* Original copy of Z grid used for contouring */
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Trace and bin contours...\n");
		for (c = 0; c < P->n_colors+1; c++) {	/* For each color change */

			/* Reset markers and set up new zero-contour */

			cval = (c == P->n_colors) ? P->data[c-1].z_high : P->data[c].z_low;

			if (cval < Z->header->z_min || cval > Z->header->z_max) continue;

			GMT_Report (API, GMT_MSG_INFORMATION, "Now tracing contour interval %8g\n", cval);
			/* Old version of loop below could give round-off since we kept subtracting the increments between successive contours.
			 * The safer way is to always start with original grid and subtract current contour value instead, as in grdcontour.
			   PW, 11/18/2011 */
			gmt_M_grd_loop (GMT, Topo, row, col, ij) {
				if (!gmt_M_is_fnan (Z_orig->data[ij])) Z->data[ij] = Z_orig->data[ij] - (gmt_grdfloat)cval;
				if (Z->data[ij] == 0.0) Z->data[ij] += (gmt_grdfloat)small;
			}

			begin = true;
			while ((ns = gmt_contours (GMT, Z, Ctrl->S.value, GMT->current.setting.interpolant, 0, edge, &begin, &x, &y)) > 0) {
				n = (uint64_t)ns;
				i_bin_old = j_bin_old = -1;
				for (pt = 1; pt < n; pt++) {
					/* Compute the lower-left bin i,j of the tile cut by the start of the contour (first 2 points) */
					i_bin = irint (floor (((0.5 * (x[pt-1] + x[pt]) - Z->header->wesn[XLO]) / Z->header->inc[GMT_X])));
					j_bin = irint (floor (((Z->header->wesn[YHI] - 0.5 * (y[pt-1] + y[pt])) / Z->header->inc[GMT_Y]))) + 1;
					if (i_bin != i_bin_old || j_bin != j_bin_old) {	/* Entering new bin */
						bin = j_bin * Z->header->n_columns + i_bin;
						this_cont = grdview_get_cont_struct (GMT, bin, binij, cval);
						this_cont->value = cval;
						this_cont->first_point = grdview_get_point (GMT, x[pt-1], y[pt-1]);
						this_point = this_cont->first_point;
						i_bin_old = i_bin;
						j_bin_old = j_bin;
					}
					this_point->next_point = grdview_get_point (GMT, x[pt], y[pt]);
					this_point = this_point->next_point;
				}
				gmt_M_free (GMT, x);
				gmt_M_free (GMT, y);
			}
			if (ns < 0) Return (-ns);
		}

		/* Remove temporary variables */

		gmt_M_free (GMT, edge);

		/* Destroy original grid and use the copy in Z_orig instead */

		if (Ctrl->G.active) {
			if (GMT_Destroy_Data (API, &Drape[0]) != GMT_NOERROR) {
				Return (API->error);
			}
			Drape[0] = Z_orig;
		}
		else {
			if (GMT_Destroy_Data (API, &Topo) != GMT_NOERROR) {
				Return (API->error);
			}
			Topo = Z_orig;
			xval = Topo->x;
			yval = Topo->y;
		}
		Z = Z_orig;
		gmt_change_grdreg (GMT, Z->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration, again */
	}

	if (use_intensity_grid && !Ctrl->I.derive) {	/* Illumination wanted from external file */

		GMT_Report (API, GMT_MSG_INFORMATION, "Processing illumination grid\n");

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY|GMT_GRID_NEEDS_PAD2, wesn, Ctrl->I.file, Intens) == NULL) {	/* Get intensity grid */
			Return (API->error);
		}
		if (Intens->header->n_columns != Topo->header->n_columns || Intens->header->n_rows != Topo->header->n_rows) {
			GMT_Report (API, GMT_MSG_ERROR, "Intensity grid has improper dimensions!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		i_reg = gmt_change_grdreg (GMT, Intens->header, GMT_GRID_NODE_REG);	/* Ensure gridline registration */
	}

	inc2[GMT_X] = 0.5 * Z->header->inc[GMT_X];	inc2[GMT_Y] = 0.5 * Z->header->inc[GMT_Y];

	for (row = 0; row < (openmp_int)Topo->header->n_rows - 1; row++) {
		/* Nodes part of tiles completely outside -R is set to NaN and thus not considered below */
		ij = gmt_M_ijp (Topo->header, row, 0);
		for (col = 0; col < (openmp_int)Topo->header->n_columns - 1; col++, ij++) {
			for (jj = n_out = 0; jj < 2; jj++) {	/* Loop over the 4 nodes making up one tile */
				for (ii = 0; ii < 2; ii++) if (gmt_map_outside (GMT, xval[col+ii], yval[row+jj])) n_out++;
			}
			if (n_out == 4) Topo->data[ij] = Topo->data[ij+1] = Topo->data[ij+Topo->header->mx] = Topo->data[ij+Topo->header->mx+1] = GMT->session.f_NaN;	/* Entire tile is outside */
		}
	}

	max_alloc = 2 * (MAX (1, Ctrl->S.value) * (n_max + 2)) + 1;	/* Allocation length for xx and yy */

	GMT_Report (API, GMT_MSG_INFORMATION, "Start creating PostScript plot\n");

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) {
		Return (GMT_RUNTIME_ERROR);
	}

	x = gmt_M_memory (GMT, NULL, max_alloc, double);
	y = gmt_M_memory (GMT, NULL, max_alloc, double);
	z = gmt_M_memory (GMT, NULL, max_alloc, double);
	v = gmt_M_memory (GMT, NULL, max_alloc, double);

	PSL_setformat (PSL, 3);

	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_AFTER, GMT_BASEMAP_ANNOT_AFTER);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);
	if (GMT->current.proj.z_pars[0] == 0.0) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	gmt_plane_perspective (GMT, -1, 0.0);

	xx = gmt_M_memory (GMT, NULL, max_alloc, double);
	yy = gmt_M_memory (GMT, NULL, max_alloc, double);
	if (Ctrl->N.active) {
		PSL_comment (PSL, "Plot the plane at desired level\n");
		if (Ctrl->N.implicit) {
			Ctrl->N.level = Topo->header->z_min;
			if (GMT->common.R.wesn[ZLO] < GMT->common.R.wesn[ZHI] && GMT->common.R.wesn[ZLO] < Ctrl->N.level)
				Ctrl->N.level = GMT->common.R.wesn[ZLO];
			GMT_Report (API, GMT_MSG_INFORMATION, "Plane/facade level determined to be z = %lg\n", Ctrl->N.level);
		}
		gmt_setpen (GMT, &Ctrl->W.pen[2]);
		if (!GMT->current.proj.z_project.draw[0]) {	/* Southern side */
			if (GMT->common.R.oblique) {
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[0], GMT->current.proj.z_project.corner_y[0], Ctrl->N.level, &xx[0], &yy[0]);
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[1], GMT->current.proj.z_project.corner_y[1], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
			else {
				for (col = 0; col < (openmp_int)Z->header->n_columns; col++) gmt_geoz_to_xy (GMT, gmt_M_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YLO], Ctrl->N.level, &xx[col], &yy[col]);
				PSL_plotline (PSL, xx, yy, Z->header->n_columns, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[2]) {	/* Northern side */
			if (GMT->common.R.oblique) {
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[3], GMT->current.proj.z_project.corner_y[3], Ctrl->N.level, &xx[0], &yy[0]);
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[2], GMT->current.proj.z_project.corner_y[2], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
			else {
				for (col = 0; col < (openmp_int)Z->header->n_columns; col++) gmt_geoz_to_xy (GMT, gmt_M_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YHI], Ctrl->N.level, &xx[col], &yy[col]);
				PSL_plotline (PSL, xx, yy, Z->header->n_columns, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[3]) {	/* Western side */
			if (GMT->common.R.oblique) {
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[0], GMT->current.proj.z_project.corner_y[0], Ctrl->N.level, &xx[0], &yy[0]);
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[3], GMT->current.proj.z_project.corner_y[3], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
			else {
				for (row = 0; row < (openmp_int)Z->header->n_rows; row++) gmt_geoz_to_xy (GMT, Z->header->wesn[XLO], gmt_M_grd_row_to_y (GMT, row, Z->header), Ctrl->N.level, &xx[row], &yy[row]);
				PSL_plotline (PSL, xx, yy, Z->header->n_rows, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[1]) {	/* Eastern side */
			if (GMT->common.R.oblique) {
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[1], GMT->current.proj.z_project.corner_y[1], Ctrl->N.level, &xx[0], &yy[0]);
				gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[2], GMT->current.proj.z_project.corner_y[2], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
			else {
				for (row = 0; row < (openmp_int)Z->header->n_rows; row++) gmt_geoz_to_xy (GMT, Z->header->wesn[XHI], gmt_M_grd_row_to_y (GMT, row, Z->header), Ctrl->N.level, &xx[row], &yy[row]);
				PSL_plotline (PSL, xx, yy, Z->header->n_rows, PSL_MOVE|PSL_STROKE|PSL_CLOSE);
			}
		}

		if (!GMT->common.R.oblique) {
			gmt_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YLO], Ctrl->N.level, &xx[0], &yy[0]);
			gmt_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YLO], Ctrl->N.level, &xx[1], &yy[1]);
			gmt_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YHI], Ctrl->N.level, &xx[2], &yy[2]);
			gmt_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YHI], Ctrl->N.level, &xx[3], &yy[3]);
			if (!gmt_M_is_fnan (Topo->data[nw])) {
				gmt_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YHI], (double)(Topo->data[nw]), &x_left, &y_top);
				PSL_plotsegment (PSL, x_left, y_top, xx[3], yy[3]);
			}
			if (!gmt_M_is_fnan (Topo->data[ne])) {
				gmt_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YHI], (double)(Topo->data[ne]), &x_right, &y_top);
				PSL_plotsegment (PSL, x_right, y_top, xx[2], yy[2]);
			}
			if (!gmt_M_is_fnan (Topo->data[se])) {
				gmt_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YLO], (double)(Topo->data[se]), &x_right, &y_bottom);
				PSL_plotsegment (PSL, x_right, y_bottom, xx[1], yy[1]);
			}
			if (!gmt_M_is_fnan (Topo->data[sw])) {
				gmt_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YLO], (double)(Topo->data[sw]), &x_left, &y_bottom);
				PSL_plotsegment (PSL, x_left, y_bottom, xx[0], yy[0]);
			}
		}
	}

	if (Ctrl->T.active)	/* Plot colored graticules instead */
		gmt_plot_image_graticules (GMT, Topo, Intens, P, (Ctrl->T.outline) ? &Ctrl->T.pen : NULL, Ctrl->T.skip, Ctrl->I.constant ? &Ctrl->I.value : NULL);
	else if (Ctrl->Q.mode == GRDVIEW_IMAGE) {	/* Plot image */
		int nx_i, ny_i, ip, jp, min_i, max_i, min_j, max_j, dist;
		int done, layers, last_i, last_j;
		int *top_jp = NULL, *bottom_jp = NULL, *ix = NULL, *iy = NULL;
		uint64_t d_node, nm_i, node, kk, p;
		double xp, yp, sum_w, w, sum_i, x_width, y_width, value;
		double sum_r, sum_g, sum_b, intval = 0.0, *y_drape = NULL, *x_drape = NULL;
		gmt_grdfloat *int_drape = NULL;
		unsigned char *bitimage_24 = NULL, *bitimage_8 = NULL;

		GMT_Report (API, GMT_MSG_INFORMATION, "Place image\n");

		if (Ctrl->C.active && P->has_pattern)
			GMT_Report (API, GMT_MSG_WARNING, "Patterns in CPT will not work with -Qi\n");
		GMT_Report (API, GMT_MSG_INFORMATION, "Get and store projected vertices\n");

		PSL_comment (PSL, "Plot 3-D surface using scanline conversion of polygons to raster image\n");

		x_width = GMT->current.proj.z_project.xmax - GMT->current.proj.z_project.xmin;	/* Size of image in inches */
		y_width = GMT->current.proj.z_project.ymax - GMT->current.proj.z_project.ymin;
		nx_i = irint (x_width * Ctrl->Q.dpi);	/* Size of image in pixels */
		ny_i = irint (y_width * Ctrl->Q.dpi);
		last_i = nx_i - 1;	last_j = ny_i - 1;
		min_i = min_j = INT_MAX;	max_i = max_j = -INT_MAX;

		if (drape_resample) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Resampling relief grid to drape grid resolution\n");
			if (use_intensity_grid) GMT_Report (API, GMT_MSG_INFORMATION, "Resampling illumination grid to drape grid resolution\n");
			ix = gmt_M_memory (GMT, NULL, Z->header->nm, int);
			iy = gmt_M_memory (GMT, NULL, Z->header->nm, int);
			x_drape = Z->x;
			y_drape = Z->y;
			if (use_intensity_grid) int_drape = gmt_M_memory (GMT, NULL, Z->header->mx*Z->header->my, gmt_grdfloat);
			bin = 0;	/* bin cycles over a grid with no padding, hence bin++ is good enough */
			gmt_M_grd_loop (GMT, Z, row, col, ij) {	/* Get projected coordinates converted to pixel locations */
				value = gmt_bcr_get_z (GMT, Topo, x_drape[col], y_drape[row]);	/* Relief value at drape coordinate */
				if (gmt_M_is_dnan (value))	/* Outside -R or NaNs not used */
					ix[bin] = iy[bin] = -1;
				else {	/* Valid relief, compute projected point on 3-D surface */
					gmt_geoz_to_xy (GMT, x_drape[col], y_drape[row], value, &xp, &yp);
					/* Make sure ix,iy pixel values fall in the range (0,nx_i-1), (0,ny_i-1) */
					ix[bin] = MAX(0, MIN(irint (floor((xp - GMT->current.proj.z_project.xmin) * Ctrl->Q.dpi)), last_i));
					iy[bin] = MAX(0, MIN(irint (floor((yp - GMT->current.proj.z_project.ymin) * Ctrl->Q.dpi)), last_j));
				}
				if (use_intensity_grid)	/* Get intensity value at drape coordinate */
					int_drape[ij] = (gmt_grdfloat)gmt_bcr_get_z (GMT, Intens, x_drape[col], y_drape[row]);
				if (ix[bin] < min_i) min_i = ix[bin];
				if (ix[bin] > max_i) max_i = ix[bin];
				if (iy[bin] < min_j) min_j = iy[bin];
				if (iy[bin] > max_j) max_j = iy[bin];
				bin++;
			}
			if (use_intensity_grid) {	/* Reset intensity grid so that we have no boundary row/cols */
				saved_data_pointer = Intens->data;
				Intens->data = int_drape;
			}
		}
		else {	/* Drape and relief grid [and optional intensity grid] all of same dimensions */
			ix = gmt_M_memory (GMT, NULL, Topo->header->nm, int);
			iy = gmt_M_memory (GMT, NULL, Topo->header->nm, int);
			bin = 0;
			gmt_M_grd_loop (GMT, Z, row, col, ij) {	/* Get projected relief coordinates converted to pixel locations */
				if (gmt_M_is_fnan (Topo->data[ij]))	/* Outside -R or NaNs not used */
					ix[bin] = iy[bin] = -1;
				else {
					gmt_geoz_to_xy (GMT, xval[col], yval[row], (double)Topo->data[ij], &xp, &yp);
					/* Make sure ix,iy fall in the range (0,nx_i-1), (0,ny_i-1) */
					ix[bin] = MAX(0, MIN(irint (floor((xp - GMT->current.proj.z_project.xmin) * Ctrl->Q.dpi)), last_i));
					iy[bin] = MAX(0, MIN(irint (floor((yp - GMT->current.proj.z_project.ymin) * Ctrl->Q.dpi)), last_j));
					if (ix[bin] < min_i) min_i = ix[bin];
					if (ix[bin] > max_i) max_i = ix[bin];
					if (iy[bin] < min_j) min_j = iy[bin];
					if (iy[bin] > max_j) max_j = iy[bin];
				}
				bin++;
			}
		}

#if 0
		/* Get a sensible zero distance adjustment for Shepard weights in blending colors for pixels inside a tile.
		 * each pixel will get color contribution from the 4 nodes making up the tile, and we base it on the distance
		 * from the current pixel to each of the 4 nodes.  We weigh the contributions using w = 1/(d_off+dist) so
		 * that we don't blow up at a node.  We choose d_off as 10% of typical pixels per tile. This filters the values
		 * a little bit.
		 * PW Feb-24-2018: Commented out for now.  What we had is not that bad but suffers from aliasing when the dpi
		 * is low.  Perhaps a scheme that sets d_off to tiny when dpi passes some threshold.  */

		d_off = 0.1 * hypot ((max_i - min_i)/((double)Z->header->n_columns), (max_j - min_j)/((double)Z->header->n_rows));
		GMT_Report (API, GMT_MSG_DEBUG, "Zero-distance 1/(off+r) offset for tile averaging: %g\n", d_off);
#endif

		/* Allocate image array and set background to PAGE_COLOR */

		if (Ctrl->Q.monochrome) {
			char gray;

			nm_i = (uint64_t)nx_i * ny_i;
			layers = 1;
			bitimage_8 = gmt_M_memory (GMT, NULL, nm_i, unsigned char);
			gray = gmt_M_u255 (gmt_M_yiq (GMT->current.setting.ps_page_rgb));
			memset (bitimage_8, gray, nm_i * sizeof (unsigned char));
		}
		else {
			nm_i = nx_i * ny_i * 3;
			layers = 3;
			if (Ctrl->Q.mask) PS_colormask_off = 3;
			bitimage_24 = gmt_M_memory (GMT, NULL, nm_i + PS_colormask_off, unsigned char);
			if (Ctrl->C.active && Ctrl->Q.mask)
				gmt_M_rgb_copy (rgb, P->bfn[GMT_NAN].rgb);
			else
				gmt_M_rgb_copy (rgb, GMT->current.setting.ps_page_rgb);
			kk = 0;
			while (kk < (nm_i + PS_colormask_off)) {
				for (k = 0; k < 3; k++) bitimage_24[kk++] = gmt_M_u255 (rgb[k]);
			}
		}

		if (!Ctrl->Q.mask) {	/* Set up arrays for staircase clippath and initialize them */
			top_jp = gmt_M_memory (GMT, NULL, nx_i, int);
			bottom_jp = gmt_M_memory (GMT, NULL, nx_i, int);
			for (ip = 0; ip < nx_i; ip++) bottom_jp[ip] = ny_i;
		}

		/* Plot from back to front */

		gmt_M_memset (rgb, 4, double);
		GMT_Report (API, GMT_MSG_INFORMATION, "Start rasterization\n");
		for (j = start[0]; j != stop[0]; j += inc[0]) {

			GMT_Report (API, GMT_MSG_DEBUG, "Scan line conversion at j-line %.6ld\n", j);

			for (i = start[1]; i != stop[1]; i += inc[1]) {
				if (id[0] == GMT_Y) {
					bin = gmt_M_ij0 (Z->header, j, i);
					ij = gmt_M_ijp (Z->header, j, i);
				}
				else {
					bin = gmt_M_ij0 (Z->header, i, j);
					ij = gmt_M_ijp (Z->header, i, j);
				}
				for (k = bad = 0; !bad && k < 4; k++) bad = (ix[bin+bin_inc[k]] < 0 || iy[bin+bin_inc[k]] < 0);
				if (bad) continue;
				min_i = max_i = ix[bin];
				min_j = max_j = iy[bin];
				for (k = 1; k < 4; k++) {	/* Loop over the tile made up of bin and its 3 neighbors to the E, NE, and N */
					p = bin+bin_inc[k];
					if (ix[p] < min_i) min_i = ix[p];
					if (ix[p] > max_i) max_i = ix[p];
					if (iy[p] < min_j) min_j = iy[p];
					if (iy[p] > max_j) max_j = iy[p];
				}
				for (jp = min_j; jp <= max_j; jp++) {	/* Loop over all the pixels (rectangular domain) that may make up this tile */
					if (jp < 0 || jp >= ny_i) continue;
					for (ip = min_i; ip <= max_i; ip++) {
						if (ip < 0 || ip >= nx_i) continue;
						if (!grdview_pixel_inside (GMT, ip, jp, ix, iy, bin, bin_inc)) continue;	/* Checks if actually inside the projected tile polygon */
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
									rgb[kk] = gmt_M_is255 (Drape[kk]->data[d_node]);
									if (rgb[kk] < 0.0) rgb[kk] = 0; else if (rgb[kk] > 1.0) rgb[kk] = 1.0;
								}
								if (Ctrl->C.active && gmt_M_same_rgb (rgb, P->bfn[GMT_NAN].rgb)) continue;	/* Skip NaN colors */
							}
							else {		/* Use lookup to get color from either relief or drape grid (Z points to it) */
								gmt_get_rgb_from_z (GMT, P, Z->data[d_node], rgb);
								if (gmt_M_is_fnan (Z->data[d_node])) continue;	/* Skip NaNs in the z-data*/
							}
							if (use_intensity_grid && gmt_M_is_fnan (Intens->data[d_node])) continue;	/* Skip NaNs in the intensity data*/
							/* We don't want to blend in the (typically) gray NaN colors with the others. */

							good++;
							dist = grdview_quick_idist (ip, jp, ix[node], iy[node]);
							if (dist == 0) {	/* Only need this node value */
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
						if (Ctrl->Q.special) gmt_get_rgb_from_z (GMT, P, Z->data[ij], rgb);
						if (Ctrl->I.active && good) gmt_illuminate (GMT, intval, rgb);
						kk = layers * ((ny_i-jp-1) * nx_i + ip) + PS_colormask_off;
						if (Ctrl->Q.monochrome) /* gmt_M_yiq transformation */
							bitimage_8[kk] = (unsigned char) gmt_M_yiq (rgb);
						else {
							for (k = 0; k < 3; k++) bitimage_24[kk++] = gmt_M_u255 (rgb[k]);
						}
					}
				}
			}
		}

		if (!Ctrl->Q.mask) {	/* Must implement the clip path for the perspective image */
			/* We now have the top and bottom j-pixel per i-pixel and will build a closed clip path.
			 * First, we find the first and last i-pixel of the actual image, then do the loop.
			 * Because pixels are square and of finite size we build a stair-step curve around them,
			 * hence we need 2x the number of coordinates for both the top and bottom sides */
			int ip_l, ip_r, first_ip = 0, last_ip = nx_i - 1;
			bool go_right = true, go_left = true;
			double mid_y = 0.5 * (GMT->current.proj.z_project.ymin + GMT->current.proj.z_project.ymax);

			x_pixel_size = x_width / (double)nx_i;
			y_pixel_size = y_width / (double)ny_i;

			for (ip_l = 0, ip_r = nx_i - 1; ip_l < nx_i; ip_l++, ip_r--) {
				if (go_right && top_jp[ip_l] < bottom_jp[ip_l])	/* No pixels yet in this column, we can move to the right */
					first_ip = ip_l + 1;
				else	/* First non-empty sliver */
					go_right = false;
				if (go_left && top_jp[ip_r] < bottom_jp[ip_r])	/* No pixels yet in this column, we can move to the left */
					last_ip = ip_r - 1;
				else	/* First non-empty sliver */
					go_left = false;
			}
			/* Compute number of points needed to draw the staircase clip path */
			n4 = 4 * (last_ip - first_ip + 1);	/* 2 sides, and 2 points per pixel */
			x_imask = gmt_M_memory (GMT, NULL, n4, double);
			y_imask = gmt_M_memory (GMT, NULL, n4, double);
			nk = n4 - 1;

			for (ip = first_ip, k = 0; ip <= last_ip; ip++, k += 2) {	/* From first to last i-pixel */
				k1 = k + 1;
				x_imask[k]  = x_imask[nk-k]  = GMT->current.proj.z_project.xmin + ip * x_pixel_size;
				x_imask[k1] = x_imask[nk-k1] = x_imask[k] + x_pixel_size;
				if (top_jp[ip] < bottom_jp[ip]) {	/* No pixels set in this column (e.g., in the middle due to NaNs?) */
					y_imask[k] = y_imask[k1] = y_imask[nk-k] = y_imask[nk-k1] = mid_y;	/* Somewhat arbitrary */
				}
				else {	/* Set top of upper pixel and bottom of lower pixel steps */
					y_imask[k] = y_imask[k1] = GMT->current.proj.z_project.ymin + (top_jp[ip] + 1) * y_pixel_size;
					y_imask[nk-k] = y_imask[nk-k1] = GMT->current.proj.z_project.ymin + bottom_jp[ip] * y_pixel_size;
				}
			}
#if 0
			{	/* For debugging the clip path - set the if# to 1 */
				FILE *fp = fopen ("image_mask.txt", "w");
				for (k = 0; k < n4; k++)
					fprintf (fp, "%lg\t%lg\n", x_imask[k], y_imask[k]);
				fclose (fp);
			}
#endif
			PSL_beginclipping (PSL, x_imask, y_imask, n4, GMT->session.no_rgb, 3);
			gmt_M_free (GMT, x_imask);
			gmt_M_free (GMT, y_imask);
		}

		GMT_Report (API, GMT_MSG_INFORMATION, "Creating PostScript image ");
		if (Ctrl->Q.monochrome) {
			if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Report (API, GMT_MSG_INFORMATION, "[B/W image]\n");
			PSL_plotcolorimage (PSL, GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin,
			                    x_width, y_width, PSL_BL, bitimage_8, nx_i, ny_i, 8);
			gmt_M_free (GMT, bitimage_8);
		}
		else {
			if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Report (API, GMT_MSG_INFORMATION, "[color image]\n");
			PSL_plotcolorimage (PSL, GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin,
			                    x_width, y_width, PSL_BL, bitimage_24, Ctrl->Q.mask ? -nx_i : nx_i, ny_i, 24);
			gmt_M_free (GMT, bitimage_24);
		}

		if (!Ctrl->Q.mask){
			PSL_endclipping (PSL, 1);	/* Undo mask clipping */
			gmt_M_free (GMT, top_jp);
			gmt_M_free (GMT, bottom_jp);
		}

		gmt_M_free (GMT, ix);
		gmt_M_free (GMT, iy);
	}

	else if (Ctrl->Q.mode == GRDVIEW_WATERFALL_Y) {	/* Plot Y waterfall */
		unsigned int ix = (id[0] == GMT_X) ? 0 : 1, iy = (id[0] == GMT_Y) ? 0 : 1;
		double z_base = Ctrl->N.active ? Ctrl->N.level : Z->header->z_min;
		GMT_Report (API, GMT_MSG_INFORMATION, "Place Y waterfall plot\n");
		PSL_comment (PSL, "Start of waterfall plot\n");
		gmt_setpen (GMT, &Ctrl->W.pen[1]);
		gmt_setfill (GMT, &Ctrl->Q.fill, 1);
		if (Ctrl->Q.monochrome)
			Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = gmt_M_yiq (Ctrl->Q.fill.rgb);	/* Do gmt_M_yiq transformation */
		for (i = start[ix]+1; i != stop[ix]; i += inc[ix]) {
			for (k = 0, j = start[iy]-1; j != stop[iy]; j += inc[iy], k++) {
				ij = gmt_M_ijp (Topo->header, j, i);
				if (gmt_M_is_fnan (Topo->data[ij+ij_inc[0]])) continue;
				gmt_geoz_to_xy (GMT, xval[i], yval[j], (double)(Topo->data[ij+ij_inc[1]]), &xx[k], &yy[k]);
			}
			gmt_geoz_to_xy (GMT, xval[i], yval[start[iy]-1] + (k - 1) * (inc[ix] * Z->header->inc[GMT_Y]), z_base, &xx[k], &yy[k]);
			gmt_geoz_to_xy (GMT, xval[i], yval[start[iy]-1], z_base, &xx[k+1], &yy[k+1]);
			PSL_plotpolygon (PSL, xx, yy, k+2);
		}
	}

	else if (Ctrl->Q.mode == GRDVIEW_WATERFALL_X) {	/* Plot X waterfall */
		unsigned int ix = (id[0] == GMT_X) ? 0 : 1, iy = (id[0] == GMT_Y) ? 0 : 1;
		double z_base = Ctrl->N.active ? Ctrl->N.level : Z->header->z_min;
		GMT_Report (API, GMT_MSG_INFORMATION, "Place X waterfall plot\n");
		PSL_comment (PSL, "Start of waterfall plot\n");
		gmt_setpen (GMT, &Ctrl->W.pen[1]);
		gmt_setfill (GMT, &Ctrl->Q.fill, 1);
		if (Ctrl->Q.monochrome)
			Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = gmt_M_yiq (Ctrl->Q.fill.rgb);	/* Do gmt_M_yiq transformation */
		for (j = start[iy]-1; j != stop[iy]; j += inc[iy]) {
			for (k = 0, i = start[ix]+1; i != stop[ix]; i += inc[ix], k++) {
				ij = gmt_M_ijp (Topo->header, j, i);
				if (gmt_M_is_fnan (Topo->data[ij+ij_inc[0]])) continue;
				gmt_geoz_to_xy (GMT, xval[start[ix]+1] + k * (inc[ix] * Z->header->inc[GMT_X]), yval[j],
					(double)(Topo->data[ij+ij_inc[1]]), &xx[k], &yy[k]);
			}
			gmt_geoz_to_xy (GMT, xval[start[ix]+1] + (k - 1) * (inc[ix] * Z->header->inc[GMT_X]), yval[j], z_base, &xx[k], &yy[k]);
			gmt_geoz_to_xy (GMT, xval[start[ix]+1], yval[j], z_base, &xx[k+1], &yy[k+1]);
			PSL_plotpolygon (PSL, xx, yy, k+2);
		}
	}

	else if (Ctrl->Q.mode == GRDVIEW_MESH) {	/* Plot mesh */
		GMT_Report (API, GMT_MSG_INFORMATION, "Do mesh plot with mesh color %s\n", gmt_putcolor (GMT, Ctrl->Q.fill.rgb));
		PSL_comment (PSL, "Start of mesh plot\n");
		gmt_setpen (GMT, &Ctrl->W.pen[1]);
		if (Ctrl->Q.monochrome)
			Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = gmt_M_yiq (Ctrl->Q.fill.rgb);	/* Do gmt_M_yiq transformation */
		for (j = start[0]; j != stop[0]; j += inc[0]) {
			for (i = start[1]; i != stop[1]; i += inc[1]) {
				if (id[0] == GMT_Y) {
					y_bottom = yval[j];
					x_left = xval[i];
					bin = gmt_M_ij0 (Z->header, j, i);
					ij = gmt_M_ijp (Topo->header, j, i);
				}
				else {
					y_bottom = yval[i];
					x_left = xval[j];
					bin = gmt_M_ij0 (Z->header, i, j);
					ij = gmt_M_ijp (Topo->header, i, j);
				}
				for (k = bad = 0; !bad && k < 4; k++) bad += gmt_M_is_fnan (Topo->data[ij+ij_inc[k]]);
				if (bad) continue;
				y_top = y_bottom + Z->header->inc[GMT_Y];
				x_right = x_left + Z->header->inc[GMT_X];
				gmt_geoz_to_xy (GMT, x_left, y_bottom, (double)(Topo->data[ij+ij_inc[0]]), &xx[0], &yy[0]);
				gmt_geoz_to_xy (GMT, x_right, y_bottom, (double)(Topo->data[ij+ij_inc[1]]), &xx[1], &yy[1]);
				gmt_geoz_to_xy (GMT, x_right, y_top, (double)(Topo->data[ij+ij_inc[2]]), &xx[2], &yy[2]);
				gmt_geoz_to_xy (GMT, x_left, y_top, (double)(Topo->data[ij+ij_inc[3]]), &xx[3], &yy[3]);
				gmt_setfill (GMT, &Ctrl->Q.fill, 1);
				PSL_plotpolygon (PSL, xx, yy, 4);
				if (Ctrl->W.contour) {
					pen_set = false;
					if (binij[bin].first_cont == NULL) continue;
					for (this_cont = binij[bin].first_cont->next_cont; this_cont; this_cont = this_cont->next_cont) {
						for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
							z_val = (Ctrl->G.active) ? gmt_bcr_get_z (GMT, Topo, (double)this_point->x, (double)this_point->y) : this_cont->value;
							if (gmt_M_is_dnan (z_val)) continue;
							gmt_geoz_to_xy (GMT, (double)this_point->x, (double)this_point->y, z_val, &xx[k], &yy[k]);
							k++;
						}
						if (!pen_set) {
							gmt_setpen (GMT, &Ctrl->W.pen[0]);
							pen_set = true;
						}
						PSL_plotline (PSL, xx, yy, k, PSL_MOVE|PSL_STROKE);
					}
					if (pen_set) gmt_setpen (GMT, &Ctrl->W.pen[1]);
				}
			}
		}
	}

	else if (Ctrl->Q.mode == GRDVIEW_SURF) {	/* Plot surface using closed polygons */
		int start_side, entry_side, exit_side, next_side, low, ncont, nw_se_diagonal, check;
		int corner[2], bad_side[2][2], p, p1, p2, saddle_sign;
		double *xcont = NULL, *ycont = NULL, *zcont = NULL, *vcont = NULL, X_vert[4], Y_vert[4], saddle_small;
		gmt_grdfloat Z_vert[4];

		GMT_Report (API, GMT_MSG_INFORMATION, "Place filled surface\n");
		/* PW: Bugs fixed in Nov, 2011: Several problems worth remembering:
			1) Earlier [2004] we had fixed grdcontour but not grdview in dealing with the current zero contour.  Because
			   of gmt_grdfloat precision we cannot take the grid and repeatedly subtract the difference in contour values.
			   Instead for each contour value cval, we must subtract cval from the original grid to get the tmp grid.
			2) To avoid never to have contours go through nodes EXACTLY, we check if there are nodes that equal zero.
			   If so, we add small (a small amount to make it different from zero).  We then get contours.  However,
			   for -Qs we again use the grid node values to determine which way to loop around a polygon piece when
			   stitching it together from pieces of contours and node values. Thus it is important that we use node
			   values that have been adjusted as above and not the original nodes.
			3) We should make sure that the small value exceeeds single-precision EPS (~ 1.2e-7), otherwise adding small
			   will not make the node non-zero.  This does not seem to have been a problem yet but I added a new check
			   just in case and if so set small to 1.0e-7.
			4) Given zgrd is gmt_grdfloat, there is a difference between zgrd[node] -= (gmt_grdfloat)cval and zgrd[node] -= cval,
			   since cval is double precision.  We had the cast during the contouring but here under -Qs we had some
			   comparisons without the cast, the result being both sides got promoted to double prior to the test and
			   we can get a different result.
			5) In the case of no contour we paint the entire tile.  However, in the case of discontinuous CPTs we
			   based the color on the lower-left node value (since it should not matter which corner we pick).  But it
			   does: If that node HAPPENS to be one that had small added to it and as a result no contour goes through,
			   we have no way of adjusting the node accordingly.  Consequently, it is safer to always take the average
			   of the 4 nodes as the other 3 will pull the average into the middle somewhere.
		*/

		xcont = gmt_M_memory (GMT, NULL, max_alloc, double);
		ycont = gmt_M_memory (GMT, NULL, max_alloc, double);
		zcont = gmt_M_memory (GMT, NULL, max_alloc, double);
		vcont = gmt_M_memory (GMT, NULL, max_alloc, double);

		PSL_comment (PSL, "Start of filled surface\n");
		if (Ctrl->Q.outline) gmt_setpen (GMT, &Ctrl->W.pen[1]);

		for (j = start[0]; j != stop[0]; j += inc[0]) {
			for (i = start[1]; i != stop[1]; i += inc[1]) {
				if (id[0] == GMT_Y) {
					y_bottom = yval[j];
					x_left = xval[i];
					bin = gmt_M_ij0 (Topo->header, j, i);
					ij = gmt_M_ijp (Topo->header, j, i);
				}
				else {
					y_bottom = yval[i];
					x_left = xval[j];
					bin = gmt_M_ij0 (Topo->header, i, j);
					ij = gmt_M_ijp (Topo->header, i, j);
				}
				y_top = y_bottom + Z->header->inc[GMT_Y];
				x_right = x_left + Z->header->inc[GMT_X];
				for (k = bad = 0; !bad && k < 4; k++) bad += gmt_M_is_fnan (Topo->data[ij+ij_inc[k]]);
				if (bad) {
					if (P->bfn[GMT_NAN].skip || GMT->current.proj.three_D) continue;

					X_vert[0] = X_vert[3] = x_left;	X_vert[1] = X_vert[2] = x_right;
					Y_vert[0] = Y_vert[1] = y_bottom;	Y_vert[2] = Y_vert[3] = y_top;
					for (k = 0; k < 4; k++) gmt_geoz_to_xy (GMT, X_vert[k], Y_vert[k], 0.0, &xmesh[k], &ymesh[k]);
					grdview_paint_it (GMT, PSL, P, xmesh, ymesh, 4, GMT->session.d_NaN, false, Ctrl->Q.monochrome, 0.0, Ctrl->Q.outline);
					continue;
				}

				if (Ctrl->I.active) {
					if (use_intensity_grid) {
						this_intensity = grdview_get_intensity (Intens, ij);
						if (gmt_M_is_dnan (this_intensity)) continue;
					}
					else
						this_intensity = Ctrl->I.value;
				}

				PSL_comment (PSL, "Filled surface bin (%d, %d)\n", j, i);
				/* Get mesh polygon */

				X_vert[0] = X_vert[3] = x_left;		X_vert[1] = X_vert[2] = x_right;
				Y_vert[0] = Y_vert[1] = y_bottom;	Y_vert[2] = Y_vert[3] = y_top;

				/* Also get z-values at the 4 nodes.  Because some nodes may have been adjusted by small during
				 * the contouring stage we need to make the same adjustments below */

				for (k = 0; k < 4; k++) Z_vert[k] = Z->data[ij+ij_inc[k]];	/* First a straight copy */

				if (get_contours && binij[bin].first_cont) {	/* Contours go through here */

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
						Z_vert[k] -= (gmt_grdfloat)this_cont->value;	/* Note we cast to gmt_grdfloat to get the same precision as for contours */
						if (Z_vert[k] == 0.0) Z_vert[k] += (gmt_grdfloat)small;
						Z_vert[k] += (gmt_grdfloat)this_cont->value;
					}
					/* Here, Z_vert reflects what the grid was when contouring was determined */

					if (saddle) {	/* Must deal with this separately */

						this_point = this_cont->first_point;
						entry_side = grdview_get_side (this_point->x, this_point->y, x_left, y_bottom, Z->header->inc, inc2);
						while (this_point->next_point) this_point = this_point->next_point;	/* Go to end */
						exit_side  = grdview_get_side (this_point->x, this_point->y, x_left, y_bottom, Z->header->inc, inc2);


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
							grdview_add_node (NULL, x, y, z, v, &n, low, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[low]);
							next_side = low;
							way = 0;

							for (this_cont = start_cont; this_cont; this_cont = this_cont->next_cont) {

								/* First get all the x/y pairs for this contour */

								for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
									xcont[k] = this_point->x;
									ycont[k] = this_point->y;
									zcont[k] = (Ctrl->G.active) ? gmt_bcr_get_z (GMT, Topo, xcont[k], ycont[k]) : this_cont->value;
									if (gmt_M_is_dnan (zcont[k])) continue;
									vcont[k] = this_cont->value;
									k++;
								}
								ncont = k;

								entry_side = grdview_get_side (xcont[0], ycont[0], x_left, y_bottom, Z->header->inc, inc2);
								exit_side  = grdview_get_side (xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

								if (entry_side == bad_side[p][0] || entry_side == bad_side[p][1]) continue;
								if (exit_side == bad_side[p][0] || exit_side == bad_side[p][1]) continue;

								/* OK, got the correct contour */

								next_up = (this_cont->next_cont) ? this_cont->next_cont->value : DBL_MAX;

								exit_side  = grdview_get_side (xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

								if (way == 0 || next_side == entry_side)	/* Just hook up */
									grdview_copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								else if (next_side == exit_side)	/* Just hook up but reverse */
									grdview_copy_points_bw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);

								/* Compute the xy from the xyz triplets */

								for (k = 0; k < n; k++) gmt_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);
								z_ave = (P->is_continuous) ? grdview_get_z_ave (v, next_up, n) : this_cont->value;

								/* Now paint the polygon piece */

								grdview_paint_it (GMT, PSL, P, xx, yy, (int)n, z_ave-saddle_small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, 0);

								/* Reset the anchor points to previous contour */

								n = 0;
								grdview_copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = exit_side;
								way = (Z_vert[low] < (gmt_grdfloat)this_cont->value) ? -1 : 1;
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
							grdview_add_node (NULL, x, y, z, v, &n, p1, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[p1]);
							grdview_add_node (NULL, x, y, z, v, &n, p2, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[p2]);

							/* Compute the xy from the xyz triplets */

							for (k = 0; k < n; k++) gmt_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);

							z_ave = (P->is_continuous) ? grdview_get_z_ave (v, next_up, n) : v[0];

							/* Now paint the polygon piece */

							grdview_paint_it (GMT, PSL, P, xx, yy, (int)n, z_ave+saddle_small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, 0);

						} /* End triangular piece */

					} /* End Saddle section */
					else {
						bool nused[4] = {false, false, false, false};	/* PW: Used to avoid using nodes twice - I did not extend this to the saddle case as not sure if it is needed */
						/* Ok, here we do not have to worry about saddles */

						/* Find lowest corner (id = low) */

						for (k = 1, low = 0; k < 4; k++) if (Z_vert[k] < Z_vert[low]) low = k;

						/* Set this points as the start anchor */

						n = 0;
						grdview_add_node (nused, x, y, z, v, &n, low, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[low]);
						start_side = next_side = low;
						way = 1;

						this_cont = start_cont;
						while (this_cont) {

							next_up = (this_cont->next_cont) ? this_cont->next_cont->value : DBL_MAX;
							/* First get all the x/y pairs for this contour */

							for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
								xcont[k] = this_point->x;
								ycont[k] = this_point->y;
								zcont[k] = (Ctrl->G.active) ? gmt_bcr_get_z (GMT, Topo, xcont[k], ycont[k]) : this_cont->value;
								if (gmt_M_is_dnan (zcont[k])) continue;
								vcont[k] = this_cont->value;
								k++;
							}
							ncont = k;

							entry_side = grdview_get_side (xcont[0], ycont[0], x_left, y_bottom, Z->header->inc, inc2);
							exit_side  = grdview_get_side (xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

							while (!(next_side == entry_side || next_side == exit_side)) {	/* Must add intervening corner */
								if (way == 1) next_side = (next_side + 1) % 4;
								grdview_add_node (nused, x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[next_side]);
								if (way == -1) next_side = (next_side - 1 + 4) % 4;
							}
							if (next_side == entry_side) {	/* Just hook up */
								grdview_copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = exit_side;
							}
							else if (next_side == exit_side) {	/* Just hook up but reverse */
								grdview_copy_points_bw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = entry_side;
							}
							/* Now we must complete the polygon if necessary */

							while (!(start_side == next_side)) {	/* Must add intervening corner */
								if (way == 1) next_side = (next_side + 1) % 4;
								grdview_add_node (nused, x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[next_side]);
								if (way == -1) next_side = (next_side - 1 + 4) % 4;
							}

							/* Compute the xy from the xyz triplets */

							for (k = 0; k < n; k++) gmt_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);
							z_ave = (P->is_continuous) ? grdview_get_z_ave (v, next_up, n) : this_cont->value;

							/* Now paint the polygon piece */

							grdview_paint_it (GMT, PSL, P, xx, yy, (int)n, z_ave-small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, 0);

							/* Reset the anchor points to previous contour */

							n = 0;
							grdview_copy_points_fw (x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
							next_side = exit_side;
							start_side = entry_side;
							way = (Z_vert[start_side] < (gmt_grdfloat)this_cont->value) ? -1 : 1;

							this_cont = this_cont->next_cont;	/* Goto next contour */
 						}

						/* Final contour needs to compete with corners only */

						while (!(start_side == next_side)) {	/* Must add intervening corner */
							if (way == 1) next_side = (next_side +1) % 4;
							grdview_add_node (nused, x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z_vert, ij+ij_inc[next_side]);
							if (way == -1) next_side = (next_side - 1 + 4) % 4;
						}

						/* Compute the xy from the xyz triplets */

						for (k = 0; k < n; k++) gmt_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);

						z_ave = (P->is_continuous) ? grdview_get_z_ave (v, next_up, n) : v[0];

						/* Now paint the polygon piece */

						grdview_paint_it (GMT, PSL, P, xx, yy, (int)n, z_ave+small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, 0);

					} /* End non-saddle case */

					/* Draw contour lines if desired */

					pen_set = false;
					for (this_cont = start_cont; Ctrl->W.contour && this_cont; this_cont = this_cont->next_cont) {
						for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
							z_val = (Ctrl->G.active) ? gmt_bcr_get_z (GMT, Topo, (double)this_point->x, (double)this_point->y) : this_cont->value;
							if (gmt_M_is_dnan (z_val)) continue;

							gmt_geoz_to_xy (GMT, (double)this_point->x, (double)this_point->y, z_val, &xx[k], &yy[k]);
							k++;
						}
						if (!pen_set) {
							gmt_setpen (GMT, &Ctrl->W.pen[0]);
							pen_set = true;
						}
						PSL_plotline (PSL, xx, yy, k, PSL_MOVE|PSL_STROKE);
					}
					if (pen_set) gmt_setpen (GMT, &Ctrl->W.pen[1]);
					if (Ctrl->Q.outline) {
						for (k = 0; k < 4; k++) gmt_geoz_to_xy (GMT, X_vert[k], Y_vert[k], (double)(Topo->data[ij+ij_inc[k]]), &xmesh[k], &ymesh[k]);
						PSL_setfill (PSL, GMT->session.no_rgb, 1);
						PSL_plotpolygon (PSL, xmesh, ymesh, 4);
					}
				}
				else {	/* No Contours */

					/* For stability, take the color corresponding to the average value of the four corners */
					z_ave = 0.25 * (Z_vert[0] + Z_vert[1] + Z_vert[2] + Z_vert[3]);

					/* Now paint the polygon piece */

					for (k = 0; k < 4; k++) gmt_geoz_to_xy (GMT, X_vert[k], Y_vert[k], (double)(Topo->data[ij+ij_inc[k]]), &xmesh[k], &ymesh[k]);
					grdview_paint_it (GMT, PSL, P, xmesh, ymesh, 4, z_ave+small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, Ctrl->Q.outline);
				}
			}
		}
		gmt_M_free (GMT, xcont);
		gmt_M_free (GMT, ycont);
		gmt_M_free (GMT, zcont);
		gmt_M_free (GMT, vcont);
	}

	PSL_setdash (PSL, NULL, 0);

	if (GMT->current.proj.z_pars[0] == 0.0) gmt_map_clip_off (GMT);

	if (Ctrl->N.facade) {	/* Cover the two front sides */
		PSL_comment (PSL, "Painting the frontal facade\n");
		gmt_setpen (GMT, &Ctrl->W.pen[2]);
		gmt_setfill (GMT, &Ctrl->N.fill, 1);
		Z = Topo;	/* Ensure we use the topo grid, not the draped grid for this */
		if (!GMT->current.proj.z_project.draw[0])	{	/* Southern side */
			for (col = 0, n = 0, ij = sw; col < (openmp_int)Z->header->n_columns; col++, ij++) {
				if (gmt_M_is_fnan (Topo->data[ij])) continue;
				gmt_geoz_to_xy (GMT, gmt_M_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YLO], (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (col = Z->header->n_columns ; col > 0; col--, n++)
				gmt_geoz_to_xy (GMT, gmt_M_grd_col_to_x (GMT, col-1, Z->header), Z->header->wesn[YLO], Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
		if (!GMT->current.proj.z_project.draw[1]) {	/*	Eastern side */
			for (row = 0, n = 0, ij = ne; row < (openmp_int)Z->header->n_rows; row++, ij += Topo->header->mx) {
				if (gmt_M_is_fnan (Topo->data[ij])) continue;
				gmt_geoz_to_xy (GMT, Z->header->wesn[XHI], gmt_M_grd_row_to_y (GMT, row, Z->header), (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (row = (openmp_int)Z->header->n_rows; row > 0; row--, n++)
				gmt_geoz_to_xy (GMT, Z->header->wesn[XHI], gmt_M_grd_row_to_y (GMT, row-1, Z->header), Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
		if (!GMT->current.proj.z_project.draw[2])	{	/* Northern side */
			for (col = 0, n = 0, ij = nw; col < (openmp_int)Z->header->n_columns; col++, ij++) {
				if (gmt_M_is_fnan (Topo->data[ij])) continue;
				gmt_geoz_to_xy (GMT, gmt_M_grd_col_to_x (GMT, col, Z->header), Z->header->wesn[YHI], (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (col = (openmp_int)Z->header->n_columns; col > 0; col--, n++)
				gmt_geoz_to_xy (GMT, gmt_M_grd_col_to_x (GMT, col-1, Z->header), Z->header->wesn[YHI], Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
		if (!GMT->current.proj.z_project.draw[3]) {	/*	Western side */
			for (row = 0, n = 0, ij = nw; row < (openmp_int)Z->header->n_rows; row++, ij += Topo->header->mx) {
				if (gmt_M_is_fnan (Topo->data[ij])) continue;
				gmt_geoz_to_xy (GMT, Z->header->wesn[XLO], gmt_M_grd_row_to_y (GMT, row, Z->header), (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (row = (openmp_int)Z->header->n_rows; row > 0; row--, n++)
				gmt_geoz_to_xy (GMT, Z->header->wesn[XLO], gmt_M_grd_row_to_y (GMT, row-1, Z->header), Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, (int)n);
		}
	}

	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_map_basemap (GMT);	/* Plot basemap last if not 3-D */
	if (GMT->current.proj.three_D)
		gmt_vertical_axis (GMT, 2);	/* Draw foreground axis */
	gmt_plane_perspective (GMT, -1, 0.0);

	gmt_plotend (GMT);

	/* Free memory */

	if (get_contours) {
		for (ij = 0; ij < Topo->header->nm; ij++) {
			if (!binij[ij].first_cont) continue;
			last_cont = binij[ij].first_cont;
			for (this_cont = binij[ij].first_cont->next_cont; this_cont; this_cont = this_cont->next_cont) {
				if (this_cont->first_point) {
					last_point = this_cont->first_point;
					for (this_point = this_cont->first_point->next_point; this_point; this_point = this_point->next_point) {
						gmt_M_free (GMT, last_point);
						last_point = this_point;
					}
					gmt_M_free (GMT, last_point);
				}
				gmt_M_free (GMT, last_cont);
				last_cont = this_cont;
			}
			gmt_M_free (GMT, last_cont);
		}
		gmt_M_free (GMT, binij);
	}

	gmt_change_grdreg (GMT, Topo->header, t_reg);	/* Reset registration, if required */
	if (use_intensity_grid) {
		gmt_change_grdreg (GMT, Intens->header, i_reg);	/* Reset registration, if required */
		if (saved_data_pointer) {
			gmt_M_free (GMT, Intens->data);
			Intens->data = saved_data_pointer;
		}
	}
	gmt_M_free (GMT, xx);
	gmt_M_free (GMT, yy);
	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	gmt_M_free (GMT, z);
	gmt_M_free (GMT, v);
	if (Ctrl->G.active) for (k = 0; k < Ctrl->G.n; k++) {
		gmt_change_grdreg (GMT, Drape[k]->header, d_reg[k]);	/* Reset registration, if required */
	}
	if (get_contours && GMT_Destroy_Data (API, &Z) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free Z\n");
	}
	if (Ctrl->C.active) {
		if (Ctrl->C.savecpt && GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, 0, NULL, Ctrl->C.savecpt, P) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to save the used CPT in file: %s\n", Ctrl->C.savecpt);
		}
		if (GMT_Destroy_Data (API, &P) != GMT_NOERROR) {Return (API->error);}
	}

	Return (GMT_NOERROR);
}
