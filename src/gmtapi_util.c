/*--------------------------------------------------------------------
 *	$Id: gmtapi_util.c,v 1.3 2006-03-27 07:38:53 pwessel Exp $
 *
 *	Copyright (c) 1991-2006 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
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
 * Author: Paul Wessel
 * Date:	25-MAR-2006
 * Version:	0.9
 */

#include "gmtapi.h"

#define GMTAPI_INPUT	0
#define GMTAPI_OUTPUT	1

struct GMTAPI_CTRL *GMT_FORTRAN;	/* Global structure needed for FORTRAN */


/* Private functions used by the library */

void GMT_put_val (void *ptr, double val, size_t i, int type);
double GMT_get_val (void *ptr, size_t i, int type);
int GMT_Register_IO (struct GMTAPI_CTRL *API, int method, void **input, double parameters[], int direction);
size_t GMT_2D_to_index_C (int row, int col, int dim);
void GMT_index_to_2D_C (int *row, int *col, size_t index, int dim);
size_t GMT_2D_to_index_F (int row, int col, int dim);
void GMT_index_to_2D_F (int *row, int *col, size_t index, int dim);
void ** GMT_duplicate_string (char *string);
void GMT_par_to_ipar (double par[], int ipar[]);
void GMT_grdheader_to_info (struct GRD_HEADER *h, double info[]);
void GMT_info_to_grdheader (struct GRD_HEADER *h, double info[]);
int GMT_Next_Import_Source (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO);

/* Here are the GMT API utility functions */

int GMT_Create_Session (struct GMTAPI_CTRL **API, int flags)
{
	/* Initializes the GMT API for a new session. */
	
	struct GMTAPI_CTRL * G;
	char *argv[1] = {"GMTAPI"};
	
 	G = (struct GMTAPI_CTRL *) GMT_memory (VNULL, 1, sizeof (struct GMTAPI_CTRL), "GMT_New_Session");
	G->n_alloc = GMT_SMALL_CHUNK;
	G->data = (struct GMTAPI_DATA_OBJECT **) GMT_memory (VNULL, G->n_alloc, sizeof (struct GMTAPI_DATA_OBJECT *), "GMT_New_Session");
	G->GMT_2D_to_index[0] = (PFI)GMT_2D_to_index_C;
	G->GMT_2D_to_index[1] = (PFI)GMT_2D_to_index_F;
	G->GMT_index_to_2D[0] = (PFV)GMT_index_to_2D_C;
	G->GMT_index_to_2D[1] = (PFV)GMT_index_to_2D_F;
	G->GMTAPI_size[GMTAPI_BYTE]   = 1;
	G->GMTAPI_size[GMTAPI_SHORT]  = 2;
	G->GMTAPI_size[GMTAPI_SHORT]  = 4;
	G->GMTAPI_size[GMTAPI_FLOAT]  = 4;
	G->GMTAPI_size[GMTAPI_DOUBLE] = 8;
	
	GMT_begin (1, argv);
	
	*API = G;

	return (GMTAPI_OK);
}

/* Fortran binding */
int GMT_Create_Session_ (int *flags)
{	/* Fortran version: We pass the hidden global GMT_FORTRAN structure*/
	return (GMT_Create_Session (&GMT_FORTRAN, *flags));
}

int GMT_Destroy_Session (struct GMTAPI_CTRL *API)
{
	int i;
	char *argv[1] = {"GMTAPI"};

	/* This terminates the information for the specified session and frees memory */
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);	/* GMT_New_Session has not been called */

	for (i = 0; i < API->n_alloc; i++) if (API->data[i]) GMT_free ((void *)API->data[i]);
 	GMT_free ((void *)API->data);
 	GMT_free ((void *)API);
	
	GMT_end (1, argv);

	return (GMTAPI_OK);
}

/* Fortran binding */
int GMT_Destroy_Session_ ()
{	/* Fortran version: We pass the hidden global GMT_FORTRAN structure*/
	return (GMT_Destroy_Session (GMT_FORTRAN));
}

void GMT_Error (int error)
{
	fprintf (stderr, "GMT: Error returned from GMT API: %d\n",error);
	exit (EXIT_FAILURE);
}

