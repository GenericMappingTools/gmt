/*--------------------------------------------------------------------
 *	$Id: grdimage_func.c,v 1.61 2011-05-16 08:47:59 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: grdimage will read one (or 3) grid file and image the area
 * with optional shading.
 *
 */

#include "pslib.h"
#include "gmt.h"

/* Control structure for grdimage */

struct GRDIMAGE_CTRL {
	struct In {
		GMT_LONG active;
		GMT_LONG do_rgb;
		char *file[3];
	} In;
	struct C {	/* -C<cptfile> */
		GMT_LONG active;
		char *file;
	} C;
	struct D {	/* -D to read GDAL file */
		GMT_LONG active;
		GMT_LONG mode;	/* Use info of -R option to reference image */
	} D;
	struct A {	/* -A to write a GDAL file */
		GMT_LONG active;
		char *file;
		char *driver;
	} A;
	struct E {	/* -Ei|<dpi> */
		GMT_LONG active;
		GMT_LONG device_dpi;
		GMT_LONG dpi;
	} E;
	struct G {	/* -G[f|b]<rgb> */
		GMT_LONG active;
		double f_rgb[4];
		double b_rgb[4];
	} G;
	struct I {	/* -I<intensfile> */
		GMT_LONG active;
		char *file;
	} I;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
};

void *New_grdimage_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDIMAGE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDIMAGE_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->G.b_rgb[0] = C->G.b_rgb[1] = C->G.b_rgb[2] = 1.0;

	return ((void *)C);
}

