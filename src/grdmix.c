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
 * Brief synopsis: grdmix.c reads two grid files and writes a new file with
 * the first two pasted together along their common edge.
 *
 * Author:	Walter Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdmix"
#define THIS_MODULE_MODERN_NAME	"grdmix"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Transparency-blending of grids or images"
#define THIS_MODULE_KEYS	"<G{2,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vf"

struct GRDMIX_CTRL {
	struct GRDMIX_In {
		bool active;
		unsigned int n_in;
		char *file[2];
	} In;
	struct GRDMIX_A {
		bool active;
		unsigned int mode;	/* 0 a file, 1 a constant */
		char *file;
		double alpha;
	} A;
	struct GRDMIX_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
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
	gmt_M_str_free (C->In.file[GMT_IN]);
	gmt_M_str_free (C->In.file[GMT_OUT]);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid|img1> [<grid|img2>] -A<wgrid|img> -G<outgrid|img> [%s] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\twhere items <grid1> and <grid2> are to be combined into <outgrid>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<grid1> and <grid2> must have same dx,dy and one edge in common.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tIf in doubt, run grdinfo first and check your files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output grid file.\n");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,f,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDMIX_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdmix and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (Ctrl->In.n_in == 0 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->Ctrl->In.n_in.file[Ctrl->In.n_in++] = strdup (opt->arg);
				else if (n_in == 1 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[Ctrl->In.n_in++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Only two files may be pasted\n");
				}
				break;

			/* Processes program-specific parameters */

 			case 'A':
				if ((Ctrl->A.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0) {
					Ctrl->A.file = strdup (opt->arg);
					Ctrl->A.mode = 0;
				}
				else if ((Ctrl->A.alpha = atof (opt->arg)) < 0.0 || Ctrl->A.alpha > 1.0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -A: A constant must be in the 0-1 range\n");
					n_errors++;
				}
				else
					Ctrl->A.mode = 1;
				break;


 			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file[0], "Must specify at least one input file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->A.active, "Option -A: Must specify blend grid, image or constant\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* True if grid is a COARDS/CF netCDF file */
GMT_LOCAL inline bool grdmix_is_nc_grid (struct GMT_GRID *grid) {
	return
		grid->header->type == GMT_GRID_IS_NB ||
		grid->header->type == GMT_GRID_IS_NS ||
		grid->header->type == GMT_GRID_IS_NI ||
		grid->header->type == GMT_GRID_IS_NF ||
		grid->header->type == GMT_GRID_IS_ND;
}

EXTERN_MSC int GMT_grdmix (void *V_API, int mode, void *args) {
	int error = 0, type;
	uint64_t row, col, node, k;

	char format[GMT_BUFSIZ];

	float *weights = NULL, w1, w2;

	void *W = NULL;

	struct GMT_GRID *GIn[2] = {NULL,  NULL}, *G = NULL, *Wg = NULL;
	struct GMT_IMAGE *IIn[2] = {NULL,  NULL}, *I = NULL, *Wi = NULL;
	struct GMT_GRIDHEADER *h[2] = {NULL, NULL};
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

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grids\n");
	gmt_set_pad (GMT, 0); /* No padding */

	for (k = 0; k < Ctrl->In.n_in; k++) {	/* Determine if a grid or image */
		Ctrl->In.type[k] = gmt_raster_type (GMT, Ctrl->In.file[k]);
	}

	if (Ctrl->In.n_in == 1 && Ctrl->In.type[0] == GMT_NOTSET) {
		GMT_Report (API, GMT_MSG_ERROR, "For a single input raster it must be an image\n");
		Return (GMT_RUNTIME_ERROR);		
	}

	for (k = 0; k < Ctrl->In.n_in; k++) {	/* Read headers */
		if (Ctrl->In.type[k] == GMT_NOTSET) {
			if ((Gin[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = Gin[k]->header;
		}
		else {
			if ((Iin[k] = GMT_Read_Data (API, GMT_IS_IMAGE GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = Iin[k]->header;
		}
	}
	if (Ctrl->In.n_in == 2) {
		if (h[0]->registration != h[1]->registration || (h[0]->n_rows != h[1]->n_rows) || (h[0]->n_columns != h[1]->n_columns)) {
			GMT_Report (API, GMT_MSG_ERROR, "Dimension/registrations are not compatible!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->In.type[0] != Ctrl->In.type[1]) {
			GMT_Report (API, GMT_MSG_ERROR, "Both inputs must either be images or grids, not a mix\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->A.mode == 0) {
		struct GMT_GRIDHEADER *ht = NULL;
		type = gmt_raster_type (GMT, Ctrl->A.file);
		if (type == GMT_NOTSET) {
			if ((Wg = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->A.file, NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			weights = Wg->data;
			ht = Wg->header;
			W = Wg;
		}
		else {	/* Read image header */
			if ((Wi = GMT_Read_Data (API, GMT_IS_IMAGE GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->A.file, NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			ht = Wt->header;
			if (ht->n_bands != 1) {
				GMT_Report (API, GMT_MSG_ERROR, "The blend image must be grayscale only!\n");
				Return (GMT_RUNTIME_ERROR);
			}
			weights = gmt_M_memory (GMT, NULL, ht->size, float);
			gmt_M_img_loop (GMT, Wi, row, col, node) {
				weights[node] = gmt_M_is255 (Wi->data[node]);
			}
			W = Wi;
		}
		if (h[0]->registration != ht->registration || (h[0]->n_rows != ht->n_rows) || (h[0]->n_columns != ht->n_columns)) {
			GMT_Report (API, GMT_MSG_ERROR, "Dimension/registrations are not compatible with blend grid/image!\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->In.type[0] == GMT_NOTSET) {	/* Create output grid */
		if ((G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Gin[0])) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate a grid for output!\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else {	/* Create output image */
		if ((I = GMT_Duplicate_Data (API, GMT_IS_IMAGE, GMT_DUPLICATE_ALLOC, Iin[0])) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to duplicate an image for output!\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Load in the grids or images */

	for (k = 0; k < Ctrl->In.n_in; k++) {	/* Read data */
		if (Ctrl->In.type[k] == GMT_NOTSET) {
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[k], Gin[k]) == NULL) {	/* Get data only */
				Return (API->error);
			}
		}
		else {
			if (GMT_Read_Data (API, GMT_IS_IMAGE GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], Iin[k]) == NULL) {	/* Get data only */
				Return (API->error);
			}
		}
	}

	/* Now do the work which depends on 1 or 2 input items */

	if (Ctrl->In.n_in == 1) {	/* Add transparency from -A to the input image */
		/* Create alpha array for I and copy from A grid */
		/* Write out image */
		if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, I) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Blend two grids or images using the weights given via -A */
		w1 = Ctrl->A.alpha;	w2 = 1.0 - w1;	/* Initialize in case we were given a constant */
		if (Ctrl->In.type[k] == GMT_NOTSET) {	/* Two grids, just do the blend loop */
			gmt_M_grd_loop (GMT, G, row, col, node) {
				if (Ctrl->A.mode == 0) {
					w1 = weights[node];
					w2 = 1.0 - w1;
				}
				G->data[node] = Gin[0]->data[node] * w1 + Gin[1]->data[node] * w2;
			}
			/* Write out grid */
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
				Return (API->error);
			}
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		else {	/* Images, do the r,g,b blend */
			gmt_M_grd_loop (GMT, W, row, col, node) {
				if (Ctrl->A.mode == 0) {
					w1 = weights[node];
					w2 = 1.0 - w1;
				}
				for (k = 0; k < 3; k++)
					I->data[node+k] = (unsigned char)urint(Iin[0]->data[node+k] * w1 + Iin[1]->data[node+k] * w2);
			}
			/* Write out image */
			if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, I) != GMT_NOERROR) {
				Return (API->error);
			}
		}
	}

	if (type != GMT_NOTSET) gmt_M_free (GMT, weights);
	Return (GMT_NOERROR);
}