void GMT_Error_ (int *error)
{
	GMT_Error (*error);
}

int GMT_Register_Import (struct GMTAPI_CTRL *API, int method, void **input, double parameters[])
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
	 * GMT_Register_Import will return a unique Data ID integer and allocate and populate
	 * a GMTAPI_DATA_OBJECT structure which is assigned to the list in GMTAPI_CTRL.
	 */
	
	return (GMT_Register_IO (API, method, input, parameters, GMTAPI_INPUT));
}

int GMT_Register_Input_ (int *method, void *input, double parameters[])
{	/* Fortran version */
	return (GMT_Register_Import (GMT_FORTRAN, *method, &input, parameters));
}

int GMT_Register_Export (struct GMTAPI_CTRL *API, int method, void **output, double parameters[])
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
	 * GMT_Register_Import will return a unique Data ID integer and allocate and populate
	 * a GMTAPI_DATA_OBJECT structure which is assigned to the list in GMTAPI_CTRL.
	 */
	 
	return (GMT_Register_IO (API, method, output, parameters, GMTAPI_OUTPUT));
}

int GMT_Register_Output_ (struct GMTAPI_CTRL *API, int *method, void *output, double parameters[])
{	/* Fortran version: We pass the global GMT_FORTRAN structure*/
	return (GMT_Register_Export (GMT_FORTRAN, *method, &output, parameters));
}

int GMT_Register_IO (struct GMTAPI_CTRL *API, int method, void **input, double parameters[], int direction)
{
	static char *name[2] = {"Input", "Output"};
	int i;
	struct GMTAPI_DATA_OBJECT *S;

	if (API == NULL) return (GMTAPI_NOT_A_SESSION);

	S = (struct GMTAPI_DATA_OBJECT *) GMT_memory (VNULL, 1, sizeof (struct GMTAPI_DATA_OBJECT), "GMT_Register_IO");

	S->direction = direction;
	switch (method)
	{
		case GMT_IS_FILE:
			S->ptr = GMT_duplicate_string ((char *)(*input));
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
			memcpy ((void *)S->arg, (void *)parameters, GMTAPI_N_ARRAY_ARGS * sizeof (double));
			S->method = GMT_IS_ARRAY;
			break;

	 	case GMT_IS_GRIDFILE:
			S->ptr = GMT_duplicate_string ((char *)(*input));
			S->method = GMT_IS_GRIDFILE;
			break;

	 	case GMT_IS_GRID:
			S->ptr = input;
			memcpy ((void *)S->arg, (void *)parameters, GMTAPI_N_GRID_ARGS * sizeof (double));
			S->method = GMT_IS_GRID;
			break;

	 	case GMT_IS_GMTGRID:
			S->ptr = input;
			S->method = GMT_IS_GMTGRID;
			break;

		default:
			fprintf (stderr, "GMT: Error in GMT_Register_%s: Unrecognized method %d\n", name[direction], method);
			return (-1);
			break;
	}

	i = GMT_Add_Data_Object (API, S);

	return (i);
}

int GMT_Add_Data_Object (struct GMTAPI_CTRL *API, struct GMTAPI_DATA_OBJECT *object)
{	/* Hook object to linked list and assign a unique id (> 0) which is returned */

	int i, ID;
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);	/* GMT_New_Session has not been called */
	
	/* Find the first entry in the API->data array which is unoccupied, and if
	 * they are all occupied then reallocate the array to make more space.
	 * We thus find and return the lowest available ID. */
	 
	for (i = 0, ID = 1; i < API->n_alloc && API->data[i] && API->data[i]->ID == ID; i++, ID++);
	if (i == API->n_alloc) {	/* Must allocate more space for data descriptors */
		API->n_alloc += GMT_SMALL_CHUNK;
		API->data = (struct GMTAPI_DATA_OBJECT **) GMT_memory ((void *)API->data, API->n_alloc, sizeof (struct GMTAPI_DATA_OBJECT *), "GMT_Add_Data_Object");
	}
	object->ID = ID;		/* This is the lowest available ID */
	API->data[i] = object;		/* Hook the current object into this spot in the list */
	API->n_data++;			/* Tally of how many data sets are currently registered */
	
	return (ID);
}