void Free_grdimage_Ctrl (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *C) {	/* Deallocate control structure */
	GMT_LONG k;
	if (!C) return;
	for (k = 0; k < 3; k++) if (C->In.file[k]) free ((void *)C->In.file[k]);	
	if (C->C.file) free ((void *)C->C.file);
	if (C->I.file) free ((void *)C->I.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_grdimage_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdimage %s [API] - Plot grid files in 2-D\n\n", GMT_VERSION);
#ifdef USE_GDAL
	GMT_message (GMT, "usage: grdimage <grd_z|grd_r grd_g grd_b> %s [%s] [-A<out_img=driver>] [-C<cpt_file>] [-D[r]] [-Ei|<dpi>] [-G[f|b]<rgb>]\n", 
			GMT_J_OPT, GMT_B_OPT);
#else
	GMT_message (GMT, "usage: grdimage <grd_z|grd_r grd_g grd_b> %s [%s] [-C<cpt_file>] [-Ei|<dpi>] [-G[f|b]<rgb>]\n", GMT_J_OPT, GMT_B_OPT);
#endif
	GMT_message (GMT, "\t[-I<intensity_file>] [-K] [-M] [-N] [-O] [-P] [-Q] [%s] [-T]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n\n", 
			GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_n_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grd_z> is data set to be plotted.  Its z-values are in user units and will be\n");
	GMT_message (GMT, "\t  converted to rgb colors via the cpt file.  Alternatively, give three separate\n");
	GMT_message (GMT, "\t  grid files that contain the red, green, and blue components in the 0-255 range.\n");
#ifdef USE_GDAL
	GMT_message (GMT, "\t  If -D is used then <grd_z> is instead expected to be an image.\n");
#endif
	GMT_explain_options (GMT, "j");
	GMT_message (GMT, "\n\tOPTIONS:\n");
#ifdef USE_GDAL
	GMT_message (GMT, "\t-A Save image in a raster format instead of postscript. Append =<driver> to select.\n");
	GMT_message (GMT, "\t   the image format. The 'driver' is the driver code name used by GDAL. For example\n");
	GMT_message (GMT, "\t   -Aimg.tif=GTiff will write a GeoTiff image. Note: any vector elements are lost. \n");
#endif
	GMT_explain_options (GMT, "b");
	GMT_message (GMT, "\t-C Color palette file to convert z to rgb.\n");
#ifdef USE_GDAL
	GMT_message (GMT, "\t-D Use to read an image via GDAL. Append r to equate image region to -R region.\n");
#endif
	GMT_message (GMT, "\t-E Sets dpi for the projected grid which must be constructed\n");
	GMT_message (GMT, "\t   if -Jx or -Jm is not selected [Default gives same size as input grid].\n");
	GMT_message (GMT, "\t   Give i to do the interpolation in PostScript at device resolution.\n");
	GMT_rgb_syntax (GMT, 'G', "Sets transparency color for images that otherwise would result in 1-bit images\n\t  ");
	GMT_message (GMT, "\t-I Use illumination. Append name of intensity grid file\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-M Force monochrome image.\n");
	GMT_message (GMT, "\t-N Do not clip image at the map boundary.\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q Use PS Level 3 colormasking to make nodes with z = NaN transparent.\n");
	GMT_explain_options (GMT, "R");
	GMT_explain_options (GMT, "UVXcfnpt.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdimage_parse (struct GMTAPI_CTRL *C, struct GRDIMAGE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdimage and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;
#ifdef USE_GDAL
	GMT_LONG n;
#endif

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one or three is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files < 3) Ctrl->In.file[n_files] = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* CPT file */
				Ctrl->C.active = TRUE;
				Ctrl->C.file = strdup (opt->arg);
				break;
#ifdef USE_GDAL
			case 'D':	/* Get via GDAL */
				Ctrl->D.active = TRUE;
				Ctrl->D.mode = (opt->arg[0] == 'r');
				break;
			case 'A':	/* Get image file name plus driver name to write via GDAL */
				Ctrl->A.active = TRUE;
				Ctrl->A.file = strdup (opt->arg);
				n = strlen(Ctrl->A.file) - 1;
				while (Ctrl->A.file[n] != '=' && n > 0) n--;
				if (n == 0) {
					GMT_report (GMT, GMT_MSG_FATAL, "ERROR: missing driver name in option -A.\n");
					n_errors++;
				}
				else {
					Ctrl->A.file[n] = '\0';		/* Strip =driver from file name */
					Ctrl->A.driver = strdup(&Ctrl->A.file[n+1]);
				}
				break;
#endif
			case 'E':	/* Sets dpi */
				Ctrl->E.active = TRUE;
				if (opt->arg[0] == 'i')	/* Interpolate image to device resolution */
					Ctrl->E.device_dpi = TRUE;
				else
					Ctrl->E.dpi = atoi (opt->arg);
				break;
			case 'G':	/* 1-bit fore or background color for transparent masks */
				Ctrl->G.active = TRUE;
				switch (opt->arg[0]) {
					case 'F':
					case 'f':
						if (GMT_getrgb (GMT, &opt->arg[1], Ctrl->G.f_rgb)) {
							GMT_rgb_syntax (GMT, 'G', " ");
							n_errors++;
						}
						else
							Ctrl->G.b_rgb[0] = -1;
						break;
					case 'B':
					case 'b':
						if (GMT_getrgb (GMT, &opt->arg[1], Ctrl->G.b_rgb)) {
							GMT_rgb_syntax (GMT, 'G', " ");
							n_errors++;
						}
						else
							Ctrl->G.f_rgb[0] = -1;
						break;
					default:	/* Same as -Gf */
						if (GMT_getrgb (GMT, opt->arg, Ctrl->G.f_rgb)) {
							GMT_rgb_syntax (GMT, 'G', " ");
							n_errors++;
						}
						else
							Ctrl->G.b_rgb[0] = -1;
						break;
				}
				break;
			case 'I':	/* Use intensity grid */
				Ctrl->I.active = TRUE;
				Ctrl->I.file = strdup (opt->arg);
				break;
			case 'M':	/* Monochrome image */
				Ctrl->M.active = TRUE;
				break;
			case 'N':	/* Do not clip at map boundary */
				Ctrl->N.active = TRUE;
				break;
			case 'Q':	/* PS3 colormasking */
				Ctrl->Q.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (n_files == 3) Ctrl->In.do_rgb = TRUE;
#ifdef USE_GDAL
	if (Ctrl->D.active) {} else
#endif
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, 
					"Syntax error: Must specify a map projection with the -J option\n");
#ifdef USE_GDAL
	n_errors += GMT_check_condition (GMT, !Ctrl->C.file && !Ctrl->In.do_rgb && !Ctrl->D.active, 
					"Syntax error: Must specify color palette table\n");
#else
	n_errors += GMT_check_condition (GMT, !Ctrl->C.file && !Ctrl->In.do_rgb, 
					"Syntax error: Must specify color palette table\n");
#endif
	n_errors += GMT_check_condition (GMT, !(n_files == 1 || n_files == 3), 
					"Syntax error: Must specify one (or three) input file(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->I.file, 
					"Syntax error -I option: Must specify intensity file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && !Ctrl->E.device_dpi && Ctrl->E.dpi <= 0, 
					"Syntax error -E option: dpi must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.f_rgb[0] < 0 && Ctrl->G.b_rgb[0] < 0, 
					"Syntax error -G option: Only one of fore/back-ground can be transparent for 1-bit images\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && Ctrl->Q.active,
					"Syntax error -Q option:  Cannot use -M when doing colormasking\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void GMT_set_proj_limits (struct GMT_CTRL *GMT, struct GRD_HEADER *r, struct GRD_HEADER *g)
{
	/* Sets the projected extent of the grid given the map projection
	 * The extreme x/y coordinates are returned in r, and dx/dy, and
	 * nx/ny are set accordingly.  Not that some of these may change
	 * if GMT_project_init is called at a later stage */

	GMT_LONG i, k, all_lats = FALSE, all_lons = FALSE;
	double x, y;

	r->nx = g->nx;	r->ny = g->ny;
	r->registration = g->registration;
	r->n_bands = g->n_bands;

	/* By default, use entire plot region */

	GMT_memcpy (r->wesn, GMT->current.proj.rect, 4, double);
	
	if (GMT->current.proj.projection == GMT_GENPER && GMT->current.proj.g_width != 0.0) return;

	if (GMT_is_geographic (GMT, GMT_IN)) {
		all_lats = GMT_180_RANGE (g->wesn[YHI], g->wesn[YLO]);
		all_lons = GMT_360_RANGE (g->wesn[XHI], g->wesn[XLO]);
		if (all_lons && all_lats) return;	/* Whole globe */
	}
	
	/* Must search for extent along perimeter */

	r->wesn[XLO] = r->wesn[YLO] = +DBL_MAX;
	r->wesn[XHI] = r->wesn[YHI] = -DBL_MAX;
	k = (g->registration == GMT_GRIDLINE_REG) ? 1 : 0;
	
	for (i = 0; i < g->nx - k; i++) {	/* South and north sides */
		GMT_geo_to_xy (GMT, g->wesn[XLO] + i * g->inc[GMT_X], g->wesn[YLO], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
		GMT_geo_to_xy (GMT, g->wesn[XHI] - i * g->inc[GMT_X], g->wesn[YHI], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
	}
	for (i = 0; i < g->ny - k; i++) {	/* East and west sides */
		GMT_geo_to_xy (GMT, g->wesn[XLO], g->wesn[YHI] - i * g->inc[GMT_Y], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
		GMT_geo_to_xy (GMT, g->wesn[XHI], g->wesn[YLO] + i * g->inc[GMT_Y], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
	}
	if (all_lons) {	/* Full 360, use min/max for x */
		r->wesn[XLO] = GMT->current.proj.rect[XLO];	r->wesn[XHI] = GMT->current.proj.rect[XHI];
	}
	if (all_lats) {	/* Full -90/+90, use min/max for y */
		r->wesn[YLO] = GMT->current.proj.rect[YLO];	r->wesn[YHI] = GMT->current.proj.rect[YHI];
	}
}

#define Return(code) {Free_grdimage_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdimage (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, done, need_to_project, normal_x, normal_y, resampled = FALSE;
	GMT_LONG k, byte, nx = 0, ny = 0, index = 0, grid_registration = GMT_GRIDLINE_REG, n_grids, row, actual_row, col;
	GMT_LONG colormask_offset = 0, nm, node, kk, try, gray_only = FALSE;
	GMT_LONG node_RGBA = 0;		/* Counter for the RGB(A) image array. */
	
	unsigned char *bitimage_8 = NULL, *bitimage_24 = NULL, *rgb_used = NULL, i_rgb[3];

	double dx, dy, x_side, y_side, x0 = 0.0, y0 = 0.0, rgb[4] = {0.0, 0.0, 0.0, 0.0};
	double *NaN_rgb = NULL, red[4] = {1.0, 0.0, 0.0, 0.0}, wesn[4];

	struct GMT_GRID *Grid_orig[3] = {NULL, NULL, NULL}, *Grid_proj[3] = {NULL, NULL, NULL};
	struct GMT_GRID *Intens_orig = NULL, *Intens_proj = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GRDIMAGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;	/* General PSL interal parameters */
	struct GRD_HEADER *header_work = NULL;	/* Pointer to a GMT header for the image or grid */

#ifdef USE_GDAL
	GMT_LONG do_indexed = FALSE;
	double *r_table = NULL, *g_table = NULL, *b_table = NULL;
	struct GMT_IMAGE *I = NULL, *Img_proj = NULL;		/* A GMT image datatype, if GDAL is used */
	struct GMT_GRID *G2 = NULL;
	struct GDALWRITE_CTRL *to_GDALW = NULL;
#endif

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdimage_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdimage_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdimage", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRf", "BKOPUXxYycnpt>" GMT_OPT("S"), options))) Return (error);
	Ctrl = (struct GRDIMAGE_CTRL *) New_grdimage_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdimage_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the grdimage main code ----------------------------*/

	n_grids = (Ctrl->In.do_rgb) ? 3 : 1;

	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);		/* Enables data input and sets access mode */

	/* Read the illumination grid header right away so we can use its region to set that of an image (if requested) */
	if (Ctrl->I.active) {	/* Illumination wanted */

		GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read intensity file\n");

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->I.file), 
			(void **)&Intens_orig)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	}

#ifdef USE_GDAL
	if (Ctrl->D.active) {
		/* One more test though */
		if (Ctrl->D.mode && !GMT->common.R.active) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning: -Dr without -R makes no sense. Ignoring -Dr.\n");
			Ctrl->D.mode = FALSE;
		}

		if (Ctrl->I.active && GMT->common.R.active) {
			if (GMT->common.R.wesn[XLO] < Intens_orig->header->wesn[XLO] || GMT->common.R.wesn[XHI] > Intens_orig->header->wesn[XHI] || 
			    GMT->common.R.wesn[YLO] < Intens_orig->header->wesn[YLO] || GMT->common.R.wesn[YHI] > Intens_orig->header->wesn[YHI]) {
				GMT_report (GMT, GMT_MSG_FATAL, "Requested region exceeds illumination extents\n");
				Return (EXIT_FAILURE);
			}
		}

		if (!Ctrl->D.mode && Ctrl->I.active && !GMT->common.R.active)	/* Apply illumination to an image but no -R provided */
			GMT_memcpy (GMT->common.R.wesn, Intens_orig->header->wesn, 4, double);

		if (GMT_Get_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, 
			(void **)&(Ctrl->In.file[0]), (void **)&I)) Return (GMT_DATA_READ_ERROR);

		if (!Ctrl->D.mode && !Ctrl->I.active && !GMT->common.R.active)	/* No -R or -I. Use image dimensions as -R */
			GMT_memcpy (GMT->common.R.wesn, I->header->wesn, 4, double);

		if ( (Ctrl->D.mode && GMT->common.R.active) || (!Ctrl->D.mode && Ctrl->I.active) ) {
			GMT_memcpy (I->header->wesn, GMT->common.R.wesn, 4, double);
			/* Get actual size of each pixel */
			dx = GMT_get_inc (I->header->wesn[XLO], I->header->wesn[XHI], I->header->nx, I->header->registration);
			dy = GMT_get_inc (I->header->wesn[YLO], I->header->wesn[YHI], I->header->ny, I->header->registration);
			I->header->inc[GMT_X] = dx;	I->header->inc[GMT_Y] = dy;
			I->header->r_inc[GMT_X] = 1.0 / dx;	/* Get inverse increments to avoid divisions later */
			I->header->r_inc[GMT_Y] = 1.0 / dy;
		}

		Ctrl->In.do_rgb = (I->n_bands >= 3);
		n_grids = 0;	/* Flag that we are using a GMT_IMAGE */

		if (I->ProjRefPROJ4 != NULL) GMT_report (GMT, GMT_MSG_NORMAL, "Data projection (Proj4 type)\n\t%s\n", I->ProjRefPROJ4);

		header_work = I->header;	/* OK, that's what what we'll use to send to GMT_grd_setregion */
	}
