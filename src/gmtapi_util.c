/*--------------------------------------------------------------------
 *	$Id: gmtapi_util.c,v 1.31 2011-03-03 21:02:51 guru Exp $
 *
 *	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
 
/*
 * User definitions and function prototypes for GMT API.
 *
 * Author: 	Paul Wessel
 * Date:	25-MAR-2006
 * Version:	0.1
 */

#include "gmtapi_util.h"

struct GMTAPI_CTRL *GMT_FORTRAN;	/* Global structure needed for FORTRAN-77 */

/* Private functions used by this library */

void GMTAPI_put_val (void *ptr, double val, size_t i, int type);
void GMTAPI_index_to_2D_C (int *row, int *col, size_t index, int dim);
void GMTAPI_index_to_2D_F (int *row, int *col, size_t index, int dim);
void **GMTAPI_duplicate_string (char *string);
void GMTAPI_par_to_ipar (double par[], int ipar[]);
void GMTAPI_grdheader_to_info (struct GRD_HEADER *h, double info[]);
void GMTAPI_info_to_grdheader (struct GRD_HEADER *h, double info[]);
int GMTAPI_Next_IO_Source (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO, int direction);
int GMTAPI_Add_IO_Session (struct GMTAPI_CTRL *API);
int GMTAPI_Init_Import_Session (struct GMTAPI_CTRL *API, int ID[]);
int GMTAPI_Init_Export_Session (struct GMTAPI_CTRL *API, int ID);
int GMTAPI_Prep_IO_Session (struct GMTAPI_CTRL *API, int ID[], int direction);
int GMTAPI_Add_Data_Object (struct GMTAPI_CTRL *GMT, struct GMTAPI_DATA_OBJECT *D);
size_t GMTAPI_2D_to_index_C (int row, int col, int dim);
size_t GMTAPI_2D_to_index_F (int row, int col, int dim);
double GMTAPI_get_val (void *ptr, size_t i, int type);

/*========================================================================================================
 *          HERE ARE THE PUBLIC GMT API UTILITY FUNCTIONS, WITH FORTRAN BINDINGS
 *========================================================================================================
 */

/*===>  Create a new GMT Session */

int GMTAPI_Create_Session (struct GMTAPI_CTRL **API, FILE *log, int *error)
{
	/* Initializes the GMT API for a new session. */
	
	struct GMTAPI_CTRL * G;
	char *argv[1] = {"GMTAPI"};
	
	/* Allocate the structure */
	
 	G = (struct GMTAPI_CTRL *) GMT_memory (VNULL, 1, sizeof (struct GMTAPI_CTRL), "GMT_New_Session");
	
	/* Set error reporting pointer to stderr unless another destination has been provided */
	
	G->log = (log) ? log : stderr;
	
	/* Allocate pointers to data objects */
	
	G->n_data_alloc = GMT_SMALL_CHUNK;
	G->data = (struct GMTAPI_DATA_OBJECT **) GMT_memory (VNULL, G->n_data_alloc, sizeof (struct GMTAPI_DATA_OBJECT *), "GMT_New_Session");
	
	/* Allocate pointers to io sessions */
	
	G->n_io_sessions_alloc = GMT_TINY_CHUNK;
	G->io_session = (struct GMTAPI_IO **) GMT_memory (VNULL, G->n_io_sessions_alloc, sizeof (struct GMTAPI_IO *), "GMT_New_Session");
	
	/* Set function pointers for the two ij index functions */
	
	G->GMT_2D_to_index[0] = (PFI)GMTAPI_2D_to_index_C;
	G->GMT_2D_to_index[1] = (PFI)GMTAPI_2D_to_index_F;
	G->GMT_index_to_2D[0] = (PFV)GMTAPI_index_to_2D_C;
	G->GMT_index_to_2D[1] = (PFV)GMTAPI_index_to_2D_F;
	
	/* Store the size (in bytes) or the various data elements */
	
	G->GMTAPI_size[GMTAPI_BYTE]   = 1;
	G->GMTAPI_size[GMTAPI_SHORT]  = 2;
	G->GMTAPI_size[GMTAPI_INT]    = 4;
	G->GMTAPI_size[GMTAPI_FLOAT]  = 4;
	G->GMTAPI_size[GMTAPI_DOUBLE] = 8;
	
	GMT_begin (1, argv);	/* Initialize GMT machinery */
	
	*API = G;

	return ((*error = GMTAPI_OK));
}

/* Fortran binding */
void GMTAPI_Create_Session_ (int *fhandle, int *error)
{	/* Fortran version: We pass the hidden global GMT_FORTRAN structure*/
	FILE *log = NULL;
	if (*fhandle) log = GMT_fdopen (*fhandle, "w");
	(void)GMTAPI_Create_Session (&GMT_FORTRAN, log, error);
}

/*===>  Destroy a registered GMT Session */

int GMTAPI_Destroy_Session (struct GMTAPI_CTRL *API, int *error)
{
	int i;
	char *argv[1] = {"GMTAPI"};

	/* This terminates the information for the specified session and frees memory */
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);	/* GMT_New_Session has not been called */

	/* Free memory for data registration */
	
	for (i = 0; i < API->n_data_alloc; i++) if (API->data[i]) GMT_free ((void *)API->data[i]);
 	GMT_free ((void *)API->data);
	
	/* Free memory for io registration */
	
	for (i = 0; i < API->n_io_sessions_alloc; i++) if (API->io_session[i]) {
		GMT_free ((void *)API->io_session[i]->ID);
		GMT_free ((void *)API->io_session[i]);
	}
 	GMT_free ((void *)API->io_session);
	
 	GMT_free ((void *)API);
	
	GMT_end (1, argv);	/* Terminate GMT machinery */

	return ((*error = GMTAPI_OK));
}

void GMTAPI_Destroy_Session_ (int *error)
{	/* Fortran version: We pass the hidden global GMT_FORTRAN structure*/
	(void)GMTAPI_Destroy_Session (GMT_FORTRAN, error);
}

/*===>  Error message reporting */

int GMTAPI_Report_Error (struct GMTAPI_CTRL *API, int error)
{
	fprintf (stderr, "GMT: Error returned from GMT API: %d\n", error);
	GMT_exit (EXIT_FAILURE);
}