int GMT_Remove_Data_Object (struct GMTAPI_CTRL *API, int object_ID)
{	/* Remove object from internal list of objects */
	int i;
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);	/* GMT_New_Session has not been called */
		 
	for (i = 0; i < API->n_alloc && API->data[i] && API->data[i]->ID != object_ID; i++);
	if (i == API->n_alloc) return (GMTAPI_NOT_A_VALID_ID);	/* No such object found */
	GMT_free ((void *)API->data[i]);			/* Free the current object */
	API->n_data--;						/* Tally of how many data sets are left */
	
	return (GMTAPI_OK);
}

int GMT_Import_Table (struct GMTAPI_CTRL *API, int inarg[], struct GMT_LINE_SEGMENT **S)
{
	int i, ID, n_segments = 0, item, col, row, n_cols, par[GMTAPI_N_ARRAY_ARGS];
	size_t ij;
	struct GMT_LINE_SEGMENT *segment = NULL;
	
	i = n_cols = 0;
	
	while ((ID = inarg[i]) > 0) {	/* We end when we encounter the terminator ID == 0 */
		item = ID - 1;
		GMT_par_to_ipar (API->data[item]->arg, par);
		switch (API->data[item]->method) {	/* File, array, stream etc ? */
			case GMT_IS_FILE:	/* Import all the segments, then count total number of records */
	 		case GMT_IS_STREAM:
	 		case GMT_IS_FDESC:
				n_segments = GMT_import_segments ((*API->data[item]->ptr), API->data[item]->method, &segment, 0.0, FALSE, FALSE, TRUE);
				break;
	 		case GMT_IS_ARRAY:
				/* Add one more segment to a possibly empty list of segments */
				segment = (struct GMT_LINE_SEGMENT *)GMT_memory ((void *)segment, n_segments+1, sizeof (struct GMT_LINE_SEGMENT), "GMT_Assemble_Data");
				segment[n_segments].coord = (double **)GMT_memory (VNULL, par[GMTAPI_NCOL], sizeof (double *), "GMT_Assemble_Data");
				for (col = 0; col < par[GMTAPI_NCOL]; col++) segment[n_segments].coord[col] = (double *)GMT_memory (VNULL, par[GMTAPI_NROW], sizeof (double), "GMT_Assemble_Data");
				for (row = 0; row < par[GMTAPI_NROW]; row++) {
					for (col = 0; col < par[GMTAPI_NCOL]; col++) {
						ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row, col, par[GMTAPI_DIML]);
						segment[n_segments].coord[col][row] = GMT_get_val (API->data[item]->ptr, ij, par[GMTAPI_TYPE]);
					}
				}
				segment[n_segments].n_rows = par[GMTAPI_NROW];
				segment[n_segments].n_columns = par[GMTAPI_NCOL];
				n_segments++;
				if (par[GMTAPI_FREE] == 0) GMT_free (*API->data[item]->ptr);
				break;
		}
		if (n_cols == 0)	/* First time we import we set n_cols */
			n_cols = segment[n_segments-1].n_columns;
		else if (n_cols != segment[n_segments].n_columns)	/* Later, we check that all segments have the same # of columns */
			return (GMTAPI_N_COLS_VARY);
			
		i++;	/* Check the next source */
	}
	*S = segment;

	return (n_segments);		
}