#endif

	/* Get/calculate a color palette file */

	if (!Ctrl->In.do_rgb) {
		if (Ctrl->C.active) {		/* Read palette file */
			if (GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->C.file, (void **)&P)) 
				Return (GMT_DATA_READ_ERROR);
			gray_only = (P && P->is_gray);
		}
#ifdef USE_GDAL
		else if (Ctrl->D.active) {
			GMT_LONG ID, dim[1] = {256};
			/* We won't use much of the next 'P' but we still need to use some of its fields */
			if ((error = GMT_Create_Data (API, GMT_IS_CPT, dim, (void **)&P, GMT_IN, &ID))) Return (error);
			P->model = GMT_RGB;
			if (I->ColorMap == NULL && !strncmp (I->ColorInterp, "Gray", 4)) {
				r_table = GMT_memory (GMT, NULL, 256, double);
				for (k = 0; k < 256; k++) r_table[k] = GMT_is255 (k);
				gray_only = TRUE;
			}
			else if (I->ColorMap != NULL) {
				r_table = GMT_memory (GMT, NULL, 256, double);
				g_table = GMT_memory (GMT, NULL, 256, double);
				b_table = GMT_memory (GMT, NULL, 256, double);
				for (k = 0; k < 256; k++) {
					r_table[k] = GMT_is255 (I->ColorMap[k*4]);	/* 4 because color table is RGBA */
					g_table[k] = GMT_is255 (I->ColorMap[k*4 + 1]);
					b_table[k] = GMT_is255 (I->ColorMap[k*4 + 2]);
				}
				do_indexed = TRUE;		/* Now it will be RGB */
				gray_only = FALSE;
			}
		}
