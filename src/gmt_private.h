/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
	unsigned int ID;			/* Unique identifier which is >= 0 */
	unsigned int level;			/* Nested module level when object was allocated */
	bool selected;				/* true if requested by current module, false otherwise */
	bool close_file;			/* true if we opened source as a file and thus need to close it when done */
	bool region;				/* true if wesn was passed, false otherwise */
	size_t n_alloc;				/* Number of items allocated so far if writing to memory */
	unsigned int alloc_mode;		/* GMTAPI_REFERENCE or GMTAPI_ALLOCATED */
	unsigned int direction;			/* GMT_IN or GMT_OUT */
	unsigned int family;			/* One of GMT_IS_{DATASET|TEXTSET|CPT|IMAGE|GMTGRID} */
	unsigned int method;			/* One of GMT_IS_{FILE,STREAM,FDESC,ARRAY,GRID,COPY,REF|READONLY} */
	unsigned int geometry;			/* One of GMT_POINT, GMT_LINE, GMT_POLY, GMT_SURF */
	unsigned int status;			/* 0 when first registered, 1 after reading/writing has started, 2 when finished */
	double wesn[GMTAPI_N_GRID_ARGS];	/* Grid domain limits */
	void *resource;				/* Points to registered data container (if appropriate) */
	void *data;				/* Points to container associated with this object [for garbage collection purposes] */
	FILE *fp;				/* Pointer to source/destination stream [For rec-by-rec procession, NULL if memory location] */
	char *filename;				/* Filename, stream, of file handle (otherwise NULL) */
	void * (*import) (struct GMT_CTRL *, FILE *, uint64_t *, int *);	/* Pointer to input function (for DATASET/TEXTSET only) */
};

struct GMTAPI_CTRL {
	/* Master controller which holds all GMT API related information at run-time for a single session.
	 * Users can run several GMT sessions concurrently; each session requires its own structure.
	 * Use GMTAPI_Create_Session to initialize a new session and GMTAPI_Destroy_Session to end it. */

	uint64_t current_rec[2];		/* Current record number >= 0 in the combined virtual dataset (in and out) */
	unsigned int n_objects;			/* Number of currently active input and output data objects */
	unsigned int unique_ID;			/* Used to create unique IDs for duration of session */
	unsigned int session_ID;		/* ID of this session */
	unsigned int current_item[2];		/* Array number of current dataset being processed (in and out)*/
	unsigned int pad;			/* Session default for number of rows/cols padding for grids [2] */
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
	bool deep_debug;			/* temprorary for debug */
	int (*print_func) (FILE *, const char *);	/* Pointer to fprintf function (may be reset by external APIs like MEX) */
	unsigned int do_not_exit;		/* 0 by default, mieaning it is OK to call exit  (may be reset by external APIs like MEX to call return instead) */
};

EXTERN_MSC void GMT_list_API (struct GMTAPI_CTRL *ptr, char *txt);
EXTERN_MSC int GMTAPI_report_error	(void *C, int error);

/* Macro to test if filename is a special name indicating memory location */

#define GMT_File_Is_Memory(file) (file && !strncmp (file, "@GMTAPI@-", 9U))

#ifdef __cplusplus
}
#endif

#endif /* _GMTAPI_PRIVATE_H */
