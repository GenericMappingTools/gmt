/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: read a grid file and find the values which divide its range
 * into n_cell number of quantiles.
 *
 * Author:	W.H.F. Smith
 * Date: 	31 May 1990
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdhisteq"
#define THIS_MODULE_MODERN_NAME	"grdhisteq"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Perform histogram equalization for a grid"
#define THIS_MODULE_KEYS	"<G{,GG},DD)"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-RVh"

struct GRDHISTEQ_CTRL {
	struct GRDHISTEQ_In {
		bool active;
		char *file;
	} In;
	struct GRDHISTEQ_C {	/* -C<n_cells>*/
		bool active;
		unsigned int value;
	} C;
	struct GRDHISTEQ_D {	/* -D[<file>] */
		bool active;
		char *file;
	} D;
	struct GRDHISTEQ_G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct GRDHISTEQ_N {	/* -N[<norm>] */
		bool active;
		double norm;
	} N;
	struct GRDHISTEQ_Q {	/* -Q */
		bool active;
	} Q;
};

struct INDEXED_DATA {
	gmt_grdfloat x;
	uint64_t i;
};

struct GRDHISTEQ_CELL {
	unsigned int row;
	gmt_grdfloat low;
	gmt_grdfloat high;
};

EXTERN_MSC int gmtlib_compare_observation (const void *a, const void *b);

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDHISTEQ_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDHISTEQ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.value = 16;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDHISTEQ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s [-C<n_cells>] [-D[<table>]] [-G%s] [-N[<norm>]] [-Q] "
		"[%s] [%s] [%s] [%s]\n", name, GMT_INGRID, GMT_OUTGRID, GMT_Rgeo_OPT, GMT_V_OPT, GMT_ho_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of input grid");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-C<n_cells>");
	GMT_Usage (API, -2, "Set how many cells (divisions) of data range to make [16].");
	GMT_Usage (API, 1, "\n-D[<table>]");
	GMT_Usage (API, -2, "Dump level information to <table> or standard output if not given.");
	gmt_outgrid_syntax (API, 'G', "Create an equalized output grid file called <outgrid>");
	GMT_Usage (API, 1, "\n-N[<norm>]");
	GMT_Usage (API, -2, "Use with -G to make an output grid file with standard normal scores. "
		"Alternatively, append <norm> to normalize the scores to -<norm>/+<norm>.");
	GMT_Usage (API, 1, "\n-Q Use quadratic equalization scaling [Default is linear].");
	GMT_Option (API, "R,V,h,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDHISTEQ_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdhisteq and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int sval;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) {n_errors++; continue; }
				Ctrl->In.active = true;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Get # of cells */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				sval = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, sval <= 0, "Option -C: n_cells must be positive\n");
				Ctrl->C.value = sval;
				break;
			case 'D':	/* Dump info to file or stdout */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				break;
			case 'G':	/* Output file for equalized grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'N':	/* Get normalized scores */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				if (opt->arg[0]) Ctrl->N.norm = atof (opt->arg);
				break;
			case 'Q':	/* Use quadratic scaling */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Must specify a single input grid file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Must specify input grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->G.active, "Option -N: Must also specify output grid file with -G\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->Q.active, "Option -N: Cannot be combined with -Q\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->C.value <= 0, "Option -C: n_cells must be positive\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->D.active && !Ctrl->G.active, "Either -D or -G is required for output\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.file && !strcmp (Ctrl->In.file, "="), "Piping of input grid file not supported!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL gmt_grdfloat grdhisteq_get_cell (gmt_grdfloat x, struct GRDHISTEQ_CELL *cell, unsigned int n_cells_m1, unsigned int last_cell) {
	unsigned int low, high, i;
	/* Quick bisector search for relevant cell */
	low = 0;
	high = n_cells_m1;
	i = last_cell;

	do {
		if (cell[i].low <= x && cell[i].high >= x) {
			return ((gmt_grdfloat)i);
		}
		else if (cell[low].low <= x && cell[low].high >= x) {
			return ((gmt_grdfloat)low);
		}
		else if (cell[high].low <= x && cell[high].high >= x) {
			return ((gmt_grdfloat)high);
		}
		else if (cell[i].low > x) {
			high = i;
			i = (low + high) / 2;
		}
		else if (cell[i].high < x) {
			low = i;
			i = (low + high) / 2;
		}
	} while (true);
	return (0.0);	/* Cannot get here - just used to quiet compiler */
}