void GMTAPI_Report_Error_ (int *error)
{	/* Fortran version: We pass the hidden global GMT_FORTRAN structure*/
	GMTAPI_Report_Error (GMT_FORTRAN, *error);
}

/*===>  Register in input source  */

int GMTAPI_Register_Import (struct GMTAPI_CTRL *API, int method, void **input, double parameters[], int *object_ID, int *error)
{
	/* The main program uses this routine to pass information about input data to GMT.
	 * The method argument specifies what we are trying to pass:
	 * GMT_IS_FILE
	 *	A file name is given via input.
	 * GMT_IS_STREAM
	 *	A file pointer to an open file is passed via input.
	 * GMT_IS_FDESC
	 *	A file descriptor to an open file is passed via input.
	 * GMT_IS_GRIDFILE
	 *	A GMT grid file is passed via input
	 * GMT_IS_GRID
	 *	A 2-D user grid is passed via input, plus we need 12 parameters:
	 *	parameter[0] = data type (GMTAPI_{BYTE|SHORT|INT|FLOAT|DOUBLE)
	 *	parameter[1] = Dimensionality of data.  Must be 2
	 *	parameter[2] = number_of_rows
	 *	parameter[3] = number_of_columns
	 *	parameter[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran))
	 *	parameter[5] = dimension of row (C) or column (Fortran)
	 *	parameter[6] = 0 to free array after use, 1 to leave alone
	 *	parameter[7] = registration: 0 for node and 1 for pixel
	 *	parameter[8] = x_min
	 *	parameter[9] = x_max
	 *	parameter[10] = y_min
	 *	parameter[11] = y_max
	 * GMT_IS_GMTGRID
	 *	A structure with float grid and header is passed via input
	 * GMT_IS_ARRAY
	 *	An array of data is passed via input.  In addition, we need 7 parameters
	 *	to be passed with the following information:
	 *	parameter[0] = data type (GMTAPI_{BYTE|SHORT|INT|FLOAT|DOUBLE)
	 *	parameter[1] = Dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D)
	 *	parameter[2] = number_of_rows (or length of 1-D array)
	 *	parameter[3] = number_of_columns (1 for 1-D array)
	 *	parameter[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran))
	 *	parameter[5] = dimension of row (C) or column (Fortran)
	 *	parameter[6] = 0 to free array after use, 1 to leave alone
	 *
	 * Since GMT internally uses doubles in C arrangement, anything else is converted.
	 *
	 * GMTAPI_Register_Import will return a unique Data ID integer and allocate and populate
	 * a GMTAPI_DATA_OBJECT structure which is assigned to the list in GMTAPI_CTRL.
	 */
	
	return (GMTAPI_Register_IO (API, method, input, parameters, GMT_IN, object_ID, error));
}

void GMTAPI_Register_Import_ (int *method, void *input, double parameters[], int *object_ID, int *error)
{	/* Fortran version */
	(void) GMTAPI_Register_Import (GMT_FORTRAN, *method, &input, parameters, object_ID, error);
}

/*===>  Register in output source  */

int GMTAPI_Register_Export (struct GMTAPI_CTRL *API, int method, void **output, double parameters[], int *object_ID, int *error)
{
	/* The main program uses this routine to pass information about output data from GMT.
	 * The method argument specifies what we are trying to receive:
	 * GMT_IS_FILE
	 *	A file name is given via output.  The program will write data to this file
	 * GMT_IS_STREAM
	 *	A file pointer to an open file is passed via output. --"--
	 * GMT_IS_FDESC
	 *	A file descriptor to an open file is passed via output. --"--
	 * GMT_IS_GRIDFILE
	 *	A GMT grid file is passed via output
	 * GMT_IS_GRID
	 *	A 2-D user grid is passed via output, plus we need 12 parameters:
	 *	parameter[0] = data type (GMTAPI_{BYTE|SHORT|INT|FLOAT|DOUBLE)
	 *	parameter[1] = Dimensionality of data.  Must be 2
	 *	parameter[2] = number_of_rows
	 *	parameter[3] = number_of_columns
	 *	parameter[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran))
	 *	parameter[5] = dimension of row (C) or column (Fortran)
	 *	parameter[6] = 0 to free array after use, 1 to leave alone
	 *	parameter[7] = registration: 0 for node and 1 for pixel
	 *	parameter[8] = x_min
	 *	parameter[9] = x_max
	 *	parameter[10] = y_min
	 *	parameter[11] = y_max
	 * GMT_IS_GMTGRID
	 *	A structure with float grid and header is passed via output
	 * GMT_IS_ARRAY
	 *	An array of data is passed via output.  In addition, parameters
	 *	must be passed with the following information:
	 *	parameter[0] = data type (GMTAPI_{BYTE|SHORT|INT|FLOAT|DOUBLE)
	 *	parameter[1] = Dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D)
	 *	parameter[2] = number_of_rows (or length of 1-D array)
	 *	parameter[3] = number_of_columns (1 for 1-D array)
	 *	parameter[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran))
	 *	parameter[5] = dimension of row (C) or column (Fortran)
	 *	parameter[6] = 0 = GMT must allocate space, 1 = space already allocated
	 *
	 * Since GMT internally uses doubles in C arrangement, anything else is converted.
	 *
	 * GMTAPI_Register_Import will return a unique Data ID integer and allocate and populate
	 * a GMTAPI_DATA_OBJECT structure which is assigned to the list in GMTAPI_CTRL.
	 */
	 
	return (GMTAPI_Register_IO (API, method, output, parameters, GMT_OUT, object_ID, error));
}

void GMTAPI_Register_Export_ (int *method, void *output, double parameters[], int *object_ID, int *error)
{	/* Fortran version: We pass the global GMT_FORTRAN structure*/
	(void) GMTAPI_Register_Export (GMT_FORTRAN, *method, &output, parameters, object_ID, error);
}

int GMTAPI_Unregister_Export (struct GMTAPI_CTRL *API, int object_ID, int *error)
{
	/* Removes a registerd export object from the list of objects.
	 * object_ID is the registered ID of the object to be removed */
	 
	return (GMTAPI_Unregister_IO (API, object_ID, GMT_OUT, error));
}

