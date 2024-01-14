/*	ttt_search.c
 *
 * ttt_search is used to determine how much a point must be moved to
 * place it at a specified water depth.  Also gives an estimate
 * of the likely max delay in propagation from original point
 * to the relocated point.  If the original point is actually on
 * land then we first move it to the shore.
 *
 * AUTHOR:	Paul Wessel, GEOWARE
 * UPDATED:	January 1, 2024
 */

#define THIS_MODULE_CLASSIC_NAME	"geoware-ttt_search"
#define THIS_MODULE_MODERN_NAME	"geoware-ttt_search"
#define THIS_MODULE_LIB		"TTT"
#define THIS_MODULE_PURPOSE	"Estimate distance to move point to specified depth - Geoware (c) 1993-%s"
#define THIS_MODULE_KEYS	"ED},<G{"
#define THIS_MODULE_NEEDS       ""
#define THIS_MODULE_OPTIONS	"-Vhi:"

#include "gmt_dev.h"
#include "ttt.h"

#define		RADIUS		5.0     /* Search radius in degrees */
#define		THRESHOLD	0.1
#define		TTT_CONV_LIMIT	1.0e-8

double ttt_great_circle_dist(double lon1, double lat1, double lon2, double lat2);

struct TTTSEARCH_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct G {	/* -G<gridfile> */
		bool active;
		char *file;
	} G;
	struct T {	/* -T[h|m|s] Desired time units */
		bool active;
		double hours_to_unit;
	} T;
	struct Z {	/* -Z Sample station at sufficient water depth */
		bool active;
		char *file;
		double search_depth;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TTTSEARCH_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct TTTSEARCH_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->T.hours_to_unit = 1.0;	/* Default hour output */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct TTTSEARCH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->In.file);	
	gmt_M_free (GMT, C);	
}

