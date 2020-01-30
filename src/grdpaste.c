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
 * Brief synopsis: grdpaste.c reads two grid files and writes a new file with
 * the first two pasted together along their common edge.
 *
 * Author:	Walter Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdpaste"
#define THIS_MODULE_MODERN_NAME	"grdpaste"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Join two grids along their common edge"
#define THIS_MODULE_KEYS	"<G{2,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vf"

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

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDPASTE_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDPASTE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDPASTE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->In.file[GMT_IN]);
	gmt_M_str_free (C->In.file[GMT_OUT]);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid1> <grid2> -G<outgrid> [%s] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\twhere grids <grid1> and <grid2> are to be combined into <outgrid>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<grid1> and <grid2> must have same dx,dy and one edge in common.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tIf in doubt, run grdinfo first and check your files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tUse grdpaste and/or grdsample to adjust files as necessary.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tIf grids are geographic and adds to full 360-degree range then grid1\n");
	GMT_Message (API, GMT_TIME_NONE, "\tdetermines west.  Use grdedit -S to rotate grid to another -Rw/e/s/n.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output grid file.\n");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,f,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDPASTE_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (n_in == 0 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[n_in++] = strdup (opt->arg);
				else if (n_in == 1 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[n_in++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Only two files may be pasted\n");
				}
				break;

			/* Processes program-specific parameters */

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

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file[0] || !Ctrl->In.file[1], "Must specify two input files\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* True if grid is a COARDS/CF netCDF file */
GMT_LOCAL inline bool is_nc_grid (struct GMT_GRID *grid) {
	return
		grid->header->type == GMT_GRID_IS_NB ||
		grid->header->type == GMT_GRID_IS_NS ||
		grid->header->type == GMT_GRID_IS_NI ||
		grid->header->type == GMT_GRID_IS_NF ||
		grid->header->type == GMT_GRID_IS_ND;
}

int GMT_grdpaste (void *V_API, int mode, void *args) {
	int error = 0, way = 0;
	unsigned int one_or_zero;
	bool common_y = false;

	char format[GMT_BUFSIZ];

	double x_noise, y_noise;

	struct GMT_GRID *A = NULL, *B = NULL, *C = NULL;
	struct GMT_GRID_HEADER_HIDDEN *AH = NULL, *BH = NULL;
	struct GRDPASTE_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the grdpaste main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grids\n");
	gmt_set_pad (GMT, 0); /* No padding */

	/* Try to find a common side to join on  */

	if ((A = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[0], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	if ((B = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[1], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (A->header->registration != B->header->registration)
		error++;
	if ((A->header->z_scale_factor != B->header->z_scale_factor) || (A->header->z_add_offset != B->header->z_add_offset)) {
		GMT_Report (API, GMT_MSG_ERROR, "Scale/offset not compatible!\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (! (fabs (A->header->inc[GMT_X] - B->header->inc[GMT_X]) < 1.0e-6 && fabs (A->header->inc[GMT_Y] - B->header->inc[GMT_Y]) < 1.0e-6)) {
		GMT_Report (API, GMT_MSG_ERROR, "Grid intervals do not match!\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if ((C = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, A)) == NULL) Return (API->error);	/* Just to get a header */

	one_or_zero = A->header->registration == GMT_GRID_NODE_REG;
	x_noise = GMT_CONV4_LIMIT * C->header->inc[GMT_X] * 10;
	y_noise = GMT_CONV4_LIMIT * C->header->inc[GMT_Y] * 10;

	AH = gmt_get_H_hidden (A->header);
	BH = gmt_get_H_hidden (B->header);

	common_y = (fabs (A->header->wesn[YLO] - B->header->wesn[YLO]) < y_noise && fabs (A->header->wesn[YHI] - B->header->wesn[YHI]) < y_noise);

	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must be careful in determining a match since grids may differ by +/-360 in x */
		double del;
		if (common_y) {	/* A and B are side-by-side, may differ by +-360 +- 1 pixel width */
			del = A->header->wesn[XLO] - B->header->wesn[XHI];	/* Test if B left of A */
			if (doubleAlmostEqual (fabs (del), 360.0)) {	/* Let A remain left of B */
				/* Since new range is a full 360 we always let the first grid decide west boundary.
				 * If this is not desirable the user should switch A and B or sue grdedit -S */
				way = 4;
			}
			else if (del < (360.0 + C->header->inc[GMT_X] + x_noise) && del > (360.0 - C->header->inc[GMT_X] - x_noise)) {
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

	gmt_M_memcpy (C->header->wesn, A->header->wesn, 4, double);	/* Output region is set as the same as A... */
	if (common_y) {

		C->header->n_rows = A->header->n_rows;

		if (way == 4 || fabs (A->header->wesn[XHI] - B->header->wesn[XLO]) < x_noise) {	/* A is on the left of B */
			way = 4;
			C->header->n_columns = A->header->n_columns + B->header->n_columns - one_or_zero;
			C->header->wesn[XHI] = B->header->wesn[XHI];			/* ...but not for east */
		}
		else if (fabs (A->header->wesn[XLO] - B->header->wesn[XHI]) < x_noise) {			/* A is on the right of B */
			way = 3;
			C->header->n_columns = A->header->n_columns + B->header->n_columns - one_or_zero;
			C->header->wesn[XLO] = B->header->wesn[XLO];			/* ...but not for west */
		}
		else if ((fabs (A->header->wesn[XLO] - B->header->wesn[XHI]) < (C->header->inc[GMT_X] + x_noise)) ) {
			/* A is on right of B but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 32;
			else                    /* Pixel registration - overlap */
				way = 33;
			C->header->n_columns = A->header->n_columns + B->header->n_columns - !one_or_zero;
			C->header->wesn[XLO] = B->header->wesn[XLO];			/* ...but not for west */
		}
		else if ((fabs (A->header->wesn[XHI] - B->header->wesn[XLO]) < (C->header->inc[GMT_X] + x_noise)) ) {
			/* A is on left of B but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 43;
			else                    /* Pixel registration - overlap */
				way = 44;
			C->header->n_columns = A->header->n_columns + B->header->n_columns - !one_or_zero;
			C->header->wesn[XHI] = B->header->wesn[XHI];			/* ...but not for east */
		}
		else {
			GMT_Report (API, GMT_MSG_ERROR, "Grids do not share a common edge!\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else if (fabs (A->header->wesn[XLO] - B->header->wesn[XLO]) < x_noise && fabs (A->header->wesn[XHI] - B->header->wesn[XHI]) < x_noise) {

		C->header->n_columns = A->header->n_columns;

		if (fabs (A->header->wesn[YHI] - B->header->wesn[YLO]) < y_noise) {			/* B is exactly on top of A */
			way = 1;
			C->header->n_rows = A->header->n_rows + B->header->n_rows - one_or_zero;
			C->header->wesn[YHI] = B->header->wesn[YHI];			/* ...but not for north */
		}
		else if (fabs (A->header->wesn[YLO] - B->header->wesn[YHI]) < y_noise) {	/* A is exactly on top of B */
			way = 2;
			C->header->n_rows = A->header->n_rows + B->header->n_rows - one_or_zero;
			C->header->wesn[YLO] = B->header->wesn[YLO];			/* ...but not for south */
		}
		else if ((fabs (A->header->wesn[YHI] - B->header->wesn[YLO]) < (C->header->inc[GMT_Y] + y_noise)) ) {
			/* B is on top of A but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 10;
			else                    /* Pixel registration - overlap */
				way = 11;
			C->header->n_rows = A->header->n_rows + B->header->n_rows - !one_or_zero;
			C->header->wesn[YHI] = B->header->wesn[YHI];			/* ...but not for north */
		}
		else if ((fabs (A->header->wesn[YLO] - B->header->wesn[YHI]) < (C->header->inc[GMT_Y] + y_noise)) ) {
			/* A is on top of B but their pixel|grid reg limits under|overlap by one cell */
			if (one_or_zero)        /* Grid registration - underlap */
				way = 21;
			else                    /* Pixel registration - overlap */
				way = 22;
			C->header->n_rows = A->header->n_rows + B->header->n_rows - !one_or_zero;
			C->header->wesn[YLO] = B->header->wesn[YLO];			/* ...but not for south */
		}
		else {
			GMT_Report (API, GMT_MSG_ERROR, "Grids do not share a common edge!\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else {
		GMT_Report (API, GMT_MSG_ERROR, "Grids do not share a common edge!\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_M_is_geographic (GMT, GMT_IN) && C->header->wesn[XHI] > 360.0) {	/* Take out 360 */
		C->header->wesn[XLO] -= 360.0;
		C->header->wesn[XHI] -= 360.0;
	}

	/* Now we can do it  */

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		sprintf (format, "%%s\t%s\t%s\t%s\t%s\t%s\t%s\t%%d\t%%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, "File\tW\tE\tS\tN\tdx\tdy\tnx\tny\n");
		GMT_Report (API, GMT_MSG_INFORMATION, format, Ctrl->In.file[0], A->header->wesn[XLO], A->header->wesn[XHI], A->header->wesn[YLO], A->header->wesn[YHI], A->header->inc[GMT_X], A->header->inc[GMT_Y], A->header->n_columns, A->header->n_rows);
		GMT_Report (API, GMT_MSG_INFORMATION, format, Ctrl->In.file[1], B->header->wesn[XLO], B->header->wesn[XHI], B->header->wesn[YLO], B->header->wesn[YHI], B->header->inc[GMT_X], B->header->inc[GMT_Y], B->header->n_columns, B->header->n_rows);
		GMT_Report (API, GMT_MSG_INFORMATION, format, Ctrl->G.file, C->header->wesn[XLO], C->header->wesn[XHI], C->header->wesn[YLO], C->header->wesn[YHI], C->header->inc[GMT_X], C->header->inc[GMT_Y], C->header->n_columns, C->header->n_rows);
	}

	gmt_set_grddim (GMT, C->header);
	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL,
		NULL, 0, 0, C) == NULL) Return (API->error);	/* Note: 0 for pad since no BC work needed */
	A->data = B->data = C->data;	/* A and B share the same final matrix declared for C */
	A->header->size = B->header->size = C->header->size;	/* Set A & B's size to the same as C */
	AH->no_BC = BH->no_BC = true;	/* We must disable the BC machinery */

	switch (way) {    /* How A and B are positioned relative to each other */
		case 1:         /* B is on top of A */
		case 10:		/* B is on top of A but their grid reg limits underlap by one cell */
		case 11:        /* B is on top of A but their pixel reg limits overlap by one cell */
			if (is_nc_grid(A)) {
				AH->data_offset = B->header->n_columns * (B->header->n_rows - one_or_zero);
				if (way == 11)
					AH->data_offset -= B->header->n_columns;
				else if (way == 10)
					AH->data_offset += B->header->n_columns;
			}
			else {
				GMT->current.io.pad[YHI] = B->header->n_rows - one_or_zero;
				if (way == 11)
					GMT->current.io.pad[YHI]--;
				else if (way == 10)
					GMT->current.io.pad[YHI]++;
				gmt_M_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->my = C->header->my;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				gmt_set_pad (GMT, 0U); /* Reset padding */
			}
			else {
				GMT->current.io.pad[YHI] = 0;
				GMT->current.io.pad[YLO] = A->header->n_rows - one_or_zero;
				if (way == 11)
					GMT->current.io.pad[YLO]--;
				else if (way == 10)
					GMT->current.io.pad[YLO]++;
				gmt_M_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->my = C->header->my;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
		case 2:         /* A is on top of B */
		case 21:        /* A is on top of B but their grid reg limits underlap by one cell */
		case 22:        /* A is on top of B but their pixel reg limits overlap by one cell */
			if (!is_nc_grid(A)) {
				GMT->current.io.pad[YLO] = B->header->n_rows - one_or_zero;
				if (way == 22)
					GMT->current.io.pad[YLO]--;
				else if (way == 21)
					GMT->current.io.pad[YLO]++;
				gmt_M_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->my = C->header->my;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				gmt_set_pad (GMT, 0U); /* Reset padding */
				BH->data_offset = A->header->n_columns * (A->header->n_rows - one_or_zero);
				if (way == 22)
					BH->data_offset -= A->header->n_columns;
				else if (way == 21)
					BH->data_offset += A->header->n_columns;
			}
			else {
				GMT->current.io.pad[YLO] = 0;
				GMT->current.io.pad[YHI] = A->header->n_rows - one_or_zero;
				if (way == 22)
					GMT->current.io.pad[YHI]--;
				else if (way == 21)
					GMT->current.io.pad[YHI]++;
				gmt_M_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->my = C->header->my;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
		case 3:         /* A is on the right of B */
		case 32:        /* A is on right of B but their grid reg limits underlap by one cell */
		case 33:        /* A is on right of B but their pixel reg limits overlap by one cell */
			if (is_nc_grid(A)) {
				AH->stride = C->header->n_columns;
				AH->data_offset = B->header->n_columns - one_or_zero;
				if (way == 33)
					AH->data_offset--;
				else if (way == 32)
					AH->data_offset++;
			}
			else {
				GMT->current.io.pad[XLO] = B->header->n_columns - one_or_zero;
				if (way == 33)
					GMT->current.io.pad[XLO]--;
				else if (way == 32)
					GMT->current.io.pad[XLO]++;
				gmt_M_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->mx = C->header->mx;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				gmt_set_pad (GMT, 0U); /* Reset padding */
				BH->stride = C->header->n_columns;
			}
			else {
				GMT->current.io.pad[XLO] = 0; GMT->current.io.pad[XHI] = A->header->n_columns - one_or_zero;
				if (way == 33)
					GMT->current.io.pad[XHI]--;
				else if (way == 32)
					GMT->current.io.pad[XHI]++;
				gmt_M_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->mx = C->header->mx;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
		case 4:         /* A is on the left of B */
		case 43:        /* A is on left of B but their grid reg limits underlap by one cell */
		case 44:        /* A is on left of B but their pixel reg limits overlap by one cell */
			if (is_nc_grid(A)) {
				AH->stride = C->header->n_columns;
			}
			else {
				GMT->current.io.pad[XHI] = B->header->n_columns - one_or_zero;
				if (way == 44)
					GMT->current.io.pad[XHI]--;
				else if (way == 43)
					GMT->current.io.pad[XHI]++;
				gmt_M_grd_setpad (GMT, A->header, GMT->current.io.pad);
			}
			A->header->mx = C->header->mx;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[0], A) == NULL) {  /* Get data from A */
				Return (API->error);
			}
			if (is_nc_grid(B)) {
				gmt_set_pad (GMT, 0U); /* Reset padding */
				BH->stride = C->header->n_columns;
				BH->data_offset = A->header->n_columns - one_or_zero;
				if (way == 44)
					BH->data_offset--;
				else if (way == 43)
					BH->data_offset++;
			}
			else {
				GMT->current.io.pad[XHI] = 0;
				GMT->current.io.pad[XLO] = A->header->n_columns - one_or_zero;
				if (way == 44)
					GMT->current.io.pad[XLO]--;
				else if (way == 43)
					GMT->current.io.pad[XLO]++;
				gmt_M_grd_setpad (GMT, B->header, GMT->current.io.pad);
			}
			B->header->mx = C->header->mx;		/* Needed if grid is read by gmt_gdal_read_grd */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file[1], B) == NULL) {  /* Get data from B */
				Return (API->error);
			}
			break;
	}

	gmt_set_pad (GMT, 0U); /* Reset padding */
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, C)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, C) != GMT_NOERROR) {
		Return (API->error);
	}
	A->data = B->data = NULL; /* Since these were never actually allocated */

	gmt_set_pad (GMT, API->pad); /* Restore to GMT Defaults */
	Return (GMT_NOERROR);
}