int GMT_Export_Table (struct GMTAPI_CTRL *API, int outarg, struct GMT_LINE_SEGMENT *S, int n_segments)
{
	int seg, row, col, offset, par[GMTAPI_N_ARRAY_ARGS];
	size_t ij;
	void *ptr;
		
	if (outarg == 0) return (GMTAPI_NOT_A_VALID_ID);
	
	outarg--;	/* Since IDs start at 1 */
	GMT_par_to_ipar (API->data[outarg]->arg, par);
	
	switch (API->data[outarg]->method) {	/* File, array, stream etc ? */
		case GMT_IS_FILE:
	 	case GMT_IS_STREAM:
	 	case GMT_IS_FDESC:
			GMT_export_segments (*API->data[outarg]->ptr, API->data[outarg]->method, S, n_segments, TRUE);
			break;
	 	case GMT_IS_ARRAY:
			if (par[GMTAPI_FREE] == 0) {	/* Must allocate output space */
				size_t size;
				void **v;
				size = GMT_n_segment_points (S, n_segments);
				size *= (((size_t)(S[0].n_columns)) * ((size_t)(API->GMTAPI_size[par[GMTAPI_TYPE]])));
				v = (void **)GMT_memory (VNULL, 1, sizeof (void *), "GMT_Assemble_Data");
				*v = GMT_memory (VNULL, size, sizeof (char), "GMT_Assemble_Data");
				*(API->data[outarg]->ptr) = *v;
			}
			ptr = *API->data[outarg]->ptr;
				
			for (seg = offset = 0; seg < n_segments; seg++) {
				for (row = 0; row < S[seg].n_rows; row++) {
					for (col = 0; col < S[seg].n_columns; col++) {
						ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row + offset, col, par[GMTAPI_DIML]);
						GMT_put_val (ptr, S[seg].coord[col][row], ij, par[GMTAPI_TYPE]);
					}
				}
				offset += S[seg].n_rows;	/* Since row starts at 0 for each segment */
			}
			break;
	}
	
	return (GMTAPI_OK);		
}

int GMT_Import_Grid (struct GMTAPI_CTRL *API, int inarg, struct GMT_GRID **grid)
{
	struct GMT_GRID *G;
	int row, col, ij, pad[4] = {0, 0, 0, 0}, par[GMTAPI_N_ARRAY_ARGS];
	char *argv[1] = {"GMT_Import_Grid"};
	
	if (inarg == 0) return (GMTAPI_NOT_A_VALID_ID);
	inarg--;	/* Since IDs start at 1 */
	GMT_par_to_ipar (API->data[inarg]->arg, par);
	
	switch (API->data[inarg]->method) {
		case GMT_IS_GRIDFILE:	/* Name of a grid file on disk */
			G = (struct GMT_GRID *) GMT_memory (VNULL, 1, sizeof (struct GMT_GRID), "GMT_Import_Grid");
			G->header = (struct GRD_HEADER *) GMT_memory (VNULL, 1, sizeof (struct GRD_HEADER), "GMT_Import_Grid");
			if (GMT_access ((char *)(*API->data[inarg]->ptr), R_OK)) {
				fprintf (stderr, "%s: File %s not found\n", GMT_program, (char *)(*API->data[inarg]->ptr));
				return (GMTAPI_FILE_NOT_FOUND);
			}
			GMT_grd_init (G->header, 1, argv, FALSE);
			if (GMT_read_grd_info ((char *)(*API->data[inarg]->ptr), G->header)) {
				fprintf (stderr, "%s: Error opening grid file %s\n", GMT_program, (char *)(*API->data[inarg]->ptr));
				return (GMTAPI_BAD_PERMISSION);
			}
			G->data = (float *) GMT_memory (VNULL, (size_t)G->header->nx * G->header->ny, sizeof (float), GMT_program);
			if (GMT_read_grd ((char *)(*API->data[inarg]->ptr), G->header, G->data, 0.0, 0.0, 0.0, 0.0, pad, FALSE)) {
				fprintf (stderr, "%s: Error reading grid file %s\n", GMT_program, (char *)(*API->data[inarg]->ptr));
				return (GMTAPI_GRID_READ_ERROR);
			}
			break;
	 	case GMT_IS_GRID:	/* The user's 2-D grid array of some sort, + info in the args */
			G = (struct GMT_GRID *) GMT_memory (VNULL, 1, sizeof (struct GMT_GRID), "GMT_Import_Grid");
			G->header = (struct GRD_HEADER *) GMT_memory (VNULL, 1, sizeof (struct GRD_HEADER), "GMT_Import_Grid");
			GMT_grd_init (G->header, 1, argv, FALSE);
			GMT_info_to_grdheader (G->header, API->data[inarg]->arg);	/* Populate a GRD header structure */
			if (par[GMTAPI_KIND] == GMT_COLUMN_FORMAT && par[GMTAPI_TYPE] == GMTAPI_FLOAT) {	/* Can just pass the pointer */
				G->data = (float *)(*API->data[inarg]->ptr);
			}
			else {	/* Must convert to new array */
				G->data = (float *) GMT_memory (VNULL, (size_t)G->header->nx * G->header->ny, sizeof (float), GMT_program);
				for (row = 0; row < G->header->ny; row++) {
					for (col = 0; col < G->header->nx; col++) {
						ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row, col, par[GMTAPI_DIML]);
						G->data[ij] = GMT_get_val (API->data[inarg]->ptr, ij, par[GMTAPI_TYPE]);
					}
				}
				if (par[GMTAPI_FREE] == 0) GMT_free (*API->data[inarg]->ptr);
			}
			break;
	 	case GMT_IS_GMTGRID:	/* GMT grid and header in a GMT_GRID container object */
			G = (struct GMT_GRID *)(*API->data[inarg]->ptr);
			break;
	}

	*grid = G;
	
	return (G->header->nx * G->header->ny);		
}