#endif
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read data file\n");

	if (!Ctrl->D.active) {
		for (k = 0; k < n_grids; k++) {
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file[k]), 
				(void **)&Grid_orig[k])) Return (GMT_DATA_READ_ERROR);	/* Get header only */
		}
	}

	if (n_grids) header_work = Grid_orig[0]->header;	/* OK, we are in GRID mode and this was not set further above. Do it now */

	if (n_grids && Ctrl->In.do_rgb) {	/* Must ensure all three grids are coregistered */
		if (!GMT_grd_same_region (Grid_orig[0], Grid_orig[1])) error++;
		if (!GMT_grd_same_region (Grid_orig[0], Grid_orig[2])) error++;
		if (!(Grid_orig[0]->header->inc[GMT_X] == Grid_orig[1]->header->inc[GMT_X] && Grid_orig[0]->header->inc[GMT_X] == 
			Grid_orig[2]->header->inc[GMT_X])) error++;
		if (!(Grid_orig[0]->header->nx == Grid_orig[1]->header->nx && Grid_orig[0]->header->nx == Grid_orig[2]->header->nx)) error++;
		if (!(Grid_orig[0]->header->ny == Grid_orig[1]->header->ny && Grid_orig[0]->header->ny == Grid_orig[2]->header->ny)) error++;
		if (!(Grid_orig[0]->header->registration == Grid_orig[1]->header->registration && Grid_orig[0]->header->registration == 
			Grid_orig[2]->header->registration)) error++;
		if (error) {
			GMT_report (GMT, GMT_MSG_FATAL, "The r, g, and b grids are not congruent\n");
			Return (EXIT_FAILURE);
		}
	}

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active && n_grids) GMT_memcpy (GMT->common.R.wesn, Grid_orig[0]->header->wesn, 4, double);

	GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "");
	
	/* Determine if grid is to be projected */

	need_to_project = (GMT_IS_NONLINEAR_GRATICULE (GMT) || Ctrl->E.dpi > 0);
	if (need_to_project) GMT_report (GMT, GMT_MSG_DEBUG, "Projected grid is non-orthogonal, nonlinear, or dpi was changed\n");
	
	/* Determine the wesn to be used to read the grid file; or bail if file is outside -R */

	if (!GMT_grd_setregion (GMT, header_work, wesn, need_to_project * GMT->common.n.interpolant)) {
		/* No grid to plot; just do empty map and bail */
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
		GMT_plotinit (API, PSL, options);
		GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT_map_basemap (GMT, PSL);
		GMT_plane_perspective (GMT, PSL, -1, 0.0);
		GMT_plotend (GMT, PSL);
		Return (EXIT_SUCCESS);
	}

	if (n_grids) {
		nx = GMT_get_n (wesn[XLO], wesn[XHI], Grid_orig[0]->header->inc[GMT_X], Grid_orig[0]->header->registration);
		ny = GMT_get_n (wesn[YLO], wesn[YHI], Grid_orig[0]->header->inc[GMT_Y], Grid_orig[0]->header->registration);
	}

