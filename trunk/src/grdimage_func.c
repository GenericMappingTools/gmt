/*--------------------------------------------------------------------
 *	$Id: grdimage_func.c,v 1.11 2011-04-11 21:15:31 remko Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
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
#ifdef USE_GDAL
	struct D {	/* -D to read gdal file */
		GMT_LONG active;
		GMT_LONG mode;	/* Use info of -R option to reference image */
	} D;
#endif
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
	struct S {	/* -S[-]b|c|l|n[/<threshold>] */
		GMT_LONG active;
		GMT_LONG antialias;
		GMT_LONG interpolant;
		double threshold;
	} S;
};

void *New_grdimage_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDIMAGE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDIMAGE_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->G.b_rgb[0] = C->G.b_rgb[1] = C->G.b_rgb[2] = 1.0;
	C->S.antialias = TRUE; C->S.interpolant = BCR_BICUBIC; C->S.threshold = 0.5;

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
	GMT_message (GMT, "usage: grdimage <grd_z|grd_r grd_g grd_b> %s [%s] [-C<cpt_file>] [-D[r]] [-Ei|<dpi>] [-G[f|b]<rgb>]\n", GMT_J_OPT, GMT_B_OPT);
#else
	GMT_message (GMT, "usage: grdimage <grd_z|grd_r grd_g grd_b> %s [%s] [-C<cpt_file>] [-Ei|<dpi>] [-G[f|b]<rgb>]\n", GMT_J_OPT, GMT_B_OPT);
#endif
	GMT_message (GMT, "\t[-I<intensity_file>] [-K] [-M] [-N] [-O] [-P] [-Q] [%s] [-S[-]b|c|l|n[/<threshold>]] [-T]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s] [%s] [%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grd_z> is data set to be plotted.  Its z-values are in user units and will be\n");
	GMT_message (GMT, "\t  converted to rgb colors via the cpt file.  Alternatively, give three separate\n");
	GMT_message (GMT, "\t  grid files that contain the red, green, and blue components in the 0-255 range.\n");
#ifdef USE_GDAL
	GMT_message (GMT, "\t  If -D is used then <grd_z> is instead expected to be an image.\n");
#endif
	GMT_explain_options (GMT, "j");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "b");
	GMT_message (GMT, "\t-C Color palette file to convert z to rgb\n");
#ifdef USE_GDAL
	GMT_message (GMT, "\t-D Use to read an image via GDAL.  Append r to equate image region to -R region.\n");
#endif
	GMT_message (GMT, "\t-E Sets dpi for the projected grid which must be constructed\n");
	GMT_message (GMT, "\t   if -Jx or -Jm is not selected [Default gives same size as input grid]\n");
	GMT_message (GMT, "\t   Give i to do the interpolation in PostScript at device resolution.\n");
	GMT_rgb_syntax (GMT, 'G', "Sets transparency color for images that otherwise would result in 1-bit images\n\t  ");
	GMT_message (GMT, "\t-I Use illumination.  Append name of intensity grid file\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-M Force monochrome image\n");
	GMT_message (GMT, "\t-N Do not clip image at the map boundary\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q Use PS Level 3 colormasking to make nodes with z = NaN transparent.\n");
	GMT_explain_options (GMT, "R");
	GMT_sample_syntax (GMT, 'S', "Determines the grid interpolation mode.");
	GMT_explain_options (GMT, "UVXcfpt.");

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

	GMT_LONG n_errors = 0, n_files = 0, j;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

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
			case 'S':	/* Interpolation mode */
				Ctrl->S.active = TRUE;
				for (j = 0; j < 3 && opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case '-':
							Ctrl->S.antialias = FALSE; break;
						case 'n':
							Ctrl->S.interpolant = BCR_NEARNEIGHBOR; break;
						case 'l':
							Ctrl->S.interpolant = BCR_BILINEAR; break;
						case 'b':
							Ctrl->S.interpolant = BCR_BSPLINE; break;
						case 'c':
							Ctrl->S.interpolant = BCR_BICUBIC; break;
						case '/':
							Ctrl->S.threshold = atof (&opt->arg[j+1]);
							j = 3; break;
						default:
							GMT_report (GMT, GMT_MSG_FATAL, "Warning: The -S option has changed meaning. Use -S[-]b|c|l|n[/threshold] to specify interpolation mode.\n");
							j = 3; break;
					}
				}
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
					"Syntax error:  Must specify a map projection with the -J option\n");
