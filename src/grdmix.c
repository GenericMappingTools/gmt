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
#define THIS_MODULE_OPTIONS "-Vf"

#define BLEND	3
#define INTENS	4

struct GRDMIX_CTRL {
	struct GRDMIX_In {
		bool active;
		unsigned int n_in;	/* Range 1-3; we don't count the -A file placed in file[BLEND], nor -I file in file[INTENS] */
		int type[5];	/* GMT_IS_IMAGE or GMT_NOTSET (grid) */
		char *file[5];
	} In;
	struct GRDMIX_A {	/* Blend weight raster */
		bool active;
		unsigned int mode;	/* 0 a file, 1 a constant */
		char *file;
		double value;
	} A;
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
	struct GRDMIX_I {	/* Intensity grid or value */
		bool active;
		unsigned int mode;	/* 0 a file, 1 a constant */
		char *file;
		double value;
	} I;
	struct GRDMIX_M {	/* -M */
		bool active;
	} M;
	struct GRDMIX_N {	/* -N */
		bool active;
	} N;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMIX_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDMIX_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDMIX_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->A.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->In.file[GMT_IN]);
	gmt_M_str_free (C->In.file[GMT_OUT]);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	static char *type[2] = {"grid or image", "image"};
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <raster1> [<raster2> [<raster3>]] -G<outraster> [-A<grid|image|value>] [-C] [-D]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<intens>] [-M] [-N] [%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<raster1> is the main %s to be used.\n", type[API->external]);
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output %s file.\n", type[API->external]);
	GMT_Message (API, GMT_TIME_NONE, "\t   If -C is used the name is a template and must contain %%c for layer code\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify a grid file, image file, or a constant blend value (0-1).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Construct image from 1 (gray) or 3 (r, g, b) input component grids.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   You may optionally supply transparency (-A) and/or intensity (-I).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Deconstruct image into 1 or 3 output component grids, plus any transparency.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Apply intensities to final image colors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Force monochrome final image [same as input].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize grids from 0-255 to 0-1 [input grids already normalized]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<raster?> are optional second/third %s to be used.\n", type[API->external]);
	GMT_Option (API, "V,f,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDMIX_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdmix and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input rasters */
				if (Ctrl->In.n_in < 3 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[Ctrl->In.n_in++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "A maximum of three rasters may be provided\n");
				}
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				if (!gmt_access (GMT, opt->arg, R_OK)) {
					Ctrl->A.file = Ctrl->In.file[BLEND] = strdup (opt->arg);	/* Place this in In.file[BLEND] for convenience later */
					Ctrl->A.mode = 0;	/* Flag we got a file */
				}
				else if ((Ctrl->A.value = atof (opt->arg)) < 0.0 || Ctrl->A.value > 1.0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -A: A constant blend must be in the 0-1 range\n");
					n_errors++;
				}
				else	/* Got a valid constant */
					Ctrl->A.mode = 1;
				break;

			case 'C':
				Ctrl->C.active = true;
				break;

			case 'D':
				Ctrl->D.active = true;
				break;

			case 'G':	/* Does not matter if we pass GMT_IS_GRID or GMT_IS_IMAGE */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			case 'I':
				Ctrl->I.active = true;
				if (!gmt_access (GMT, opt->arg, R_OK)) {
					Ctrl->I.file = Ctrl->In.file[INTENS] = strdup (opt->arg);	/* Place this in In.file[INTENS] for convenience later */
					Ctrl->I.mode = 0;	/* Flag we got a file */
				}
				else if ((Ctrl->I.value = atof (opt->arg)) < -1.0 || Ctrl->I.value > 1.0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -I: A constant intensity must be in the +-1 range\n");
					n_errors++;
				}
				else	/* Got a valid constant */
					Ctrl->I.mode = 1;
				break;

			case 'M':
				Ctrl->M.active = true;
				break;

			case 'N':
				Ctrl->N.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file[0], "Must specify at least one input raster file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active, "Can only use one of -C and -D\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->A.active, "Option -A: Not used with -D\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !(Ctrl->In.n_in == 1 || Ctrl->In.n_in == 3), "Option -C: Requires one or three rasters");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->In.n_in != 1, "Option -D: Only a single raster can be specified");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->D.active, "Option -I: Not available when -D is selected\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output raster file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->G.file && strstr (Ctrl->G.file, "%c") == NULL, "Option -G: With -D, output name must be template name containing %%c\n");

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

EXTERN_MSC int GMT_grdmix (void *V_API, int mode, void *args) {
	bool write_image = true;

	int error = 0;
	unsigned int img = 0, k, band;
	uint64_t row, col, node, pix;

	float *weights = NULL, *intens = NULL;

	double rgb[4];

	struct GMT_GRID *Gin[5] = {NULL, NULL, NULL, NULL, NULL}, *G = NULL;
	struct GMT_IMAGE *Iin[5] = {NULL, NULL, NULL, NULL, NULL}, *I = NULL;
	struct GMT_GRID_HEADER *h[5] = {NULL, NULL, NULL, NULL, NULL};
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

	GMT_Set_Default (API, "API_IMAGE_LAYOUT", "TRB ");	/* Easiest to manipulate images with separate bands */
	gmt_set_pad (GMT, 0); /* No padding needed in this module */

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input rasters\n");

	for (k = 0; k < 5; k++) {	/* Determine if we got grids or images */
		if (Ctrl->In.file[k] == NULL) continue;
		Ctrl->In.type[k] = gmt_raster_type (GMT, Ctrl->In.file[k]);
	}

	if (Ctrl->In.n_in == 1 && Ctrl->In.type[0] == GMT_NOTSET && !Ctrl->D.active) {
		GMT_Report (API, GMT_MSG_ERROR, "For a single input raster it must be an image\n");
		Return (GMT_RUNTIME_ERROR);		
	}

	for (k = 0; k < 5; k++) {	/* Read the headers (1-5 files) */
		if (Ctrl->In.file[k] == NULL) continue;
		if (Ctrl->In.type[k] == GMT_NOTSET) {	/* Got a grid */
			if ((Gin[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = Gin[k]->header;	/* Pointer to grid header */
		}
		else {	/* Got an image */
			if ((Iin[k] = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = Iin[k]->header;	/* Pointer to image header */
		}
	}

	if (Ctrl->In.n_in == 2 && Ctrl->In.type[0] != Ctrl->In.type[1]) {
		GMT_Report (API, GMT_MSG_ERROR, "Both inputs must either be images or grids, not a mix\n");
		Return (GMT_RUNTIME_ERROR);
	}
	for (k = 1; k < 5; k++) {	/* Make sure all rasters have matching dimensions */
		if (Ctrl->In.file[k] == NULL) continue;
		if (h[0]->registration != h[k]->registration || (h[0]->n_rows != h[k]->n_rows) || (h[0]->n_columns != h[k]->n_columns)) {
			GMT_Report (API, GMT_MSG_ERROR, "Dimensions/registrations of %s are not compatible for the other rasters!\n", Ctrl->In.file[k]);
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->In.type[0] == GMT_NOTSET)
			write_image = false;	/* Dealing with grids, so set a flag for our output */
		else	/* Get the array index to the image with the largest number of bands (should they differ) */
			if (h[k]->n_bands >= h[img]->n_bands) img = k;
	}

	/* Also make sure any given blend image/grid has the same dimensions */
	if (h[BLEND] && h[BLEND]->n_bands != 1) {
		GMT_Report (API, GMT_MSG_ERROR, "The blend image must be gray-scale only!\n");
		Return (GMT_RUNTIME_ERROR);
	}

	for (k = 0; k < 5; k++) {	/* Read data from the 1-5 files */
		if (Ctrl->In.file[k] == NULL) continue;
		if (Ctrl->In.type[k] == GMT_NOTSET) {	/* Read the grid */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[k], Gin[k]) == NULL) {	/* Get data only */
				Return (API->error);
			}
			if (Ctrl->N.active) {	/* Normalize the grid */
				gmt_M_grd_loop (GMT, Gin[k], row, col, node)
					Gin[k]->data[node] /= 255.0;
			}
		}
		else {	/* Read the image */
			if (GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[k], Iin[k]) == NULL) {	/* Get data only */
				Return (API->error);
			}
#if DEBUG
			if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) grdmix_dump_image (GMT, Iin[k], Ctrl->In.file[k]);
#endif
		}
	}

	if (Ctrl->A.active) {	/* Set up the blend weights */
		weights = gmt_M_memory (GMT, NULL, h[0]->size, float);
		if (Ctrl->A.mode == 0) {	/* Got a weights grid or image */
			if (Ctrl->In.type[BLEND] == GMT_NOTSET) {	/* Got a grid */
				uint64_t n_bad = 0;
				gmt_M_grd_loop (GMT, Gin[BLEND], row, col, node) {
					weights[node] = Gin[BLEND]->data[node];
					if (weights[node] < 0.0 || weights[node] > 1.0) n_bad++;
				}
				if (n_bad) {
					GMT_Report (API, GMT_MSG_ERROR, "Blend grid not in 0-1 range!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if (GMT_Destroy_Data (API, &Gin[BLEND]) != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to free grid from -A!\n");
					Return (GMT_RUNTIME_ERROR);
				}
			}
			else {	/* Got an image */
				gmt_M_grd_loop (GMT, Iin[BLEND], row, col, node)
					weights[node] = gmt_M_is255 (Iin[BLEND]->data[node]);
				if (GMT_Destroy_Data (API, &Iin[BLEND]) != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to free image from -A!\n");
					Return (GMT_RUNTIME_ERROR);
				}
			}
		}
		else {	/* Make a constant array of weights */
			for (node = 0; node < h[0]->size; node++)
				weights[node] = Ctrl->A.value;
		}
	}

	if (Ctrl->I.active) {	/* Set up the intensities */
		intens = gmt_M_memory (GMT, NULL, h[0]->size, float);
		if (Ctrl->I.mode == 0) {	/* Got a weights grid */
			uint64_t n_bad = 0;
			gmt_M_grd_loop (GMT, Gin[INTENS], row, col, node) {
				intens[node] = Gin[INTENS]->data[node];
				if (intens[node] < -1.0 || intens[node] > 1.0) n_bad++;
			}
			if (n_bad) {
				GMT_Report (API, GMT_MSG_ERROR, "Intens grid not in -1/+1 range!\n");
				Return (GMT_RUNTIME_ERROR);
			}
			if (GMT_Destroy_Data (API, &Gin[INTENS]) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to free grid from -I!\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else {	/* Make a constant array of intensities */
			for (node = 0; node < h[0]->size; node++)
				intens[node] = Ctrl->I.value;
		}
	}

	/* Deal with operations -C and -D up front, then exit */

	if (Ctrl->C.active) {	/* Combine 1 or 3 grids into a new single image, while handling the optional -A -I information */
		uint64_t dim[GMT_DIM_SIZE] = {h[0]->n_columns, h[0]->n_rows, Ctrl->In.n_in, 0};
		if ((I = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate an image for output!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
			for (band = 0; band < I->header->n_bands; band++)	/* March across the RGB values in both images and increment counters */
				rgb[band] = Gin[band]->data[node];
			if (Ctrl->I.active)	{	/* Modify colors based on intensity */
				if (Ctrl->In.n_in == 1)	/* Duplicate grays so illuminate can work */
				rgb[1] = rgb[2] = rgb[0];
				gmt_illuminate (GMT, intens[node], rgb);
			}
			pix = node;	/* Index to the red (or gray) pixel location */
			for (band = 0; band < I->header->n_bands; band++, pix += I->header->size)	/* March across the RGB values */
				I->data[pix] = gmt_M_u255 (rgb[band]);
		}
		if (Ctrl->A.active) {	/* Set transparency as well */
			if ((I->alpha = gmt_M_memory_aligned (GMT, NULL, I->header->size, unsigned char)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to allocate alpha layer\n");
				Return (GMT_RUNTIME_ERROR);
			}
			for (node = 0; node < h[0]->size; node++)	/* Scale transparency from 0-1 to 0-255 range */
				I->alpha[node] = gmt_M_u255 (weights[node]);
		}
		goto write_the_image;
	}

	if (Ctrl->D.active) {	/* De-construct single image into its gray or R,G,B [plus optional A] layers */
		char code[4] = {'R', 'G', 'B', 'A'}, file[PATH_MAX] = {""};
		uint64_t off = 0;
		double wesn[4], inc[2] = {1.0, 1.0};
		wesn[XLO] = wesn[YLO] = 0.0;
		wesn[XHI] = h[0]->n_columns;
		wesn[YHI] = h[0]->n_rows;
		if (h[0]->n_bands == 1) code[0] = 'g';	/* Grayscale */
		for (band = 0; band < h[0]->n_bands; band++) {	/* March across the RGB values in both images and increment counters */
			if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn, inc, GMT_GRID_PIXEL_REG, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a grid for output!\n");
				Return (GMT_RUNTIME_ERROR);
			}
			off = band * h[0]->size;
			gmt_M_grd_loop (GMT, G, row, col, node)
				G->data[node] = gmt_M_is255 (Iin[0]->data[node+off]);
			sprintf (file, Ctrl->G.file, code[band]);
			/* Write out grid */
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
				Return (API->error);
			}
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, G) != GMT_NOERROR) {
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to free grid!\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		Return (GMT_NOERROR);
	}

	if (Ctrl->In.type[0] == GMT_NOTSET) {	/* Create output grid for the blend */
		if ((G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Gin[0])) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate a grid for output!\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else {	/* Create output image for the blend or for the transparent image */
		if ((I = GMT_Duplicate_Data (API, GMT_IS_IMAGE, GMT_DUPLICATE_ALLOC, Iin[img])) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate an image for output!\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Now do the work which depends on whether we have 1 or 2 input items */

	if (Ctrl->In.n_in == 1) {	/* Pass intensity and/or transparency from -A to the input image */
		gmt_M_memcpy (I->data, Iin[0]->data, I->header->size * I->header->n_bands, char);	/* Duplicate the input image */
		/* Create alpha array for I and copy from A grid/image */
		if (Ctrl->A.active) { 	/* Add transparency layer */
			if ((I->alpha = gmt_M_memory_aligned (GMT, NULL, I->header->size, unsigned char)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to allocate alpha layer\n");
				Return (GMT_RUNTIME_ERROR);
			}
			for (node = 0; node < h[0]->size; node++)	/* Scale to 0-255 range */
				I->alpha[node] = gmt_M_u255 (weights[node]);
		}
		if (Ctrl->I.active) {	/* Modify colors by the intensity */
			gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
				for (band = 0; band < h[0]->n_bands; band++)	/* Get the normalized r,g,b triplet */
					rgb[band] = gmt_M_is255 (I->data[node+band*h[0]->size]);
				if (h[0]->n_bands == 1)	/* Duplicate grays so illuminate can work */
					rgb[1] = rgb[2] = rgb[0];
				gmt_illuminate (GMT, intens[node], rgb);
				for (band = 0, pix = node; band < h[0]->n_bands; band++, pix += h[0]->size)	/* March across the RGB values */
					I->data[pix] = gmt_M_u255 (rgb[band]);
			}
		}
	}
	else {	/* Blend two grids or images using the weights given via -A */
		if (Ctrl->In.type[0] == GMT_NOTSET) {	/* Two grids, just do the blend loop */
			gmt_M_grd_loop (GMT, G, row, col, node) {
				G->data[node] = weights[node] * (Gin[0]->data[node] - Gin[1]->data[node]) + Gin[1]->data[node];
			}
			/* Write out grid */
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
				Return (API->error);
			}
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		else {	/* Two images, do the r,g,b blend, watch out for any input transparency value to skip */
			int p0, p1, v;
			gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
				pix = node;	/* Index to the red (or gray) pixel location */
				for (band = 0; band < I->header->n_bands; band++) {	/* March across the RGB values in both images and increment counters */
					p0 = (band < Iin[0]->header->n_bands) ? Iin[0]->data[pix] : Iin[0]->data[node];	/* Get corresponding r,g,b or just duplicate gray three times */
					p1 = (band < Iin[1]->header->n_bands) ? Iin[1]->data[pix] : Iin[1]->data[node];	/* Get corresponding r,g,b or just duplicate gray three times */
					rgb[band] = gmt_M_is255 (weights[node] * (p0 - p1) + p1);
					pix += I->header->size;	/* Next band */
				}
				if (Ctrl->I.active)	/* Modify colors by the intensity */
					gmt_illuminate (GMT, intens[node], rgb);
				for (band = 0, pix = node; band < I->header->n_bands; band++, pix += I->header->size)	/* Place final r,g,b values */
					I->data[pix] = gmt_M_u255 (rgb[band]);
			}
		}
	}
	
write_the_image:

	if (Ctrl->A.active) gmt_M_free (GMT, weights);
	if (Ctrl->I.active) gmt_M_free (GMT, intens);

	for (k = 0; k < 3; k++) {	/* Free up memory no longer needed here */
		void *W = (Ctrl->In.type[k] == GMT_NOTSET) ? (void *)Gin[k] : (void *)Iin[k];
		if (W == NULL) continue;
		if (GMT_Destroy_Data (API, &W) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to free grid or image associated with file %s\n", Ctrl->In.file[k]);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (write_image) {	/* Write image here - any grid output was written earlier */
		/* Turn off the automatic creation of aux files by GDAL */
#ifdef WIN32
		if (_putenv ("GDAL_PAM_ENABLED=NO"))
#else
		if (setenv ("GDAL_PAM_ENABLED", "NO", 0))
#endif
			GMT_Report (API, GMT_MSG_WARNING, "Unable to set GDAL_PAM_ENABLED to prevent writing of auxiliary files\n");

		if (gmt_M_is_geographic (GMT, GMT_IN)) {
			char buf[GMT_LEN128] = {""};
			/* See if we have valid proj info the chosen projection has a valid PROJ4 setting */
			sprintf (buf, "+proj=longlat +a=%f +b%f +no_defs", GMT->current.setting.ref_ellipsoid[k].eq_radius,
				GMT->current.setting.ref_ellipsoid[k].eq_radius);
			if (I->header->ProjRefPROJ4 == NULL)
				I->header->ProjRefPROJ4 = strdup (buf);
		}

		if (Ctrl->M.active) {	/* Convert to monochrome image using the YIQ transformation first */
			unsigned char *data = NULL;
			if ((data = gmt_M_memory_aligned (GMT, NULL, I->header->size, unsigned char)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to allocate gray image layer\n");
				Return (GMT_RUNTIME_ERROR);
			}
			gmt_M_grd_loop (GMT, I, row, col, node) {	/* The node is one per pixel in a band, so stride into additional bands */
				for (band = 0, pix = node; band < I->header->n_bands; band++, pix += band*I->header->size)	/* Get the normalized r,g,b triplet */
					rgb[band] = gmt_M_is255 (I->data[pix]);
				data[node] = gmt_M_u255 (gmt_M_yiq (rgb));
			}
			gmt_M_free_aligned (GMT, I->data);	/* Free old rgb image and add gray image */
			I->data = data;
			I->header->n_bands = 1;
		}
		else
			GMT_Change_Layout (API, GMT_IS_IMAGE, "TRP", 0, I, NULL, NULL);		/* Convert from TRB to TRP (TRPa if there is alpha) */
		/* Write out image */
		if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, I) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	Return (GMT_NOERROR);
}
