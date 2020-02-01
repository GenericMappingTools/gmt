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
	struct In {
		bool active;
		char *file;
	} In;
	struct C {	/* -C<n_cells>*/
		bool active;
		unsigned int value;
	} C;
	struct D {	/* -D[<file>] */
		bool active;
		char *file;
	} D;
	struct G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct N {	/* -N[<norm>] */
		bool active;
		double norm;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
};

struct INDEXED_DATA {
	gmt_grdfloat x;
	uint64_t i;
};

struct CELL {
	gmt_grdfloat low;
	gmt_grdfloat high;
};

EXTERN_MSC int gmtlib_compare_observation (const void *a, const void *b);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDHISTEQ_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDHISTEQ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.value = 16;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDHISTEQ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid> [-G<outgrid>] [-C[<n_cells>]] [-D[<table>]] [-N[<norm>]] [-Q]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_ho_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is name of input grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set how many cells (divisions) of data range to make [16].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Dump level information to <table> or stdout if not given.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Create an equalized output grid file called <outgrid>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Use with -G to make an output grid file with standard normal scores.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append <norm> to normalize the scores to <-1,+1>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Use quadratic equalization scaling [Default is linear].\n");
	GMT_Option (API, "R,V,h,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDHISTEQ_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdhisteq and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int sval;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Get # of cells */
				Ctrl->C.active = true;
				sval = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, sval <= 0, "Option -C: n_cells must be positive\n");
				Ctrl->C.value = sval;
				break;
			case 'D':	/* Dump info to file or stdout */
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				break;
			case 'G':	/* Output file for equalized grid */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'N':	/* Get normalized scores */
				Ctrl->N.active = true;
				if (opt->arg[0]) Ctrl->N.norm = atof (opt->arg);
				break;
			case 'Q':	/* Use quadratic scaling */
				Ctrl->Q.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
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

GMT_LOCAL gmt_grdfloat get_cell (gmt_grdfloat x, struct CELL *cell, unsigned int n_cells_m1, unsigned int last_cell) {
	unsigned int low, high, i;

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
	return (0.0f);	/* Cannot get here - just used to quiet compiler */
}

GMT_LOCAL int do_hist_equalization_cart (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *outfile, unsigned int n_cells, bool quadratic, bool dump_intervals) {
	/* Do basic Cartesian histogram equalization */
	uint64_t i, j, nxy;
	unsigned int n_cells_m1 = 0, current_cell, pad[4];
	double delta_cell, target, out[3];
	struct CELL *cell = NULL;
	struct GMT_GRID *Orig = NULL;
	struct GMT_RECORD *Out = NULL;

	cell = gmt_M_memory (GMT, NULL, n_cells, struct CELL);

	/* Sort the data and find the division points */

	gmt_M_memcpy (pad, Grid->header->pad, 4, int);	/* Save the original pad */
	gmt_grd_pad_off (GMT, Grid);	/* Undo pad if one existed so we can sort the entire grid */
	if (outfile) {
		if ((Orig = GMT_Duplicate_Data (GMT->parent, GMT_IS_GRID, GMT_DUPLICATE_DATA, Grid)) == NULL) {	/* Must keep original if readonly */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid duplication failed - memory error?\n");
			gmt_M_free (GMT, cell);
			return (GMT->parent->error);
		}
	}
	gmt_sort_array (GMT, Grid->data, Grid->header->nm, GMT_FLOAT);

	nxy = Grid->header->nm;
	while (nxy > 0 && gmt_M_is_fnan (Grid->data[nxy-1])) nxy--;	/* Only deal with real numbers */

	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
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

		if (dump_intervals) {	/* Write records to file or stdout */
			out[GMT_X] = (double)Grid->data[i]; out[GMT_Y] = (double)Grid->data[j]; out[GMT_Z] = (double)current_cell;
			GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, Out);
		}

		i = j;
		current_cell++;
	}
	if (dump_intervals && GMT_End_IO (GMT->parent, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data ioutput */
		gmt_M_free (GMT, cell);
		gmt_M_free (GMT, Out);
		return (GMT->parent->error);
	}

	if (outfile) {	/* Must re-read the grid and evaluate since it got sorted and trodden on... */
		unsigned int last_cell = n_cells / 2;
		for (i = 0; i < Grid->header->nm; i++) Grid->data[i] = (gmt_M_is_fnan (Orig->data[i])) ? GMT->session.f_NaN : get_cell (Orig->data[i], cell, n_cells_m1, last_cell);
		if (GMT_Destroy_Data (GMT->parent, &Orig) != GMT_NOERROR) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to free Orig\n");
		}
	}

	gmt_grd_pad_on (GMT, Grid, pad);	/* Reinstate the original pad */
	gmt_M_free (GMT, cell);
	gmt_M_free (GMT, Out);
	return (0);
}

