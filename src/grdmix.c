/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: grdmix.c reads either one, two or three rasters, an optionally
 * a blend raster and an intensity raster.  What we do depends on options:
 * If -C then we combine grids into a single image
 * If -D then we decompose an image into grid layers.
 * Else, if one raster was given we use the blend raster to set
 * transparency and possibly use intensities to modulate the colors,
 # else we blend the two rasters using the blend as the weight
 * for raster1 and (1-weight) for raster two. with optional intensity modulation.
 *
 * Author:	Paul Wessel
 * Date:	9-MAY-2020
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdmix"
#define THIS_MODULE_MODERN_NAME	"grdmix"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Blending and transforming grids and images"
#define THIS_MODULE_KEYS	"<I{3,A?(,GI},I?("	/* For external use we are limited to images */
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-VRf"

#define ALPHA	3
#define BLEND	4
#define INTENS	5
#define N_ITEMS	6

struct GRDMIX_AIW {	/* For various grid, image, or constant arguments */
	bool active;
	unsigned int mode;	/* 0 a file, 1 a constant */
	char *file;
	double value;
};

struct GRDMIX_CTRL {
	struct GRDMIX_In {
		bool active;
		unsigned int n_in;	/* Range 1-3; we don't count the -A file placed in file[ALPHA], -I file in file[INTENS, or -W file in file[BLEND] */
		int type[N_ITEMS];	/* GMT_IS_IMAGE or GMT_NOTSET (grid) */
		char *file[N_ITEMS];
	} In;
	struct GRDMIX_AIW A; /* alpha raster */
	struct GRDMIX_C {	/* -C */
		bool active;
	} C;
	struct GRDMIX_D {	/* -D */
		bool active;
	} D;
	struct GRDMIX_G {	/* -G<output_raster> */
		bool active;
		char *file;
	} G;
	struct GRDMIX_AIW I; /* Intensity grid or value */
	struct GRDMIX_M {	/* -M */
		bool active;
	} M;
	struct GRDMIX_N {	/* -N[i|o][<factor>] */
		bool active[2];
		double factor[2];
	} N;
	struct GRDMIX_Q {	/* -Q */
		bool active;
	} Q;
	struct GRDMIX_AIW W; /* Blend weight raster */
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMIX_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDMIX_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.factor[0] = C->N.factor[1] = 255.0;	/* Default normalizing value */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDMIX_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->A.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->W.file);
	gmt_M_str_free (C->In.file[GMT_IN]);
	gmt_M_str_free (C->In.file[GMT_OUT]);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	static char *type[2] = {"grid(s) or image(s)", "image(s)"};
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <raster1> [<raster2> [<raster3>]] -G<outraster> [-A<transp>] [-C] [-D] "
		"[-I<intens>] [-M] [-N[i|o][<divisor>]] [-Q] [%s] [%s] [-W<weight>] [%s] [%s]\n",
		name, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<raster?> are the main %s to be used (1, 2, or 3).", type[API->external]);
	GMT_Usage (API, 1, "\n-G<outraster>");
	GMT_Usage (API, -2, "Specify file name for output %s file. "
		"Note: With -D the name is a template and must contain %%c to be used for the layer code.", type[API->external]);
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A<transp>");
	GMT_Usage (API, -2, "Specify a transparency grid or image, or set a constant transparency value [no transparency]. "
		"An image must have 0-255 values, while a grid or constant must be in the 0-1 range.");
	GMT_Usage (API, 1, "\n-C Construct an image from 1 (gray) or 3 (r, g, b) input component grids. "
		"You may optionally supply transparency (-A) and/or intensity (-I).");
	GMT_Usage (API, 1, "\n-D Deconstruct an image into 1 or 3 output component grids, plus any transparency. "
		"We write the raw layer values (0-255); use -N to normalize the layers (0-1).");
	GMT_Usage (API, 1, "\n-I<intens>");
	GMT_Usage (API, -2, "Specify an intensity grid file, or set a constant intensity value [no intensity]. "
		"The grid or constant must be in the -1 to +1 range).");
	GMT_Usage (API, 1, "\n-M Force a monochrome final image [same number of bands as the input].");
	GMT_Usage (API, 1, "\n-N[i|o][<divisor>]");
	GMT_Usage (API, -2, "Set normalization divisor for (i)nput or (o)utput grids [Default sets both]:");
	GMT_Usage (API, 3, "i: Input grids will be divided by <divisor> [255].");
	GMT_Usage (API, 3, "o: Output grids will be multiplied by <divisor> [255].");
	GMT_Usage (API, -2, "Append <factor> to use another factor than 255.");
	GMT_Usage (API, 1, "\n-Q Make the final image opaque by removing any alpha layer.");
	GMT_Option (API, "R,V");
	GMT_Usage (API, 1, "\n-W<weight>");
	GMT_Usage (API, -2, "Specify a blend weight grid or image, or set a constant weight [no blend weight]. "
		"An image must have 0-255 values, while a grid or constant must be in the 0-1 range.");
	GMT_Option (API, "f,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL char *grdmix_parseitem (struct GMT_CTRL *GMT, struct GMT_OPTION *opt, struct GRDMIX_AIW *X) {
	X->active = true;
	if (!gmt_access (GMT, opt->arg, R_OK)) {
		X->file = strdup (opt->arg);	/* Place this in In.file[??] for convenience later */
		X->mode = 0;	/* Flag we got a file */
	}
	else if (gmt_not_numeric (GMT, opt->arg)) {	/* Bad value or missing file */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option %c: Bad value or a file that was not found: %s\n", opt->option, opt->arg);
		X->mode = 2;
	}
	else {
		X->value = atof (opt->arg);
		X->mode = 1;
	}
	return (X->file);
}

