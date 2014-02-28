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
 * Brief synopsis: grdpaste.c reads two grid files and writes a new file with
 * the first two pasted together along their common edge.
 *
 * Author:	Walter Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"grdpaste"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Join two grids along their common edge"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-Vf"

struct GRDPASTE_CTRL {
	struct In {
		bool active;
		char *file[2];
	} In;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
};

void *New_grdpaste_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDPASTE_CTRL *C = NULL;

	C = GMT_memory (GMT, NULL, 1, struct GRDPASTE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_grdpaste_Ctrl (struct GMT_CTRL *GMT, struct GRDPASTE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);	
	if (C->In.file[0]) free (C->In.file[0]);	
	if (C->In.file[1]) free (C->In.file[1]);	
	GMT_free (GMT, C);	
}

int GMT_grdpaste_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdpaste <grid1> <grid2> -G<outgrid> [%s] [%s]\n\n", GMT_V_OPT, GMT_f_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\twhere grids <grid1> and <grid2> are to be combined into <outgrid>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<grid1> and <grid2> must have same dx,dy and one edge in common.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tIf in doubt, run grdinfo first and check your files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tUse grdpaste and/or grdsample to adjust files as necessary.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,f,.");

	return (EXIT_FAILURE);
}

int GMT_grdpaste_parse (struct GMT_CTRL *GMT, struct GRDPASTE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdpaste and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_in = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_in == 0 && GMT_check_filearg (GMT, '<', opt->arg, GMT_IN))
					Ctrl->In.file[n_in++] = strdup (opt->arg);
				else if (n_in == 1 && GMT_check_filearg (GMT, '<', opt->arg, GMT_IN))
					Ctrl->In.file[n_in++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Only two files may be pasted\n");
				}
				break;

			/* Processes program-specific parameters */

 			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file[0] || !Ctrl->In.file[1], "Syntax error: Must specify two input files\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdpaste_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

/* True if grid is a COARDS/CF netCDF file */
static inline bool is_nc_grid (struct GMT_GRID *grid) {
	return
		grid->header->type == GMT_GRID_IS_NB ||
		grid->header->type == GMT_GRID_IS_NS ||
		grid->header->type == GMT_GRID_IS_NI ||
		grid->header->type == GMT_GRID_IS_NF ||
		grid->header->type == GMT_GRID_IS_ND;
}

