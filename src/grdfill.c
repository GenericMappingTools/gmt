/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: grdfill.c reads a grid that has one or more holes in
 * it flagged by NaNs and interpolates across the holes.
 *
 * Author:	Paul Wessel
 * Date:	7-FEB-2017
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"grdfill"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Interpolate across holes in a grid"
#define THIS_MODULE_KEYS	"<G{,>G}"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-RVf"

struct GRDFILL_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct G {	/* -G<outgrid> */
		bool active;
		char *file;
	} G;
	struct L {	/* -L */
		bool active;
	} L;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFILL_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRDFILL_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDFILL_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);	
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdfill <ingrid> [-G<outgrid>] [-L]\n\t[%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is the grid file with NaN-holes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G <outgrid> is the file to write the filled-in grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Just list the subregions w/e/s/n of each hole.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   No grid fill takes place and -G is ignored.\n");
	GMT_Option (API, "R,V,f,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDFILL_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdfill and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input and Output files */
				/* Since grdfill allowed output grid to be given without -G we must actually
				 * check for two input files and assign the 2nd as the actual output file */
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) {
					Ctrl->In.file= strdup (opt->arg);
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Cannot find input file %s\n", opt->arg);
					n_errors++;
				}
				break;
			case '>':	/* Output file  */
				Ctrl->G.active = true;
				if (gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_GRID))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'G':
				Ctrl->G.active = true;
				if (Ctrl->G.file) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Specify only one output file\n");
					n_errors++;
				}
				else
					Ctrl->G.file = strdup (opt->arg);
				break;

			case 'L':
				Ctrl->L.active = true;
				break;
	
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input grid file\n");
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->L.active || Ctrl->G.file), "Syntax error: Must specify output grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

