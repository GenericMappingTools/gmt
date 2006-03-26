/*--------------------------------------------------------------------
 *	$Id: gmtapi_util.c,v 1.1 2006-03-26 10:56:13 pwessel Exp $
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

void GMT_put_val (void *ptr, double val, int i, int type);
double GMT_get_val (void *ptr, int i, int type);
int GMT_Register_IO (struct GMTAPI_CTRL *API, int method, void **input, int parameters[], int direction);
size_t GMT_2D_to_index_C (int row, int col, int dim);
void GMT_index_to_2D_C (int *row, int *col, size_t index, int dim);
size_t GMT_2D_to_index_F (int row, int col, int dim);
void GMT_index_to_2D_F (int *row, int *col, size_t index, int dim);

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

	/* This terminates the information for the specified session and frees memory */
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);	/* GMT_New_Session has not been called */

	for (i = 0; i < API->n_alloc; i++) if (API->data[i]) GMT_free ((void *)API->data[i]);
 	GMT_free ((void *)API->data);
 	GMT_free ((void *)API);
	
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

int GMT_Register_Input (struct GMTAPI_CTRL *API, int method, void **input, int parameters[])
{
	/* The main program uses this routine to pass information about input data to GMT.
	 * The method argument specifies what we are trying to pass:
	 * GMTAPI_FILE
	 *	A file name is given via input.
	 * GMTAPI_STREAM
	 *	A file pointer to an open file is passed via input.
	 * GMT_INPUT_GRD
	 *	A GMT grid is passed via the pointer
	 * GMTAPI_ARRAY
	 *	An array of data is passed via input.  In addition, parameters
	 *	must be passed with the following information:
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
	 * GMT_Register_Input will return a unique Data ID integer and allocate and populate
	 * a GMTAPI_DATA_OBJECT structure which is assigned to the list in GMTAPI_CTRL.
	 */
	
	return (GMT_Register_IO (API, method, input, parameters, GMTAPI_INPUT));
}

int GMT_Register_Input_ (int *method, void *input, int parameters[])
{	/* Fortran version */
	return (GMT_Register_Input (GMT_FORTRAN, *method, &input, parameters));
}

int GMT_Register_Output (struct GMTAPI_CTRL *API, int method, void **output, int parameters[])
{
	/* The main program uses this routine to pass information about output data from GMT.
	 * The method argument specifies what we are trying to receive:
	 * GMTAPI_FILE
	 *	A file name is given via output.  The program will write data to this file
	 * GMTAPI_STREAM
	 *	A file pointer to an open file is passed via output. --"--
	 * GMT_OUTPUT_GRID
	 *	A GMT grid is passed via the pointer
	 * GMTAPI_ARRAY
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
	 * GMT_Register_Input will return a unique Data ID integer and allocate and populate
	 * a GMTAPI_DATA_OBJECT structure which is assigned to the list in GMTAPI_CTRL.
	 */
	 
	return (GMT_Register_IO (API, method, output, parameters, GMTAPI_OUTPUT));
}

int GMT_Register_Output_ (struct GMTAPI_CTRL *API, int *method, void *output, int parameters[])
{	/* Fortran version: We pass the global GMT_FORTRAN structure*/
	return (GMT_Register_Output (GMT_FORTRAN, *method, &output, parameters));
}

