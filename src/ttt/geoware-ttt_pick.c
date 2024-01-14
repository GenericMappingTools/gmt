/*	ttt_pick.c
 *
 * ttt_pick reads a xyfile, opens the 2d gridded traveltime file, 
 * and samples the dataset at the xy positions with a bilinear interpolation.
 * If the nearest node is NaN it will search for the nearest non-NaN node.
 * This new data is added to the input as an extra column and printed
 * to standard output.  In order to evaluate derivatives along the edges
 * of the surface, I assume natural bi-cubic spline conditions, i.e.
 * both the second and third normal derivatives are zero, and that the
 * dxdy derivative in the corners are zero, too.  Proper map projection
 * is assumed to have been done.
 *
 * PROGRAM:	ttt.c
 * PURPOSE:	Sample ttt grids at given coordinates
 * AUTHOR:	Paul Wessel, GEOWARE
 * DATE:	June 16 1993
 * UPDATED:	August 1, 2024
 * VERSION:	4.0
 */

#define THIS_MODULE_CLASSIC_NAME	"geoware-ttt_pick"
#define THIS_MODULE_MODERN_NAME	"geoware-ttt_pick"
#define THIS_MODULE_LIB		"TTT"
#define THIS_MODULE_PURPOSE	"Sample tsunami travel time grid as locations - Geoware (c) 1993-%s"
#define THIS_MODULE_KEYS	"ED},<G{"
#define THIS_MODULE_NEEDS       ""
#define THIS_MODULE_OPTIONS	"-Vhi:"

#include "gmt_dev.h"
#include "ttt.h"

#define		SEARCH_RADIUS		1.12415		/* Default 125 km search radius (in spherical degrees) for moving a station off land or to deep-enough water */

struct TTTPICK_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct E {
		bool active;
		unsigned int n_sources;	/* Number of points making up the epicenter source */
		double *q_lon, *q_lat;	/* Array with source coordinates */
		char *source;	/* ASCII file with source coordinates or just a lon/at pair */
	} E;
	struct D {	/* -D give distances */
		bool active;
		double depth_threshold;
	} D;
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
	struct TTTPICK_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct TTTPICK_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->T.hours_to_unit = 1.0;	/* Default hour output */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct TTTPICK_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->G.file);	
	if (C->G.file) free (C->G.file);	
	if (C->Z.file) free (C->Z.file);	
	gmt_M_free (GMT, C);	
}

