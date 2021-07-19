/*
 * grad2vs30: convert topographic slope to Vs30
 *
 * Takes three input files, each formatted GMT grd files
 * and point for point co-registered with each other;
 * gradient_file contains the topographic slope expressed as
 * a unitless ratio (e.g., meters per meter), landmask_file
 * is 0 for water and 1 (one) for land; craton_file is a
 * weight ranging from 1 (one) on stable shields (craton) and
 * 0 in active tectonic regions -- values in between will
 * be computed as the weighted average of the craton and
 * tectonic models.
 * The optional numerical argument "water" is the value
 * that water-covered areas will be set to; the default is 600.
 * The output file name is specified with "output_file" (required).
 * 
 * LICENSE
 * (https://github.com/usgs/earthquake-global_vs30/blob/master/LICENSE.md)
 * 
 * Unless otherwise noted, This project is in the public domain in the United States because it
 * contains materials that originally came from the United States Geological Survey, an agency
 * of the United States Department of Interior. For more information, see the official USGS
 * copyright policy at https://www2.usgs.gov/visual-id/credit_usgs.html#copyright
 *
 * Additionally, we waive copyright and related rights in the work worldwide through the CC0 1.0
 * Universal public domain dedication.
 *
 * The getpar package (src/getpar.c, src/libget.h, src/getpar.3) is included with the express
 * permission of the author, Robert W. Clayton.
 * 
 * J. Luis
 * re-writen after https://github.com/usgs/earthquake-global_vs30/blob/master/src/grad2vs30.c
 * Version:	6 API
 */

//shake vs30.grd -Glixo.grd -Lline.dat+uk -Ci -Vl -Rvs30.grd

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"vs30"
#define THIS_MODULE_MODERN_NAME		"vs30"
#define THIS_MODULE_LIB		"seis"
#define THIS_MODULE_PURPOSE	"Compute VS30"
#define THIS_MODULE_KEYS	"<G{,CD(=,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-:RVbir"

const float vs30_min = 180;
const float vs30_max = 900;

/* Control structure */

struct VS30_CTRL {
	struct VS30_In {
		bool active;
		char *file;
	} In;
	struct VS30_C {		/* -C<val> | fname[+g] */
		bool active;
		bool is_grid;	/* To signal that a craton grid was provided */
		char *file;
		float val;		/* a [0 1] val where 0 (craton) and 1 (active) */
	} C;
	struct VS30_G {
		bool active;
		char *file;
	} G;
	struct VS30_W {
		bool active;
		float water;
	} W;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct VS30_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct VS30_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->W.water = 600;                  /*  */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct VS30_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free(GMT, C);
}

/*
 * Slope to Vs30 uses Wald & Allen (2007) for craton, and Allen & Wald (2009) for active tectonic.
 *
 * Split up the tables just in case future work makes them have different numbers of rows
 * Columns are: vs30_min vs30_max slope_min slope_max
 */
const unsigned int rows_active = 6;
double active_table[6][4] =
		{{180, 240, 3.0e-4,  3.5e-3},
		 {240, 300, 3.5e-3,  0.01},
		 {300, 360, 0.01,    0.018},
		 {360, 490, 0.018,   0.05},
		 {490, 620, 0.05,    0.10},
		 {620, 760, 0.10,    0.14}};

const unsigned int rows_craton = 6;
double craton_table[6][4] =
		{{180, 240, 2.0e-5,  2.0e-3},
		 {240, 300, 2.0e-3,  4.0e-3},
		 {300, 360, 4.0e-3,  7.2e-3},
		 {360, 490, 7.2e-3,  0.013},
		 {490, 620, 0.013,   0.018},
		 {620, 760, 0.018,   0.025}};