int GMT_Export_Grid (struct GMTAPI_CTRL *API, int outarg, struct GMT_GRID *G)
{
	int row, col, pad[4] = {0, 0, 0, 0}, par[GMTAPI_N_ARRAY_ARGS];
	size_t ij;
	void *ptr;
	
	if (outarg == 0) return (GMTAPI_NOT_A_VALID_ID);
	outarg--;	/* Since IDs start at 1 */
	
	switch (API->data[outarg]->method) {
		case GMT_IS_GRIDFILE:	/* Name of a grid file on disk */
			if (GMT_write_grd ((char *)(*API->data[outarg]->ptr), G->header, G->data, 0.0, 0.0, 0.0, 0.0, pad, FALSE)) {
				fprintf (stderr, "%s: Error writing file %s\n", GMT_program, (char *)(*API->data[outarg]->ptr));
				return (GMTAPI_GRID_WRITE_ERROR);
			}
			break;
	 	case GMT_IS_GRID:	/* The user's 2-D grid array of some sort, + info in the args */
			GMT_grdheader_to_info (G->header, API->data[outarg]->arg);	/* Populate an array with GRD header information */
			GMT_par_to_ipar (API->data[outarg]->arg, par);
			if (par[GMTAPI_KIND] == GMT_COLUMN_FORMAT && par[GMTAPI_FREE] == GMTAPI_FLOAT) {	/* Can just pass the pointer */
				API->data[outarg]->ptr = (void **)(&(G->data));
			}
			else {	/* Must convert to new array */
				if (par[GMTAPI_FREE] == 0) {	/* Must allocate output space */
					size_t size;
					void **v;
					size = ((size_t)G->header->nx) * ((size_t)G->header->ny) * ((size_t)(API->GMTAPI_size[par[GMTAPI_TYPE]]));
					v = (void **)GMT_memory (VNULL, 1, sizeof (void *), "GMT_Export_Grid");
					*v = GMT_memory (VNULL, size, sizeof (char), "GMT_Export_Grid");
					*(API->data[outarg]->ptr) = *v;
				}
				ptr = *API->data[outarg]->ptr;
				for (row = 0; row < G->header->ny; row++) {
					for (col = 0; col < G->header->nx; col++) {
						ij = API->GMT_2D_to_index[par[GMTAPI_KIND]] (row, col, par[GMTAPI_DIML]);
						GMT_put_val (ptr, (double)G->data[ij], ij, par[GMTAPI_TYPE]);
					}
				}
			}
			break;
	 	case GMT_IS_GMTGRID:	/* GMT grid and header in a GMT_GRID container object - just pass the reference */
			API->data[outarg]->ptr = (void **)(&G);
			break;
	}

	return (G->header->nx * G->header->ny);		
}