#ifdef USE_GDAL
	n_errors += GMT_check_condition (GMT, !Ctrl->C.file && !Ctrl->In.do_rgb && !Ctrl->D.active, 
					"Syntax error:  Must specify color palette table\n");
#else
	n_errors += GMT_check_condition (GMT, !Ctrl->C.file && !Ctrl->In.do_rgb, 
					"Syntax error:  Must specify color palette table\n");
#endif
	n_errors += GMT_check_condition (GMT, !(n_files == 1 || n_files == 3), 
					"Syntax error:  Must specify one (or three) input file(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->I.file, 
					"Syntax error -I option:  Must specify intensity file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && !Ctrl->E.device_dpi && Ctrl->E.dpi <= 0, 
					"Syntax error -E option:  dpi must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.f_rgb[0] < 0 && Ctrl->G.b_rgb[0] < 0, 
					"Syntax error -G option:  Only one of fore/back-ground can be transparent for 1-bit images\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.threshold < 0.0 || Ctrl->S.threshold > 1.0), 
					"Syntax error -S option:  threshold must be in [0,1] range\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void GMT_set_proj_limits (struct GMT_CTRL *GMT, struct GRD_HEADER *r, struct GRD_HEADER *g)
{
	/* Sets the projected extent of the grid given the map projection
	 * The extreme x/y coordinates are returned in r, and dx/dy, and
	 * nx/ny are set accordingly.  Not that some of these may change
	 * if GMT_grdproject_init is called at a later stage */

	GMT_LONG row, col, all_lats = FALSE, all_lons = FALSE;
	double lon, lat, x, y;

	r->nx = g->nx;	r->ny = g->ny;
	r->registration = g->registration;

	if (GMT->current.proj.projection == GMT_GENPER && GMT->current.proj.g_width != 0.0) {
		GMT_memcpy (r->wesn, GMT->current.proj.rect, 4, double);
		return;
	}

	if (GMT_is_geographic (GMT, GMT_IN)) {
		all_lats = GMT_180_RANGE (g->wesn[YHI], g->wesn[YLO]);
		all_lons = GMT_360_RANGE (g->wesn[XHI], g->wesn[XLO]);
		if (all_lons && all_lats) {	/* Whole globe, get rectangular box */
			GMT_memcpy (r->wesn, GMT->current.proj.rect, 4, double);
			return;
		}
	}
	
	/* Must search for extent along perimeter */

	r->wesn[XLO] = r->wesn[YLO] = +DBL_MAX;
	r->wesn[XHI] = r->wesn[YHI] = -DBL_MAX;

	for (col = 0; col < g->nx; col++) {	/* South and north sides */
		lon = GMT_grd_col_to_x (col, g);
		GMT_geo_to_xy (GMT, lon, g->wesn[YLO], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x);	r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y);	r->wesn[YHI] = MAX (r->wesn[YHI], y);
		GMT_geo_to_xy (GMT, lon, g->wesn[YHI], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x);	r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y);	r->wesn[YHI] = MAX (r->wesn[YHI], y);
	}
	for (row = 0; row < g->ny; row++) {	/* East and west sides */
		lat = GMT_grd_row_to_y (row, g);
		GMT_geo_to_xy (GMT, g->wesn[XLO], lat, &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x);	r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y);	r->wesn[YHI] = MAX (r->wesn[YHI], y);
		GMT_geo_to_xy (GMT, g->wesn[XHI], lat, &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x);	r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y);	r->wesn[YHI] = MAX (r->wesn[YHI], y);
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
	GMT_LONG k, byte, nx, ny, index = 0, grid_registration = 0, n_grids, row, actual_row, col;
	GMT_LONG colormask_offset = 0, nm, node, kk, try;
	
	unsigned char *bitimage_8 = NULL, *bitimage_24 = NULL, *rgb_used = NULL, i_rgb[3];

	double dx, dy, x_side, y_side, x0 = 0.0, y0 = 0.0, rgb[4] = {0.0, 0.0, 0.0, 0.0}, *NaN_rgb = NULL, wesn[4];

	struct GMT_GRID *Grid_orig[3] = {NULL, NULL, NULL}, *Grid_proj[3] = {NULL, NULL, NULL};
	struct GMT_GRID *Intens_orig = NULL, *Intens_proj = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_EDGEINFO edgeinfo;
	struct GRDIMAGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;	/* General PSL interal parameters */