static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);

	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid> -G<outgrid> [-C<val>|fname[+g]] [-W<water_vel>] [%s] [%s]\n",
	             name, GMT_Rgeoz_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> The input grid name of the topography.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G File name for output grid with the Vs30 velocities.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Argument can be one of three:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   - A value between 0 and 1, where 0 means a stable Craton and 1 an Active region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   - The name of a multi-segment file with the 'cratons' polygons. In this case the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     polygons will be feed to grdmask to compute a cratons/active tectonic mask.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   - The name of a grid with the cratons/active tectonic regions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W water_vel sets the Vs30 value used in areas designated as water in the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   landmask [default=600]\n\n");
	GMT_Option (API, "R,V");
	GMT_Option (API, "bi,i,r,:");
	
	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct VS30_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options) {
	/* This parses the options provided to shake and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, pos = 0;
	char txt_a[GMT_LEN256] = {""}, p[GMT_LEN16] = {""}, *pch;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	gmt_M_memset (io, 1, struct GMT_Z_IO);

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':		/* Polygon file with the Cratons limits */
				Ctrl->C.active = true;
				Ctrl->C.file = gmt_get_filename (API, opt->arg, "");
				if ((pch = strstr(opt->arg, "+g")) != NULL)
					Ctrl->C.is_grid = true;			/* Useless if Ctrl->C.file = NULL is set below */

				if (!gmt_check_filearg (GMT, 'C', Ctrl->C.file, GMT_IN, GMT_IS_DATASET)) {
					Ctrl->C.file = NULL;
					Ctrl->C.val = atof (opt->arg);
					if (Ctrl->C.val < 0 || Ctrl->C.val > 1) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Errorn in -C. Must provide either a file name or a value in the [0 1] interval.\n", p);
						n_errors++;
					}
				}
				break;
			case 'G':	/* Output file */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'W':
				Ctrl->W.water = (float)atof (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output grid file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->C.active, "Syntax error -C option: Must specify a value or a file name.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static int check_grid_compat (struct GMTAPI_CTRL *API, struct GMT_GRID *A, struct GMT_GRID *B) {
	/* Check that grids A & B are compatible. If they are, return 0. Otherwise:
	   return 1		if increments differ more than 0.2 %
	   return 1		if any of the corners differ more than 1/5 of the grid spacing
	   return 3		if registrations are not equal
	*/
	if (fabs((A->header->inc[GMT_X] - B->header->inc[GMT_X]) / A->header->inc[GMT_X]) > 0.002 ||
		fabs((A->header->inc[GMT_Y] - B->header->inc[GMT_Y]) / A->header->inc[GMT_Y]) > 0.002)
			return 1;

	if (fabs((A->header->wesn[XLO] - B->header->wesn[XLO]) / A->header->inc[GMT_X]) > 0.2 ||
	    fabs((A->header->wesn[XHI] - B->header->wesn[XHI]) / A->header->inc[GMT_X]) > 0.2 ||
		fabs((A->header->wesn[YLO] - B->header->wesn[YLO]) / A->header->inc[GMT_Y]) > 0.2 ||
	    fabs((A->header->wesn[YHI] - B->header->wesn[YHI]) / A->header->inc[GMT_Y]) > 0.2)
			return 2;

	if (A->header->registration != B->header->registration)
			return 3;

	return 0;
}

/*
 * Function interpVs30 interpolates (or extrapolates) vs30 from a row of one of the tables
 * (tables have already been log()'ed so the exp() returns vs30 in linear units)
 * (I was going to pre-compute the differences (tt[1]-tt[0] and * tt[3]-tt[2]) and make this
 * function a #define for speed, but the execution time is utterly dwarfed by the read/write
 * times so there was no point.)
 */