static int parse (struct GMT_CTRL *GMT, struct TTTSEARCH_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to geoware-ttt_pick and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	uint64_t n_sources = 0;
	int j, n_files = 0, n_errors = 0;
	char *c = NULL, q, *string = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
			case '<':	/* Input files */
				Ctrl->In.active = true;
				if (n_files == 0) Ctrl->In.file = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'T':	/* Select output time unit */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				n_errors += gmt_get_required_char (GMT, opt->arg, opt->option, 0, &q);
				switch (q) {
					case 'h':	/* Want hours on output */
						Ctrl->T.hours_to_unit = 1.0;
						break;
					case 'm':	/* Want minutes on output */
						Ctrl->T.hours_to_unit = 60.0;
						break;
					case 's':	/* Want seconds on output */
						Ctrl->T.hours_to_unit = 3600.0;
						break;
				}
				break;
			case 'G':	/* Gave a grid input file name */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_LOCAL, &(Ctrl->G.file));
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &string);
				if ((c = strchr (string, ','))) {	/* Gave depth and file, comma-separated */
					c[0] = ' ';	/* Replace with a space */
					Ctrl->Z.search_depth = atof (string);
					Ctrl->Z.file = strdup (&c[1]);
				}
				if (Ctrl->Z.search_depth > 0.0) {
					GMT_Report (API, GMT_TIME_NONE, "Option -Z: Depth level must be negative!\n");
					n_errors++;
				}
				gmt_M_str_free (string);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files > 1, "geoware-ttt_pick: Only one input file may be specified!\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.active, "Option -G: Must specify an input travel time grid file!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <xyfile> [-G%s] [-Th|m|s] [-Z<depth>,<depthgrid>] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_INGRID, GMT_V_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<xyfile>");
	GMT_Usage (API, -2, "is a multicolumn ASCII file with (lon,lat) in the first two columns\n");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Set name of the input tsunami travel time grid file");
	GMT_Usage (API, 1, "\n-Z<depth>,<depthgrid>]");
	GMT_Usage (API, -2, "Sample travel time at point nearest to station where water depth is <depth> meters or deeper. "
		"Give <depthgrid> as the name of the bathymetry grid used to calculate the input travel time grid. "
		"Note: <depth> is negative below the sea surface [Default is no depth search]");
	GMT_Option (API, "V,h,i,:,.");

	return (GMT_MODULE_USAGE);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int geoware_ttt_search (void *V_API, int mode, void *args) {
	int i, i_0, j_0, k, k_0, ix, iy, row_width, col_height, n_read, n_points = 0, rec = 0;
	int i1, i2, j, n_header_recs = 1, swabbing, ij, imin, imax, jmin, jmax;
 	uint64_t tbl, seg, row, hdr;
	
	bool error = false, flip = false, header = false, verbose = false;
	bool read_short = false;
	
	float *ttt, *z, *f;

	double value, out[12], xy[2], i_xinc, i_yinc, x, y, shortest_dist, hours_to_unit = 1.0;
	double search_depth = 0.0, z_value, xx, yy, dx, dy, d, x0, y0, z0, x1, y1, z1, NaN_dist, delay;
		
	FILE *fp = NULL;
	
	struct GMT_GRID *G = NULL, *Z = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	
 	struct TTTSEARCH_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMT_RECORD *Out = NULL;
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

	/*---------------------------- This is the geoware-ttt_pick main code ----------------------------*/
	
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->In.file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to read input file %s\n", Ctrl->In.file);
		Return (GMT_DATA_READ_ERROR);
	}
	if (D->n_columns < 2) {	/* Not 2 or more columns as needed */
		GMT_Report (API, GMT_MSG_ERROR, "Not enough columns in the input file %s (need at least longitude latitude)\n", Ctrl->In.file);
		Return (GMT_DATA_READ_ERROR);
	}

	if ((Ctrl->G.active + Ctrl->Z.active) == 0) {
		fprintf (stderr, "ttt_search: Must specify at least one grid (ttt -G or depth -Z)\n");
		exit (EXIT_FAILURE);
	}
	
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, Ctrl->G.file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -G: Unable to read input tsunami travel time grid file %s\n", Ctrl->G.file);
		Return (GMT_DATA_READ_ERROR);
	}
	ttt = G->data;
	
	row_width = G->header->n_columns + 2;
	col_height = G->header->n_rows + 2;

	i_xinc = 1.0 / G->header->inc[GMT_X];
	i_yinc = 1.0 / G->header->inc[GMT_Y];

	/* Set natural cubic spline BC's */

	for (i1 = 0, i2 = (col_height-1)*row_width; i1 < row_width; i1++, i2++) {
		ttt[i1] = (float)(2.0 * ttt[i1+row_width] - ttt[i1+2*row_width]);
		ttt[i2] = (float)(2.0 * ttt[i2-row_width] - ttt[i2-2*row_width]);
	}
	for (j = 1, i1 = row_width - 2; j < col_height-1; j++) {
		i1 = j * row_width;
		ttt[i1] = (float)(2.0 * ttt[i1+1] - ttt[i1+2]);
		i1 += row_width - 2;
		ttt[i1] = (float)(2.0 * ttt[i1-1] - ttt[i1-2]);
	}

	if (Ctrl->Z.active) {
		if ((Z = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, Ctrl->Z.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -Z: Unable to read depth grid file %s\n", Ctrl->Z.file);
			Return (GMT_DATA_READ_ERROR);
		}
		if (!gmt_M_grd_same_region (GMT, G, Z)) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -Z: Travel time grid and depth grid not of same size\n");
			Return (GMT_RUNTIME_ERROR);
		}
		z = Z->data;
		row_width = Z->header->n_columns + 2;
		col_height = Z->header->n_rows + 2;
	
		i_xinc = 1.0 / Z->header->inc[GMT_X];
		i_yinc = 1.0 / Z->header->inc[GMT_Y];
	
		/* Set natural cubic spline BC's */
	
		for (i1 = 0, i2 = (col_height-1)*row_width; i1 < row_width; i1++, i2++) {
			z[i1] = (float)(2.0 * z[i1+row_width] - z[i1+2*row_width]);
			z[i2] = (float)(2.0 * z[i2-row_width] - z[i2-2*row_width]);
		}
		for (j = 1, i1 = row_width - 2; j < col_height-1; j++) {
			i1 = j * row_width;
			z[i1] = (float)(2.0 * z[i1+1] - z[i1+2]);
			i1 += row_width - 2;
			z[i1] = (float)(2.0 * z[i1-1] - z[i1-2]);
		}
	}
	
	if ((error = GMT_Set_Columns (API, GMT_OUT, 12, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	Out = gmt_new_record (GMT, out, NULL);
	for (tbl = 0; tbl < D->n_tables; tbl++) {
		for (hdr = 0; hdr < D->table[tbl]->n_headers; hdr++)
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, D->table[tbl]->header[hdr]);
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			S = D->table[tbl]->segment[seg];	/* Current segment */
			for (row = 0; row < S->n_rows; row++) {
				/* Check that we are inside the grid area */
				if (S->data[GMT_Y][row] < G->header->wesn[YLO] || S->data[GMT_Y][row] > G->header->wesn[YHI]) continue;
				S->data[GMT_X][row] -= 360.0;	/* Wind well to the west */
				while (S->data[GMT_X][row] < G->header->wesn[XLO]) S->data[GMT_X][row] += 360.0;
				if (S->data[GMT_X][row] > G->header->wesn[XHI]) continue;
			
				/* Get nearest node */
		
				i_0 = gmt_M_grd_x_to_col (GMT, S->data[GMT_X][row], G->header);
				j_0 = gmt_M_grd_y_to_row (GMT, S->data[GMT_Y][row], G->header);
				k = gmt_M_ijp (G->header, j_0, i_0);
				x0 = S->data[GMT_X][row];	y0 = S->data[GMT_Y][row];	z0 = z[k_0];	/* Original coordinates */
		
				NaN_dist = 0.0;
				f = (Ctrl->G.active) ? ttt : z;
				if (gmt_M_is_fnan (f[k_0]) || z[k_0] >= 0.0) {	/* Must search for nearest non-NaN or ocean node (in either depth or ttt) */
					dx = G->header->inc[GMT_X] * cos (D2R * xy[1]);
					imin = MAX (0, i_0 - (int)ceil (RADIUS / dx));
					imax = MIN (G->header->n_columns - 1, i_0 + (int)ceil (RADIUS / dx));
					jmin = MAX (0, j_0 - (int)ceil (RADIUS / G->header->inc[GMT_Y]));
					jmax = MIN (G->header->n_rows - 1, j_0 + (int)ceil (RADIUS / G->header->inc[GMT_Y]));
		                
					NaN_dist = 180.0;
					xx = yy = 0.0;
					for (j = jmin; j <= jmax; j++) {
						y = gmt_M_grd_row_to_y (GMT, j, G->header);
						for (i = imin; i <= imax; i++) {
							ij = gmt_M_ijp (G->header, j, i);
							if (gmt_M_is_fnan (f[ij]) || z[ij] >= 0.0) continue;
							x = gmt_M_grd_col_to_x (GMT, i, G->header);
							d = ttt_great_circle_dist (xy[0], xy[1], x, y);
							if (d < NaN_dist) {
								k_0 = ij;
								NaN_dist = d;
								xx = x;
								yy = y;
							}
						}
					}
		                
					if (NaN_dist == 180.0) {           
					GMT_Report (API, GMT_MSG_INFORMATION, "Station location more than %g degrees from the ocean - check!\n", RADIUS);
						continue;
					}
					xy[0] = xx;
					xy[1] = yy;
					NaN_dist *= GMT->current.proj.DIST_KM_PR_DEG;
				}
				/* Reset nearest node parameters */
				
				i_0 = gmt_M_grd_x_to_col (GMT, xy[0], G->header);
				j_0 = gmt_M_grd_y_to_row (GMT, xy[1], G->header);
				x1 = xy[0];	y1 = xy[1];	z1 = z[k_0];		/* Nearest node over water */
			
				/* Now xy is at the nearest water node to the epicenter */
				
				/* Must search for nearest node at specified minimum water depth */
		               
				dx = G->header->inc[GMT_X] * cos (D2R * xy[1]);
				imin = MAX (0, i_0 - (int)ceil (RADIUS / dx));
				imax = MIN (G->header->n_columns - 1, i_0 + (int)ceil (RADIUS / dx));
				jmin = MAX (0, j_0 - (int)ceil (RADIUS / G->header->inc[GMT_Y]));
				jmax = MIN (G->header->n_rows - 1, j_0 + (int)ceil (RADIUS / G->header->inc[GMT_Y]));
		                
				shortest_dist = 180.0;
				xx = yy = 0.0;
				k = k_0;
				for (j = jmin; j <= jmax; j++) {
					y = gmt_M_grd_row_to_y (GMT, j, G->header);
					for (i = imin; i <= imax; i++) {
						ij = gmt_M_ijp (G->header, j, i);
						if (Ctrl->G.active && gmt_M_is_fnan (ttt[ij])) continue;
						x = gmt_M_grd_col_to_x (GMT, i, G->header);
						d = ttt_great_circle_dist (xy[0], xy[1], x, y);
						if (gmt_M_is_fnan (z[ij])|| z[ij] >= search_depth) continue;	/* Too shallow */
						if (d < shortest_dist) {
							k = ij;
							shortest_dist = d;
							xx = x;
							yy = y;
						}
					}
				}
		               
				if (shortest_dist == 180.0) {           
					GMT_Report (API, GMT_MSG_INFORMATION, "Station location more than %g degrees from the ocean - check!\n", RADIUS);
					continue;
				}
				xy[0] = xx;
				xy[1] = yy;
				shortest_dist *= GMT->current.proj.DIST_KM_PR_DEG;
		
				/* Estimate delay */
				
				delay = (Ctrl->G.active) ? hours_to_unit * (ttt[k] - ttt[k_0]) : GMT->session.d_NaN;
				out[GMT_X] = x0;	out[GMT_Y] = y0;	out[GMT_Z] = z0;
				out[3] = x1;	out[4] = y1;	out[5] = z1;
				out[6] = xy[0];	out[7] = xy[1];	out[8] = z[k];
				out[9] = NaN_dist;	out[10] = shortest_dist;	out[11] = delay;

				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				
				n_points++;
			}
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	
	GMT_Report (API, GMT_MSG_INFORMATION, "Sampled %d points from grid %s (%d x %d)\n",
		n_points, Ctrl->G.file, G->header->n_columns, G->header->n_rows);
		
	Return (GMT_NOERROR);
}