static int parse (struct GMT_CTRL *GMT, struct GRDMIX_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdmix and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 */

	unsigned int n_errors = 0, k;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input rasters */
				if (Ctrl->In.n_in >= 3) {
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "A maximum of three rasters may be provided\n");
				}
				else {
					Ctrl->In.file[Ctrl->In.n_in] = strdup (opt->arg);
					if (GMT_Get_FilePath (GMT->parent, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file[Ctrl->In.n_in]))) n_errors++;
					Ctrl->In.n_in++;
				}
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->In.file[ALPHA] = grdmix_parseitem (GMT, opt, &(Ctrl->A));
				if (Ctrl->A.mode == 2) n_errors++;
				break;

			case 'C':
				Ctrl->C.active = true;
				break;

			case 'D':
				Ctrl->D.active = true;
				break;

			case 'G':	/* Does not matter if we pass GMT_IS_GRID or GMT_IS_IMAGE */
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;

			case 'I':
				Ctrl->In.file[INTENS] = grdmix_parseitem (GMT, opt, &(Ctrl->I));
				if (Ctrl->I.mode == 2) n_errors++;
			break;

			case 'M':
				Ctrl->M.active = true;
				break;

			case 'N':
				switch (opt->arg[0]) {
					case 'i': k = GMT_IN; break;
					case 'o': k = GMT_OUT; break;
					default:  k = GMT_IO; break;
				}
				if (k == GMT_IO) {	/* Turn on both in and out grid normalization */
					Ctrl->N.active[GMT_IN] = Ctrl->N.active[GMT_OUT] = true;
					if (opt->arg[0]) Ctrl->N.factor[GMT_IN] = Ctrl->N.factor[GMT_OUT] = atof (opt->arg);					
				}
				else {	/* Just activate in or out grid normalization */
					Ctrl->N.active[k] = true;
					if (opt->arg[1]) Ctrl->N.factor[k] = atof (&opt->arg[1]);
				}
				break;

			case 'Q':
				Ctrl->Q.active = true;
				break;

			case 'W':
				Ctrl->In.file[BLEND] = grdmix_parseitem (GMT, opt, &(Ctrl->W));
				if (Ctrl->W.mode == 2) n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file[0], "Must specify at least one input raster file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.n_in == 1 && !(Ctrl->A.active || Ctrl->D.active || Ctrl->I.active || Ctrl->M.active || Ctrl->Q.active),
		"For one input image you must specify one or more of -A, -D, -I, -M, -Q\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.n_in == 2 && !Ctrl->W.active, "For two input images you must provide weights in -W\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.n_in == 3 && !Ctrl->C.active, "For three input images you must select -C\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.mode && (Ctrl->A.value < 0.0 || Ctrl->A.value > 1.0), "Option -A: A constant transparency must be in the 0-1 range\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active, "Can only use one of -C and -D\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->A.active, "Option -A: Not used with -D\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !(Ctrl->In.n_in == 1 || Ctrl->In.n_in == 3), "Option -C: Requires one or three rasters\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->In.n_in != 1, "Option -D: Only a single raster can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->D.active, "Option -I: Not available when -D is selected\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output raster file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active[0] && Ctrl->N.factor[0] == 0.0, "Option -N: Normalization factor for input grids cannot be zero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active[1] && Ctrl->N.factor[1] == 0.0, "Option -N: Normalization factor for output grids cannot be zero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->A.active, "Option -A: Not available when -A is selected\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->G.file && strstr (Ctrl->G.file, "%c") == NULL, "Option -G: With -D, output name must be template name containing %%c\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.mode && (Ctrl->I.value < -1.0 || Ctrl->I.value > 1.0), "Option -I: A constant intensity must be in the +/- 1 range\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.mode && (Ctrl->W.value < 0.0 || Ctrl->W.value > 1.0), "Option -W: A constant blend weight must be in the 0-1 range\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