void GMTAPI_Unregister_Export_ (int *object_ID, int *error)
{	/* Fortran version: We pass the global GMT_FORTRAN structure*/
	(void) GMTAPI_Unregister_Export (GMT_FORTRAN, *object_ID, error);
}

int GMTAPI_Unregister_Import (struct GMTAPI_CTRL *API, int object_ID, int *error)
{
	/* Removes a registerd import object from the list of objects.
	 * object_ID is the registered ID of the object to be removed */
	 
	return (GMTAPI_Unregister_IO (API, object_ID, GMT_IN, error));
}

void GMTAPI_Unregister_Import_ (int *object_ID, int *error)
{	/* Fortran version: We pass the global GMT_FORTRAN structure*/
	(void) GMTAPI_Unregister_Import (GMT_FORTRAN, *object_ID, error);
}

int GMTAPI_Register_IO (struct GMTAPI_CTRL *API, int method, void **input, double parameters[], int direction, int *object_ID, int *error)
{
	/* Adds a new data object to the list of registered objects and returns a unique object ID.
	 * Arguments are as listed for GMTAPI_Register_Im|Export ()
	 */
	static char *name[2] = {"Input", "Output"};
	struct GMTAPI_DATA_OBJECT *S;

	if (API == NULL) return ((*error = GMTAPI_NOT_A_SESSION));

	S = (struct GMTAPI_DATA_OBJECT *) GMT_memory (VNULL, 1, sizeof (struct GMTAPI_DATA_OBJECT), "GMTAPI_Register_IO");

	S->direction = direction;
	switch (method)
	{
		case GMT_IS_FILE:
			S->ptr = GMTAPI_duplicate_string ((char *)(*input));
			S->method = GMT_IS_FILE;
			break;

	 	case GMT_IS_STREAM:
			S->ptr = input;
			S->method = GMT_IS_STREAM;
			break;

	 	case GMT_IS_FDESC:
			S->ptr = input;
			S->method = GMT_IS_FDESC;
			break;

	 	case GMT_IS_ARRAY:
			S->ptr = input;
			if (!parameters) {
				fprintf (stderr, "GMT: Error in GMT_Register_%s: Parameters are NULL\n", name[direction]);
				return ((*error = GMTAPI_NO_PARAMETERS));
			}
			memcpy ((void *)S->arg, (void *)parameters, GMTAPI_N_ARRAY_ARGS * sizeof (double));
			S->method = GMT_IS_ARRAY;
			break;

	 	case GMT_IS_GRIDFILE:
			S->ptr = GMTAPI_duplicate_string ((char *)(*input));
			S->method = GMT_IS_GRIDFILE;
			break;

	 	case GMT_IS_GRID:
			S->ptr = input;
			if (!parameters) {
				fprintf (stderr, "GMT: Error in GMT_Register_%s: Parameters are NULL\n", name[direction]);
				return ((*error = GMTAPI_NO_PARAMETERS));
			}
			memcpy ((void *)S->arg, (void *)parameters, GMTAPI_N_GRID_ARGS * sizeof (double));
			S->method = GMT_IS_GRID;
			break;

	 	case GMT_IS_GMTGRID:
			S->ptr = input;
			S->method = GMT_IS_GMTGRID;
			break;

		default:
			fprintf (stderr, "GMT: Error in GMT_Register_%s: Unrecognized method %d\n", name[direction], method);
			return ((*error = GMTAPI_NOT_A_VALID_METHOD));
			break;
	}

	*object_ID = GMTAPI_Add_Data_Object (API, S);
	return ((*error = GMTAPI_OK));
}

void GMTAPI_Register_IO_ (int *method, void *input, double parameters[], int *direction, int *object_ID, int *error)
{	/* Fortran version: We pass the global GMT_FORTRAN structure*/
	(void) GMTAPI_Register_IO (GMT_FORTRAN, *method, &input, parameters, *direction, object_ID, error);
}

int GMTAPI_Unregister_IO (struct GMTAPI_CTRL *API, int object_ID, int direction, int *error)
{	/* Remove object from internal list of objects */
	int i, k;
	
	if (API == NULL) return ((*error = GMTAPI_NOT_A_SESSION));	/* GMT_New_Session has not been called */
	
	 /* Search for the object in the list */
	for (i = 0, k = -1; k == -1 && i < API->n_data_alloc; i++) {
		 if (API->data[i] && API->data[i]->ID == object_ID) k = i;
	}
	if (k == -1) return ((*error = GMTAPI_NOT_A_VALID_ID));	/* No such object found */
	
	/* OK, we found the object; is it the right kind (input or output)? */
	if (API->data[k]->direction != direction) {
		/* Trying to free up an input but calling it output or vice versa */
		if (direction == GMT_IN) return ((*error = GMTAPI_NOT_INPUT_OBJECT));
		if (direction == GMT_OUT) return ((*error = GMTAPI_NOT_OUTPUT_OBJECT));
	}
	
	/* OK, now it is safe to remove the object */
	
	GMT_free ((void *)API->data[k]);	/* Free the current object */
	API->data[k] = NULL;			/* Flag as unused object index */
	API->n_data--;				/* Tally of how many data sets are left */
	
	return ((*error = GMTAPI_OK));
}

void GMTAPI_Unregister_IO_ (int *object_ID, int *direction, int *error)
{	/* Fortran version: We pass the global GMT_FORTRAN structure*/
	(void) GMTAPI_Unregister_IO (GMT_FORTRAN, *object_ID, *direction, error);
}

/*========================================================================================================
 *          HERE ARE THE DEVELOPER GMT API UTILITY FUNCTIONS
 *========================================================================================================
 */