static int parse (struct GMT_CTRL *GMT, struct TTTPICK_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to geoware-ttt_pick and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	uint64_t n_sources = 0;
	int j, n_files = 0, n_errors = 0;
	char *c = NULL, *string = NULL, q;
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
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				n_errors += gmt_get_required_double (GMT, opt->arg, opt->option, 0, &Ctrl->D.depth_threshold);
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
	GMT_Usage (API, 0, "usage: %s <xyfile> [-D] [-G%s] [-Th|m|s] [-Z<depth>,<depthgrid>] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_INGRID, GMT_V_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<xyfile>");
	GMT_Usage (API, -2, "is a multicolumn ASCII file with (lon,lat) in the first two columns\n");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-D<zmin>");
	GMT_Usage (API, -2, "Specify a minimum water depth [0].  Depths shallower than 2*zmin will be adjusted "
		"so that depth goes quadratically from <zmin> to 2*<zmin> instead of 0-2*<zmin>.");
	GMT_Usage (API, 1, "\n-E<lon/lat>|<sources>");
	GMT_Usage (API, -2, "Set the location of the epicenter, either a single point or a file with multiple coordinates "
		"to mimic a non-point source.");
	gmt_ingrid_syntax (API, 0, "Set name of the input tsunami travel time grid file");
#ifndef TTT120
	GMT_Usage (API, 1, "\n-N<nodes>");
	GMT_Usage (API, -2, "Number of Huygens nodes to use (8, 16, 32, 48, 64 or 120) [%" TTT_LL "d]", N_CALC);
#endif
	GMT_Usage (API, 1, "\n-O<mm/dd/yyyy/hh/mi>[/<ss>]");
	GMT_Usage (API, -2, "Set earthquake origin time (UTC).  Use lower case -o if local time is used.\n");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-Z<depth>,<depthgrid>]");
	GMT_Usage (API, -2, "Sample travel time at point nearest to station where water depth is <depth> meters or deeper. "
		"Give <depthgrid> as the name of the bathymetry grid used to calculate the input travel time grid. "
		"Note: <depth> is negative below the sea surface [Default is no depth search]");
	GMT_Option (API, "V,h,i,:,.");

	return (GMT_MODULE_USAGE);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int geoware_ttt_pick (void *V_API, int mode, void *args) {
	TTT_LONG i, i_0, j_0, a, b, c, d, k, ix, iy, row_width, n_read;
	TTT_LONG i1, i2, j, n_points = 0, rec = 0;
	uint64_t tbl, seg, row, hdr;
	
	unsigned int error = 0, n_output = 3, z_col = 2, d_col = 0;
	
	float *f = NULL, *z = NULL;

	double value, out[5], i_xinc, i_yinc, x, y, cx, cy, cxy, dx, dy, shortest_dist;
	double z_value = 0.0;
	
	char txt[2][32], stuff[BUFSIZ], line[BUFSIZ], format1[BUFSIZ], format2[BUFSIZ];
	
	FILE *fp = NULL;
	
	struct GMT_GRID *G= NULL, *Z = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	struct GMT_RECORD *Out = NULL;
	struct TTTPICK_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the geoware-ttt_pick main code ----------------------------*/

	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->In.file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to read input file %s\n", Ctrl->In.file);
		Return (GMT_DATA_READ_ERROR);
	}
	if (D->n_columns < 2) {	/* Not 2 or more columns as needed */
		GMT_Report (API, GMT_MSG_ERROR, "Not enough columns in the input file %s (need at least longitude latitude)\n", Ctrl->In.file);
		Return (GMT_DATA_READ_ERROR);
	}
	
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, Ctrl->G.file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -G: Unable to read input tsunami travel time grid file %s\n", Ctrl->G.file);
		Return (GMT_DATA_READ_ERROR);
	}
	f = G->data;
	row_width = G->header->n_columns + 2;
	
	i_xinc = 1.0 / G->header->inc[TTT_X];
	i_yinc = 1.0 / G->header->inc[TTT_Y];
		
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
	}
	
	/* Initialize output record data and columns */

	if (Ctrl->Z.active) {
		n_output = 5;
		z_col = 3;
		d_col = 4;
	}
	else if (Ctrl->D.active) {
		n_output = 4;
		d_col = 3;
	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, n_output, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
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
		
				if (Ctrl->Z.active || gmt_M_is_fnan (f[k])) {	/* Must search for nearest non-NaN node */
					TTT_LONG i, j, ij, imin, imax, jmin, jmax;
					double xx, yy, d, s_radius = MAX (SEARCH_RADIUS, 2.0 * G->header->inc[TTT_X]);
		                
					dx = G->header->inc[TTT_X] * cos (D2R * S->data[GMT_Y][row]);
					imin = MAX (0, i_0 - (TTT_LONG)ceil (s_radius / dx));
					imax = MIN (G->header->n_columns - 1, i_0 + (TTT_LONG)ceil (s_radius / dx));
					jmin = MAX (0, j_0 - (TTT_LONG)ceil (s_radius / G->header->inc[TTT_Y]));
					jmax = MIN (G->header->n_rows - 1, j_0 + (TTT_LONG)ceil (s_radius / G->header->inc[TTT_Y]));
		                
					shortest_dist = 180.0;
					xx = yy = 0.0;
					for (j = jmin; j <= jmax; j++) {
						y = gmt_M_grd_row_to_y (GMT, j, G->header);
						for (i = imin; i <= imax; i++) {
							ij = gmt_M_ijp (G->header, j, i);
							if (gmt_M_is_fnan (f[ij])) continue;
							if (Ctrl->Z.active && (gmt_M_is_fnan (z[ij])|| z[ij] >= Ctrl->Z.search_depth)) continue;	/* Too shallow */
							x = gmt_M_grd_col_to_x (GMT, i, G->header);
							d =  0.001 * gmt_great_circle_dist_meter (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], x, y);
							if (d < shortest_dist) {
								k = ij;
								shortest_dist = d;
								xx = x;
								yy = y;
							}
						}
					}
		                
					if (shortest_dist == 180.0) {           
						GMT_Report (API, GMT_MSG_WARNING, "Station location more than %g degrees from the ocean for segment %" PRIu64 " row %" PRIu64 " - check!\n", s_radius, seg, row);
						value = z_value = GMT->session.d_NaN;
					}
					else {
						out[GMT_X] = xx;
						out[GMT_Y] = yy;
						out[GMT_Z] = f[k];
						if (Ctrl->Z.active) out[z_col] = z[k];
					}
					shortest_dist *= GMT->current.proj.DIST_KM_PR_DEG;
				}
				else {	/* May attempt to do bilinear interpolation if 4 nodes exist */
					shortest_dist = 0.0;
					x = gmt_M_grd_col_to_x (GMT, i_0, G->header);
					y = gmt_M_grd_row_to_y (GMT, j_0, G->header);
					dx = S->data[GMT_X][row] - x;
					dy = S->data[GMT_Y][row] - y;
				
					if (dx >= 0.0) {
						if (dy >= 0.0) {
							a = k;	b = k + 1;	c = k + 1 - row_width;	d = k - row_width;
						}
						else {
							a = k + row_width;	b = k + row_width + 1;	c = k + 1;	d = k;
							dy += G->header->inc[TTT_Y];
						}
					}
					else {
						dx += G->header->inc[TTT_X];
						if (dy >= 0.0) {
							a = k - 1;	b = k;	c = k - row_width;	d = k - row_width - 1;
						}
						else {
							a = k - 1 + row_width;	b = k + row_width;	c = k;	d = k - 1;
							dy += G->header->inc[TTT_Y];
						}
					}
					if (gmt_M_is_fnan (f[a]) || gmt_M_is_fnan (f[b]) || gmt_M_is_fnan (f[c]) || gmt_M_is_fnan (f[d])) {	/* Shoot, at least one NaN; pick nearest node and report distance to actual */
						shortest_dist = GMT->current.proj.DIST_KM_PR_DEG * hypot (dx * cos (D2R * S->data[GMT_Y][row]), dy);
						value = f[k];
					}
					else {	/* Here we can interpolate so we assume shortest_dist = 0.0 */
						cx = (f[b] - f[a]) * i_xinc;
						cy = (f[d] - f[a]) * i_yinc;
						cxy = (f[c] - f[b] + f[a] - f[d]) * i_xinc * i_yinc;
						value = cx * dx + cy * dy + cxy * dx * dy + f[a];
					}
					if (Ctrl->Z.active) {
						if (gmt_M_is_fnan (z[a]) || gmt_M_is_fnan (z[b]) || gmt_M_is_fnan (z[c]) || gmt_M_is_fnan (z[d]))	/* Shoot, at least one NaN; pick nearest node */
							z_value = f[k];
						else {	/* Here we can interpolate so we assume shortest_dist = 0.0 */
							cx = (z[b] - z[a]) * i_xinc;
							cy = (z[d] - z[a]) * i_yinc;
							cxy = (z[c] - z[b] + z[a] - z[d]) * i_xinc * i_yinc;
							z_value = cx * dx + cy * dy + cxy * dx * dy + z[a];
						}
						out[z_col] = z_value;
					}
					if (d_col) out[d_col] = shortest_dist;
					
				}
				value *= Ctrl->T.hours_to_unit;
				out[GMT_Z] = value;
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				n_points++;
			}
		}
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Sampled %" TTT_LL "d points from grid %s (%d x %d)\n",
		n_points, Ctrl->G.file, G->header->n_columns, G->header->n_rows);
		
	Return (GMT_NOERROR);
}
