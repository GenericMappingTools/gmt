/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * it flagged by NaNs and interpolates across the holes given a choice
 * of algorithm.
 *
 * Author:	Paul Wessel
 * Date:	7-JAN-2018
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grdfill"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Interpolate across holes in a grid"
#define THIS_MODULE_KEYS	"<G{,>G}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-RVf"

enum GRDFILL_mode {
	ALG_CONSTANT,
	ALG_NN,
	ALG_SPLINE
};

struct GRDFILL_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A<algo>[<options>] */
		bool active;
		unsigned int mode;
		double value;
	} A;
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
	GMT_Message (API, GMT_TIME_NONE, "usage: grdfill <ingrid> [-A<mode><options>] [-G<outgrid>] [-L]\n\t[%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is the grid file with NaN-holes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify algorithm and parameters for in-fill:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c<value> Fill in NaNs with the constant <value>.\n");
//	GMT_Message (API, GMT_TIME_NONE, "\t   n<radius> Fill in NaNs with nearest neighbor values;\n");
//	GMT_Message (API, GMT_TIME_NONE, "\t   append <max_radius> nodes for the outward search.\n");
//	GMT_Message (API, GMT_TIME_NONE, "\t   [Default radius is sqrt(xn^2+by^2)]\n");
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

			case 'A':
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'c':	/* Constant fill */
						Ctrl->A.mode = ALG_CONSTANT;
						Ctrl->A.value = atof (&opt->arg[1]);
						break;
					case 'n':	/* Nearest neighbor fill */
						Ctrl->A.mode = ALG_NN;
						Ctrl->A.value = (opt->arg[1]) ? atof (&opt->arg[1]) : -1;
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A: Unrecognized algorithm (%c)\n", opt->arg[0]);
						n_errors++;
				}
				break;

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

GMT_LOCAL void do_constant_fill (struct GMT_GRID *G, unsigned int limit[], gmt_grdfloat value) {
	/* Algorithm 1: Replace NaNs with a constant value */
	unsigned int row, col;
	uint64_t node;
	
	for (row = limit[YLO]; row < limit[YHI]; row++) {
		for (col = limit[XLO]; col < limit[XHI]; col++) {
			node = gmt_M_ijp (G->header, row, col);
			if (gmt_M_is_fnan (G->data[node]))
				G->data[node] = value;
		}
	}
}

