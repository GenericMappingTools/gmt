/*--------------------------------------------------------------------
 *	$Id: gmtapi.h,v 1.19 2008-05-04 00:39:22 guru Exp $
 *
 *	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
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
 * The single include file for users who which to develop applications
 * that require building blocks from the GMT Application Program Interface
 * library (the GMT API).
 * GMT developers who whish to add new GMT core applications also need to
 * include gmtapi_devel.h.
 *
 * Author: 	Paul Wessel
 * Date:	25-MAR-2006
 * Version:	0.1
 */

#ifndef _GMTAPI_H
#define _GMTAPI_H

#include "gmt.h"
#include "f2c.h"
/*=====================================================================================
 *	GMT API CONSTANTS DEFINITIONS
 *=====================================================================================
 */

#define GMTAPI_N_ARRAY_ARGS	8	/* Minimum size of information array used to specify array parameters */
#define GMTAPI_N_GRID_ARGS	12	/* Minimum size of information array used to specify grid parameters */

	/* Misc GMTAPI error codes; can be passed to GMT_Error_Message */
	
#define GMTAPI_OK			  0
#define GMTAPI_ERROR			 -1
#define GMTAPI_NOT_A_SESSION		 -2
#define GMTAPI_NOT_A_VALID_ID		 -3
#define GMTAPI_FILE_NOT_FOUND		 -4
#define GMTAPI_BAD_PERMISSION		 -5
#define GMTAPI_GRID_READ_ERROR		 -6
#define GMTAPI_GRID_WRITE_ERROR		 -7
#define GMTAPI_DATA_READ_ERROR		 -8
#define GMTAPI_DATA_WRITE_ERROR		 -9
#define GMTAPI_N_COLS_VARY		-10
#define GMTAPI_NO_INPUT			-11
#define GMTAPI_NO_OUTPUT		-12
#define GMTAPI_NOT_A_VALID_IO		-13
#define GMTAPI_NOT_A_VALID_IO_SESSION	-14
#define GMTAPI_NOT_INPUT_OBJECT		-15
#define GMTAPI_NOT_OUTPUT_OBJECT	-16
#define GMTAPI_ERROR_ON_FOPEN		-17
#define GMTAPI_ERROR_ON_FCLOSE		-18
#define GMTAPI_NO_PARAMETERS		-19
#define GMTAPI_NOT_A_VALID_METHOD	-20
#define GMTAPI_PARSE_ERROR		-21
#define GMTAPI_PROG_NOT_FOUND		-22

#define GMTAPI_RUNTIME_ERROR		-99

	/* Index parameters used to access the information arrays */

#define GMTAPI_TYPE		0	/* arg[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE}) */
#define GMTAPI_NDIM		1	/* arg[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D) */
#define GMTAPI_NROW		2	/* arg[2] = number_of_rows (or length of 1-D array) */
#define GMTAPI_NCOL		3	/* arg[3] = number_of_columns (1 for 1-D array) */
#define GMTAPI_KIND		4	/* arg[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran)) */
#define GMTAPI_DIML		5	/* arg[5] = length of dimension for row (C) or column (Fortran) */
#define GMTAPI_FREE		6	/* arg[6] = 1 to free array after use (IN) or before filling with output (OUT), 0 to leave alone */
#define GMTAPI_NODE		7	/* arg[7] = 1 for pixel registration, 0 for node */
#define GMTAPI_XMIN		8	/* arg[8] = x_min (west) of grid */
#define GMTAPI_XMAX		9	/* arg[9] = x_max (east) of grid */
#define GMTAPI_YMIN		10	/* arg[10] = y_min (south) of grid */
#define GMTAPI_YMAX		11	/* arg[11] = y_max (north) of grid */

	/* Data primitive identifiers */

#define GMTAPI_N_TYPES		5	/* The number of supported data types (below) */
#define GMTAPI_BYTE		0	/* The 1-byte data integer type */
#define GMTAPI_SHORT		1	/* The 2-byte data integer type */
#define GMTAPI_INT		2	/* The 4-byte data integer type */
#define GMTAPI_FLOAT		3	/* The 4-byte data float type */
#define GMTAPI_DOUBLE		4	/* The 8-byte data float type */

	/* Array ordering constants */
	