double GMT_get_val (void *ptr, size_t i, int type)
{
	char *b;
	short int *i2;
	int *i4;
	float *f;
	double *d;
	double val;
	
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

void GMT_put_val (void *ptr, double val, size_t i, int type)
{
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

void GMT_par_to_ipar (double par[], int ipar[])
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

void GMT_grdheader_to_info (struct GRD_HEADER *h, double info[])
{	/* Packs the necessary items of the header into a double array */
	info[GMTAPI_NROW] = (double)h->nx;
	info[GMTAPI_NCOL] = (double)h->ny;
	info[GMTAPI_NODE] = (double)h->node_offset;
	info[GMTAPI_XMIN] = h->x_min;
	info[GMTAPI_XMAX] = h->x_max;
	info[GMTAPI_YMIN] = h->y_min;
	info[GMTAPI_YMAX] = h->x_min;
}

void GMT_info_to_grdheader (struct GRD_HEADER *h, double info[])
{	/* Unpacks the necessary items into the header from a double array */
	h->nx = irint (info[GMTAPI_NROW]);
	h->ny = irint (info[GMTAPI_NCOL]);
	h->node_offset = irint (info[GMTAPI_NODE]);
	h->x_min = info[GMTAPI_XMIN];
	h->x_max = info[GMTAPI_XMAX];
	h->y_min = info[GMTAPI_YMIN];
	h->x_min = info[GMTAPI_YMAX];
	h->xy_off = (h->node_offset) ? 0.0 : 0.5;
	h->x_inc = (h->x_max - h->x_min) / (h->nx - !h->node_offset);
	h->y_inc = (h->y_max - h->y_min) / (h->ny - !h->node_offset);
}

void ** GMT_duplicate_string (char *string)
{	/* Return a void** pointer to a copy of this string.  We do this
	 * since the string might be overwritten later and change its value */
	int len;
	char **t_ptr;
	
	len = strlen (string);
	t_ptr = (char **) GMT_memory (VNULL, 1, sizeof (char *), "GMT_duplicate_string");
	*t_ptr = (char *) GMT_memory (VNULL, len+1, sizeof (char), "GMT_duplicate_string");
	strncpy (*t_ptr, string, len);
	return ((void **)t_ptr);
}

int GMT_Init_Import_Record (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO, int ID[])
{
	memset ((void *)IO, 0, sizeof (struct GMTAPI_IO));
	while (ID[IO->n_items] > 0) IO->n_items++;
	IO->ID = (int *)GMT_memory (VNULL, IO->n_items, sizeof (int), "GMT_Init_Import_Record");
	memcpy ((void *)IO->ID, (void *)ID, IO->n_items * sizeof (int));
	GMT_Next_Import_Source (API, IO);
	IO->n_expected_fields = (GMT_io.ncol[GMT_IN]) ? GMT_io.ncol[GMT_IN] : BUFSIZ;
	return (GMTAPI_OK);		
}

int GMT_Next_Import_Source (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO)
{
	int *fd, item;
	
	item = IO->ID[IO->current_item] - 1;
	IO->current_method = API->data[item]->method;
	IO->close_file = FALSE;
	switch (API->data[item]->method) {	/* File, array, stream etc ? */
		case GMT_IS_FILE:
			if ((IO->fp = GMT_fopen ((char *)(*API->data[item]->ptr), GMT_io.r_mode)) == NULL) {
				fprintf (stderr, "%s: Unable to open file %s for reading\n", GMT_program, (char *)(*API->data[item]->ptr));
				exit (EXIT_FAILURE);
			}
			IO->close_file = TRUE;
			break;
	 	case GMT_IS_STREAM:
			IO->fp = (FILE *)(*API->data[item]->ptr);
			break;
	 	case GMT_IS_FDESC:
			fd = (int *)(*API->data[item]->ptr);
			if ((IO->fp = fdopen (*fd, GMT_io.r_mode)) == NULL) {
				fprintf (stderr, "%s: Unable to open file descriptor %d for reading\n", GMT_program, *fd);
				exit (EXIT_FAILURE);
			}
			break;
	 	case GMT_IS_ARRAY:
			break;
	}
	return (GMTAPI_OK);		
}

int GMT_Init_Export_Record (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO, int ID)
{
	int *fd;
	memset ((void *)IO, 0, sizeof (struct GMTAPI_IO));
	IO->n_items = 1;
	IO->ID = (int *)GMT_memory (VNULL, IO->n_items, sizeof (int), "GMT_Init_Import_Record");
	IO->ID[0] = ID;
	ID--;
	IO->current_method = API->data[ID]->method;
	switch (API->data[ID]->method) {	/* File, array, stream etc ? */
		case GMT_IS_FILE:
			if ((IO->fp = GMT_fopen ((char *)(*API->data[ID]->ptr), GMT_io.w_mode)) == NULL) {
				fprintf (stderr, "%s: Unable to open file %s for writing\n", GMT_program, (char *)(*API->data[ID]->ptr));
				exit (EXIT_FAILURE);
			}
			break;
	 	case GMT_IS_STREAM:
			IO->fp = (FILE *)(*API->data[ID]->ptr);
			break;
	 	case GMT_IS_FDESC:
			fd = (int *)(*API->data[ID]->ptr);
			if ((IO->fp = fdopen (*fd, GMT_io.w_mode)) == NULL) {
				fprintf (stderr, "%s: Unable to open file descriptor %d for writing\n", GMT_program, *fd);
				exit (EXIT_FAILURE);
			}
			break;
	 	case GMT_IS_ARRAY:
			break;
	}
	return (GMTAPI_OK);		
}

int GMT_Import_Record (struct GMTAPI_CTRL *API, double **record, struct GMTAPI_IO *IO)
{
	BOOLEAN get_next_record = FALSE;
	int retval = 0;
	
	IO->header = FALSE;
	do {	/* We do this until we can secure the next record or run out of records (EOF) */
		switch (IO->current_method) {	/* File, array, stream etc ? */
			case GMT_IS_FILE:
		 	case GMT_IS_STREAM:
		 	case GMT_IS_FDESC:
				IO->n_columns = GMT_input (IO->fp, &(IO->n_expected_fields), record);
				if (GMT_io.status & GMT_IO_SEGMENT_HEADER) {	/* Found a segment header */
					IO->header = TRUE;			/* Notify we have a header */
				}
				else if (GMT_io.status & GMT_IO_EOF) {		/* End-of-file in current file */
					if (IO->close_file) GMT_fclose (IO->fp);	/* Close if it was a file we opened */
					IO->current_item++;				/* Advance to next source item */
					if (IO->current_item == IO->n_items)		/* That was the last source, exit */
						retval = EOF;
					else  {
						GMT_Next_Import_Source (API, IO);
						get_next_record = FALSE;
					}

				}
				break;
		 	case GMT_IS_ARRAY:
				break;
		}
	} while (get_next_record);

	return (retval);		
}

int GMT_Export_Record (struct GMTAPI_CTRL *API, double *record, struct GMTAPI_IO *IO)
{
	int outarg;
	
	outarg = IO->ID[0] - 1;
	switch (API->data[outarg]->method) {	/* File, array, stream etc ? */
		case GMT_IS_FILE:
	 	case GMT_IS_STREAM:
	 	case GMT_IS_FDESC:
			if (IO->header)
				GMT_write_segmentheader (IO->fp, IO->n_columns);
			else
				GMT_output (IO->fp, IO->n_columns, record);
			break;
	 	case GMT_IS_ARRAY:
			break;
	}
	return (GMTAPI_OK);		
}


/* Mapping of internal [row][col] indeces to a single 1-D index.
 * Internally, row and col starts at 0.  We assume both row and col
 * are 4-byte integers whereas the 1-D index is possibly 8 byte. */

size_t GMT_2D_to_index_C (int row, int col, int dim)
{	/* Maps (row,col) to 1-D index for C */
	return (((size_t)row * (size_t)dim) + (size_t)col);
}

void GMT_index_to_2D_C (int *row, int *col, size_t index, int dim)
{	/* Maps 1-D index to (row,col) for C */
	*col = (int)(index % (size_t)dim);
	*row = (int)(index / (size_t)dim);
}

size_t GMT_2D_to_index_F (int row, int col, int dim)
{	/* Maps (row,col) to 1-D index for Fortran */
	return (((size_t)col * (size_t)dim) + (size_t)row);
}

void GMT_index_to_2D_F (int *row, int *col, size_t index, int dim)
{	/* Maps 1-D index to (row,col) for Fortran */
	*col = (int)(index / (size_t)dim);
	*row = (int)(index % (size_t)dim);
}
