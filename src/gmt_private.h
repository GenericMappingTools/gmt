/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * THese are the private parts of the GMTAPI_CTRL which we will not expose
 * to the API users; they are only accessible by GMT developers.
 *
 * Author: 	Paul Wessel
 * Date:	06-FEB-2013
 * Version:	5 API
 */

#ifndef _GMTAPI_PRIVATE_H
#define _GMTAPI_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Visual C++ only implements C90, which has no bool type. C99 added support
 * for bool via the <stdbool.h> header, but Visual C++ does not support this.
 */
#ifndef __bool_true_false_are_defined
#	if defined _MSC_VER
#		define bool _Bool
#		define true 1
#		define false 0
#		define __bool_true_false_are_defined 1
#	else
#		include <stdbool.h>
#	endif /* _MSC_VER */
#endif /* !__bool_true_false_are_defined */

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

enum GMT_enum_apidim {
	GMTAPI_N_GRID_ARGS	= 4,	/* Minimum size of information array used to specify grid parameters */
	GMTAPI_N_ARRAY_ARGS	= 8	/* Minimum size of information array used to specify array parameters */
};

/* Index parameters used to access the information arrays [PW: Is this still relevant?] */

#if 0
enum GMT_enum_pars {GMTAPI_TYPE = 0,	/* ipar[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE}) */
	GMTAPI_NDIM,		/* ipar[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D) */
	GMTAPI_NROW,		/* ipar[2] = number_of_rows (or length of 1-D array) */
	GMTAPI_NCOL,		/* ipar[3] = number_of_columns (1 for 1-D array) */
	GMTAPI_KIND,		/* ipar[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran)) */
	GMTAPI_DIML,		/* ipar[5] = length of dimension for row (C) or column (Fortran) */
	GMTAPI_FREE,		/* ipar[6] = 1 to free array after use (IN) or before filling with output (OUT), 0 to leave alone */
	GMTAPI_NODE};		/* ipar[7] = 1 for pixel registration, 0 for node */
#endif


/*=====================================================================================
 *	GMT API STRUCTURE DEFINITIONS
 *===================================================================================*/

struct GMT_CTRL; /* forward declaration of GMT_CTRL */

struct GMTAPI_DATA_OBJECT {
	/* Information for each input or output data entity, including information
	 * needed while reading/writing from a table (file or array) */
	uint64_t n_rows;			/* Number or rows in this array [GMT_DATASET and GMT_TEXTSET to/from MATRIX/VETOR only] */
	uint64_t n_columns;			/* Number of columns to process in this dataset [GMT_DATASET only] */
	uint64_t n_expected_fields;		/* Number of expected columns for this dataset [GMT_DATASET only] */
	size_t n_alloc;				/* Number of items allocated so far if writing to memory */
	unsigned int ID;			/* Unique identifier which is >= 0 */
	unsigned int alloc_level;		/* Nested module level when object was allocated */
	unsigned int status;			/* 0 when first registered, 1 after reading/writing has started, 2 when finished */
	bool selected;				/* true if requested by current module, false otherwise */
	bool close_file;			/* true if we opened source as a file and thus need to close it when done */
	bool region;				/* true if wesn was passed, false otherwise */
	bool no_longer_owner;			/* true if the data pointed to by the object was passed on to another object */
	bool messenger;				/* true for output objects passed from the outside to receive data from GMT. If true we destroy data pointer before writing */
	enum GMT_enum_alloc alloc_mode;		/* GMT_ALLOCATED_{BY_GMT|EXTERNALLY} */
	enum GMT_io_enum direction;		/* GMT_IN or GMT_OUT */
	enum GMT_enum_family family;		/* One of GMT_IS_{DATASET|TEXTSET|CPT|IMAGE|GRID|MATRIX|VECTOR|COORD} */
	enum GMT_enum_family actual_family;	/* May be GMT_IS_MATRIX|VECTOR when one of the others are created via those */
	unsigned method;		/* One of GMT_IS_{FILE,STREAM,FDESC,DUPLICATE,REFERENCE} or sum with enum GMT_enum_via (GMT_VIA_{NONE,VECTOR,MATRIX,OUTPUT}); using unsigned type because sum exceeds enum GMT_enum_method */
	enum GMT_enum_geometry geometry;	/* One of GMT_IS_{POINT|LINE|POLY|PLP|SURFACE|NONE} */
	double wesn[GMTAPI_N_GRID_ARGS];	/* Grid domain limits */
	void *resource;				/* Points to registered filename, memory location, etc., where data can be obtained from with GMT_Get_Data. */
	void *data;				/* Points to GMT object that was read from a resource */
	FILE *fp;				/* Pointer to source/destination stream [For rec-by-rec procession, NULL if memory location] */
	char *filename;				/* Filename, stream, of file handle (otherwise NULL) */
	void * (*import) (struct GMT_CTRL *, FILE *, uint64_t *, int *);	/* Pointer to input function (for DATASET/TEXTSET only) */
#ifdef DEBUG
	struct GMT_GRID *G;
	struct GMT_DATASET *D;
	struct GMT_TEXTSET *T;
	struct GMT_PALETTE *C;
	struct GMT_MATRIX *M;
	struct GMT_VECTOR *V;
#endif
#ifdef HAVE_GDAL
	struct GMT_IMAGE *I;
#endif
};