int GMTAPI_Import_Dataset (struct GMTAPI_CTRL *API, int inarg[], struct GMT_DATASET **data)
{
	int i, ID, item, col, row, n_cols, par[GMTAPI_N_ARRAY_ARGS];
	size_t ij, n_alloc = GMT_TINY_CHUNK;
	struct GMT_DATASET *D = NULL;
	int col_check (struct GMT_TABLE *T, int *n_cols);
	
	D = (struct GMT_DATASET *)GMT_memory (VNULL, 1, sizeof (struct GMT_DATASET), "GMTAPI_Import_Dataset");
	D->table = (struct GMT_TABLE **)GMT_memory (VNULL, n_alloc, sizeof (struct GMT_TABLE *), "GMTAPI_Import_Dataset");
	i = n_cols = 0;
	
	while ((ID = inarg[i]) > 0) {	/* We end when we encounter the terminator ID == 0 */
		item = ID - 1;
		GMTAPI_par_to_ipar (API->data[item]->arg, par);
		switch (API->data[item]->method) {	/* File, array, stream etc ? */
			case GMT_IS_FILE:	/* Import all the segments, then count total number of records */
	 		case GMT_IS_STREAM:
	 		case GMT_IS_FDESC:
				GMT_import_table ((*API->data[item]->ptr), API->data[item]->method, &(D->table[D->n_tables]), 0.0, FALSE, FALSE, TRUE);
				break;
	 		case GMT_IS_ARRAY:
				/* Each array source becomes a separate table with one segment */
				D->table[D->n_tables] = (struct GMT_TABLE *)GMT_memory (VNULL, 1, sizeof (struct GMT_TABLE), "GMTAPI_Import_Dataset");
				D->table[D->n_tables]->segment = (struct GMT_LINE_SEGMENT **)GMT_memory (VNULL, 1, sizeof (struct GMT_LINE_SEGMENT *), "GMTAPI_Import_Dataset");
				D->table[D->n_tables]->segment[0] = (struct GMT_LINE_SEGMENT *)GMT_memory (VNULL, 1, sizeof (struct GMT_LINE_SEGMENT), "GMTAPI_Import_Dataset");
				GMT_alloc_segment (D->table[D->n_tables]->segment[0], par[GMTAPI_NROW], par[GMTAPI_NCOL], TRUE);
				for (row = 0; row < par[GMTAPI_NROW]; row++) {
					for (col = 0; col < par[GMTAPI_NCOL]; col++) {
						ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row, col, par[GMTAPI_DIML]);
						D->table[D->n_tables]->segment[0]->coord[col][row] = GMTAPI_get_val (API->data[item]->ptr, ij, par[GMTAPI_TYPE]);
					}
				}
				D->table[D->n_tables]->segment[0]->n_rows = par[GMTAPI_NROW];
				D->table[D->n_tables]->segment[0]->n_columns = D->table[D->n_tables]->n_columns = par[GMTAPI_NCOL];
				D->table[D->n_tables]->n_records += par[GMTAPI_NROW];
				D->table[D->n_tables]->n_segments = 1;
				if (par[GMTAPI_FREE] == 1) GMT_free (*API->data[item]->ptr);
				break;
		}
		D->n_segments += D->table[D->n_tables]->n_segments;	/* Sum up total number of segments across the data set */
		D->n_records += D->table[D->n_tables]->n_records;	/* Sum up total number of records across the data set */
		D->n_tables++;
		if (D->n_tables == n_alloc) {	/* Must allocate space for more tables */
			n_alloc += GMT_TINY_CHUNK;
			D->table = (struct GMT_TABLE **)GMT_memory ((void *)D->table, n_alloc, sizeof (struct GMT_TABLE *), "GMTAPI_Import_Dataset");
		}
		if (col_check (D->table[D->n_tables-1], &n_cols)) return (GMTAPI_N_COLS_VARY);
			
		i++;	/* Check the next source */
	}

	if (D->n_tables < n_alloc) D->table = (struct GMT_TABLE **)GMT_memory ((void *)D->table, (int)D->n_tables, sizeof (struct GMT_TABLE *), "GMTAPI_Import_Dataset");
	
	*data = D;

	return (GMTAPI_OK);		
}

int GMTAPI_Export_Dataset (struct GMTAPI_CTRL *API, int outarg, struct GMT_DATASET *D)
{
	int tbl, seg, row, col, offset, par[GMTAPI_N_ARRAY_ARGS];
	size_t ij;
	void *ptr;
		
	if (outarg == 0) return (GMTAPI_NOT_A_VALID_ID);
	
	outarg--;	/* Since IDs start at 1 */
	GMTAPI_par_to_ipar (API->data[outarg]->arg, par);
	
	switch (API->data[outarg]->method) {	/* File, array, stream etc ? */
		case GMT_IS_FILE:
	 	case GMT_IS_STREAM:
	 	case GMT_IS_FDESC:
			for (tbl = 0; tbl < D->n_tables; tbl++)
				GMT_export_table (*API->data[outarg]->ptr, API->data[outarg]->method, D->table[tbl], TRUE);
			break;
	 	case GMT_IS_ARRAY:
			if (par[GMTAPI_FREE] == 1) {	/* Must allocate output space */
				size_t size;
				void **v;
				size = D->n_records;
				if (GMT_io.multi_segments[GMT_OUT]) size += D->n_segments;
				size *= (((size_t)(D->table[0]->n_columns)) * ((size_t)(API->GMTAPI_size[par[GMTAPI_TYPE]])));
				v = (void **)GMT_memory (VNULL, 1, sizeof (void *), "GMTAPI_Import_Dataset");
				*v = GMT_memory (VNULL, size, sizeof (char), "GMTAPI_Import_Dataset");
				*(API->data[outarg]->ptr) = *v;
			}
			ptr = *API->data[outarg]->ptr;
				
			for (tbl = offset = 0; tbl < D->n_tables; tbl++) {
				for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
					for (row = 0; row < D->table[tbl]->segment[seg]->n_rows; row++) {
						for (col = 0; col < D->table[tbl]->segment[seg]->n_columns; col++) {
							ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row + offset, col, par[GMTAPI_DIML]);
							GMTAPI_put_val (ptr, D->table[tbl]->segment[seg]->coord[col][row], ij, par[GMTAPI_TYPE]);
						}
					}
					offset += D->table[tbl]->segment[seg]->n_rows;	/* Since row starts at 0 for each segment */
				}
			}
			break;
	}
	
	return (GMTAPI_OK);		
}