#ifdef USE_GDAL
	GMT_LONG do_indexed = FALSE, j, ij;
	double *r_table = NULL, *g_table = NULL, *b_table = NULL;
	struct GDALREAD_CTRL *to_gdalread = NULL;
	struct GD_CTRL *from_gdalread = NULL;
#endif

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdimage_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdimage_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdimage", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRf", "BKOPUXxYycpt>", options))) Return (error);
	Ctrl = (struct GRDIMAGE_CTRL *) New_grdimage_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdimage_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the grdimage main code ----------------------------*/

	n_grids = (Ctrl->In.do_rgb) ? 3 : 1;

	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);		/* Enables data input and sets access mode */

#ifdef USE_GDAL
	if (Ctrl->D.active) {
		/* One more test though */
		if (Ctrl->I.active) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error cannot use -D and -I options.\n");
			Return (EXIT_FAILURE);
		}
		if (Ctrl->D.mode && !GMT->common.R.active) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning: -Dr without -R makes no sense. Ignoring -Dr.\n");
			Ctrl->D.mode = FALSE;
		}

		/* Just a testing line to import images via GDAL. Data and header are all in the I(mage) container
		if (GMT_Get_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, 
			(void **)&(Ctrl->In.file[0]), (void **)&I)) Return (GMT_DATA_READ_ERROR);
		*/

		/* Allocate new control structures */
		to_gdalread = GMT_memory (GMT, NULL, 1, struct GDALREAD_CTRL);
		from_gdalread = GMT_memory (GMT, NULL, 1, struct GD_CTRL);
		to_gdalread->F.active = 1;

		if (GMT->common.R.active && !Ctrl->D.mode) {
			char strR [128]; 
			sprintf (strR, "-R%.10f/%.10f/%.10f/%.10f", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI],
								    GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
			to_gdalread->R.region = strR;
		}

		to_gdalread->p.active = to_gdalread->p.pad = 0;

		j = (int)strlen(Ctrl->In.file[0]) - 1;
		while (j && Ctrl->In.file[0][j] && Ctrl->In.file[0][j] != '+') j--;	/* See if we have a band request */
		if (j && Ctrl->In.file[0][j+1] == 'b') {
			Ctrl->In.file[0][j] = '\0';			/* Strip the band request string and put in the -B option */
			to_gdalread->B.active = 1;
			to_gdalread->B.bands = strdup(&Ctrl->In.file[0][j+2]);	/* Band parsing and error testing is done in gmt_gdalread */

		}

		if (GMT_gdalread (GMT, Ctrl->In.file[0], to_gdalread, from_gdalread)) {
			GMT_report (GMT, GMT_MSG_FATAL, "ERROR reading file with gdalread.\n");
			Return (EXIT_FAILURE);
		}

		if (!from_gdalread->UInt8.active) {
			GMT_report (GMT, GMT_MSG_FATAL, "Using data type other than byte (unsigned char) is not implemented\n");
			Return (EXIT_FAILURE);
		}

		Ctrl->In.do_rgb = (from_gdalread->RasterCount >= 3);
		if (Ctrl->In.do_rgb) n_grids = 3;	/* To be compatible with original algorithm */

		if (from_gdalread->ProjectionRefPROJ4 != NULL)
			GMT_report (GMT, GMT_MSG_NORMAL, "Data projection (Proj4 type)\n\t%s\n", from_gdalread->ProjectionRefPROJ4);

		if (!Ctrl->D.mode) {
			for (k = 0; k < 3; k++) {
				Grid_orig[k] = GMT_create_grid (GMT); 
				Grid_proj[k] = GMT_create_grid (GMT); 
				GMT_memcpy (Grid_orig[k]->header->wesn, from_gdalread->hdr, 4, double);
				Grid_orig[k]->header->inc[GMT_X] = from_gdalread->hdr[7];
				Grid_orig[k]->header->inc[GMT_Y] = from_gdalread->hdr[8];
			}
		}
		else {
			for (k = 0; k < 3; k++) {
				Grid_orig[k] = GMT_create_grid (GMT); 
				Grid_proj[k] = GMT_create_grid (GMT); 
				GMT_memcpy (Grid_orig[k]->header->wesn, GMT->common.R.wesn, 4, double);
				Grid_orig[k]->header->inc[GMT_X] = (Grid_orig[0]->header->wesn[XHI] - Grid_orig[0]->header->wesn[XLO]) /
								    from_gdalread->RasterXsize;
				Grid_orig[k]->header->inc[GMT_Y] = (Grid_orig[0]->header->wesn[YHI] - Grid_orig[0]->header->wesn[YLO]) /
								    from_gdalread->RasterYsize;
			}
		}
		for (k = 0; k < 3; k++) {
			Grid_orig[k]->header->nx = from_gdalread->RasterXsize;
			Grid_orig[k]->header->ny = from_gdalread->RasterYsize;
			Grid_orig[k]->header->registration = (int)from_gdalread->hdr[6];
			GMT_set_grddim (GMT, Grid_orig[k]->header);		/* Update all dimensions (nm, size, etc.) */
		}
	}