struct GMTAPI_CTRL {
	/* Master controller which holds all GMT API related information at run-time for a single session.
	 * Users can run several GMT sessions concurrently; each session requires its own structure.
	 * Use GMTAPI_Create_Session to initialize a new session and GMTAPI_Destroy_Session to end it. */

	uint64_t current_rec[2];		/* Current record number >= 0 in the combined virtual dataset (in and out) */
	unsigned int n_objects;			/* Number of currently active input and output data objects */
	unsigned int unique_ID;			/* Used to create unique IDs for duration of session */
	unsigned int session_ID;		/* ID of this session */
	unsigned int unique_var_ID;		/* Used to create unique object IDs (grid,dataset, etc) for duration of session */
	unsigned int current_item[2];		/* Array number of current dataset being processed (in and out)*/
	unsigned int pad;			/* Session default for number of rows/cols padding for grids [2] */
	unsigned int mode;			/* 1 if called via external API (Matlab, Python) [0] */
	unsigned int leave_grid_scaled;		/* 1 if we dont want to unpack a grid after we packed it for writing [0] */
	unsigned int verbose;			/* Used until GMT is set up */
	bool registered[2];			/* true if at least one source/destination has been registered (in and out) */
	bool io_enabled[2];			/* true if access has been allowed (in and out) */
	size_t n_objects_alloc;			/* Allocation counter for data objects */
	int error;				/* Error code from latest API call [GMT_OK] */
	int last_error;				/* Error code from previous API call [GMT_OK] */
	int shelf;				/* Place to pass hidden values within API */
	unsigned int io_mode[2];		/* 1 if access as set, 0 if record-by-record */
	struct GMT_CTRL *GMT;			/* Key structure with low-level GMT internal parameters */
	struct GMTAPI_DATA_OBJECT **object;	/* List of registered data objects */
	char *session_tag;			/* Name tag for this session (or NULL) */
	bool internal;				/* true if session was initiated by gmt.c */
	bool deep_debug;			/* temporary for debugging */
	int (*print_func) (FILE *, const char *);	/* Pointer to fprintf function (may be reset by external APIs like MEX) */
	unsigned int do_not_exit;		/* 0 by default, mieaning it is OK to call exit  (may be reset by external APIs like MEX to call return instead) */
	struct Gmt_libinfo *lib;		/* List of shared libs to consider */
	unsigned int n_shared_libs;		/* How many in lib */
};

#ifdef DEBUG
EXTERN_MSC void GMT_list_API (struct GMTAPI_CTRL *ptr, char *txt);
#endif
EXTERN_MSC int GMTAPI_report_error	(void *C, int error);

/* Macro to test if filename is a special name indicating memory location */

#define GMT_File_Is_Memory(file) (file && !strncmp (file, "@GMTAPI@-", 9U))

#ifdef __cplusplus
}
#endif

#endif /* _GMTAPI_PRIVATE_H */
