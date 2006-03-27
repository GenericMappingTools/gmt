/*--------------------------------------------------------------------
 *	$Id: gmtapi.h,v 1.3 2006-03-27 07:38:53 pwessel Exp $
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

#ifndef _GMTAPI_H
#define _GMTAPI_H

#include "gmt.h"
#include <f2c.h>

#define GMTAPI_N_ARRAY_ARGS	8		/* Size of array used to specify array parameters */
#define GMTAPI_N_GRID_ARGS	12		/* Size of array used to specify grid parameters */

#define GMTAPI_OK		0		/* Misc GMTAPI error codes */
#define GMTAPI_ERROR		1
#define GMTAPI_NOT_A_SESSION	2
#define GMTAPI_NOT_A_VALID_ID	3
#define GMTAPI_FILE_NOT_FOUND	4
#define GMTAPI_BAD_PERMISSION	5
#define GMTAPI_GRID_READ_ERROR	6
#define GMTAPI_GRID_WRITE_ERROR	7
#define GMTAPI_N_COLS_VARY	8

#define GMTAPI_TYPE		0		/* arg[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE}) */
#define GMTAPI_NDIM		1		/* arg[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D) */
#define GMTAPI_NROW		2		/* arg[2] = number_of_rows (or length of 1-D array) */
#define GMTAPI_NCOL		3		/* arg[3] = number_of_columns (1 for 1-D array) */
#define GMTAPI_KIND		4		/* arg[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran)) */
#define GMTAPI_DIML		5		/* arg[5] = length of dimension for row (C) or column (Fortran) */
#define GMTAPI_FREE		6		/* arg[6] = 1 to free array after use, 0 to leave alone */
#define GMTAPI_NODE		7		/* arg[7] = 1 for pixel registration, 0 for node */
#define GMTAPI_XMIN		8		/* arg[8] = x_min (west) of grid */
#define GMTAPI_XMAX		9		/* arg[9] = x_max (east) of grid */
#define GMTAPI_YMIN		10		/* arg[10] = y_min (south) of grid */
#define GMTAPI_YMAX		11		/* arg[11] = y_max (north) of grid */

#define GMTAPI_N_TYPES		5		/* The number of supported data types (below) */
#define GMTAPI_BYTE		0		/* The 1-byte data integer type */
#define GMTAPI_SHORT		1		/* The 2-byte data integer type */
#define GMTAPI_INT		2		/* The 4-byte data integer type */
#define GMTAPI_FLOAT		3		/* The 4-byte data float type */
#define GMTAPI_DOUBLE		4		/* The 8-byte data float type */

#define GMTAPI_ORDER_ROW	0		/* C-style array order: as index increase we move across rows */
#define GMTAPI_ORDER_COL	1		/* Fortran-style array order: as index increase we move down columns */

struct GMTAPI_DATA_OBJECT {
	/* Information for each input or output data entity */
	int ID;					/* Unique identifier which is > 0 */
	int direction;				/* 0 = in, 1 = out */
	int method;				/* One of GMTAPI_{FILE,STREAM,FDESC,ARRAY,GRID} */
	double arg[GMTAPI_N_GRID_ARGS];		/* Array/grid size/dimension/domain parameters */
	void **ptr;				/* Points to the source|destination */
};

struct GMTAPI_IO {
	BOOLEAN header;
	BOOLEAN close_file;
	int current_item;
	int n_items;
	int current_rec;
	int current_method;
	int n_columns;
	int n_expected_fields;
	int *ID;	/* Points to the list of IDs */
	FILE *fp;	/* Pointer to source/destination */
};

struct GMTAPI_CTRL {
	/* Master controller which holds all GMT API related information at run-time */
	int n_data;				/* Number of currently active data objects */
	int n_alloc;				/* Allocation counter */
	int GMTAPI_size[GMTAPI_N_TYPES];	/* Size of various data types in bytes */
	struct GMTAPI_DATA_OBJECT **data;	/* List of registered data objects */
	PFI GMT_2D_to_index[2];			/* Pointers to the row or column-order index functions */
	PFV GMT_index_to_2D[2];			/* Pointers to the inverse index functions */
};

struct GMTAPI_GRID_OBJECT {
	struct GRD_HEADER *h;	/* Standard GMT header structure for grids */
	void **ptr;		/* Pointer to the array for the grid */
};

/* Function prototypes to be used by developers */

extern int GMT_Create_Session  (struct GMTAPI_CTRL **GMT, int flags);
extern int GMT_Destroy_Session (struct GMTAPI_CTRL *GMT);
extern int GMT_Add_Data_Object (struct GMTAPI_CTRL *GMT, struct GMTAPI_DATA_OBJECT *D);
extern int GMT_Remove_Data_Object (struct GMTAPI_CTRL *GMT, int object_ID);
extern int GMT_Register_Import  (struct GMTAPI_CTRL *GMT, int method, void **source,   double parameters[]);
extern int GMT_Register_Export (struct GMTAPI_CTRL *GMT, int method, void **receiver, double parameters[]);
extern int GMT_Import_Table (struct GMTAPI_CTRL *GMT, int inarg[], struct GMT_LINE_SEGMENT **S);
extern int GMT_Export_Table (struct GMTAPI_CTRL *GMT, int outarg,  struct GMT_LINE_SEGMENT *S, int n_segments);
extern int GMT_Import_Grid (struct GMTAPI_CTRL *API, int inarg,  struct GMT_GRID **G);
extern int GMT_Export_Grid (struct GMTAPI_CTRL *API, int outarg, struct GMT_GRID *G);
extern int GMT_Init_Import_Record (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO, int ID[]);
extern int GMT_Init_Export_Record (struct GMTAPI_CTRL *API, struct GMTAPI_IO *IO, int ID);
extern int GMT_Import_Record (struct GMTAPI_CTRL *API, double **record, struct GMTAPI_IO *IO);
extern int GMT_Export_Record (struct GMTAPI_CTRL *API, double *record, struct GMTAPI_IO *IO);
extern void GMT_Error (int error);

/* These are the prototype GMT "applications" that can be called */

extern int GMT_copy_all (struct GMTAPI_CTRL *GMT, char *command, int inarg[], int outarg);
extern int GMT_copy_line (struct GMTAPI_CTRL *GMT, char *command, int inarg[], int outarg);

#endif /* _GMTAPI_H */