GMT_LOCAL struct GRDHISTEQ_CELL *grdhisteq_do_hist_equalization_cart (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *outfile, unsigned int n_cells, bool quadratic) {
	/* Do basic Cartesian histogram equalization */
	uint64_t i, j, nxy;
	unsigned int n_cells_m1 = 0, current_cell, pad[4];
	double delta_cell, target;
	struct GRDHISTEQ_CELL *cell = NULL;
	struct GMT_GRID *Orig = NULL;

	cell = gmt_M_memory (GMT, NULL, n_cells, struct GRDHISTEQ_CELL);

	/* Sort the data and find the division points */

	gmt_M_memcpy (pad, Grid->header->pad, 4, int);	/* Save the original pad */
	gmt_grd_pad_off (GMT, Grid);	/* Undo pad if one existed so we can sort the entire grid */
	if (outfile && (Orig = GMT_Duplicate_Data (GMT->parent, GMT_IS_GRID, GMT_DUPLICATE_DATA, Grid)) == NULL) {	/* Must keep original if readonly */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid duplication failed - memory error?\n");
		gmt_M_free (GMT, cell);
		return (NULL);
	}
	gmt_sort_array (GMT, Grid->data, Grid->header->nm, GMT_FLOAT);

	nxy = Grid->header->nm;
	while (nxy > 0 && gmt_M_is_fnan (Grid->data[nxy-1])) nxy--;	/* Only deal with real numbers since NaNs will be at end */

	n_cells_m1 = n_cells - 1;
	current_cell = 0;
	i = 0;
	delta_cell = ((double)nxy) / ((double)n_cells);

	while (current_cell < n_cells) {

		if (current_cell == n_cells_m1)
			j = nxy - 1;
		else if (quadratic) {	/* Use y = 2x - x**2 scaling  */
			target = (current_cell + 1.0) / n_cells;
			j = lrint (floor (nxy * (1.0 - sqrt (1.0 - target))));
		}
		else	/* Use simple linear scale  */
			j = lrint (floor ((current_cell + 1) * delta_cell)) - 1;

		cell[current_cell].low  = Grid->data[i];
		cell[current_cell].high = Grid->data[j];
		cell[current_cell].row  = current_cell;

		i = j;
		current_cell++;
	}

	if (outfile) {	/* Must re-read the grid and evaluate since it got sorted and trodden on... */
		unsigned int last_cell = n_cells / 2;
		for (i = 0; i < Grid->header->nm; i++) Grid->data[i] = (gmt_M_is_fnan (Orig->data[i])) ? GMT->session.f_NaN : grdhisteq_get_cell (Orig->data[i], cell, n_cells_m1, last_cell);
		if (GMT_Destroy_Data (GMT->parent, &Orig) != GMT_NOERROR) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to free Orig\n");
		}
	}

	gmt_grd_pad_on (GMT, Grid, pad);	/* Reinstate the original pad */

	return (cell);	/* Pass out the cell boundaries */
}