#endif

	/* Get/calculate a color palette file */

	if (!Ctrl->In.do_rgb) {
		if (Ctrl->C.active) {		/* Read palette file */
			if (GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->C.file, (void **)&P)) 
				Return (GMT_DATA_READ_ERROR);
		}
#ifdef USE_GDAL
		else if (Ctrl->D.active) {
			if (from_gdalread->ColorMap == NULL && !strncmp (from_gdalread->ColorInterp, "Gray", 4)) {
				r_table = GMT_memory (GMT, NULL, 256, double);
				for (k = 0; k < 256; k++) r_table[k] = GMT_is255 (k);
				P->is_gray = TRUE;
			}
			else if (from_gdalread->ColorMap != NULL) {
				r_table = GMT_memory (GMT, NULL, 256, double);
				g_table = GMT_memory (GMT, NULL, 256, double);
				b_table = GMT_memory (GMT, NULL, 256, double);
				for (k = 0; k < 256; k++) {
					r_table[k] = GMT_is255 (from_gdalread->ColorMap[k*4]);	/* 4 because color table is RGBA */
					g_table[k] = GMT_is255 (from_gdalread->ColorMap[k*4 + 1]);
					b_table[k] = GMT_is255 (from_gdalread->ColorMap[k*4 + 2]);
				}
				do_indexed = TRUE;		/* Now it will be RGB */
				P->is_gray = FALSE;
			}
		}
#endif
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read data file\n");

#ifdef USE_GDAL
	if (!Ctrl->D.active) {
#endif
		for (k = 0; k < n_grids; k++) {
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file[k]), 
				(void **)&Grid_orig[k])) Return (GMT_DATA_READ_ERROR);	/* Get header only */
		}
#ifdef USE_GDAL
	}
#endif
	if (Ctrl->In.do_rgb) {	/* Must ensure all three grids are coregistered */
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

	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, Grid_orig[0]->header->wesn, 4, double);

	GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "");
	
	/* Determine if grid is to be projected */

	need_to_project = (GMT_IS_NONLINEAR_GRATICULE (GMT) || Ctrl->E.dpi > 0);
	if (need_to_project) GMT_report (GMT, GMT_MSG_DEBUG, "Projected grid is non-orthogonal, nonlinear, or dpi was changed\n");
	
	/* Determine the wesn to be used to read the grid file; or bail if file is outside -R */

	if (GMT_grd_setregion (GMT, Grid_orig[0]->header, wesn, need_to_project * Ctrl->S.interpolant)) {
		/* No grid to plot; just do empty map and bail */
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */
		GMT_plotinit (API, PSL, options);
		GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT_map_basemap (GMT, PSL);
		GMT_plane_perspective (GMT, PSL, -1, 0.0);
		GMT_plotend (GMT, PSL);
		Return (EXIT_SUCCESS);
	}

	nx = GMT_get_n (wesn[XLO], wesn[XHI], Grid_orig[0]->header->inc[GMT_X], Grid_orig[0]->header->registration);
	ny = GMT_get_n (wesn[YLO], wesn[YHI], Grid_orig[0]->header->inc[GMT_Y], Grid_orig[0]->header->registration);