int GMTAPI_Import_Grid (struct GMTAPI_CTRL *API, int inarg, struct GMT_GRID **grid)
{
	struct GMT_GRID *G = NULL;
	int allocated = TRUE;
	int row, col, ij, pad[4] = {0, 0, 0, 0}, par[GMTAPI_N_ARRAY_ARGS];
	char *argv[1] = {"GMTAPI_Import_Grid"};
	
	if (inarg == 0) return (GMTAPI_NOT_A_VALID_ID);
	inarg--;	/* Since IDs start at 1 */
	GMTAPI_par_to_ipar (API->data[inarg]->arg, par);
	
	switch (API->data[inarg]->method) {
		case GMT_IS_GRIDFILE:	/* Name of a grid file on disk */
			G = GMT_create_grid ("GMTAPI_Import_Grid");
			if (GMT_access ((char *)(*API->data[inarg]->ptr), R_OK)) {
				fprintf (stderr, "%s: File %s not found\n", GMT_program, (char *)(*API->data[inarg]->ptr));
				return (GMTAPI_FILE_NOT_FOUND);
			}
			GMT_grd_init (G->header, 1, argv, FALSE);
			if (GMT_err_pass (GMT_read_grd_info ((char *)(*API->data[inarg]->ptr), G->header), (char *)(*API->data[inarg]->ptr))) return (GMTAPI_BAD_PERMISSION);
			G->data = (float *) GMT_memory (VNULL, (size_t)G->header->nx * G->header->ny, sizeof (float), GMT_program);
			if (GMT_err_pass (GMT_read_grd ((char *)(*API->data[inarg]->ptr), G->header, G->data, 0.0, 0.0, 0.0, 0.0, pad, FALSE), (char *)(*API->data[inarg]->ptr))) return (GMTAPI_GRID_READ_ERROR);
			break;
	 	case GMT_IS_GRID:	/* The user's 2-D grid array of some sort, + info in the args */
			G = GMT_create_grid ("GMTAPI_Import_Grid");
			GMT_grd_init (G->header, 1, argv, FALSE);
			GMTAPI_info_to_grdheader (G->header, API->data[inarg]->arg);	/* Populate a GRD header structure */
			if (par[GMTAPI_KIND] == GMTAPI_ORDER_ROW && par[GMTAPI_TYPE] == GMTAPI_FLOAT && par[GMTAPI_FREE] == 0) {	/* Can just pass the pointer */
				G->data = (float *)(*API->data[inarg]->ptr);
				allocated = FALSE;	/* No memory needed to be allocated (so none should be freed later */
			}
			else {	/* Must convert to new array */
				G->data = (float *) GMT_memory (VNULL, (size_t)G->header->nx * G->header->ny, sizeof (float), GMT_program);
				for (row = 0; row < G->header->ny; row++) {
					for (col = 0; col < G->header->nx; col++) {
						ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row, col, par[GMTAPI_DIML]);
						G->data[ij] = GMTAPI_get_val (API->data[inarg]->ptr, ij, par[GMTAPI_TYPE]);
					}
				}
				if (par[GMTAPI_FREE] == 1) GMT_free (*API->data[inarg]->ptr);
			}
			break;
	 	case GMT_IS_GMTGRID:	/* GMT grid and header in a GMT_GRID container object */
			G = (struct GMT_GRID *)(*API->data[inarg]->ptr);
			break;
	}

	*grid = G;
	
	return (allocated);		
}

int GMTAPI_Export_Grid (struct GMTAPI_CTRL *API, int outarg, struct GMT_GRID *G)
{
	int row, col, pad[4] = {0, 0, 0, 0}, par[GMTAPI_N_ARRAY_ARGS];
	size_t ij;
	void *ptr;
	
	if (outarg == 0) return (GMTAPI_NOT_A_VALID_ID);
	outarg--;	/* Since IDs start at 1 */
	
	switch (API->data[outarg]->method) {
		case GMT_IS_GRIDFILE:	/* Name of a grid file on disk */
			if (GMT_err_pass (GMT_write_grd ((char *)(*API->data[outarg]->ptr), G->header, G->data, 0.0, 0.0, 0.0, 0.0, pad, FALSE), (char *)(*API->data[outarg]->ptr))) return (GMTAPI_GRID_WRITE_ERROR);
			break;
	 	case GMT_IS_GRID:	/* The user's 2-D grid array of some sort, + info in the args */
			GMTAPI_grdheader_to_info (G->header, API->data[outarg]->arg);	/* Populate an array with GRD header information */
			GMTAPI_par_to_ipar (API->data[outarg]->arg, par);
			if (par[GMTAPI_KIND] == GMTAPI_ORDER_ROW && par[GMTAPI_TYPE] == GMTAPI_FLOAT && par[GMTAPI_FREE] == 0) {	/* Can just pass the pointer */
				API->data[outarg]->ptr = (void **)(&(G->data));
			}
			else {	/* Must convert to new array */
				if (par[GMTAPI_FREE] == 1) {	/* Must allocate output space */
					size_t size;
					void **v;
					size = ((size_t)G->header->nx) * ((size_t)G->header->ny) * ((size_t)(API->GMTAPI_size[par[GMTAPI_TYPE]]));
					v = (void **)GMT_memory (VNULL, 1, sizeof (void *), "GMTAPI_Export_Grid");
					*v = GMT_memory (VNULL, size, sizeof (char), "GMTAPI_Export_Grid");
					*(API->data[outarg]->ptr) = *v;
				}
				ptr = *API->data[outarg]->ptr;
				for (row = 0; row < G->header->ny; row++) {
					for (col = 0; col < G->header->nx; col++) {
						ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row, col, par[GMTAPI_DIML]);
						GMTAPI_put_val (ptr, (double)G->data[ij], ij, par[GMTAPI_TYPE]);
					}
				}
			}
			break;
	 	case GMT_IS_GMTGRID:	/* GMT grid and header in a GMT_GRID container object - just pass the reference */
			API->data[outarg]->ptr = (void **)(&G);
			break;
	}

	return (GMTAPI_OK);		
}

int GMTAPI_Destroy_IO_Session (struct GMTAPI_CTRL *API, int session_id)
{	/* Remove session from internal list of sessions */
	struct GMTAPI_IO *IO;
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);			/* GMT_New_Session has not been called */
		 
	if (!API->io_session[session_id]) return (GMTAPI_NOT_A_VALID_IO_SESSION);	/* No such IO session found */
	
	/* Arrays allocated for output needs to have their final size determined */
	
	IO = API->io_session[session_id];
	if (IO->n_alloc && IO->current_rec < IO->n_alloc) {	/* Must trim off some memory */
		size_t size;
		size = IO->n_alloc = IO->current_rec;
		size *= (((size_t)(GMT_io.ncol[GMT_OUT])) * ((size_t)(API->GMTAPI_size[IO->type])));
		IO->ptr = GMT_memory (IO->ptr, size, sizeof (char), "GMTAPI_Export_Record");
	}

	if (IO->fp && IO->close_file) GMT_fclose (IO->fp);	/* File we opened that now should be closed */

	/* OK, now we can delete the session */
	
	GMT_free ((void *)IO->ID);			/* Free the current IO session's ID list */
	GMT_free ((void *)IO);				/* Free the current IO session */
	API->io_session[session_id] = NULL;		/* Flagged as an unused session */
	API->n_io_sessions--;				/* Tally of how many IO sessions are left */
	
	return (GMTAPI_OK);
}