#if 0
GMT_LOCAL void do_splinefill (struct GMT_GRID *G, double wesn[], unsigned int limit[], unsigned int n_in_hole, double value) {
	/* Algorithm 2: Replace NaNs with a spline */
	unsigned int row, col, row_hole, col_hole, mode, d_limit[4];
	uint64_t node, node_hole, dim[GMT_DIM_SIZE] = {0, 0, 0, 0};
	double *x = NULL, *y = NULL;
	gmt_grdfloat *z = NULL;
	struct GMT_VECTOR *V = NULL;
	struct GMT_GRID *G_hole = NULL;
	char input[GMT_STR16] = {""}, output[GMT_STR16] = {""}, args[GMT_LEN256] = {""};
	
	/* Allocate a vector container for input to greenspline */
	dim[0] = 3;	/* Want three input columns but let length be 0 - this signals that no vector allocations should take place */
	if ((V = GMT_Create_Data (API, GMT_IS_VECTOR, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) GMT_exit (API->GMT, EXIT_FAILURE);
	/* Create a virtual file to hold the resampled grid */
	if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, out_string) == GMT_NOTSET) {
		Return (API->error);
	}
	/* Add up to 2 rows/cols around hole, but watch for grid edges */
	gmt_M_memcpy (d_limit, limit, 4, unsigned int);	/* d_limit will be used to set the grid domain */
	for (k = 1; k <= 2; k++) {
		if (d_limit[XLO]) d_limit[XLO]--, wesn[XLO] -= G->header->inc[GMT_X];	/* Move one column westward */
		if (d_limit[XHI] < (G->header->n_columns-1)) d_limit[XHI]++, wesn[XHI] += G->header->inc[GMT_X];	/* Move one column eastward */
		if (d_limit[YLO]) d_limit[YLO]--, wesn[YLO] -= G->header->inc[GMT_Y];	/* Move one row northward */
		if (d_limit[YHI] < (G->header->n_rows-1)) d_limit[YHI]++, wesn[YHI] += G->header->inc[GMT_Y];	/* Move one row southward */
	}
	n_constraints = (d_limit[YHI] - d_limit[YLO] - 1) * (d_limit[XHI] - d_limit[XLO] - 1) - n_in_hole;
	x = gmt_M_memory (GMT, NULL, n_constraints, double);
	y = gmt_M_memory (GMT, NULL, n_constraints, double);
	z = gmt_M_memory (GMT, NULL, n_constraints, gmt_grdfloat);
	for (row = d_limit[YLO]; row < d_limit[YHI]; row++) {
		for (col = d_limit[XLO]; col < d_limit[XHI]; col++) {
			node = gmt_M_ijp (G->header, row, col);
			if (gmt_M_is_fnan (G->data[node])) continue;
			x[k] = gmt_M_grd_col_to_x (GMT, col, G->header);
			y[k] = gmt_M_grd_row_to_y (GMT, row, G->header);
			z[k] = G->data[node];
		}
	}
	GMT_Put_Vector (API, V, GMT_X, GMT_DOUBLE, x);
	GMT_Put_Vector (API, V, GMT_Y, GMT_DOUBLE, y);
	GMT_Put_Vector (API, V, GMT_Z, GMT_FLOAT,  z);
	V->n_rows = n_constraints;	/* Must specify how many input points we have */
   /* Associate our input data vectors with a virtual input file */
    GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, V, input);
	/* Prepare the greenspline command-line arguments */
	mode = (gmt_M_geographic (GMT, GMT_IN)) ? 2 : 1;
    sprintf (args, "%s -G%s -Sc -R%g/%g/%g/%g -I%g/%g -D%d", input, output, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], G->header->inc[GMT_X], G->header->inc[GMT_Y]);
	if (G->header->registration == GMT_GRID_PIXEL_REG) strcat (args, " -r";)
	strcat (args, " --GMT_HISTORY=false");
   	/* Run the greenspline module */
	GMT_Report (API, GMT_MSG_VERBOSE, "Calling greenspline with args %s\n", args);
  	if (GMT_Call_Module (API, "greenspline", GMT_MODULE_CMD, args)) GMT_exit (API->GMT, EXIT_FAILURE);
	if ((G_hole = GMT_Read_VirtualFile (API, out_string)) == NULL) {	/* Load in the resampled grid */
		Return (API->error);
	}
	/* Use 0-nx,0-ny for the hold index and the large grid G nodes for the holes */
	for (row_hole = 0, row = d_limit[YLO]; row_hole < G_hole->header->n_rows; row_hole++, row++) {
		for (col_hole = 0, col = d_limit[XLO]; col_hole < G_hole->header->n_columns; col_hole++, col++) {
			node = gmt_M_ijp (G->header, row, col);
			if (!gmt_M_is_fnan (G->data[node])) continue;
			node_hole = gmt_M_ijp (G_hole->header, row_hole, col_hole);
			G->data[node] = G_hole->data[node_hole];
		}
	}

	/* Close the two virtual files */
	GMT_Close_VirtualFile (API, output);
	GMT_Close_VirtualFile (API, input);
	/* Free our custom vectors */
	gmt_m_free (GMT, x);	gmt_m_free (GMT, y);	gmt_m_free (GMT, z);
}
#endif