unsigned int trace_the_hole (struct GMT_GRID *G, uint64_t node, unsigned int row, unsigned int col, int64_t *step, char *ID, unsigned int *xlow, unsigned int *xhigh, unsigned int *ylow, unsigned int *yhigh) {
	/* Determine all the direct neighbor nodes in the W/E/S/N directions that are also NaN, recursively.
	 * This is a limited form of Moore neighborhood called Von Neumann neighborhoold since we only do the
	 * four cardinal directions and ignore the diagonals.  Ignoring the diagonal means two holes that
	 * touch at a diagonal will be encoded as two separate holes whereas a full Moore neighborhood
	 * calculation would find a single hole.  For our purposes (filling in the hole), smaller holes
	 * are less computationally intensive, hence we limit ourselves to good old Von Neumann. */
	static int drow[4] = {1, 0, -1, 0}, dcol[4] = {0, 1, 0, -1};	/* Change in row,col per cardinal direction */
	unsigned int side, next_row, next_rcol, n_nodes = 0;
	int64_t ij;
	
	for (side = 0; side < 4; side++) {	/* For each of the 4 cardinal directions */
		ij = node + step[side];	/* This is the grid node (and ID node) of this cardinal point */
		if (ID[ij] == 0 && gmt_M_is_fnan (G->data[ij])) {	/* Hole extends in this direction, follow it to its next neighbor */
			next_row  = (unsigned int)(row + drow[side]);	/* Set col and row of next point */
			next_rcol = (unsigned int)(col + dcol[side]);
			ID[ij] = 1;	/* Mark this node as part of the current hole */
			if (next_rcol < *xlow) *xlow = next_rcol;	/* Update min/max row and col values */
			else if (next_rcol > *xhigh) *xhigh = next_rcol;
			if (next_row < *ylow) *ylow = next_row;
			else if (next_row > *yhigh) *yhigh = next_row;
			/* Recursively trace this nodes next neighbors as well */
			n_nodes = n_nodes + 1 + trace_the_hole (G, ij, next_row, next_rcol, step, ID, xlow, xhigh, ylow, yhigh);
		}
	}
	return (n_nodes);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdfill (void *V_API, int mode, void *args) {
	char *ID = NULL;
	int error = 0;
	unsigned int hole_number = 0, row, col, xlow, xhigh, ylow, yhigh, n_nodes;
	uint64_t node, offset;
	int64_t off[4];
	double wesn[4];
	//char command[GMT_GRID_COMMAND_LEN320] = {""};
	struct GMT_GRID *Grid = NULL;
	struct GRDFILL_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdfill main code ----------------------------*/

	if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (GMT->common.R.active) {	/* Specified a subset */
		bool global = false;
		global = gmt_M_grd_is_global (GMT, Grid->header);
		if (!global && (GMT->common.R.wesn[XLO] < Grid->header->wesn[XLO] || GMT->common.R.wesn[XHI] > Grid->header->wesn[XHI])) error++;
		if (GMT->common.R.wesn[YLO] < Grid->header->wesn[YLO] || GMT->common.R.wesn[YHI] > Grid->header->wesn[YHI]) error++;
		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "Subset exceeds data domain!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, GMT->common.R.wesn, Ctrl->In.file, Grid) == NULL) {
			Return (API->error);	/* Get subset */
		}
	}
	else if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file, Grid) == NULL) {
		Return (API->error);	/* Get all */
	}

	/* To avoid having to check every row,col for being inside the grid we set
	 * the boundary row/cols in the ID grid to 1. */
	
	ID = gmt_M_memory_aligned (GMT, NULL, Grid->header->size, char);
	/* Set the top and bottom boundary rows to UINT_MAX */
	offset = (Grid->header->pad[YHI] + Grid->header->n_rows) * Grid->header->mx;
	for (node = 0; node < Grid->header->pad[YHI]*Grid->header->mx; node++) ID[node] = ID[node+offset] = 1;
	/* Set the left and right boundary columnss to UINT_MAX */
	offset = Grid->header->pad[XLO] + Grid->header->n_columns;
	for (row = 0; row < Grid->header->my; row++) {
		for (col = 0; col < Grid->header->pad[XLO]; col++)
			ID[row*Grid->header->mx+col] = 1;
		for (col = 0; col < Grid->header->pad[XHI]; col++)
			ID[row*Grid->header->mx+offset+col] = 1;
	}
	/* Initiate the node offsets in the cardinal directions */
	off[0] = Grid->header->mx;	off[1] = 1; 	off[2] = -off[0]; off[3] = -off[1];
	
	if (Ctrl->L.active) {
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		if ((error = gmt_set_cols (GMT, GMT_OUT, 4)) != GMT_NOERROR) {	/* We don't really care or know about columns so must use 1 */
			Return (API->error);
		}
	}
	
	gmt_M_grd_loop (GMT, Grid, row, col, node) {	/* Loop over all grid nodes */
		if (ID[node]) continue;	/* Already identified as part of a hole */
		if (gmt_M_is_fnan (Grid->data[node])) {	/* Node is part of a new hole */
			xlow = xhigh = col;	/* Initiate min/max col to this single node */
			ylow = yhigh = row;	/* Initiate min/max row to this single node */
			ID[node] = 1;	/* Flag this node as part of a hole */
			++hole_number;	/* Increase the current hole number */
			/* Trace all the contiguous neighbors, updating the bounding box as we go along */
			n_nodes = 1 + trace_the_hole (Grid, node, row, col, off, ID, &xlow, &xhigh, &ylow, &yhigh);
			xhigh++;	yhigh++;	/* Allow for the other side of the last pixel */
			wesn[XLO] = gmt_M_col_to_x (GMT, xlow,  Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], 0, Grid->header->n_columns);
			wesn[XHI] = gmt_M_col_to_x (GMT, xhigh, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], 0, Grid->header->n_columns);
			wesn[YLO] = gmt_M_row_to_y (GMT, yhigh, Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], 0, Grid->header->n_columns);
			wesn[YHI] = gmt_M_row_to_y (GMT, ylow,  Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], 0, Grid->header->n_columns);
			GMT_Report (API, GMT_MSG_VERBOSE, "Hole BB %u: -R: %g/%g/%g/%g [%u nodes]\n", hole_number, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], n_nodes);
			if (Ctrl->L.active) {
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, wesn);
			}
		}
	}

	if (Ctrl->L.active) {
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Found %u holes\n", hole_number);
	gmt_M_free_aligned (GMT, ID);
	
#if 0
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid))
		Return (API->error);

	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, hmode, NULL, Ctrl->G.file, Grid) != GMT_NOERROR)
		Return (API->error);
#endif
	Return (GMT_NOERROR);
}