inline double interpVs30(double *tt, double lg) {
	return exp(tt[0] + (tt[1] - tt[0]) * (lg - tt[2]) / (tt[3] - tt[2]));
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* --------------------------------------------------------------------------------- */
EXTERN_MSC int GMT_vs30 (void *V_API, int mode, void *args) {
	unsigned int row, col, j, nr, k;
	uint64_t ij;
	int error = 0;
	char cmd[GMT_LEN256] = {""}, data_grd[GMT_LEN16] = {""};
	char crat_grd[GMT_LEN16] = {""}, mask_grd[GMT_LEN16] = {""}, grad_grd[GMT_LEN16] = {""};
	float crat, lg;
	double (*table)[4], tvs[2], vv, wesn[4];

	struct GMT_GRID *G = NULL, *Ggrad = NULL, *Gcrat = NULL, *Gland = NULL, *Gout = NULL;
	struct GMT_Z_IO io;
	struct GMT_OPTION *opt = NULL;
	struct VS30_CTRL *Ctrl = NULL;
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
	if ((error = parse (GMT, Ctrl, &io, options)) != 0) Return (error);
	
	/*---------------------------- This is the grd2xyz main code ----------------------------*/

	/* Initialize the input objects and open the files */
	GMT_Report (API, GMT_MSG_VERBOSE, "Reading input files...\n");

	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) 	/* Get header only */
		Return (API->error);

	if (gmt_M_is_subset (GMT, G->header, wesn))	/* If subset requested make sure wesn matches header spacing */
		gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, G->header), "");

	/* Read data */
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL)
		Return (API->error);

	gmt_M_memcpy (wesn, G->header->wesn, 4, double);	/* Need the wesn further down */

	/* ------------------------------------------------------------------------------- */
	GMT_Report (API, GMT_MSG_VERBOSE, "Compute landmask grid\n");
	/* Create a virtual file to hold the landmask grid */
	if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, mask_grd))
		Return (API->error);

	/* Prepare the grdlandmask arguments */
	sprintf (cmd, "-G%s -I%f/%f -Df -R%.16g/%.16g/%.16g/%.16g --GMT_HISTORY=false ",
	         mask_grd, G->header->inc[GMT_X], G->header->inc[GMT_Y], wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calling grdlandmask with args %s\n", cmd);
	if (GMT_Call_Module (API, "grdlandmask", GMT_MODULE_CMD, cmd))
		Return (API->error);

	/* Obtain the data from the virtual file */
	if ((Gland = GMT_Read_VirtualFile (API, mask_grd)) == NULL)
		Return (API->error);
	/* ------------------------------------------------------------------------------- */

	/* ------------------------------------------------------------------------------- */
	GMT_Report (API, GMT_MSG_VERBOSE, "Derive slope grid from input data grid\n");
	/* Create a virtual file to hold the gradient grid */
	if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, grad_grd))
		Return (API->error);

	/* Prepare the grdgradient arguments */
	sprintf (cmd, "-S%s -n+bg -fg -D -R%.16g/%.16g/%.16g/%.16g --GMT_HISTORY=false ",
	         grad_grd, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);

	strcat (cmd, Ctrl->In.file);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calling grdgradient with args %s\n", cmd);
	if (GMT_Call_Module (API, "grdgradient", GMT_MODULE_CMD, cmd))
		Return (API->error);

	/* Obtain the data from the virtual file */
	if ((Ggrad = GMT_Read_VirtualFile (API, grad_grd)) == NULL)
		Return (API->error);
	/* ------------------------------------------------------------------------------- */

	if (Ctrl->C.file && !Ctrl->C.is_grid) {		/* Read the Cratons multi-seg polygons and compute a mask */
		GMT_Report (API, GMT_MSG_VERBOSE, "Derive cratons grid from input file %s\n", Ctrl->C.file);
		/* Create a virtual file to hold the gradient grid */
		if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, crat_grd))
			Return (API->error);

		/* Prepare the grdmask arguments */
		sprintf (cmd, "-G%s -I%f/%f -R%.16g/%.16g/%.16g/%.16g -N1/0/0 --GMT_HISTORY=false ",
				 crat_grd, G->header->inc[GMT_X], G->header->inc[GMT_Y], wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);

		strcat (cmd, Ctrl->C.file);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calling grdmask with args %s\n", cmd);
		if (GMT_Call_Module (API, "grdmask", GMT_MODULE_CMD, cmd))
			Return (API->error);

		/* Obtain the data from the virtual file */
		if ((Gcrat = GMT_Read_VirtualFile (API, crat_grd)) == NULL)
			Return (API->error);
	}
	else if (Ctrl->C.file && Ctrl->C.is_grid) {		/* Read a cratons grid */
		if ((Gcrat = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY,
		                            NULL, Ctrl->C.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}

		if (check_grid_compat (API, Gcrat, G)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cratons and topography grids are not compatible.\n", cmd);
			Return (API->error);
		}

		/* Read data */
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->C.file, Gcrat) == NULL) {
			Return (API->error);
		}
	}

	/* The output file has the same dimensions as Ggrad * so write the header,
	 * prep the output object, then open the output for writing.
	 */
	if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn, G->header->inc,
	                             GMT->common.R.registration, 2, NULL)) == NULL)
		Return (API->error);

	/* * We're doing log-log interpolation, so log() everything in the tables first, for efficiency */
	for (row = 0; row < rows_active; row++)
		for (col = 0; col < 4; col++)
			active_table[row][col] = log(active_table[row][col]);

	for (row = 0; row < rows_craton; row++)
		for (col = 0; col < 4; col++)
			craton_table[row][col] = log(craton_table[row][col]);

	for (row = 0; row < Gout->header->n_rows; row++) {
		for (col = 0; col < Gout->header->n_columns; col++) {
			ij = gmt_M_ijp(G->header, row, col);
			if (Gland->data[ij] == 0) {			/* Set areas covered by water to the water value */
				Gout->data[ij] = Ctrl->W.water;
				continue;
			}

			crat = Ctrl->C.file != NULL ? Gcrat->data[ij] : Ctrl->C.val;

			/* This is the slope value to be converted to vs30 */
			lg = log(Ggrad->data[ij]);

			for (k = 0; k < 2; k++) {	/* Get the Vs30 for both craton and active */
				/* k == 0 => craton, k == 1 => active */
				if (k == 0) {
					table = craton_table;
					nr = rows_craton;
				}
				else {
					table = active_table;
					nr = rows_active;
				}

				/*
				* Handle slopes lower than the minimum in the table by extrapolation capped by the minimum vs30
				* (this isn't necessary when the table contains the minimum vs30, but it's cheap insurance if
				* we change the table or minimum -- ditto for the max, below)
				*/
				if (lg <= table[0][2]) {
					vv = interpVs30(table[0], lg);
					if (vv < vs30_min) vv = vs30_min;
					tvs[k] = vv;
					continue;
				}
				/*
				* Handle slopes greater than the maximum in the table by extrapolation capped by the maximum vs30
				*/
				if (lg >= table[nr-1][3]) {
					vv = interpVs30(table[nr-1],lg);
					if (vv > vs30_max) vv = vs30_max;
					tvs[k] = vv;
					continue;
				}
				/* All other slopes should be handled within the tables */
				for (j = 0; j < nr; j++) {
					if (lg <= table[j][3]) {
						tvs[k] = interpVs30(table[j], lg);
						break;
					}
				}
			}
			/* Do a weighted average of craton and active vs30 */
			Gout->data[ij] = (float)(crat * tvs[0] + (1.0 - crat) * tvs[1]);
		}

		//if (row % 100 == 0)
			//GMT_Report (API, GMT_MSG_VERBOSE, "Done with %ld of %ld elements\r", row * Gout->header->n_columns, Gout->header->nm);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Writing output file...\n");
	if (GMT_Write_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Gout) != 0)
		Return (API->error);

	GMT_Report (API, GMT_MSG_VERBOSE, "Done.\n");

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) 	/* Disables further data output */
		Return (API->error);

	Return (GMT_NOERROR);
}