int GMT_grdpaste (void *V_API, int mode, void *args)
{
	int error = 0, way;
	unsigned int one_or_zero;
	bool common_y = false;

	char format[GMT_BUFSIZ];

	double x_noise, y_noise;

	struct GMT_GRID *A = NULL, *B = NULL, *C = NULL;
	struct GRDPASTE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdpaste_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdpaste_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdpaste_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdpaste_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdpaste_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdpaste main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grids\n");
	GMT_set_pad (GMT, 0); /* No padding */

	/* Try to find a common side to join on  */

	if ((A = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file[0], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	if ((B = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file[1], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (A->header->registration != B->header->registration)
		error++;
	if ((A->header->z_scale_factor != B->header->z_scale_factor) || (A->header->z_add_offset != B->header->z_add_offset)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Scale/offset not compatible!\n");
		Return (EXIT_FAILURE);
	}

	if (! (fabs (A->header->inc[GMT_X] - B->header->inc[GMT_X]) < 1.0e-6 && fabs (A->header->inc[GMT_Y] - B->header->inc[GMT_Y]) < 1.0e-6)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Grid intervals do not match!\n");
		Return (EXIT_FAILURE);
	}

	if ((C = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, A)) == NULL) Return (API->error);	/* Just to get a header */
	
	one_or_zero = A->header->registration == GMT_GRID_NODE_REG;
	x_noise = GMT_SMALL * C->header->inc[GMT_X];
	y_noise = GMT_SMALL * C->header->inc[GMT_Y];

	common_y = (fabs (A->header->wesn[YLO] - B->header->wesn[YLO]) < y_noise && fabs (A->header->wesn[YHI] - B->header->wesn[YHI]) < y_noise);
	
	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Must be careful in determining a match since grids may differ by +/-360 in x */
		double del;
		if (common_y) {	/* A and B are side-by-side, may differ by +-360 +- 1 pixel width */
			del = A->header->wesn[XLO] - B->header->wesn[XHI];	/* Test if B left of A */
			if (del < (360.0 + C->header->inc[GMT_X] + x_noise) && del > (360.0 - C->header->inc[GMT_X] - x_noise)) {
				B->header->wesn[XLO] += 360.0;	B->header->wesn[XHI] += 360.0;
			}
			else if (del < (-360.0 + C->header->inc[GMT_X] + x_noise) && del > (-360.0 - C->header->inc[GMT_X] - x_noise)) {
				A->header->wesn[XLO] += 360.0;	A->header->wesn[XHI] += 360.0;
			}
			else {	/* Neither, check if A is left of B */
				del = B->header->wesn[XLO] - A->header->wesn[XHI];
				if (del < (360.0 + C->header->inc[GMT_X] + x_noise) && del > (360.0 - C->header->inc[GMT_X] - x_noise)) {
					A->header->wesn[XLO] += 360.0;	A->header->wesn[XHI] += 360.0;
				}
				else if (del < (-360.0 + C->header->inc[GMT_X] + x_noise) && del > (-360.0 - C->header->inc[GMT_X] - x_noise)) {
					B->header->wesn[XLO] += 360.0;	B->header->wesn[XHI] += 360.0;
				}
			}
		}
		else {	/* A and B are on top of each other, may differ by +-360 */
			del = A->header->wesn[XLO] - B->header->wesn[XLO];	/* Test if B left of A */
			if (del < (360.0 + x_noise) && del > (360.0 - x_noise)) {
				B->header->wesn[XLO] += 360.0;	B->header->wesn[XHI] += 360.0;
			}
			else if (del < (-360.0 + x_noise) && del > (-360.0 - x_noise)) {
				A->header->wesn[XLO] += 360.0;	A->header->wesn[XHI] += 360.0;
			}	
		}
	}

	GMT_memcpy (C->header->wesn, A->header->wesn, 4, double);	/* Output region is set as the same as A... */
	if (common_y) {

		C->header->ny = A->header->ny;

		if (fabs (A->header->wesn[XLO] - B->header->wesn[XHI]) < x_noise) {			/* A is on the right of B */
			way = 3;
			C->header->nx = A->header->nx + B->header->nx - one_or_zero;
			C->header->wesn[XLO] = B->header->wesn[XLO];			/* ...but not for west */
		}
		else if (fabs (A->header->wesn[XHI] - B->header->wesn[XLO]) < x_noise) {	/* A is on the left of B */
			way = 4;
			C->header->nx = A->header->nx + B->header->nx - one_or_zero;
			C->header->wesn[XHI] = B->header->wesn[XHI];			/* ...but not for east */
		}
		else if ((fabs (A->header->wesn[XLO] - B->header->wesn[XHI]) < (C->header->inc[GMT_X] + x_noise)) ) {
			/* A is on right of B but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 32;
			else                    /* Pixel registration - overlap */
				way = 33;
			C->header->nx = A->header->nx + B->header->nx - !one_or_zero;
			C->header->wesn[XLO] = B->header->wesn[XLO];			/* ...but not for west */
		}
		else if ((fabs (A->header->wesn[XHI] - B->header->wesn[XLO]) < (C->header->inc[GMT_X] + x_noise)) ) {
			/* A is on left of B but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 43;
			else                    /* Pixel registration - overlap */
				way = 44;
			C->header->nx = A->header->nx + B->header->nx - !one_or_zero;
			C->header->wesn[XHI] = B->header->wesn[XHI];			/* ...but not for east */
		}
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Grids do not share a common edge!\n");
			Return (EXIT_FAILURE);
		}
	}
	else if (fabs (A->header->wesn[XLO] - B->header->wesn[XLO]) < x_noise && fabs (A->header->wesn[XHI] - B->header->wesn[XHI]) < x_noise) {

		C->header->nx = A->header->nx;

		if (fabs (A->header->wesn[YHI] - B->header->wesn[YLO]) < y_noise) {			/* B is exactly on top of A */
			way = 1;
			C->header->ny = A->header->ny + B->header->ny - one_or_zero;
			C->header->wesn[YHI] = B->header->wesn[YHI];			/* ...but not for north */
		}
		else if (fabs (A->header->wesn[YLO] - B->header->wesn[YHI]) < y_noise) {	/* A is exactly on top of B */
			way = 2;
			C->header->ny = A->header->ny + B->header->ny - one_or_zero;
			C->header->wesn[YLO] = B->header->wesn[YLO];			/* ...but not for south */
		}
		else if ((fabs (A->header->wesn[YHI] - B->header->wesn[YLO]) < (C->header->inc[GMT_Y] + y_noise)) ) {
			/* B is on top of A but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 10;
			else                    /* Pixel registration - overlap */
				way = 11;
			C->header->ny = A->header->ny + B->header->ny - !one_or_zero;
			C->header->wesn[YHI] = B->header->wesn[YHI];			/* ...but not for north */
		}
		else if ((fabs (A->header->wesn[YLO] - B->header->wesn[YHI]) < (C->header->inc[GMT_Y] + y_noise)) ) {
			/* A is on top of B but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 21;
			else                    /* Pixel registration - overlap */
				way = 22;
			C->header->ny = A->header->ny + B->header->ny - !one_or_zero;
			C->header->wesn[YLO] = B->header->wesn[YLO];			/* ...but not for south */
		}
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Grids do not share a common edge!\n");
			Return (EXIT_FAILURE);
		}
	}
	else {
		GMT_Report (API, GMT_MSG_NORMAL, "Grids do not share a common edge!\n");
		Return (EXIT_FAILURE);
	}
	if (GMT_is_geographic (GMT, GMT_IN) && C->header->wesn[XHI] > 360.0) {	/* Take out 360 */
		C->header->wesn[XLO] -= 360.0;
		C->header->wesn[XHI] -= 360.0;
	}

	/* Now we can do it  */

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "%%s\t%s\t%s\t%s\t%s\t%s\t%s\t%%d\t%%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, "File\tW\tE\tS\tN\tdx\tdy\tnx\tny\n");
		GMT_Report (API, GMT_MSG_VERBOSE, format, Ctrl->In.file[0], A->header->wesn[XLO], A->header->wesn[XHI], A->header->wesn[YLO], A->header->wesn[YHI], A->header->inc[GMT_X], A->header->inc[GMT_Y], A->header->nx, A->header->ny);
		GMT_Report (API, GMT_MSG_VERBOSE, format, Ctrl->In.file[1], B->header->wesn[XLO], B->header->wesn[XHI], B->header->wesn[YLO], B->header->wesn[YHI], B->header->inc[GMT_X], B->header->inc[GMT_Y], B->header->nx, B->header->ny);
		GMT_Report (API, GMT_MSG_VERBOSE, format, Ctrl->G.file, C->header->wesn[XLO], C->header->wesn[XHI], C->header->wesn[YLO], C->header->wesn[YHI], C->header->inc[GMT_X], C->header->inc[GMT_Y], C->header->nx, C->header->ny);
	}

	GMT_set_grddim (GMT, C->header);
	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL, 0, 0, C) == NULL) Return (API->error);
	A->data = B->data = C->data;	/* A and B share the same final matrix declared for C */
	A->header->size = B->header->size = C->header->size;	/* Set A & B's size to the same as C */
	A->header->no_BC = B->header->no_BC = true;	/* We must disable the BC machinery */

	switch (way) {    /* How A and B are positioned relative to each other */
		case 1:         /* B is on top of A */
		case 10:		/* B is on top of A but their grid reg limits underlap by one cell */
		case 11:        /* B is on top of A but their pixel reg limits overlap by one cell */
			if (is_nc_grid(A)) {
				A->header->data_offset = B->header->nx * (B->header->ny - one_or_zero);
				if (way == 11)
					A->header->data_offset -= B->header->nx;
				else if (way == 10)
					A->header->data_offset += B->header->nx;
			}
			else {
				GMT->current.io.pad[YHI] = B->header->ny - one_or_zero;
				if (way == 11)
					GMT->current.io.pad[YHI]--;
				else if (way == 10)
					GMT->current.io.pad[YHI]++;
				GMT_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->my = C->header->my;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				GMT_set_pad (GMT, 0U); /* Reset padding */
			}
			else {
				GMT->current.io.pad[YHI] = 0;
				GMT->current.io.pad[YLO] = A->header->ny - one_or_zero;
				if (way == 11)
					GMT->current.io.pad[YLO]--;
				else if (way == 10)
					GMT->current.io.pad[YLO]++;
				GMT_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->my = C->header->my;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
		case 2:         /* A is on top of B */
		case 21:        /* A is on top of B but their grid reg limits underlap by one cell */
		case 22:        /* A is on top of B but their pixel reg limits overlap by one cell */
			if (!is_nc_grid(A)) {
				GMT->current.io.pad[YLO] = B->header->ny - one_or_zero;
				if (way == 22)
					GMT->current.io.pad[YLO]--;
				else if (way == 21)
					GMT->current.io.pad[YLO]++;
				GMT_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->my = C->header->my;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				GMT_set_pad (GMT, 0U); /* Reset padding */
				B->header->data_offset = A->header->nx * (A->header->ny - one_or_zero);
				if (way == 22)
					B->header->data_offset -= A->header->nx;
				else if (way == 21)
					B->header->data_offset += A->header->nx;
			}
			else {
				GMT->current.io.pad[YLO] = 0;
				GMT->current.io.pad[YHI] = A->header->ny - one_or_zero;
				if (way == 22)
					GMT->current.io.pad[YHI]--;
				else if (way == 21)
					GMT->current.io.pad[YHI]++;
				GMT_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->my = C->header->my;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
		case 3:         /* A is on the right of B */
		case 32:        /* A is on right of B but their grid reg limits underlap by one cell */
		case 33:        /* A is on right of B but their pixel reg limits overlap by one cell */
			if (is_nc_grid(A)) {
				A->header->stride = C->header->nx;
				A->header->data_offset = B->header->nx - one_or_zero;
				if (way == 33)
					A->header->data_offset--;
				else if (way == 32)
					A->header->data_offset++;
			}
			else {
				GMT->current.io.pad[XLO] = B->header->nx - one_or_zero;
				if (way == 33)
					GMT->current.io.pad[XLO]--;
				else if (way == 32)
					GMT->current.io.pad[XLO]++;
				GMT_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->mx = C->header->mx;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				GMT_set_pad (GMT, 0U); /* Reset padding */
				B->header->stride = C->header->nx;
			}
			else {
				GMT->current.io.pad[XLO] = 0; GMT->current.io.pad[XHI] = A->header->nx - one_or_zero;
				if (way == 33)
					GMT->current.io.pad[XHI]--;
				else if (way == 32)
					GMT->current.io.pad[XHI]++;
				GMT_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->mx = C->header->mx;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
		case 4:         /* A is on the left of B */
		case 43:        /* A is on left of B but their grid reg limits underlap by one cell */
		case 44:        /* A is on left of B but their pixel reg limits overlap by one cell */
			if (is_nc_grid(A)) {
				A->header->stride = C->header->nx;
			}
			else {
				GMT->current.io.pad[XHI] = B->header->nx - one_or_zero;
				if (way == 44)
					GMT->current.io.pad[XHI]--;
				else if (way == 43)
					GMT->current.io.pad[XHI]++;
				GMT_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->mx = C->header->mx;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				GMT_set_pad (GMT, 0U); /* Reset padding */
				B->header->stride = C->header->nx;
				B->header->data_offset = A->header->nx - one_or_zero;
				if (way == 44)
					B->header->data_offset--;
				else if (way == 43)
					B->header->data_offset++;
			}
			else {
				GMT->current.io.pad[XHI] = 0;
				GMT->current.io.pad[XLO] = A->header->nx - one_or_zero;
				if (way == 44)
					GMT->current.io.pad[XLO]--;
				else if (way == 43)
					GMT->current.io.pad[XLO]++;
				GMT_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->mx = C->header->mx;		/* Needed if grid is read by GMT_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
	}

	GMT_set_pad (GMT, 0U); /* Reset padding */
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, C)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, C) != GMT_OK) {
		Return (API->error);
	}
	A->data = B->data = NULL; /* Since these were never actually allocated */

	GMT_set_pad (GMT, API->pad); /* Restore to GMT Defaults */
	Return (GMT_OK);
}