#ifdef USE_GDAL
	if (Ctrl->D.active) {	/* Trust more on info from gdal to make it more stable against pixel vs grid registration troubles */
		nx = I->header->nx;
		ny = I->header->ny;
	}
#endif

	if (!Ctrl->A.active) {	/* Otherwise we are not writting any postscript */
		GMT_plotinit (API, PSL, options);
		GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		if (!Ctrl->N.active) GMT_map_clip_on (GMT, PSL,GMT->session.no_rgb, 3);
	}

	/* Read data */

	for (k = 0; k < n_grids; k++) {
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file[k]), 
			(void **)&Grid_orig[k])) Return (GMT_DATA_READ_ERROR);	/* Get grid data */
	}

	/* If given, get intensity file or compute intensities */

	if (Ctrl->I.active) {	/* Illumination wanted */

		GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read intensity file\n");

		/* Remember, the illumination header was already read at the top */
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->I.file), 
			(void **)&Intens_orig)) Return (GMT_DATA_READ_ERROR);	/* Get grid data */
		if (n_grids && (Intens_orig->header->nx != Grid_orig[0]->header->nx || Intens_orig->header->ny != Grid_orig[0]->header->ny)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Intensity file has improper dimensions!\n");
			Return (EXIT_FAILURE);
		}

#ifdef USE_GDAL
		if (Ctrl->D.active && (I->header->nx != Intens_orig->header->nx || I->header->ny != Intens_orig->header->ny)) {
			/* Resize illumination grid to the image's size */

			GMT_LONG object_ID, status = 0;			/* Status code from GMT API */
			char in_string[GMTAPI_STRLEN], out_string[GMTAPI_STRLEN], cmd[GMT_BUFSIZ];
			/* Create option list, register G as input source via reference */
			if (GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_IN, (void **)&Intens_orig, NULL, (void *)Intens_orig, &object_ID)) 
				return (EXIT_FAILURE);
			GMT_Encode_ID (API, in_string, object_ID);	/* Make filename with embedded object ID for grid G */

			if (GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_OUT, (void **)&G2, NULL, (void *)G2, &object_ID)) 
				return (EXIT_FAILURE);
			GMT_Encode_ID (GMT->parent, out_string, object_ID);	/* Make filename with embedded object ID for result grid G2 */

			sprintf (cmd, "%s -G%s -I%ld+/%ld+", in_string, out_string, nx, ny);
			status = GMT_grdsample_cmd (GMT->parent, 0, (void *)cmd);	/* Do the resampling */
			Intens_orig->alloc_mode = GMT_ALLOCATED;	/* So we may destroy it */
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Intens_orig);
			Intens_orig = G2;
			Intens_orig->alloc_mode = GMT_ALLOCATED;	/* So we may destroy at the end */
		}
#endif

	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (need_to_project) {	/* Need to resample the grd file */
		GMT_LONG nx_proj = 0, ny_proj = 0;
		double inc[2] = {0.0, 0.0};
		GMT_report (GMT, GMT_MSG_NORMAL, "project grid files\n");

		if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
			nx_proj = nx;
			ny_proj = ny;
		}
#ifdef USE_GDAL
		if (Ctrl->D.active) { 
			Img_proj = GMT_create_image (GMT);
			grid_registration = GMT_PIXEL_REG;	/* Force pixel */
			GMT_set_proj_limits (GMT, Img_proj->header, I->header);
			GMT_err_fail (GMT, GMT_project_init (GMT, Img_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration), Ctrl->In.file[0]);
			if (Ctrl->A.active)
				for (k = 0; k < 3; k++) GMT->current.setting.color_patch[GMT_NAN][k] = 1.0;	/* For img GDAL write use white as bg color */
			Img_proj->data = GMT_memory (GMT, NULL, Img_proj->header->size * Img_proj->header->n_bands, unsigned char);
			GMT_img_project (GMT, I, Img_proj, FALSE);
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&I);
		}