int GMTAPI_Create_IO_Session (struct GMTAPI_CTRL *API, int in_or_out, int ID[])
{
	int session_id;
	
	switch (in_or_out) {
		case GMT_IN:
			session_id = GMTAPI_Init_Import_Session (API, ID);
			break;
		case GMT_OUT:
			session_id = GMTAPI_Init_Export_Session (API, ID[0]);
			break;
		default:
			session_id = GMTAPI_NOT_A_VALID_IO;
			break;
	}
	
	return (session_id);
}


int GMTAPI_Init_Import_Session (struct GMTAPI_CTRL *API, int ID[])
{
	return (GMTAPI_Prep_IO_Session (API, ID, GMT_IN));
}

int GMTAPI_Init_Export_Session (struct GMTAPI_CTRL *API, int ID)
{
	return (GMTAPI_Prep_IO_Session (API, &ID, GMT_OUT));	/* Treat the ID as an array of length 1 */
}

int GMTAPI_Import_Record (struct GMTAPI_CTRL *API, int io_session, double **record)
{
	/* Retrieves the next data record and returns the number of columns found.
	 * If current record is a segment header then we return 0.
	 * If we reach EOF then we return EOF
	 */

	GMT_LONG get_next_record = FALSE;
	int retval = 0, col, n_nan;
	size_t ij;
	struct GMTAPI_IO *IO;
	
	IO = API->io_session[io_session];	/* Shorthand */
	
	do {	/* We do this until we can secure the next record or run out of records (EOF) */
		switch (IO->current_method) {
			case GMT_IS_FILE:	/* File, stream, ad fd are all the same for us */
		 	case GMT_IS_STREAM:
		 	case GMT_IS_FDESC:
				retval = IO->n_columns = GMT_input (IO->fp, &(IO->n_expected_fields), record);
				if (GMT_io.status & GMT_IO_EOF) {			/* End-of-file in current file */
					if (IO->close_file) GMT_fclose (IO->fp);	/* Close if it was a file we opened */
					IO->current_item++;				/* Advance to next source item */
					if (IO->current_item == IO->n_items)		/* That was the last source, exit */
						retval = EOF;
					else  {
						GMTAPI_Next_IO_Source (API, IO, GMT_IN);
						get_next_record = FALSE;
					}

				}
				break;
				
		 	case GMT_IS_ARRAY:
				if (IO->current_rec >= IO->n_rows) return (EOF);	/* Done with this array */
				for (col = n_nan = 0; col < IO->n_columns; col++) {
					ij = API->GMT_2D_to_index[IO->kind] (IO->current_rec, col, IO->dimension);
					*record[col] = GMTAPI_get_val (IO->ptr, ij, IO->type);
					if (GMT_is_dnan (*record[col])) n_nan++;
				}
				if (n_nan == IO->n_columns && GMT_io.multi_segments[GMT_IN]) {	/* Flag as multisegment header */
					GMT_io.status = GMT_IO_SEGMENT_HEADER;
				}
				break;
			default:
				fprintf (stderr, "GMTAPI: Internal Error: GMT_Import_record called with illegal method\n");
				break;
		}
	} while (get_next_record);
	
	IO->current_rec++;

	return (retval);		
}

int GMTAPI_Export_Record (struct GMTAPI_CTRL *API, int io_session, int n_columns, double *record)
{	/* n_columns == 0 signals a multisegment header */
	int outarg, col;
	size_t ij;
	struct GMTAPI_IO *IO;
	
	IO = API->io_session[io_session];
	outarg = IO->ID[0] - 1;
	switch (API->data[outarg]->method) {	/* File, array, stream etc ? */
		case GMT_IS_FILE:
	 	case GMT_IS_STREAM:
	 	case GMT_IS_FDESC:
			if (n_columns == 0) {
				if (GMT_io.multi_segments[GMT_OUT]) GMT_write_segmentheader (IO->fp, GMT_io.ncol[GMT_OUT]);
			}
			else
				GMT_output (IO->fp, n_columns, record);
			break;
	 	case GMT_IS_ARRAY:
			if (IO->n_rows && IO->current_rec >= IO->n_rows) {
				fprintf (stderr, "GMTAPI: GMT_Export_record exceeding limits on rows(?)\n");
			}
			if (n_columns == 0 && GMT_io.multi_segments[GMT_OUT]) {	/* Multisegment header - flag in data as NaNs */
				n_columns = GMT_io.ncol[GMT_OUT];
				for (col = 0; col < n_columns; col++) record[col] = GMT_d_NaN;
			}
			for (col = 0; col < n_columns; col++) {
				ij = API->GMT_2D_to_index[IO->kind] (IO->current_rec, col, IO->dimension);
				GMTAPI_put_val (IO->ptr, record[col], ij, IO->type);
			}
			break;
		default:
			fprintf (stderr, "GMTAPI: Internal Error: GMT_Export_record called with illegal method\n");
			break;
	}
	if (n_columns) IO->current_rec++;	/* Only increment if we placed a data record on the output */
	
	if (IO->n_alloc && IO->current_rec == IO->n_alloc) {	/* Must allocate more memory */
		size_t size;
		IO->n_alloc += GMT_CHUNK;
		size = IO->n_alloc;
		size *= (((size_t)(GMT_io.ncol[GMT_OUT])) * ((size_t)(API->GMTAPI_size[IO->type])));
		IO->ptr = GMT_memory (IO->ptr, size, sizeof (char), "GMTAPI_Export_Record");
	}
	
	return (GMTAPI_OK);		
}

/*==================================================================================================
 *		PRIVATE FUNCTIONS ONLY USED BY THIS LIBRARY FILE
 *==================================================================================================
 */