GMT_LOCAL struct GRDHISTEQ_CELL *grdhisteq_do_hist_equalization_geo (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *outfile, unsigned int n_cells, bool quadratic) {
	/* Do basic area-weighted histogram equalization */
	uint64_t i, j, node, nxy = 0;
	openmp_int row, col;
	unsigned int n_cells_m1 = 0, current_cell;
	double cell_w, delta_w, target_w, wsum = 0.0;
	struct GRDHISTEQ_CELL *cell = gmt_M_memory (GMT, NULL, n_cells, struct GRDHISTEQ_CELL);
	struct GMT_GRID *W = gmt_duplicate_grid (GMT, Grid, GMT_DUPLICATE_ALLOC);
	struct GMT_OBSERVATION *pair = gmt_M_memory (GMT, NULL, Grid->header->nm, struct GMT_OBSERVATION);

	/* Determine the area weights */
	gmt_get_cellarea (GMT, W);
	/* Fill in the observation (w,z) observation pairs and find # of points nxy */
	gmt_M_grd_loop (GMT, Grid, row, col, node) {
		if (gmt_M_is_fnan (Grid->data[node]) || gmt_M_is_dnan (W->data[node]))
			continue;
		pair[nxy].value    = Grid->data[node];
		pair[nxy++].weight = W->data[node];
		wsum += W->data[node];
	}
	gmt_free_grid (GMT, &W, true);	/* Done with the area weights grid */
	/* Sort observations on z */
	qsort (pair, nxy, sizeof (struct GMT_OBSERVATION), gmtlib_compare_observation);
	/* Compute normalized cumulative weights */
	wsum = 1.0 / wsum;	/* Do avoid division later */
	pair[0].weight *= (gmt_grdfloat)wsum;
	for (i = 1; i < nxy; i++) {
		pair[i].weight *= (gmt_grdfloat)wsum;
		pair[i].weight += pair[i-1].weight;
	}

	/* Find the division points using the normalized 0-1 weights */

	n_cells_m1 = n_cells - 1;
	current_cell = 0;
	i = j = 0;
	cell_w = delta_w = 1.0 / n_cells;
	while (current_cell < n_cells) {

		if (current_cell == n_cells_m1)
			j = nxy - 1;	/* End at the last sorted point */
		else if (quadratic) {	/* Use y = 2x - x**2 scaling to stretch the target weight at end of box */
			target_w = (1.0 - sqrt (1.0 - cell_w));
			while (j < nxy && pair[j].weight < target_w) j++;
		}
		else	/* Use simple linear scale  */
			while (j < nxy && pair[j].weight < cell_w) j++;

		cell[current_cell].low  = pair[i].value;
		cell[current_cell].high = pair[j].value;
		cell[current_cell].row  = current_cell;

		i = j;
		current_cell++;
		cell_w += delta_w;
	}

	if (outfile) {	/* Evaluate grid given its original values */
		unsigned int last_cell = n_cells / 2;
		gmt_M_grd_loop (GMT, Grid, row, col, node) {
			Grid->data[node] = (gmt_M_is_fnan (Grid->data[node])) ? GMT->session.f_NaN : grdhisteq_get_cell (Grid->data[node], cell, n_cells_m1, last_cell);
		}
	}

	gmt_M_free (GMT, pair);

	return (cell);	/* Pass out the cell boundaries */
}

GMT_LOCAL struct GRDHISTEQ_CELL * grdhisteq_do_hist_equalization (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *outfile, unsigned int n_cells, bool quadratic) {
	struct GRDHISTEQ_CELL *C;
	if (gmt_M_is_geographic (GMT, GMT_IN))
		C = grdhisteq_do_hist_equalization_geo (GMT, Grid, outfile, n_cells, quadratic);
	else
		C = grdhisteq_do_hist_equalization_cart (GMT, Grid, outfile, n_cells, quadratic);
	return (C);
}

GMT_LOCAL int grdhisteq_compare_indexed_floats (const void *point_1, const void *point_2) {
	if (((struct INDEXED_DATA *)point_1)->x < ((struct INDEXED_DATA *)point_2)->x) return (-1);
	if (((struct INDEXED_DATA *)point_1)->x > ((struct INDEXED_DATA *)point_2)->x) return (+1);
	return (0);
}

GMT_LOCAL int grdhisteq_compare_indices (const void *point_1, const void *point_2) {
	if (((struct INDEXED_DATA *)point_1)->i < ((struct INDEXED_DATA *)point_2)->i) return (-1);
	if (((struct INDEXED_DATA *)point_1)->i > ((struct INDEXED_DATA *)point_2)->i) return (1);
	return (0);
}