#ifdef USE_GDAL
	if (Ctrl->D.active) {	/* Trust more on info from gdal to make it more stable against pixel vs grid registration troubles */
		nx = from_gdalread->RasterXsize;
		ny = from_gdalread->RasterYsize;
	}
#endif

	GMT_boundcond_init (GMT, &edgeinfo);

	/* Read data */

#ifdef USE_GDAL
	if (Ctrl->D.active) {
		for (k = j = 0; k < n_grids; k++) {
			Grid_orig[k]->data = GMT_memory (GMT, NULL, Grid_orig[k]->header->size, float);
			GMT_grd_loop (Grid_orig[k], row, col, ij) Grid_orig[k]->data[ij] = (float)from_gdalread->UInt8.data[j++];
		}
	}
	else {
#endif
		for (k = 0; k < n_grids; k++) {
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file[k]), 
				(void **)&Grid_orig[k])) Return (GMT_DATA_READ_ERROR);	/* Get grid data */
		}
#ifdef USE_GDAL
	}
#endif

	/* If given, get intensity file or compute intensities */

	if (Ctrl->I.active) {	/* Illumination wanted */

		GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read intensity file\n");

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->I.file), 
			(void **)&Intens_orig)) Return (GMT_DATA_READ_ERROR);	/* Get header only */

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->I.file), 
			(void **)&Intens_orig)) Return (GMT_DATA_READ_ERROR);	/* Get grid data */
		if (Intens_orig->header->nx != Grid_orig[0]->header->nx || Intens_orig->header->ny != Grid_orig[0]->header->ny) {
			GMT_report (GMT, GMT_MSG_FATAL, "Intensity file has improper dimensions!\n");
			Return (EXIT_FAILURE);
		}
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

	if (need_to_project) {	/* Need to resample the grd file */
		GMT_LONG nx_proj = 0, ny_proj = 0;
		double inc[2] = {0.0, 0.0};
		GMT_report (GMT, GMT_MSG_NORMAL, "project grid files\n");

		if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
			nx_proj = Grid_orig[0]->header->nx;
			ny_proj = Grid_orig[0]->header->ny;
		}
		for (k = 0; k < n_grids; k++) {
			if (!Grid_proj[k]) Grid_proj[k] = GMT_create_grid (GMT);
			GMT_set_proj_limits (GMT, Grid_proj[k]->header, Grid_orig[k]->header);
			grid_registration = (Ctrl->E.dpi > 0) ? GMT_PIXEL_REG : Grid_orig[k]->header->registration;	/* Force pixel if dpi is set */
			GMT_err_fail (GMT, GMT_grdproject_init (GMT, Grid_proj[k], inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration), Ctrl->In.file[k]);
			Grid_proj[k]->data = GMT_memory (GMT, NULL, Grid_proj[k]->header->size, float);
			GMT_grd_project (GMT, Grid_orig[k], Grid_proj[k], &edgeinfo, Ctrl->S.antialias, Ctrl->S.interpolant, Ctrl->S.threshold, FALSE);
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid_orig[k]);
		}
		if (Ctrl->I.active) {
			Intens_proj = GMT_create_grid (GMT);

			GMT_memcpy (Intens_proj->header->wesn, Grid_proj[0]->header->wesn, 4, double);
			if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
				nx_proj = Intens_orig->header->nx;
				ny_proj = Intens_orig->header->ny;
			}
			GMT_err_fail (GMT, GMT_grdproject_init (GMT, Intens_proj, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration), Ctrl->I.file);
			Intens_proj->data = GMT_memory (GMT, NULL, Intens_proj->header->size, float);
			GMT_grd_project (GMT, Intens_orig, Intens_proj, &edgeinfo, Ctrl->S.antialias, Ctrl->S.interpolant, Ctrl->S.threshold, FALSE);
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
		grid_registration = Grid_orig[0]->header->registration;
	}
	nm = Grid_proj[0]->header->nm;

	GMT_plotinit (API, PSL, options);

	GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	if (!Ctrl->N.active) GMT_map_clip_on (GMT, PSL, GMT->session.no_rgb, 3);

	if (P && P->has_pattern) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Patterns in cpt file only apply to -T\n");
	GMT_report (GMT, GMT_MSG_NORMAL, "Evaluate pixel colors\n");

	if (Ctrl->Q.active) rgb_used = GMT_memory (GMT, NULL, 256*256*256, unsigned char);
	if (Ctrl->M.active || (P && P->is_gray))
		bitimage_8 = GMT_memory (GMT, NULL, nm, unsigned char);
	else {
		if (Ctrl->Q.active) colormask_offset = 3;
		bitimage_24 = GMT_memory (GMT, NULL, 3 * nm + colormask_offset, unsigned char);
		if (P && Ctrl->Q.active) {
			bitimage_24[0] = GMT_u255 (P->patch[GMT_NAN].rgb[0]);
			bitimage_24[1] = GMT_u255 (P->patch[GMT_NAN].rgb[1]);
			bitimage_24[2] = GMT_u255 (P->patch[GMT_NAN].rgb[2]);
		}
	}
	normal_x = !(GMT->current.proj.projection == GMT_LINEAR && !GMT->current.proj.xyz_pos[0] && !resampled);
	normal_y = !(GMT->current.proj.projection == GMT_LINEAR && !GMT->current.proj.xyz_pos[1] && !resampled);
	
	NaN_rgb = (P) ? P->patch[GMT_NAN].rgb : GMT->current.setting.color_patch[GMT_NAN];
	for (try = 0, done = FALSE; !done && try < 2; try++) {	/* Evaluate colors at least once, or twice if -Q and we need to select another NaN color */
		for (row = 0, byte = colormask_offset; row < Grid_proj[0]->header->ny; row++) {
			actual_row = (normal_y) ? row : Grid_proj[0]->header->ny - row - 1;
			kk = GMT_IJP (Grid_proj[0]->header, actual_row, 0);
			for (col = 0; col < Grid_proj[0]->header->nx; col++) {	/* Compute rgb for each pixel */
				node = kk + (normal_x ? col : Grid_proj[0]->header->nx - col - 1);
#ifdef USE_GDAL
				if (Ctrl->D.active && !Ctrl->In.do_rgb) {
					index = GMT_NAN - 3;	/* Ensures no illumination done later */
					rgb[0] = r_table[(int)Grid_proj[0]->data[node]];
					if (do_indexed) {
						rgb[1] = g_table[(int)Grid_proj[0]->data[node]];
						rgb[2] = b_table[(int)Grid_proj[0]->data[node]];
					}
				}
				else
#endif
				if (Ctrl->In.do_rgb) {
					for (k = 0; k < 3; k++) {
						if (GMT_is_fnan (Grid_proj[k]->data[node])) {	/* If one is NaN they are all assumed to be NaN */
							k = 3;
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

				if (Ctrl->I.active && index != GMT_NAN - 3) GMT_illuminate (GMT, Intens_proj->data[node], rgb);
				
				if (P && P->is_gray)	/* Color table only has grays, pick r */
					bitimage_8[byte++] = GMT_u255 (rgb[0]);
				else if (Ctrl->M.active)	/* Convert rgb to gray using the GMT_YIQ transformation */
					bitimage_8[byte++] = GMT_u255 (GMT_YIQ (rgb));
				else {
					for (k = 0; k < 3; k++) bitimage_24[byte++] = i_rgb[k] = GMT_u255 (rgb[k]);
					if (Ctrl->Q.active && index != GMT_NAN - 3) /* Keep track of all r/g/b combinations used except for NaN */
						rgb_used[(i_rgb[0]*256 + i_rgb[1])*256+i_rgb[2]] = TRUE;
				}
			}
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
		if (need_to_project) /* Must remove locally created grids */
			GMT_free_grid (GMT, &Intens_proj, TRUE);
		else
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Intens_proj);
	}

	/* Get actual size of each pixel */

	dx = GMT_get_inc (Grid_proj[0]->header->wesn[XLO], Grid_proj[0]->header->wesn[XHI], Grid_proj[0]->header->nx, Grid_proj[0]->header->registration);
	dy = GMT_get_inc (Grid_proj[0]->header->wesn[YLO], Grid_proj[0]->header->wesn[YHI], Grid_proj[0]->header->ny, Grid_proj[0]->header->registration);

	/* Set lower left position of image on map */

	x0 = Grid_proj[0]->header->wesn[XLO];	y0 = Grid_proj[0]->header->wesn[YLO];
	if (grid_registration == GMT_GRIDLINE_REG) {	/* Grid registration, move 1/2 pixel down/left */
		x0 -= 0.5 * dx;
		y0 -= 0.5 * dy;
	}

	x_side = dx * Grid_proj[0]->header->nx;
	y_side = dy * Grid_proj[0]->header->ny;

	GMT_report (GMT, GMT_MSG_NORMAL, "Creating PostScript image ");

	if (P && P->is_gray) for (kk = 0, P->is_bw = TRUE; P->is_bw && kk < nm; kk++) if (!(bitimage_8[kk] == 0 || bitimage_8[kk] == 255)) P->is_bw = FALSE;

	if (P && P->is_bw) {	/* Can get away with 1 bit image */
		GMT_LONG nx8, shift, b_or_w, nx_pixels, k8;
		unsigned char *bit = NULL;

		GMT_report (GMT, GMT_MSG_NORMAL, "[1-bit B/W image]\n");

		nx8 = (GMT_LONG)ceil (Grid_proj[0]->header->nx / 8.0);	/* Image width must equal a multiple of 8 bits */
		nx_pixels = nx8 * 8;
		bit = GMT_memory (GMT, NULL, nx8 * Grid_proj[0]->header->ny, unsigned char);

		for (row = k = k8 = 0; row < Grid_proj[0]->header->ny; row++) {
			shift = byte = 0;
			for (col = 0; col < Grid_proj[0]->header->nx; col++, k++) {
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
		PSL_plotbitimage (PSL, x0, y0, x_side, y_side, PSL_BL, bit, nx_pixels, Grid_proj[0]->header->ny, Ctrl->G.f_rgb, Ctrl->G.b_rgb);
		GMT_free (GMT, bit);
	}
	else if ((P && P->is_gray) || Ctrl->M.active) {
		GMT_report (GMT, GMT_MSG_NORMAL, "[8-bit grayshade image]\n");
		PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_8, Grid_proj[0]->header->nx, Grid_proj[0]->header->ny, (Ctrl->E.device_dpi ? -8 : 8));
		GMT_free (GMT, bitimage_8);
	}
	else {
		GMT_report (GMT, GMT_MSG_NORMAL, "24-bit color image\n");
		PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_24, (Ctrl->Q.active ? -1 : 1) * Grid_proj[0]->header->nx, Grid_proj[0]->header->ny, (Ctrl->E.device_dpi ? -24 : 24));
		GMT_free (GMT, bitimage_24);
	}

	if (!Ctrl->N.active) GMT_map_clip_off (GMT, PSL);

	GMT_map_basemap (GMT, PSL);
	GMT_plane_perspective (GMT, PSL, -1, 0.0);
	GMT_plotend (GMT, PSL);

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&P);
	if (need_to_project)
		GMT_free_grid (GMT, &Grid_proj[0], TRUE);
	else
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid_proj[0]);

#ifdef USE_GDAL
	if (Ctrl->D.active) {
		if (r_table) GMT_free (GMT, r_table);
		if (g_table) {
			GMT_free (GMT, g_table);
			GMT_free (GMT, b_table);
		}
		GMT_free (GMT, to_gdalread);
		GMT_free (GMT, from_gdalread->UInt8.data);
		if (from_gdalread->ColorMap == NULL) GMT_free (GMT, from_gdalread->ColorMap);
		GMT_free (GMT, from_gdalread);
	}
#endif

	Return (EXIT_SUCCESS);
}