int GMTAPI_Add_Data_Object (struct GMTAPI_CTRL *API, struct GMTAPI_DATA_OBJECT *object)
{	/* Hook object to linked list and assign a unique id (> 0) which is returned */

	int i, ID;
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);	/* GMT_New_Session has not been called */
	
	/* Find the first entry in the API->data array which is unoccupied, and if
	 * they are all occupied then reallocate the array to make more space.
	 * We thus find and return the lowest available ID. */
	 
	for (i = 0; i < API->n_data_alloc && API->data[i]; i++);
	if (i == API->n_data_alloc) {	/* Must allocate more space for data descriptors */
		API->n_data_alloc += GMT_SMALL_CHUNK;
		API->data = (struct GMTAPI_DATA_OBJECT **) GMT_memory ((void *)API->data, API->n_data_alloc, sizeof (struct GMTAPI_DATA_OBJECT *), "GMTAPI_Add_Data_Object");
	}
	ID = i + 1;
	object->ID = ID;		/* This is the lowest available ID */
	API->data[i] = object;		/* Hook the current object into this spot in the list */
	API->n_data++;			/* Tally of how many data sets are currently registered */
	
	return (ID);
}

int GMTAPI_Add_IO_Session (struct GMTAPI_CTRL *API)
{
	int i;
	
	/* Find the first entry in the API->io_session array which is unoccupied, and if
	 * they are all occupied then reallocate the array to make more space.
	 * We thus find and return the lowest available ID. */
	 
	for (i = 0; i < API->n_io_sessions_alloc && API->io_session[i]; i++);
	if (i == API->n_io_sessions_alloc) {	/* Must allocate more space for io_session descriptors */
		API->n_io_sessions_alloc += GMT_TINY_CHUNK;
		API->io_session = (struct GMTAPI_IO **) GMT_memory ((void *)API->io_session, API->n_io_sessions_alloc, sizeof (struct GMTAPI_IO *), "GMTAPI_Init_Import_Session");
	}
	API->io_session[i] = (struct GMTAPI_IO *) GMT_memory (VNULL, 1, sizeof (struct GMTAPI_IO), "GMTAPI_Init_Import_Session");
	API->n_io_sessions++;			/* Tally of how many IO sessions are currently registered */
	
	return (i);
}

int GMTAPI_Prep_IO_Session (struct GMTAPI_CTRL *API, int ID[], int direction)
{
	int session_id;
	struct GMTAPI_IO *IO;
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);	/* GMT_New_Session has not been called */
	
	session_id = GMTAPI_Add_IO_Session (API);
	IO = API->io_session[session_id];	/* For shorthand purposes only */
	if (direction == GMT_IN) {	/* Determine how many input sources there are */
		while (ID[IO->n_items] > 0) IO->n_items++;
		IO->n_expected_fields = (GMT_io.ncol[GMT_IN]) ? GMT_io.ncol[GMT_IN] : GMT_MAX_COLUMNS;			/* Init for the GMT_input machinery */
	}
	else	/* Only one output destination at the time */
		IO->n_items = 1;
	IO->ID = (int *)GMT_memory (VNULL, IO->n_items, sizeof (int), "GMTAPI_Init_Import_Session");	/* Allocate space for the list of sources */
	memcpy ((void *)IO->ID, (void *)ID, IO->n_items * sizeof (int));				/* Copy the list */
	GMTAPI_Next_IO_Source (API, IO, direction);								/* Initialize for the first source */

	return (session_id);		
}

int GMTAPI_Next_IO_Source (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO, int direction)
{
	int *fd, item, par[GMTAPI_N_ARRAY_ARGS];
	static char *name[2] = {"Input", "Output"};
	char *mode;
	
	mode = (direction == GMT_IN) ? GMT_io.r_mode : GMT_io.w_mode;
	item = IO->ID[IO->current_item] - 1;
	IO->current_method = API->data[item]->method;
	IO->close_file = FALSE;	/* Do not want to close file pointers passed to us */
	IO->direction = direction;
	switch (API->data[item]->method) {	/* File, array, stream etc ? */
		case GMT_IS_FILE:
			if ((IO->fp = GMT_fopen ((char *)(*API->data[item]->ptr), mode)) == NULL) {
				fprintf (stderr, "%s: Unable to open file %s for %s\n", GMT_program, (char *)(*API->data[item]->ptr), name[direction]);
				GMT_exit (EXIT_FAILURE);
			}
			IO->close_file = TRUE;	/* We do want to close files we are opening */
			break;
	 	case GMT_IS_STREAM:
			IO->fp = (FILE *)(*API->data[item]->ptr);
			break;
	 	case GMT_IS_FDESC:
			fd = (int *)(*API->data[item]->ptr);
			if ((IO->fp = GMT_fdopen (*fd, mode)) == NULL) {
				fprintf (stderr, "%s: Unable to open file descriptor %d for %s\n", GMT_program, *fd, name[direction]);
				GMT_exit (EXIT_FAILURE);
			}
			break;
	 	case GMT_IS_ARRAY:
			GMTAPI_par_to_ipar (API->data[item]->arg, par);
			if (direction == GMT_OUT && par[GMTAPI_FREE] == 1) {	/* Must allocate output space */
				size_t size;
				void **v;
				size = IO->n_alloc = GMT_CHUNK;
				size *= (((size_t)(par[GMTAPI_NCOL])) * ((size_t)(API->GMTAPI_size[par[GMTAPI_TYPE]])));
				v = (void **)GMT_memory (VNULL, 1, sizeof (void *), "GMTAPI_Next_IO_Source");
				*v = GMT_memory (VNULL, size, sizeof (char), "GMTAPI_Next_IO_Source");
				*(API->data[item]->ptr) = *v;
				/* IO->n_rows is 0 which means we are allocating more space as we need it */
			}
			else
				IO->n_rows = par[GMTAPI_NROW];	/* Hard-wired limit as pass in from calling program */
			IO->ptr = (*API->data[item]->ptr);	/* Pass along the pointer */
			IO->type = par[GMTAPI_TYPE];
			IO->kind = par[GMTAPI_KIND];
			IO->dimension = par[GMTAPI_DIML];
			GMT_io.ncol[direction] = par[GMTAPI_NCOL];	/* Basically, we are doing what GMT calls binary i/o */
			GMT_io.binary[direction] = TRUE;
			break;
		default:
			fprintf (stderr, "GMTAPI: Internal Error: GMTAPI_Next_IO_Source called with illegal method\n");
			break;
	}

	return (GMTAPI_OK);		
}