GMT_LOCAL int grdhisteq_do_gaussian_scores (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double norm) {
	/* Make an output grid file with standard normal scores */
	openmp_int row, col;
	uint64_t i = 0, j = 0, ij, nxy;
	double dnxy;
	struct INDEXED_DATA *indexed_data = NULL;

	indexed_data = gmt_M_memory (GMT, NULL, Grid->header->nm, struct INDEXED_DATA);

	nxy = Grid->header->nm;
	gmt_M_grd_loop (GMT, Grid, row, col, ij) {
		if (gmt_M_is_fnan (Grid->data[ij])) {	/* Put NaNs in the back */
			nxy--;
			indexed_data[nxy].i = ij;
			indexed_data[nxy].x = Grid->data[ij];
		}
		else {
			indexed_data[j].i = ij;
			indexed_data[j].x = Grid->data[ij];
			j++;
		}
	}

	/* Sort on data value  */

	qsort (indexed_data, nxy, sizeof (struct INDEXED_DATA), grdhisteq_compare_indexed_floats);

	dnxy = 1.0 / (nxy + 1);

	if (norm != 0.0) norm /= fabs (gmt_zcrit (GMT, dnxy));	/* Normalize by abs(max score) */

	for (i = 0; i < nxy; i++) {
		indexed_data[i].x = (gmt_grdfloat)gmt_zcrit (GMT, (i + 1.0) * dnxy);
		if (norm != 0.0) indexed_data[i].x *= (gmt_grdfloat)norm;
	}

	/* Sort on data index  */

	qsort (indexed_data, Grid->header->nm, sizeof (struct INDEXED_DATA), grdhisteq_compare_indices);

	i = 0;
	gmt_M_grd_loop (GMT, Grid, row, col, ij) Grid->data[ij] = indexed_data[i++].x;	/* Load up the grid */

	gmt_M_free (GMT, indexed_data);
	return (0);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdhisteq (void *V_API, int mode, void *args) {
	int error = 0;

	double wesn[4];

	struct GMT_GRID *Grid = NULL, *Out = NULL;
	struct GRDHISTEQ_CELL *Cell = NULL;
	struct GRDHISTEQ_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the grdhisteq main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid\n");
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (gmt_M_is_subset (GMT, Grid->header, wesn)) {	/* Subset requested; make sure wesn matches header spacing */
		if ((error = gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, Grid->header), "")))
			Return (error);
	}
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, Grid) == NULL) {	/* Get subset */
		Return (API->error);
	}
	(void)gmt_set_outgrid (GMT, Ctrl->In.file, false, 0, Grid, &Out);	/* true if input is a read-only array */
	gmt_grd_init (GMT, Out->header, options, true);

	if (Ctrl->N.active)
		grdhisteq_do_gaussian_scores (GMT, Out, Ctrl->N.norm);
	else {	/* Do histogram equalization and return the cell values via structure array Cell */
		if ((Cell = grdhisteq_do_hist_equalization (GMT, Out, Ctrl->G.file, Ctrl->C.value, Ctrl->Q.active)) == NULL)
			Return (GMT_RUNTIME_ERROR);	/* Some error */
	}
	if (Ctrl->G.active) {	/* Output the equalized grid */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Out) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	if (Ctrl->D.active) {	/* Wants to dump the equal area cell boundaries */
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		uint64_t dim[GMT_DIM_SIZE] = {1, 1, Ctrl->C.value, 3}, row;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			Return (API->error);
		}
		S = D->table[0]->segment[0];	/* Short-hand to the single segment */
		for (row = 0; row < S->n_rows; row++) {
			S->data[GMT_X][row] = Cell[row].low;
			S->data[GMT_Y][row] = Cell[row].high;
			S->data[GMT_Z][row] = Cell[row].row;;
		}		
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_NORMAL, NULL, Ctrl->D.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	gmt_M_free (GMT, Cell);

	Return (GMT_NOERROR);
}