GMT_LOCAL int do_hist_equalization_geo (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *outfile, unsigned int n_cells, bool quadratic, bool dump_intervals) {
	/* Do basic area-weighted histogram equalization */
	uint64_t i, j, node, nxy = 0;
	unsigned int n_cells_m1 = 0, current_cell, row, col;
	double cell_w, delta_w, target_w, wsum = 0.0, out[3];
	struct CELL *cell = gmt_M_memory (GMT, NULL, n_cells, struct CELL);
	struct GMT_GRID *W = gmt_duplicate_grid (GMT, Grid, GMT_DUPLICATE_ALLOC);
	struct GMT_OBSERVATION *pair = gmt_M_memory (GMT, NULL, Grid->header->nm, struct GMT_OBSERVATION);
	struct GMT_RECORD *Out = NULL;

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

	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
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

		if (dump_intervals) {	/* Write records to file or stdout */
			out[GMT_X] = (double)cell[current_cell].low; out[GMT_Y] = (double)cell[current_cell].high; out[GMT_Z] = (double)current_cell;
			GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, Out);
		}

		i = j;
		current_cell++;
		cell_w += delta_w;
	}
	if (dump_intervals && GMT_End_IO (GMT->parent, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data ioutput */
		gmt_M_free (GMT, cell);
		gmt_M_free (GMT, pair);
		gmt_M_free (GMT, Out);
		return (GMT->parent->error);
	}

	if (outfile) {	/* Evaluate grid given its original values */
		unsigned int last_cell = n_cells / 2;
		gmt_M_grd_loop (GMT, Grid, row, col, node) {
			Grid->data[node] = (gmt_M_is_fnan (Grid->data[node])) ? GMT->session.f_NaN : get_cell (Grid->data[node], cell, n_cells_m1, last_cell);
		}
	}

	gmt_M_free (GMT, pair);
	gmt_M_free (GMT, cell);
	gmt_M_free (GMT, Out);
	return (0);
}

GMT_LOCAL int do_hist_equalization (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *outfile, unsigned int n_cells, bool quadratic, bool dump_intervals) {
	int err = 0;
	if (gmt_M_is_geographic (GMT, GMT_IN))
		err = do_hist_equalization_geo (GMT, Grid, outfile, n_cells, quadratic, dump_intervals);
	else
		err = do_hist_equalization_cart (GMT, Grid, outfile, n_cells, quadratic, dump_intervals);
	return (err);
}

GMT_LOCAL int compare_indexed_floats (const void *point_1, const void *point_2) {
	if (((struct INDEXED_DATA *)point_1)->x < ((struct INDEXED_DATA *)point_2)->x) return (-1);
	if (((struct INDEXED_DATA *)point_1)->x > ((struct INDEXED_DATA *)point_2)->x) return (+1);
	return (0);
}

GMT_LOCAL int compare_indices (const void *point_1, const void *point_2) {
	if (((struct INDEXED_DATA *)point_1)->i < ((struct INDEXED_DATA *)point_2)->i) return (-1);
	if (((struct INDEXED_DATA *)point_1)->i > ((struct INDEXED_DATA *)point_2)->i) return (1);
	return (0);
}

GMT_LOCAL int do_gaussian_scores (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double norm) {
	/* Make an output grid file with standard normal scores */
	unsigned int row, col;
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

	qsort (indexed_data, nxy, sizeof (struct INDEXED_DATA), compare_indexed_floats);

	dnxy = 1.0 / (nxy + 1);

	if (norm != 0.0) norm /= fabs (gmt_zcrit (GMT, dnxy));	/* Normalize by abs(max score) */

	for (i = 0; i < nxy; i++) {
		indexed_data[i].x = (gmt_grdfloat)gmt_zcrit (GMT, (i + 1.0) * dnxy);
		if (norm != 0.0) indexed_data[i].x *= (gmt_grdfloat)norm;
	}

	/* Sort on data index  */

	qsort (indexed_data, Grid->header->nm, sizeof (struct INDEXED_DATA), compare_indices);

	i = 0;
	gmt_M_grd_loop (GMT, Grid, row, col, ij) Grid->data[ij] = indexed_data[i++].x;	/* Load up the grid */

	gmt_M_free (GMT, indexed_data);
	return (0);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdhisteq (void *V_API, int mode, void *args) {
	int error = 0;

	double wesn[4];

	struct GMT_GRID *Grid = NULL, *Out = NULL;
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
	if (gmt_M_is_subset (GMT, Grid->header, wesn)) gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, Grid->header), "");	/* Subset requested; make sure wesn matches header spacing */
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, Grid) == NULL) {	/* Get subset */
		Return (API->error);
	}
	(void)gmt_set_outgrid (GMT, Ctrl->In.file, false, Grid, &Out);	/* true if input is a read-only array */
	gmt_grd_init (GMT, Out->header, options, true);

	if (Ctrl->N.active)
		do_gaussian_scores (GMT, Out, Ctrl->N.norm);
	else {
		if (Ctrl->D.active) {	/* Initialize file/stdout for table output */
			int out_ID;
			/* Must register Ctrl->D.file first since we are going to writing rec-by-rec */
			if (Ctrl->D.file && (out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_OUT, NULL, Ctrl->D.file)) == GMT_NOTSET) {
				Return (GMT_RUNTIME_ERROR);
			}
			if ((error = GMT_Set_Columns (API, GMT_OUT, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
				Return (error);
			}
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
				Return (API->error);
			}
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
				Return (API->error);
			}
		}
		if ((error = do_hist_equalization (GMT, Out, Ctrl->G.file, Ctrl->C.value, Ctrl->Q.active, Ctrl->D.active)) != 0) Return (GMT_RUNTIME_ERROR);	/* Read error */
		/* do_hist_equalization will also call GMT_End_IO if Ctrl->D.active was true */
	}
	if (Ctrl->G.active) {
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Out) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	Return (GMT_NOERROR);
}