#define GMTAPI_ORDER_ROW	0	/* C-style array order: as index increase we move across rows */
#define GMTAPI_ORDER_COL	1	/* Fortran-style array order: as index increase we move down columns */

/*=====================================================================================
 *	GMT API STRUCTURE DEFINITIONS
 *=====================================================================================
 */

struct GMTAPI_CTRL {
	/* Master controller which holds all GMT API related information at run-time.
	 * It is expected that users can run several GMT sessions concurrently when
	 * GMT 5 moves from using global data to passing a GMT structure. */
	 
	int verbose;				/* Level of API verbosity */
	FILE *log;				/* Pointer used for error reporting */
	int n_data;				/* Number of currently active data objects */
	int n_data_alloc;			/* Allocation counter for data objects */
	struct GMTAPI_DATA_OBJECT **data;	/* List of registered data objects */
	int n_io_sessions;			/* Number of registered line-by-line io sessions */
	int n_io_sessions_alloc;		/* Allocation counter for session objects */
	struct GMTAPI_IO **io_session;		/* List of active i/o sessions */
	int current_io_session[2];		/* Indeces into io_session for the current i/o session */
	int GMTAPI_size[GMTAPI_N_TYPES];	/* Size of various data types in bytes */
	PFI GMT_2D_to_index[2];			/* Pointers to the row or column-order index functions */
	PFV GMT_index_to_2D[2];			/* Pointers to the inverse index functions */
};

struct GMT_OPTION {
	char option;			/* 1-char command line -<option> (e.g. D in -D) identifying the option (NULL if file) */
	BOOLEAN common;			/* TRUE if it is one of the common GMT options -R -J etc */
	char *arg;			/* If not NULL, contains the argument for this option */
	struct GMT_OPTION *next;	/* Pointer to next option in a linked list */
	struct GMT_OPTION *previous;	/* Pointer to previous option in a linked list */
};

/*=====================================================================================
 *	GMT API LIBRARY_SPECIFIC FUNCTION PROTOTYPES
 *=====================================================================================
 */

EXTERN_MSC int GMTAPI_Create_Session    (struct GMTAPI_CTRL **GMT, FILE *log, int *error);
EXTERN_MSC int GMTAPI_Destroy_Session   (struct GMTAPI_CTRL *GMT, int *error);
EXTERN_MSC int GMTAPI_Register_Import   (struct GMTAPI_CTRL *GMT, int method, void **source,   double parameters[], int *object_ID, int *error);
EXTERN_MSC int GMTAPI_Register_Export   (struct GMTAPI_CTRL *GMT, int method, void **receiver, double parameters[], int *object_ID, int *error);
EXTERN_MSC int GMTAPI_Register_IO       (struct GMTAPI_CTRL *API, int method, void **resource, double parameters[], int direction, int *object_ID, int *error);
EXTERN_MSC int GMTAPI_Unregister_Import (struct GMTAPI_CTRL *API, int object_ID, int *error);
EXTERN_MSC int GMTAPI_Unregister_Export (struct GMTAPI_CTRL *API, int object_ID, int *error);
EXTERN_MSC int GMTAPI_Unregister_IO     (struct GMTAPI_CTRL *API, int object_ID, int direction, int *error);
EXTERN_MSC int GMTAPI_Report_Error      (struct GMTAPI_CTRL *GMT, int error);

/*=====================================================================================
 *	GMT API GMT FUNCTION PROTOTYPES
 *=====================================================================================
 */

EXTERN_MSC int GMT_read_all_write_all_records (struct GMTAPI_CTRL *GMT, char *command, int inarg[], int outarg, int *error);
EXTERN_MSC int GMT_read_one_write_one_record  (struct GMTAPI_CTRL *GMT, char *command, int inarg[], int outarg, int *error);
EXTERN_MSC int GMT_read_grid_write_grid (struct GMTAPI_CTRL *GMT, char *command, int inarg[], int outarg, int *error);

#include "gmtapi_parse.h"

#endif /* _GMTAPI_H */