#endif
		for (k = 0; k < n_grids; k++) {
			if (!Grid_proj[k]) Grid_proj[k] = GMT_create_grid (GMT);
			GMT_set_proj_limits (GMT, Grid_proj[k]->header, Grid_orig[k]->header);
			if (grid_registration == GMT_GRIDLINE_REG)		/* Force pixel if dpi is set */
				grid_registration = (Ctrl->E.dpi > 0) ? GMT_PIXEL_REG : Grid_orig[k]->header->registration;
			GMT_err_fail (GMT, GMT_project_init (GMT, Grid_proj[k]->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration), Ctrl->In.file[k]);
			Grid_proj[k]->data = GMT_memory (GMT, NULL, Grid_proj[k]->header->size, float);
			GMT_grd_project (GMT, Grid_orig[k], Grid_proj[k], FALSE);
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid_orig[k]);
		}
		if (Ctrl->I.active) {
			Intens_proj = GMT_create_grid (GMT);

			if (n_grids)
				GMT_memcpy (Intens_proj->header->wesn, Grid_proj[0]->header->wesn, 4, double);
#ifdef USE_GDAL
			else
				GMT_memcpy (Intens_proj->header->wesn, Img_proj->header->wesn, 4, double);
#endif
			if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
				nx_proj = Intens_orig->header->nx;
				ny_proj = Intens_orig->header->ny;
			}
			GMT_err_fail (GMT, GMT_project_init (GMT, Intens_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration), Ctrl->I.file);
			Intens_proj->data = GMT_memory (GMT, NULL, Intens_proj->header->size, float);
			GMT_grd_project (GMT, Intens_orig, Intens_proj, FALSE);
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Intens_orig);
		}
		resampled = TRUE;
	}
	else {	/* Simply set Grid_proj[i]/Intens_proj to point to Grid_orig[i]/Intens_orig */
		struct GRD_HEADER tmp_header;
		for (k = 0; k < n_grids; k++) {	/* Must get a copy so we can change one without affecting the other */
			GMT_memcpy (&tmp_header, Grid_orig[k]->header, 1, struct GRD_HEADER);
			Grid_proj[k] = Grid_orig[k];
			GMT_set_proj_limits (GMT, Grid_proj[k]->header, &tmp_header);
		}
		if (Ctrl->I.active) Intens_proj = Intens_orig;
		if (n_grids)
			grid_registration = Grid_orig[0]->header->registration;
#ifdef USE_GDAL
		else {
			GMT_memcpy (&tmp_header, I->header, 1, struct GRD_HEADER);
			Img_proj = I;
			GMT_set_proj_limits (GMT, Img_proj->header, &tmp_header);
		}
#endif
	}

	if (n_grids) {
		Grid_proj[0]->header->n_bands = 1;
		header_work = Grid_proj[0]->header;	/* Later when need to refer to the header, use this copy */
	}
#ifdef USE_GDAL
	if (Ctrl->D.active)
		header_work = Img_proj->header;	/* Later when need to refer to the header, use this copy */