int GMT_Register_IO (struct GMTAPI_CTRL *API, int method, void **input, int parameters[], int direction)
{
	char **t;
	static char *name[2] = {"Input", "Output"};
	int i, len;
	struct GMTAPI_DATA_OBJECT *S;

	if (API == NULL) return (GMTAPI_NOT_A_SESSION);

	S = (struct GMTAPI_DATA_OBJECT *) GMT_memory (VNULL, 1, sizeof (struct GMTAPI_DATA_OBJECT), "GMT_Register_IO");

	S->direction = direction;
	switch (method)
	{
		case GMTAPI_FILE:
			len = strlen ((char *)(*input));
			t = (char **) GMT_memory (VNULL, 1, sizeof (char *), "GMT_Register_IO");
			*t = (char *) GMT_memory (VNULL, len+1, sizeof (char), "GMT_Register_IO");
			strncpy (*t, (char *)(*input), len);
			S->ptr = (void **)t;
			S->method = GMTAPI_FILE;
			break;

	 	case GMTAPI_STREAM:
			S->ptr = input;
			S->method = GMTAPI_STREAM;
			break;

	 	case GMTAPI_FDESC:
			S->ptr = input;
			S->method = GMTAPI_FDESC;
			break;

	 	case GMTAPI_ARRAY:
			S->ptr = input;
			for (i = 0; i < GMTAPI_N_ARRAY_ARGS; i++) S->arg[i] = parameters[i];
			S->method = GMTAPI_ARRAY;
			break;

	 	case GMTAPI_GRID:
			S->ptr = input;
			S->method = GMTAPI_GRID;
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

int GMT_Input_Data (struct GMTAPI_CTRL *API, int inarg[], struct GMT_LINES **S, struct GMTAPI_ARRAY_INFO *I)
{
	int i, ID, n_segments = 0, k, item, ij, col, row, n_rows, n_cols, kind, **iptr;
	struct GMT_LINES *segment = NULL;
	FILE *fp;
	
	i = n_rows = n_cols = 0;
	memset ((void *)I, 0, sizeof (struct GMTAPI_ARRAY_INFO));
	
	while ((ID = inarg[i]) > 0) {	/* We end when we encounter the terminator ID == 0 */
		item = ID - 1;
		switch (API->data[item]->method) {	/* File, array, stream etc ? */
			case GMTAPI_FILE:
				n_segments = GMT_lines_init ((char *)(*API->data[item]->ptr), &segment, 0.0, FALSE, FALSE, FALSE);
				I->n_segments += n_segments;
				for (k = 0; k < n_segments; k++) I->n_records += segment[k].np;
				if (n_cols == 0)
					n_cols = segment[0].ncol;
				else if (n_cols != segment[0].ncol)
					return (-1);
				I->n_cols = n_cols;
				break;
	 		case GMTAPI_STREAM:
				/* First need to generalize GMT_lines_init to accept a file pointer */
				break;
	 		case GMTAPI_FDESC:
				iptr = (int **)API->data[item]->ptr;
				fp = fdopen (**iptr, GMT_io.r_mode);
				/* First need to generalize GMT_lines_init to accept a file pointer */
				break;
	 		case GMTAPI_ARRAY:
				/* Add one more segment to a possibly empty list of segments */
				if (n_cols == 0)
					n_cols = API->data[item]->arg[GMTAPI_NCOL];
				else if (n_cols != API->data[item]->arg[GMTAPI_NCOL])
					return (-1);
				n_rows = API->data[item]->arg[GMTAPI_NROW];
				kind = API->data[item]->arg[GMTAPI_KIND];
				segment = (struct GMT_LINES *)GMT_memory ((void *)segment, I->n_segments+1, sizeof (struct GMT_LINES), "GMT_Assemble_Data");
				segment[I->n_segments].coord = (double **)GMT_memory (VNULL, n_cols, sizeof (double *), "GMT_Assemble_Data");
				for (col = 0; col < n_cols; col++) segment[I->n_segments].coord[col] = (double *)GMT_memory (VNULL, n_rows, sizeof (double), "GMT_Assemble_Data");
				for (row = 0; row < n_rows; row++) {
					for (col = 0; col < n_cols; col++) {
						ij = API->GMT_2D_to_index[kind] (row, col, API->data[item]->arg[GMTAPI_DIML]);
						segment[I->n_segments].coord[col][row] = GMT_get_val (API->data[item]->ptr, ij, API->data[item]->arg[GMTAPI_TYPE]);
					}
				}
				segment[I->n_segments].np = n_rows;
				I->n_cols = segment[I->n_segments].ncol = n_cols;
				I->n_segments++;
				I->n_records += n_rows;
				if (API->data[item]->arg[GMTAPI_FREE] == 0) GMT_free (*API->data[item]->ptr);
				break;
		}
		i++;	/* Check the next source */
	}
	*S = segment;

	return (I->n_segments);		
}

int GMT_Output_Data (struct GMTAPI_CTRL *API, int outarg, struct GMT_LINES *S, int n_segments, struct GMTAPI_ARRAY_INFO *I)
{
	int i, ij, k, kind, row, col, **iptr;
	double *out;
	void *ptr;
	FILE *fp;
	
	i = 0;
	out = (double *) GMT_memory (VNULL, I->n_cols, sizeof (double), "GMT_Output_Data");
	
	outarg--;	/* Since IDs start at 1 */
	switch (API->data[outarg]->method) {	/* File, array, stream etc ? */
		case GMTAPI_FILE:
				fp = GMT_fopen ((char *)(*API->data[outarg]->ptr), GMT_io.w_mode);
				for (k = 0; k < n_segments; k++) {
					if (GMT_io.multi_segments) GMT_write_segmentheader (fp, S[k].ncol);
					for (row = 0; row < S[k].np; row++) {
						for (col = 0; col < S[k].ncol; col++) out[col] = S[k].coord[col][row];
						GMT_output (fp, S[k].ncol, out);
					}
				}
				GMT_fclose (fp);
				break;
	 	case GMTAPI_STREAM:
			fp = (FILE *)(*API->data[outarg]->ptr);
			for (k = 0; k < n_segments; k++) {
				if (GMT_io.multi_segments) GMT_write_segmentheader (fp, S[k].ncol);
				for (row = 0; row < S[k].np; row++) {
					for (col = 0; col < S[k].ncol; col++) out[col] = S[k].coord[col][row];
					GMT_output (fp, S[k].ncol, out);
				}
			}
			break;
	 	case GMTAPI_FDESC:
			iptr = (int **)API->data[outarg]->ptr;
			fp = fdopen (**iptr, GMT_io.w_mode);
			for (k = 0; k < n_segments; k++) {
				if (GMT_io.multi_segments) GMT_write_segmentheader (fp, S[k].ncol);
				for (row = 0; row < S[k].np; row++) {
					for (col = 0; col < S[k].ncol; col++) out[col] = S[k].coord[col][row];
					GMT_output (fp, S[k].ncol, out);
				}
			}
			break;
	 	case GMTAPI_ARRAY:
			if (API->data[outarg]->arg[GMTAPI_FREE] == 0) {	/* Must allocate output space */
				*API->data[outarg]->ptr = (void *)GMT_memory (VNULL, I->n_cols * I->n_records * API->GMTAPI_size[API->data[outarg]->arg[GMTAPI_TYPE]], sizeof (char), "GMT_Assemble_Data");
			}
			ptr = *API->data[outarg]->ptr;
			kind = API->data[outarg]->arg[GMTAPI_KIND];
				
			for (k = 0; k < n_segments; k++) {
				for (row = 0; row < S[k].np; row++) {
					for (col = 0; col < S[k].ncol; col++) {
						ij = API->GMT_2D_to_index[kind] (row, col, API->data[outarg]->arg[GMTAPI_DIML]);
						GMT_put_val (ptr, S[k].coord[col][row], ij, API->data[outarg]->arg[GMTAPI_TYPE]);
					}
				}
			}
			break;
	}
	GMT_free ((void *)out);
	
	return (GMTAPI_OK);		
}

double GMT_get_val (void *ptr, int i, int type)
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

void GMT_put_val (void *ptr, double val, int i, int type)
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

int GMT_prepare_input ()
{
	return (GMTAPI_OK);		
}

int GMT_prepare_output ()
{
	return (GMTAPI_OK);		
}

int GMT_input_record (double *record)
{
	return (GMTAPI_OK);		
}

int GMT_output_record (struct GMTAPI_CTRL *API, double *record, int outarg, struct GMTAPI_ARRAY_INFO *I)
{
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