GMT_LOCAL unsigned int trace_the_hole (struct GMT_GRID *G, uint64_t node, unsigned int row, unsigned int col, int64_t *step, char *ID, unsigned int limit[]) {
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
			if (next_rcol < limit[XLO]) limit[XLO] = next_rcol;	/* Update min/max row and col values */
			else if (next_rcol > limit[XHI]) limit[XHI] = next_rcol;
			if (next_row < limit[YLO]) limit[YLO] = next_row;
			else if (next_row > limit[YHI]) limit[YHI] = next_row;
			/* Recursively trace this nodes next neighbors as well */
			n_nodes = n_nodes + 1 + trace_the_hole (G, ij, next_row, next_rcol, step, ID, limit);
		}
	}
	return (n_nodes);
}

GMT_LOCAL int64_t find_nearest (int64_t i, int64_t j, int64_t *r2, int64_t *is, int64_t *js, int64_t *xs, int64_t *ys) {
	/* function to find the nearest point based on previous search, smallest distance ourside a radius */
	int64_t ct = 0, nx, ny, nx1, ii, k = 0, rr;
    
	rr = *r2 + 1e7;
    
	/* starting with nx = ny */
	nx = (int64_t)(sqrt((double)(*r2)/2.0));
	/* loop over possible nx, find smallest rr */
	for (nx1 = nx; nx1 <= (int64_t)sqrt((double)(*r2))+1; nx1++) {
		if (nx1*nx1 < *r2)
			ny = (int64_t)(sqrt((*r2)-nx1*nx1));
		else
			ny = 0;
		while (nx1*nx1+ny*ny <= (*r2) && ny<=nx1)
			ny++;
		if (ny <= nx1) {
			if (rr > (nx1*nx1+ny*ny)) {
				k = 0;
				rr = nx1*nx1+ny*ny;
				xs[k] = nx1;
				ys[k] = ny;
			}
			else if (rr == (nx1*nx1+ny*ny)) {
				k++;
				xs[k] = nx1;
				ys[k] = ny;
			}
		}
	}

	/* Return the grid index, changing the order may lead to different solution, but mainly because nearest-neighbor is non-unique */
	for (ii = 0; ii <= k; ii++) {
		nx = xs[ii];
		ny = ys[ii];

		if (ny == 0) {
			js[ct+0] = 0;  js[ct+1] = 0;   js[ct+2] = nx; js[ct+3] = -nx;
			is[ct+0] = nx; is[ct+1] = -nx; is[ct+2] = 0;  is[ct+3] = 0;
			ct += 4;
		}
		else if (nx != ny){
			js[ct+0] = ny; js[ct+1] = ny;  js[ct+2] = -ny; js[ct+3] = -ny; js[ct+4] = nx; js[ct+5] = -nx; js[ct+6] = nx;  js[ct+7] = -nx;
			is[ct+0] = nx; is[ct+1] = -nx; is[ct+2] = nx;  is[ct+3] = -nx; is[ct+4] = ny; is[ct+5] = ny;  is[ct+6] = -ny; is[ct+7] = -ny; 
			ct += 8;
		}
		else {
			js[ct+0] = nx; js[ct+1] = nx; js[ct+2] = -nx;  js[ct+3] = -nx;
			is[ct+0] = nx; is[ct+1] = -nx;  is[ct+2] = nx; is[ct+3] = -nx;
			ct += 4;
		}
 	}

	for (ii = 0; ii < ct; ii++) {	/* Set absolute row/col values */
		is[ii] = is[ii] + i;
		js[ii] = js[ii] + j;
	}

	*r2 = rr;	/* Update radius */

	/* return the number of indexes of the nearest neighbor */
	return (ct);
}