#endif

	nm = header_work->nm;
	nx = header_work->nx;
	ny = header_work->ny;

	if (P && P->has_pattern) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Patterns in cpt file only apply to -T\n");
	GMT_report (GMT, GMT_MSG_NORMAL, "Evaluate pixel colors\n");

	NaN_rgb = (P) ? P->patch[GMT_NAN].rgb : GMT->current.setting.color_patch[GMT_NAN];
	if (Ctrl->Q.active) {
		if (gray_only) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Your image is grayscale only but -Q requires 24-bit; image will be converted to 24-bit.\n");
			gray_only = FALSE;
			NaN_rgb = red;	/* Arbitrarily pick red as the NaN color since image is gray only */
			GMT_memcpy (P->patch[GMT_NAN].rgb, red, 4, double);
		}
		rgb_used = GMT_memory (GMT, NULL, 256*256*256, unsigned char);
	}
	if (Ctrl->M.active || gray_only)
		bitimage_8 = GMT_memory (GMT, NULL, nm, unsigned char);
	else {
		if (Ctrl->Q.active) colormask_offset = 3;
		bitimage_24 = GMT_memory (GMT, NULL, 3 * nm + colormask_offset, unsigned char);
		if (P && Ctrl->Q.active) {
			for (k = 0; k < 3; k++) bitimage_24[k] = GMT_u255 (P->patch[GMT_NAN].rgb[k]);
		}
	}
	normal_x = !(GMT->current.proj.projection == GMT_LINEAR && !GMT->current.proj.xyz_pos[0] && !resampled);
	normal_y = !(GMT->current.proj.projection == GMT_LINEAR && !GMT->current.proj.xyz_pos[1] && !resampled);

	for (try = 0, done = FALSE; !done && try < 2; try++) {	/* Evaluate colors at least once, or twice if -Q and we need to select another NaN color */
		for (row = 0, byte = colormask_offset; row < ny; row++) {
			actual_row = (normal_y) ? row : ny - row - 1;
			kk = GMT_IJPGI (header_work, actual_row, 0);
			if (Ctrl->D.active && row == 0) node_RGBA = kk;		/* First time per row equals 'node', after grows alone */
			for (col = 0; col < nx; col++) {	/* Compute rgb for each pixel */
				node = kk + (normal_x ? col : nx - col - 1);
#ifdef USE_GDAL
				if (Ctrl->D.active) {
					if (!Ctrl->In.do_rgb) {
						rgb[0] = r_table[(int)Img_proj->data[node]];
						if (do_indexed) {
							rgb[1] = g_table[(int)Img_proj->data[node]];
							rgb[2] = b_table[(int)Img_proj->data[node]];
						}
					}
					else {
						for (k = 0; k < 3; k++) rgb[k] = (double)Img_proj->data[node_RGBA++]/255;
						if (Img_proj->header->n_bands == 4) node_RGBA++;
					}
				}
				else
#endif
				if (Ctrl->In.do_rgb) {
					for (k = 0; k < 3; k++) {
						if (GMT_is_fnan (Grid_proj[k]->data[node])) {	/* If one is NaN they are all assumed to be NaN */
							k = 3;	/* To exit the k-loop */
							GMT_rgb_copy (rgb, NaN_rgb);
							index = GMT_NAN - 3;	/* Ensures no illumination done later */
						}
						else {				/* Set color, let index = 0 so illuminate test will work */
							rgb[k] = GMT_is255 (Grid_proj[k]->data[node]);
							if (rgb[k] < 0.0) rgb[k] = 0.0; else if (rgb[k] > 1.0) rgb[k] = 1.0;	/* Clip */
							index = 0;
						}
					}
				}
				else
					index = GMT_get_rgb_from_z (GMT, P, Grid_proj[0]->data[node], rgb);

				if (Ctrl->I.active && index != GMT_NAN - 3) {
					if (!n_grids) {		/* Here we are illuminating an image. Must recompute "node" with the GMT_IJP macro */
						node = GMT_IJP (Intens_proj->header, actual_row, 0) + (normal_x ? col : nx - col - 1);
					}
					GMT_illuminate (GMT, Intens_proj->data[node], rgb);
				}
				
				if (P && gray_only)		/* Color table only has grays, pick r */
					bitimage_8[byte++] = GMT_u255 (rgb[0]);
				else if (Ctrl->M.active)	/* Convert rgb to gray using the GMT_YIQ transformation */
					bitimage_8[byte++] = GMT_u255 (GMT_YIQ (rgb));
				else {
					for (k = 0; k < 3; k++) bitimage_24[byte++] = i_rgb[k] = GMT_u255 (rgb[k]);
					if (Ctrl->Q.active && index != GMT_NAN - 3) /* Keep track of all r/g/b combinations used except for NaN */
						rgb_used[(i_rgb[0]*256 + i_rgb[1])*256+i_rgb[2]] = TRUE;
				}
			}

			if (!n_grids) node_RGBA += header_work->n_bands * (header_work->pad[XLO] + header_work->pad[XHI]);
		}

		if (P && Ctrl->Q.active) {	/* Check that we found an unused r/g/b value so colormasking will work OK */
			index = (GMT_u255(P->patch[GMT_NAN].rgb[0])*256 + GMT_u255(P->patch[GMT_NAN].rgb[1]))*256 + GMT_u255(P->patch[GMT_NAN].rgb[2]);
			if (rgb_used[index]) {	/* This r/g/b already appears in the image as a non-NaN color; we must find a replacement NaN color */
				for (index = 0, k = -1; k == -1 && index < 256*256*256; index++) if (!rgb_used[index]) k = index;
				if (k == -1) {
					GMT_report (GMT, GMT_MSG_FATAL, "Warning: Colormasking will fail as there is no unused color that can represent transparency\n");
					done = TRUE;
				}
				else {	/* Pick the first unused color (i.e., k) and let it play the role of the NaN color for transparency */
					bitimage_24[0] = (unsigned char)(k >> 16);
					bitimage_24[1] = (unsigned char)((k >> 8) & 255);
					bitimage_24[2] = (unsigned char)(k & 255);
					GMT_report (GMT, GMT_MSG_NORMAL, "Warning: transparency color reset from %s to color %d/%d/%d\n", 
						GMT_putrgb (GMT, P->patch[GMT_NAN].rgb), (int)bitimage_24[0], (int)bitimage_24[1], (int)bitimage_24[2]);
					for (k = 0; k < 3; k++) P->patch[GMT_NAN].rgb[k] = GMT_is255 (bitimage_24[k]);	/* Set new NaN color */
				}	
			}
		}
		else
			done = TRUE;
	}
	if (Ctrl->Q.active) GMT_free (GMT, rgb_used);
	
	for (k = 1; k < n_grids; k++) {	/* Not done with Grid_proj[0] yet, hence we start loop at k = 1 */
		if (need_to_project) /* Must remove locally created grids */
			GMT_free_grid (GMT, &Grid_proj[k], TRUE);
		else
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid_proj[k]);
	}
	if (Ctrl->I.active) {
		if (need_to_project || !n_grids) /* Must remove locally created grids */
			GMT_free_grid (GMT, &Intens_proj, TRUE);
		else
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Intens_proj);
	}
	
	/* Get actual size of each pixel */
	dx = GMT_get_inc (header_work->wesn[XLO], header_work->wesn[XHI], header_work->nx, header_work->registration);
	dy = GMT_get_inc (header_work->wesn[YLO], header_work->wesn[YHI], header_work->ny, header_work->registration);

#ifdef USE_GDAL
	if (Ctrl->A.active) {
		int	id, k;
		to_GDALW = GMT_memory (GMT, NULL, 1, struct GDALWRITE_CTRL);
		to_GDALW->driver = Ctrl->A.driver;
		to_GDALW->type = strdup("byte");
		to_GDALW->P.ProjectionRefPROJ4 = NULL;
		to_GDALW->flipud = 0;
		to_GDALW->geog = 0;
		to_GDALW->nx = (int)nx;
		to_GDALW->ny = (int)ny;
		to_GDALW->n_bands = (int)MIN(Img_proj->header->n_bands, 3);	/* Transparency not accounted yet */
		to_GDALW->registration = 1;
		if (!need_to_project) {
			to_GDALW->ULx = GMT->common.R.wesn[XHI];
			to_GDALW->ULy = GMT->common.R.wesn[YHI];
			to_GDALW->x_inc = dx;
			to_GDALW->y_inc = dy;
		}
		else {
			to_GDALW->ULx = GMT->current.proj.rect_m[XLO];
			to_GDALW->ULy = GMT->current.proj.rect_m[YHI];
			to_GDALW->x_inc = (GMT->current.proj.rect_m[XHI] - GMT->current.proj.rect_m[XLO]) / nx;
			to_GDALW->y_inc = (GMT->current.proj.rect_m[YHI] - GMT->current.proj.rect_m[YLO]) / ny;
		}

		for (k = 0, id = -1; id == -1 && k < GMT_N_PROJ4; k++) 
			if (GMT->current.proj.proj4[k].id == GMT->current.proj.projection) id = k;
		if (id >= 0) {			/* Valid projection for creating world file info */
			char *pstr = NULL;
			to_GDALW->P.active = TRUE;
			to_GDALW->P.ProjectionRefPROJ4 = GMT_export2proj4 (GMT, pstr);
		}
	}