double GMTAPI_get_val (void *ptr, size_t i, int type)
{	/* Retrives a double value from the <type> array pointed to by the void pointer */
	char *b;
	short int *i2;
	int *i4;
	float *f;
	double *d;
	double val = 0.0;
	
	switch (type) {
		case GMTAPI_BYTE:
			b = (char *)ptr;
			val = (double)b[i];
			break;
		case GMTAPI_SHORT:
			i2 = (short *)ptr;
			val = (double)i2[i];
			break;
		case GMTAPI_INT:
			i4 = (int *)ptr;
			val = (double)i4[i];
			break;
		case GMTAPI_FLOAT:
			f = (float *)ptr;
			val = (double)f[i];
			break;
		case GMTAPI_DOUBLE:
			d = (double *)ptr;
			val = d[i];
			break;
	}
	return (val);
}

void GMTAPI_put_val (void *ptr, double val, size_t i, int type)
{	/* Places a double value in the <type> array pointed to by the void pointer */
	char *b;
	short int *i2;
	int *i4;
	float *f;
	double *d;
	
	switch (type) {
		case GMTAPI_BYTE:
			b = (char *)ptr;
			b[i] = (char)val;
			break;
		case GMTAPI_SHORT:
			i2 = (short *)ptr;
			i2[i] = (short)val;
			break;
		case GMTAPI_INT:
			i4 = (int *)ptr;
			i4[i] = (int)val;
			break;
		case GMTAPI_FLOAT:
			f = (float *)ptr;
			f[i] = (float)val;
			break;
		case GMTAPI_DOUBLE:
			d = (double *)ptr;
			d[i] = val;
			break;
	}
}

void GMTAPI_par_to_ipar (double par[], int ipar[])
{	/* Returns the integer values in par in integer form unless pointer is NULL */
	if (par == NULL) return;
	ipar[GMTAPI_TYPE] = irint (par[GMTAPI_TYPE]);
	ipar[GMTAPI_NDIM] = irint (par[GMTAPI_NDIM]);
	ipar[GMTAPI_NROW] = irint (par[GMTAPI_NROW]);
	ipar[GMTAPI_NCOL] = irint (par[GMTAPI_NCOL]);
	ipar[GMTAPI_KIND] = irint (par[GMTAPI_KIND]);
	ipar[GMTAPI_DIML] = irint (par[GMTAPI_DIML]);
	ipar[GMTAPI_FREE] = irint (par[GMTAPI_FREE]);
}

void GMTAPI_grdheader_to_info (struct GRD_HEADER *h, double info[])
{	/* Packs the necessary items of the header into a double array */
	info[GMTAPI_NROW] = (double)h->nx;
	info[GMTAPI_NCOL] = (double)h->ny;
	info[GMTAPI_NODE] = (double)h->node_offset;
	info[GMTAPI_XMIN] = h->x_min;
	info[GMTAPI_XMAX] = h->x_max;
	info[GMTAPI_YMIN] = h->y_min;
	info[GMTAPI_YMAX] = h->y_max;
}

void GMTAPI_info_to_grdheader (struct GRD_HEADER *h, double info[])
{	/* Unpacks the necessary items into the header from a double array */
	h->nx = irint (info[GMTAPI_NROW]);
	h->ny = irint (info[GMTAPI_NCOL]);
	h->node_offset = irint (info[GMTAPI_NODE]);
	h->x_min = info[GMTAPI_XMIN];
	h->x_max = info[GMTAPI_XMAX];
	h->y_min = info[GMTAPI_YMIN];
	h->y_max = info[GMTAPI_YMAX];
	h->xy_off = (h->node_offset) ? 0.0 : 0.5;
	h->x_inc = (h->x_max - h->x_min) / (h->nx - !h->node_offset);
	h->y_inc = (h->y_max - h->y_min) / (h->ny - !h->node_offset);
}

void ** GMTAPI_duplicate_string (char *string)
{	/* Return a void** pointer to a copy of this string.  We do this
	 * since the string might be overwritten later and change its value */
	int len;
	char **t_ptr;
	
	len = strlen (string);
	t_ptr = (char **) GMT_memory (VNULL, 1, sizeof (char *), "GMTAPI_duplicate_string");
	*t_ptr = (char *) GMT_memory (VNULL, len+1, sizeof (char), "GMTAPI_duplicate_string");
	strncpy (*t_ptr, string, len+1);
	return ((void **)t_ptr);
}

int col_check (struct GMT_TABLE *T, int *n_cols) {
	int seg;
	/* Checks that all segments in this table has the correct number of columns */
	
	for (seg = 0; seg < T->n_segments; seg++) {
		if ((*n_cols) == 0 && seg == 0) *n_cols = T->segment[seg]->n_columns;
		if (T->segment[seg]->n_columns != (*n_cols)) return (TRUE);
	}
	return (FALSE);
}

/* Mapping of internal [row][col] indices to a single 1-D index.
 * Internally, row and col starts at 0.  We assume both row and col
 * are 4-byte integers whereas the 1-D index is possibly 8 byte. */

size_t GMTAPI_2D_to_index_C (int row, int col, int dim)
{	/* Maps (row,col) to 1-D index for C */
	return (((size_t)row * (size_t)dim) + (size_t)col);
}

void GMTAPI_index_to_2D_C (int *row, int *col, size_t index, int dim)
{	/* Maps 1-D index to (row,col) for C */
	*col = (int)(index % (size_t)dim);
	*row = (int)(index / (size_t)dim);
}

size_t GMTAPI_2D_to_index_F (int row, int col, int dim)
{	/* Maps (row,col) to 1-D index for Fortran */
	return (((size_t)col * (size_t)dim) + (size_t)row);
}

void GMTAPI_index_to_2D_F (int *row, int *col, size_t index, int dim)
{	/* Maps 1-D index to (row,col) for Fortran */
	*col = (int)(index / (size_t)dim);
	*row = (int)(index % (size_t)dim);
}