GMT_LOCAL void nearest_interp (struct GMT_CTRL *GMT, struct GMT_GRID *In, struct GMT_GRID *Out, int64_t radius) {
	uint64_t ij, node;
	int64_t nx = In->header->n_columns, ny = In->header->n_rows;
 	int64_t i, j, flag, ct, k, kt, kk = 1, recx = 1, recy = 1, cs = 0, rr;
 	int64_t *is = NULL, *js = NULL, *xs = NULL, *ys = NULL;
	float *m = In->data, *m_interp = Out->data;	/* Short-hand for input and output grids */
	double rad2;
 	
 	/* Allocate memory for temporary indexes */
 	is = gmt_M_memory (GMT, NULL, 2048, int64_t);
 	js = gmt_M_memory (GMT, NULL, 2048, int64_t);
 	xs = gmt_M_memory (GMT, NULL, 512, int64_t);
 	ys = gmt_M_memory (GMT, NULL, 512, int64_t);

	if (radius == -1)	/* Set default radius */
		radius = (int64_t)floor (sqrt (nx&nx + ny*ny));
	rad2 = (double)(radius * radius);
	
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Interpolating to nearest neighbour...\n");
	gmt_M_row_loop (GMT, In, i) {	/* Loop over each row in grid */
 		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Working on row %" PRIi64 "\n", i);
		kk = floor(log10 ((double)(i+1))) + 1 + 4;
		rr = 0; 
		gmt_M_col_loop (GMT, In, i, j, ij) {	/* Loop over all columns */
			if (!gmt_M_is_fnan (m[ij]))	/* Already duplicated in the calling program */
				rr = 0;
			else {	/* search nearest neighbor, use previous nearest distance to exclude certain search area. */
				flag = 0;
				/* set the starting search radius based on last nearest distance */
				if (rr >= 4 && recy > 0 && recx > 0) rr = (recx-1)*(recx-1)+(recy-1)*(recy-1)-1;
				else if (rr >= 4 && recy == 0 && recx > 0) rr = (recx-1)*(recx-1)-1;
				else if (rr >= 4 && recy > 0 && recx == 0) rr = (recy-1)*(recy-1)-1;
				else rr = 0;

				while (flag == 0 && rr <= rad2) {
					ct = find_nearest (i, j, &rr, is, js, xs, ys);
					cs++;
					if (rr <= rad2) {
						for (k = 0; k < ct; k++) {
							if (is[k] >= 0 && is[k] < ny && js[k] >=0 && js[k] < nx) {
								node = gmt_M_ijp (In->header, is[k], js[k]);
								if (!gmt_M_is_fnan (m[node])) {
									m_interp[ij] = m[node];
									flag = 1;
									kt = k;
									recx = labs (is[k]-i);
									recy = labs (js[k]-j);
									break;
								}
							}
						}
					}
				}
			}
			if (i == 0 && rr == -1)
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "(%d %d %d %d)\n", j, rr, recx, recy);
		}
	}

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "%" PRIi64 " number of searches used\n", cs);

	gmt_M_free (GMT, is);
	gmt_M_free (GMT, js);
	gmt_M_free (GMT, xs);
	gmt_M_free (GMT, ys);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdfill (void *V_API, int mode, void *args) {
	char *ID = NULL;
	int error = 0;
	unsigned int hole_number = 0, row, col, limit[4], n_nodes;
	uint64_t node, offset;
	int64_t off[4];
	double wesn[4];
	//char command[GMT_GRID_COMMAND_LEN320] = {""};
	struct GMT_GRID *Grid = NULL;
	struct GMT_RECORD *Out = NULL;
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdfill main code ----------------------------*/

	if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (GMT->common.R.active[RSET]) {	/* Specified a subset */
		bool global = false;
		global = gmt_grd_is_global (GMT, Grid->header);
		if (!global && (GMT->common.R.wesn[XLO] < Grid->header->wesn[XLO] || GMT->common.R.wesn[XHI] > Grid->header->wesn[XHI])) error++;
		if (GMT->common.R.wesn[YLO] < Grid->header->wesn[YLO] || GMT->common.R.wesn[YHI] > Grid->header->wesn[YHI]) error++;
		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "Subset exceeds data domain!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, GMT->common.R.wesn, Ctrl->In.file, Grid) == NULL) {
			Return (API->error);	/* Get subset */
		}
	}
	else if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file, Grid) == NULL) {
		Return (API->error);	/* Get all */
	}
	
	if (Ctrl->A.mode == ALG_NN) {	/* Do Eric Xu's NN algorithm */
		int64_t radius = lrint (Ctrl->A.value);
		struct GMT_GRID *New = NULL;
		if ((New = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, Grid)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to duplicate input grid!\n");
			Return (API->error);	/* Get subset */
		}
		//nearest_interp (nx,ny,m,m_interp,rr);	/* Perform the NN replacements */
		nearest_interp (GMT, Grid, New, radius);	/* Perform the NN replacements */
		
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, New)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to write output grid!\n");
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	/* To avoid having to check every row,col for being inside the grid we set
	 * the boundary row/cols in the ID grid to 1. */
	
	ID = gmt_M_memory_aligned (GMT, NULL, Grid->header->size, char);
	/* Set the top and bottom boundary rows to UINT_MAX */
	offset = (uint64_t)(Grid->header->pad[YHI] + Grid->header->n_rows) * Grid->header->mx;
	for (node = 0; node < (uint64_t)Grid->header->pad[YHI]*Grid->header->mx; node++) ID[node] = ID[node+offset] = 1;
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
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	
	Out = gmt_new_record (GMT, wesn, NULL);	/* Since we only need to worry about numerics in this module */
	gmt_M_grd_loop (GMT, Grid, row, col, node) {	/* Loop over all grid nodes */
		if (ID[node]) continue;	/* Already identified as part of a hole */
		if (gmt_M_is_fnan (Grid->data[node])) {	/* Node is part of a new hole */
			limit[XLO] = limit[XHI] = col;	/* Initiate min/max col to this single node */
			limit[YLO] = limit[YHI] = row;	/* Initiate min/max row to this single node */
			ID[node] = 1;	/* Flag this node as part of a hole */
			++hole_number;	/* Increase the current hole number */
			/* Trace all the contiguous neighbors, updating the bounding box as we go along */
			n_nodes = 1 + trace_the_hole (Grid, node, row, col, off, ID, limit);
			limit[XHI]++;	limit[YHI]++;	/* Allow for the other side of the last pixel */
			wesn[XLO] = gmt_M_col_to_x (GMT, limit[XLO], Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], 0, Grid->header->n_columns);
			wesn[XHI] = gmt_M_col_to_x (GMT, limit[XHI], Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], 0, Grid->header->n_columns);
			wesn[YLO] = gmt_M_row_to_y (GMT, limit[YHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], 0, Grid->header->n_columns);
			wesn[YHI] = gmt_M_row_to_y (GMT, limit[YLO], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], 0, Grid->header->n_columns);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Hole BB %u: -R: %g/%g/%g/%g [%u nodes]\n", hole_number, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], n_nodes);
			if (Ctrl->L.active) {
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			else {
				switch (Ctrl->A.mode) {
					case ALG_CONSTANT:	/* Fill in using a constant value */
						do_constant_fill (Grid, limit, (gmt_grdfloat)Ctrl->A.value);
						break;
					case ALG_SPLINE:	/* Fill in using a spline */
						//do_splinefill (Grid, wesn, limit, n_nodes, Ctrl->A.value);
						break;
				}
			}
		}
	}
	if (hole_number) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %u holes\n", hole_number);
	gmt_M_free_aligned (GMT, ID);
	gmt_M_free (GMT, Out);

	if (Ctrl->L.active) {
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
	}
	else if (hole_number) {	/* Must write the revised grid if there were any holes*/
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid))
			Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR)
			Return (API->error);
	}
	else {
		GMT_Report (API, GMT_MSG_VERBOSE, "No holes detected in grid - grid was not updated\n");
	}
	
	Return (GMT_NOERROR);
}