#endif

	/* Set lower left position of image on map */

	x0 = header_work->wesn[XLO];	y0 = header_work->wesn[YLO];
	if (grid_registration == GMT_GRIDLINE_REG) {	/* Grid registration, move 1/2 pixel down/left */
		x0 -= 0.5 * dx;
		y0 -= 0.5 * dy;
	}

	x_side = dx * header_work->nx;
	y_side = dy * header_work->ny;

	GMT_report (GMT, GMT_MSG_NORMAL, "Creating image ");

	if (P && gray_only) 
		for (kk = 0, P->is_bw = TRUE; P->is_bw && kk < nm; kk++) 
			if (!(bitimage_8[kk] == 0 || bitimage_8[kk] == 255)) P->is_bw = FALSE;

	if (P && P->is_bw) {	/* Can get away with 1 bit image */
		GMT_LONG nx8, shift, b_or_w, nx_pixels, k8;
		unsigned char *bit = NULL;

		GMT_report (GMT, GMT_MSG_NORMAL, "[1-bit B/W image]\n");

		nx8 = (GMT_LONG)ceil (nx / 8.0);	/* Image width must equal a multiple of 8 bits */
		nx_pixels = nx8 * 8;
		bit = GMT_memory (GMT, NULL, nx8 * ny, unsigned char);

		for (row = k = k8 = 0; row < ny; row++) {
			shift = byte = 0;
			for (col = 0; col < nx; col++, k++) {
				b_or_w = (bitimage_8[k] == 255);
				byte |= b_or_w;
				shift++;
				if (shift == 8) {	/* Time to dump out byte */
					bit[k8++] = (unsigned char) byte;
					byte = shift = 0;
				}
				else
					byte <<= 1;	/* Move the bits we have so far 1 step to the left */
			}
			if (shift) {	/* Set the remaining bits in this bit to white */
				byte |= 1;
				shift++;
				while (shift < 8) {
					byte <<= 1;
					byte |= 1;
					shift++;
				}
				bit[k8++] = (unsigned char) byte;
			}
		}
		GMT_free (GMT, bitimage_8);

		x_side = nx_pixels * dx;
		PSL_plotbitimage (PSL, x0, y0, x_side, y_side, PSL_BL, bit, nx_pixels, ny, Ctrl->G.f_rgb, Ctrl->G.b_rgb);
		GMT_free (GMT, bit);
	}
	else if ((P && gray_only) || Ctrl->M.active) {
		GMT_report (GMT, GMT_MSG_NORMAL, "[8-bit grayshade image]\n");
#ifdef USE_GDAL
		if (Ctrl->A.active) {
			to_GDALW->data = (void *)bitimage_8;
			GMT_gdalwrite(GMT, Ctrl->A.file, to_GDALW);
		}
		else
#endif
		PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_8, nx, ny, (Ctrl->E.device_dpi ? -8 : 8));
		GMT_free (GMT, bitimage_8);
	}
	else {
#ifdef USE_GDAL
		GMT_report (GMT, GMT_MSG_NORMAL, "via GDAL\n");
		if (Ctrl->A.active) {
			to_GDALW->data = (void *)bitimage_24;
			GMT_gdalwrite(GMT, Ctrl->A.file, to_GDALW);
		}
		else
#endif
		PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_24, (Ctrl->Q.active ? -1 : 1) * 
				    nx, ny, (Ctrl->E.device_dpi ? -24 : 24));

		GMT_free (GMT, bitimage_24);
	}

	if (!Ctrl->A.active) {
		if (!Ctrl->N.active) GMT_map_clip_off (GMT, PSL);

		GMT_map_basemap (GMT, PSL);
		GMT_plane_perspective (GMT, PSL, -1, 0.0);
		GMT_plotend (GMT, PSL);
	}

	if (need_to_project && n_grids)
		GMT_free_grid (GMT, &Grid_proj[0], TRUE);
	else if (n_grids)
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid_proj[0]);

#ifdef USE_GDAL
	if (Ctrl->D.active) {
		if (r_table) GMT_free (GMT, r_table);
		if (g_table) {
			GMT_free (GMT, g_table);
			GMT_free (GMT, b_table);
		}
		if (need_to_project)
			GMT_free_image (GMT, &Img_proj, TRUE);
		else
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Img_proj);
	}
	if (Ctrl->A.active) {
		if (to_GDALW->P.ProjectionRefPROJ4) free (to_GDALW->P.ProjectionRefPROJ4);
		GMT_free (GMT, to_GDALW);
		free((void *)Ctrl->A.driver);
		free((void *)Ctrl->A.file);
	}
#endif

	Return (EXIT_SUCCESS);
}