#if DEBUG
/* For developers debugging only, and under -Vd to boot */
GMT_LOCAL void grdmix_dump_image (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, char *file) {
	uint64_t row, col, node, pix = 0;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Image = %s [%s]\n", file, I->header->mem_layout);
	if (I->header->n_bands == 1) {
		gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel here */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Pixel %4d is %3.3d\n", (int)node, I->data[node]);
		}
	}
	else if (I->header->n_bands == 3) {
		gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is three per pixel here */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Pixel %4d is %3.3d/%3.3d/%3.3d\n", (int)node, I->data[pix], I->data[pix+1], I->data[pix+2]);
			pix += 3;
		}
	}
	else {	/* RGBA */
		gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is three per pixel here */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Pixel %4d is %3.3d/%3.3d/%3.3d (%3.3%d)\n", (int)node, I->data[pix], I->data[pix+1], I->data[pix+2], I->data[pix+3]);
			pix += 4;
		}
	}
}
#endif

GMT_LOCAL float *grdmix_get_array (struct GMT_CTRL *GMT, struct GRDMIX_AIW *X, int type, struct GMT_GRID **G, struct GMT_IMAGE **I, struct GMT_GRID_HEADER *h, float min, float max, char *name) {
	/* Function to either read a grid, and image, or use a constant; then build and return the array with the information */
	uint64_t row, col, node;
	float *array = gmt_M_memory (GMT, NULL, h->size, float);
	if (X->mode == 0) {	/* Got a grid or image */
		if (type == GMT_NOTSET) {	/* Got a grid */
			uint64_t n_bad = 0;
			gmt_M_grd_loop (GMT, (*G), row, col, node) {
				array[node] = (*G)->data[node];
				if (array[node] < min || array[node] > max) n_bad++;
			}
			if (n_bad) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "The %s grid not in the %g to %g range; %" PRIu64 " nodes were outside the range!\n", name, min, max, n_bad);
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "If the grid is in 0-255 range, please use -N to normalized the values.\n");
				gmt_M_free (GMT, array);
				return NULL;
			}
			if (GMT_Destroy_Data (GMT->parent, G) != GMT_NOERROR) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to free %s grid!\n", name);
				gmt_M_free (GMT, array);
				return NULL;
			}
		}
		else {	/* Got an image */
			gmt_M_grd_loop (GMT, (*I), row, col, node)
				array[node] = gmt_M_is255 ((*I)->data[node]);
			if (GMT_Destroy_Data (GMT->parent, I) != GMT_NOERROR) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to free %s image!\n", name);
				gmt_M_free (GMT, array);
				return NULL;
			}
		}
	}
	else {	/* Make a constant array of weights */
		for (node = 0; node < h->size; node++)
			array[node] = X->value;
	}
	return array;
}

EXTERN_MSC int GMT_grdmix (void *V_API, int mode, void *args) {
	char *type[N_ITEMS] = {NULL, NULL, NULL, "alpha", "blend", "intens"};

	bool got_R = false;

	int error = 0;
	unsigned int img = 0, k, band;
	int64_t row, col, node, pix;

	float *weights = NULL, *intens = NULL, *alpha = NULL;

	double rgb[4], wesn[4] = {0.0, 0.0, 0.0, 0.0};

	struct GMT_GRID *G_in[N_ITEMS], *G = NULL;
	struct GMT_IMAGE *I_in[N_ITEMS], *I = NULL;
	struct GMT_GRID_HEADER *h[N_ITEMS], *H = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH[N_ITEMS];
	struct GRDMIX_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdmix main code ----------------------------*/

	gmt_M_memset (G_in, N_ITEMS, struct GMT_GRID *);
	gmt_M_memset (I_in, N_ITEMS, struct GMT_IMAGE *);
	gmt_M_memset (HH, N_ITEMS, struct GMT_GRID_HEADER_HIDDEN *);
	gmt_M_memset (h, N_ITEMS, struct GMT_GRID_HEADER *);

	GMT_Set_Default (API, "API_IMAGE_LAYOUT", "TRB ");	/* Easiest to manipulate images with separate bands */
	gmt_set_pad (GMT, 0); /* No padding needed in this module */

	if ((got_R = GMT->common.R.active[RSET]))	/* Got -Rw/e/s/n */
		gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input rasters\n");

	for (k = 0; k < N_ITEMS; k++) {	/* Determine if we got grids or images */
		if (Ctrl->In.file[k] == NULL) continue;
		Ctrl->In.type[k] = gmt_raster_type (GMT, Ctrl->In.file[k]);
	}

	if (Ctrl->In.n_in == 1 && Ctrl->In.type[0] == GMT_NOTSET && !Ctrl->D.active) {
		GMT_Report (API, GMT_MSG_ERROR, "For a single input raster it must be an image\n");
		Return (GMT_RUNTIME_ERROR);		
	}

	for (k = 0; k < N_ITEMS; k++) {	/* Read the headers (1-N_ITEMS files) */
		if (Ctrl->In.file[k] == NULL) continue;
		if (Ctrl->In.type[k] == GMT_NOTSET) {	/* Got a grid */
			if ((G_in[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = G_in[k]->header;	/* Pointer to grid header */
		}
		else {	/* Got an image */
			if ((I_in[k] = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = I_in[k]->header;	/* Pointer to image header */
		}
		HH[k] = gmt_get_H_hidden (h[k]);
	}

	if (Ctrl->In.file[INTENS] && I_in[INTENS]) {
			GMT_Report (API, GMT_MSG_ERROR, "The intensity information must be a grid with values in the -1 to +1 range, not an image!\n");
			Return (GMT_RUNTIME_ERROR);		
	}

	if (Ctrl->In.n_in == 2 && Ctrl->In.type[0] != Ctrl->In.type[1]) {
		GMT_Report (API, GMT_MSG_ERROR, "Both inputs must either be images or grids, not a mix\n");
		Return (GMT_RUNTIME_ERROR);
	}
	for (k = 1; k < N_ITEMS; k++) {	/* Make sure all rasters have matching dimensions */
		if (Ctrl->In.file[k] == NULL) continue;
		if (h[0]->registration != h[k]->registration || (h[0]->n_rows != h[k]->n_rows) || (h[0]->n_columns != h[k]->n_columns)) {
			GMT_Report (API, GMT_MSG_ERROR, "Dimensions/registrations of %s are not compatible for the other rasters!\n", Ctrl->In.file[k]);
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->In.type[0] != GMT_NOTSET)	/* Get the array index to the image with the largest number of bands (should they differ) */
			if (h[k]->n_bands >= h[img]->n_bands) img = k;
	}

	/* Also make sure any given blend, alpha, intens images only have one band */
	for (k = BLEND; k < N_ITEMS; k++) {	/* Check the last three */
		if (h[k] && h[k]->n_bands != 1) {
			GMT_Report (API, GMT_MSG_ERROR, "The %s image must be gray-scale only!\n", type[k]);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	for (k = 0; k < N_ITEMS; k++) {	/* Read data from the 1-N_ITEMS files */
		if (Ctrl->In.file[k] == NULL) continue;
		if (Ctrl->In.type[k] == GMT_NOTSET) {	/* Read the grid */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[k], G_in[k]) == NULL) {	/* Get data only */
				Return (API->error);
			}
			if (k < 3 && Ctrl->N.active[GMT_IN]) {	/* Normalize the 1-3 input grid */
				double factor = 1.0 / Ctrl->N.factor[GMT_IN];	/* To avoid division in the loop */
				GMT_Report (API, GMT_MSG_INFORMATION, "Normalizing grid %s\n", Ctrl->In.file[k]);
				gmt_M_grd_loop (GMT, G_in[k], row, col, node)
					G_in[k]->data[node] *= factor;
			}
			gmt_M_memcpy (wesn, h[k]->wesn, 4, double);
			got_R = true;
			if (HH[k]->grdtype)	/* Got a geographic grid */
				gmt_set_geographic (GMT, GMT_IN);
		}
		else {	/* Read the image */
			if (GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY | GMT_IMAGE_NO_INDEX, NULL, Ctrl->In.file[k], I_in[k]) == NULL) {	/* Get data only */
				Return (API->error);
			}
#if DEBUG
			if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) grdmix_dump_image (GMT, I_in[k], Ctrl->In.file[k]);
#endif
		}
	}

	if (Ctrl->A.active) {	/* Set up the transparencies, then free the grid/image struct */
		if ((alpha = grdmix_get_array (GMT, &(Ctrl->A), Ctrl->In.type[ALPHA], &G_in[ALPHA], &I_in[ALPHA], h[0], 0.0, 1.0, "alpha")) == NULL) {
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->I.active) {	/* Set up the intensities, then free the grid/image struct */
		if ((intens = grdmix_get_array (GMT, &(Ctrl->I), Ctrl->In.type[INTENS], &G_in[INTENS], &I_in[INTENS], h[0], -1.0, 1.0, "intensity")) == NULL) {
			if (alpha) gmt_M_free (GMT, alpha);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->W.active) {	/* Set up the blend weights, then free the grid/image struct */
		if ((weights = grdmix_get_array (GMT, &(Ctrl->W), Ctrl->In.type[BLEND], &G_in[BLEND], &I_in[BLEND], h[0], 0.0, 1.0, "blend weight")) == NULL) {
			if (alpha) gmt_M_free (GMT, alpha);
			if (intens) gmt_M_free (GMT, intens);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Deal with operation -D up front, then exit */

	if (Ctrl->D.active) {	/* De-construct single image into its gray or R,G,B [plus optional A] layers */
		char code[4] = {'R', 'G', 'B', 'A'}, file[PATH_MAX] = {""};
		uint64_t off = 0;
		float scale = 1.0 / Ctrl->N.factor[GMT_OUT];	/* To avoid division */

		H = I_in[0]->header;
		GMT_Report (API, GMT_MSG_INFORMATION, "Deconstruct image into component grid layers\n", Ctrl->In.file[0]);
		if (H->n_bands == 1) code[0] = 'g';	/* Grayscale */
		if (gmt_M_360_range (H->wesn[XLO], H->wesn[XHI]) && gmt_M_180_range (H->wesn[YHI], H->wesn[YLO]))
			gmt_set_geographic (GMT, GMT_IN);	/* If exactly fitting the Earth then we assume geographic image */
		for (band = 0; band < H->n_bands; band++) {	/* March across the RGB values in both images and increment counters */
			if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, H->wesn, H->inc, H->registration, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a grid for output!\n");
				if (alpha) gmt_M_free (GMT, alpha);
				if (intens) gmt_M_free (GMT, intens);
				gmt_M_free (GMT, weights);
				Return (GMT_RUNTIME_ERROR);
			}
			off = band * H->size;
			if (Ctrl->N.active[GMT_OUT]) {
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node) shared(GMT,G,off,scale,I_in)
#endif
				gmt_M_grd_loop (GMT, G, row, col, node)
					G->data[node] = scale * I_in[0]->data[node+off];
			}
			else {
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node) shared(GMT,G,off,scale,I_in)
#endif
				gmt_M_grd_loop (GMT, G, row, col, node)
					G->data[node] = I_in[0]->data[node+off];

			}
			sprintf (file, Ctrl->G.file, code[band]);
			/* Write out grid */
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
				if (alpha) gmt_M_free (GMT, alpha);
				if (intens) gmt_M_free (GMT, intens);
				gmt_M_free (GMT, weights);
				Return (API->error);
			}
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, G) != GMT_NOERROR) {
				if (alpha) gmt_M_free (GMT, alpha);
				if (intens) gmt_M_free (GMT, intens);
				gmt_M_free (GMT, weights);
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to free grid!\n");
				if (alpha) gmt_M_free (GMT, alpha);
				if (intens) gmt_M_free (GMT, intens);
				gmt_M_free (GMT, weights);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (alpha) gmt_M_free (GMT, alpha);
		if (intens) gmt_M_free (GMT, intens);
		Return (GMT_NOERROR);
	}

	if (Ctrl->C.active) {	/* Combine 1 or 3 grids into a new single image, while handling the optional -A -I information */
		uint64_t dim[3] = {0,0,3};
		GMT_Report (API, GMT_MSG_INFORMATION, "Construct image from %d component grid layers\n", Ctrl->In.n_in);
		if ((I = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, dim, G_in[0]->header->wesn, G_in[0]->header->inc, G_in[0]->header->registration, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate an image for output!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		H = I->header;
		if (Ctrl->I.active && Ctrl->In.n_in == 3) {	/* Make the most work-intensive version under OpenMP */
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,band,rgb,pix) shared(GMT,I,G_in,H,intens)
#endif
			gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
				for (band = 0; band < 3; band++)	/* March across the RGB values in both images and increment counters */
					rgb[band] = G_in[band]->data[node];
				/* Modify colors based on intensity */
				gmt_illuminate (GMT, intens[node], rgb);
				for (band = 0, pix = node; band < 3; band++, pix += H->size)	/* March across the RGB values */
					I->data[pix] = gmt_M_u255 (rgb[band]);
			}
		}
		else {
			gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
				for (band = 0; band < H->n_bands; band++)	/* March across the RGB values in both images and increment counters */
					rgb[band] = G_in[band]->data[node];
				if (Ctrl->I.active)	{	/* Modify colors based on intensity */
					if (Ctrl->In.n_in == 1)	/* Duplicate grays so illuminate can work */
						rgb[1] = rgb[2] = rgb[0];
					gmt_illuminate (GMT, intens[node], rgb);
				}
				for (band = 0, pix = node; band < H->n_bands; band++, pix += H->size)	/* March across the RGB values */
					I->data[pix] = gmt_M_u255 (rgb[band]);
			}
		}
	}
	else {	/* The remaining options */
		if (Ctrl->In.type[0] == GMT_NOTSET) {	/* Create output grid for the blend */
			if ((G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, G_in[0])) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate a grid for output!\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else {	/* Create output image for the blend or for the transparent image */
			if ((I = GMT_Duplicate_Data (API, GMT_IS_IMAGE, GMT_DUPLICATE_ALLOC, I_in[0])) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate an image for output!\n");
				Return (GMT_RUNTIME_ERROR);
			}
			H = I->header;	/* Shorthand only */
		}

		/* Now do the work which depends on whether we have 1 or 2 input items */

		if (Ctrl->In.n_in == 1) {	/* Pass intensity and/or transparency from -A to the input image */
			GMT_Report (API, GMT_MSG_INFORMATION, "Adjust image via transparency and/or alpha\n");
			gmt_M_memcpy (I->data, I_in[0]->data, I->header->size * I->header->n_bands, unsigned char);	/* Duplicate the input image */
		}
		else {	/* Blend two grids or images using the weights given via -A */
			if (Ctrl->In.type[0] == GMT_NOTSET) {	/* Two grids, just do the blend loop */
				GMT_Report (API, GMT_MSG_INFORMATION, "Blend two grids via weights\n");
				gmt_M_grd_loop (GMT, G, row, col, node)
					G->data[node] = weights[node] * (G_in[0]->data[node] - G_in[1]->data[node]) + G_in[1]->data[node];
				gmt_M_free (GMT, weights);
				/* Write out grid */
				if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
					Return (API->error);
				}
				if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR) {
					Return (API->error);
				}
				Return (GMT_NOERROR);
			}
			else {	/* Two images, do the r,g,b blend, watch out for any input transparency value to skip */
				int p0, p1;
				GMT_Report (API, GMT_MSG_INFORMATION, "Blend two images via weights\n");
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,band,pix,p0,p1) shared(GMT,I,I_in,H,weights)
#endif
				gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
					for (band = 0, pix = node; band < H->n_bands; band++, pix += H->size) {	/* March across the RGB values in both images and increment counters */
						p0 = (band < I_in[0]->header->n_bands) ? I_in[0]->data[pix] : I_in[0]->data[node];	/* Get corresponding r,g,b or just duplicate gray three times */
						p1 = (band < I_in[1]->header->n_bands) ? I_in[1]->data[pix] : I_in[1]->data[node];	/* Get corresponding r,g,b or just duplicate gray three times */
						I->data[pix] = (unsigned char) urint (weights[node] * (p0 - p1) + p1);
					}
				}
				gmt_M_free (GMT, weights);
			}
		}
	}
	
	for (k = 0; k < ALPHA; k++) {	/* Free up memory no longer needed here */
		void *W = (Ctrl->In.type[k] == GMT_NOTSET) ? (void *)G_in[k] : (void *)I_in[k];
		if (W == NULL) continue;
		if (GMT_Destroy_Data (API, &W) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to free grid or image associated with file %s\n", Ctrl->In.file[k]);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->I.active) {	/* Modify colors via the intensities */
		GMT_Report (API, GMT_MSG_INFORMATION, "Modify colors via intensities\n");
		if (H->n_bands == 3) {
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,band,pix,rgb) shared(GMT,I,H,intens)
#endif
			gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
				for (band = 0, pix = node; band < 3; band++, pix += H->size)	/* Get the normalized r,g,b triplet */
					rgb[band] = gmt_M_is255 (I->data[pix]);
				gmt_illuminate (GMT, intens[node], rgb);
				for (band = 0, pix = node; band < 3; band++, pix += H->size)	/* March across the RGB values */
					I->data[pix] = gmt_M_u255 (rgb[band]);
			}
		}
		else {	/* Gray image */
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,rgb) shared(GMT,I,intens)
#endif
			gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
				rgb[0] = rgb[1] = rgb[2] = gmt_M_is255 (I->data[node]);
				gmt_illuminate (GMT, intens[node], rgb);
				I->data[node] = gmt_M_u255 (rgb[0]);
			}
		}
		gmt_M_free (GMT, intens);
	}

	if (Ctrl->A.active) { 	/* Add a transparency layer */
		GMT_Report (API, GMT_MSG_INFORMATION, "Add an alpha layer\n");
		if ((I->alpha = gmt_M_memory_aligned (GMT, NULL, H->size, unsigned char)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to allocate alpha layer\n");
			Return (GMT_RUNTIME_ERROR);
		}
#ifdef _OPENMP
#pragma omp parallel for private(node) shared(H,I,alpha)
#endif
		for (node = 0; node < (int64_t)H->size; node++)	/* Scale to 0-255 range */
			I->alpha[node] = gmt_M_u255 (alpha[node]);
		gmt_M_free (GMT, alpha);
	}

	if (Ctrl->Q.active) {	/* Remove transparency payer (make image opaQue) */
		if (I->header->n_bands == 2 || I->header->n_bands == 4) {	/* Gray or color image with alpha layer */
			unsigned char *data = NULL;
			GMT_Report (API, GMT_MSG_INFORMATION, "Remove alpha layer\n");
			if ((data = gmt_M_memory_aligned (GMT, NULL, I->header->size * (I->header->n_bands-1), unsigned char)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to allocate gray image layer\n");
				Return (GMT_RUNTIME_ERROR);
			}
			gmt_M_memcpy (data, I->data, I->header->size * (I->header->n_bands-1), unsigned char);	/* Duplicate the input image */
			gmt_M_free_aligned (GMT, I->data);	/* Free old rgb image and add gray image */
			I->data = data;
			I->header->n_bands--;
		}
		else	/* No alpha layer found */
			GMT_Report (API, GMT_MSG_INFORMATION, "No alpha layer to remove\n");
	}

	if (Ctrl->M.active) {	/* Convert to monochrome image using the YIQ transformation */
		unsigned char *data = NULL;
		if ((data = gmt_M_memory_aligned (GMT, NULL, H->size, unsigned char)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to allocate gray image layer\n");
			Return (GMT_RUNTIME_ERROR);
		}
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,band,pix,rgb) shared(GMT,I,H,data)
#endif
		gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
			for (band = 0, pix = node; band < H->n_bands; band++, pix += H->size)	/* Get the normalized r,g,b triplet */
				rgb[band] = gmt_M_is255 (I->data[pix]);
			data[node] = gmt_M_u255 (gmt_M_yiq (rgb));
		}
		gmt_M_free_aligned (GMT, I->data);	/* Free old rgb image and add the new gray image */
		I->data = data;
		H->n_bands = 1;
	}

	/* Write image here - any grid output was written earlier */
	/* Turn off the automatic creation of aux files by GDAL */
#ifdef WIN32
	if (_putenv ("GDAL_PAM_ENABLED=NO"))
#else
	if (setenv ("GDAL_PAM_ENABLED", "NO", 0))
#endif
		GMT_Report (API, GMT_MSG_WARNING, "Unable to set GDAL_PAM_ENABLED to prevent writing of auxiliary files\n");

	if (got_R) {	/* Override whatever wesn and incs are in the header */
		gmt_M_memcpy (H->wesn, wesn, 4, double);
		H->inc[GMT_X] = gmt_M_get_inc (GMT, H->wesn[XLO], H->wesn[XHI], H->n_columns, H->registration);
		H->inc[GMT_Y] = gmt_M_get_inc (GMT, H->wesn[YLO], H->wesn[YHI], H->n_rows,    H->registration);
	}
	/* Look for global images despite faulty metadata not flagging as geo */
	if (H->ProjRefPROJ4 == NULL) {	/* No geographic meta-data present, examine region */
		if (gmt_M_360_range (H->wesn[XLO], H->wesn[XHI]) && gmt_M_180_range (H->wesn[YHI], H->wesn[YLO]))
			gmt_set_geographic (GMT, GMT_IN);	/* If exactly fitting the Earth then we assume geographic image */
		if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* May be true due to -R or -fg */
			char buf[GMT_LEN128] = {""};
			/* See if we have valid proj info the chosen projection has a valid PROJ4 setting */
			sprintf (buf, "+proj=longlat +a=%f +b=%f +no_defs", GMT->current.setting.ref_ellipsoid[k].eq_radius,
				GMT->current.setting.ref_ellipsoid[k].eq_radius);
			H->ProjRefPROJ4 = strdup (buf);
		}
	}
	if (H->ProjRefPROJ4 && strstr (Ctrl->G.file, ".tif") == NULL)
		GMT_Report (API, GMT_MSG_WARNING, "The geographical metadata for you image will be lost unless you use TIF\n");
	/* Convert from TRB to TRP (TRPa if there is alpha) */
	GMT_Change_Layout (API, GMT_IS_IMAGE, "TRP", 0, I, NULL, NULL);	
	/* Write out image */
	if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, I) != GMT_NOERROR) {
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
