/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

/*
 * THese are the private parts of the GMTAPI_CTRL which we will not expose
 * to the API users; they are only accessible by GMT developers.
 *
 * Author: 	Paul Wessel
 * Date:	06-FEB-2013
 * Version:	6 API
 */

/*!
 * \file gmt_private.h
 * \brief Private parts of the GMTAPI_CTRL which we will not expose to the API users
 */

#ifndef _GMTAPI_PRIVATE_H
#define _GMTAPI_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

enum GMT_enum_apidim {
	GMTAPI_N_GRID_ARGS	= 4,	/* Minimum size of information array used to specify grid parameters */
	GMTAPI_N_ARRAY_ARGS	= 8	/* Minimum size of information array used to specify array parameters */
};

/*! p_func_uint64_t is used as a pointer to functions that returns a uint64_t index */
typedef uint64_t (*p_func_uint64_t) (uint64_t row, uint64_t col, uint64_t dim);
typedef void (*GMT_putfunction)(union GMT_UNIVECTOR *, uint64_t, double  );
typedef void (*GMT_getfunction)(union GMT_UNIVECTOR *, uint64_t, double *);

/* Index parameters used to access the information arrays [PW: Is this still relevant?] */

#if 0
enum GMT_enum_pars {GMTAPI_TYPE = 0,	/* ipar[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE}) */
	GMTAPI_NDIM,		/* ipar[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D) */
	GMTAPI_NROW,		/* ipar[2] = number_of_rows (or length of 1-D array) */
	GMTAPI_NCOL,		/* ipar[3] = number_of_columns (1 for 1-D array) */
	GMTAPI_KIND,		/* ipar[4] = arrangement of rows/col (0 = rows (C), 1 = columns (Fortran)) */
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
	uint64_t rec;				/* Current rec to read [GMT_DATASET to/from MATRIX/VECTOR only] */
	uint64_t n_rows;			/* Number or rows in this array [GMT_DATASET to/from MATRIX/VECTOR only] */
	uint64_t n_columns;			/* Number of columns to process in this dataset [GMT_DATASET only] */
	uint64_t n_expected_fields;		/* Number of expected columns for this dataset [GMT_DATASET only] */
	uint64_t delay;				/* Number of leading NaN-records we oculd not write initially before knowning the row dim */
	size_t n_alloc;				/* Number of items allocated so far if writing to memory */
	unsigned int ID;			/* Unique identifier which is >= 0 */
	unsigned int alloc_level;		/* Nested module level when object was allocated */
	unsigned int status;			/* 0 when first registered, 1 after reading/writing has started, 2 when finished */
	unsigned int orig_pad[4];		/* Original grid pad */
	unsigned int reset_pad;			/* 1 for input memory grids from which a subregion was requested */
	bool h_delay;				/* We must delay writing table headers until memory allocated */
	bool s_delay;				/* We must delay writing segment header until memory allocated  */
	bool selected;				/* true if requested by current module, false otherwise */
	bool close_file;			/* true if we opened source as a file and thus need to close it when done */
	bool region;				/* true if wesn was passed, false otherwise */
	bool no_longer_owner;			/* true if the data pointed to by the object was passed on to another object */
	bool messenger;				/* true for output objects passed from the outside to receive data from GMT. If true we destroy data pointer before writing */
	bool module_input;			/* true for input objects that will serve as module input(s) and not option inputs */
	enum GMT_enum_alloc alloc_mode;		/* GMT_ALLOCATED_{BY_GMT|EXTERNALLY} */
	enum GMT_enum_std direction;		/* GMT_IN or GMT_OUT */
	enum GMT_enum_family family;		/* One of GMT_IS_{DATASET|TEXTSET|PALETTE|IMAGE|GRID|POSTSCRIPT|MATRIX|VECTOR|COORD} */
	enum GMT_enum_family actual_family;	/* May be GMT_IS_MATRIX|VECTOR when one of the others are created via those */
	enum GMT_enum_type type;		/* Desired output array type for auto-allocated VECTOR and MATRIX */
	unsigned method;			/* One of GMT_IS_{FILE,STREAM,FDESC,DUPLICATE,REFERENCE} or sum with enum GMT_enum_via (GMT_VIA_{NONE,VECTOR,MATRIX,OUTPUT}); using unsigned type because sum exceeds enum GMT_enum_method */
	enum GMT_enum_geometry geometry;	/* One of GMT_IS_{POINT|LINE|POLY|PLP|SURFACE|NONE} */
	double wesn[GMTAPI_N_GRID_ARGS];	/* Active Grid domain limits */
	double orig_wesn[GMTAPI_N_GRID_ARGS];	/* Original Grid domain limits */
	void *resource;				/* Points to a GMT container, where data can be obtained or are placed. */
	FILE *fp;				/* Pointer to source/destination stream [For rec-by-rec procession, NULL if memory location] */
	char *filename;				/* Filename, stream, of file handle (otherwise NULL) */
	void * (*import) (struct GMT_CTRL *, FILE *, uint64_t *, int *);	/* Pointer to input function (for DATASET/TEXTSET only) */
#ifdef DEBUG
	/* Start of temporary variables for API debug - They are only set when building with /DEBUG */
	struct GMT_GRID *G;
	struct GMT_DATASET *D;
	struct GMT_PALETTE *C;
	struct GMT_POSTSCRIPT *P;
	struct GMT_MATRIX *M;
	struct GMT_VECTOR *V;
	struct GMT_IMAGE *I;
	/* End of temporary variables for API debug - will be removed eventually */
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
	int current_item[2];			/* Array number of current dataset being processed (in and out)*/
	unsigned int pad;			/* Session default for number of rows/cols padding for grids [2] */
	unsigned int external;			/* 1 if called via external API (MATLAB, Python) [0] */
	unsigned int runmode;			/* nonzero for GMT modern runmode [0 = classic] */
	enum GMT_enum_fmt shape;		/* GMT_IS_COL_FORMAT (2) if column-major (MATLAB, Fortran), GMT_IS_ROW_FORMAT (1) if row-major (Python, C/C++) [1] */
	unsigned int leave_grid_scaled;		/* 1 if we don't want to unpack a grid after we packed it for writing [0] */
	unsigned int n_cores;			/* Number of available cores on this system */
	unsigned int verbose;			/* Used until GMT is set up */
	unsigned int n_tmp_headers;		/* Number of temporarily held table headers */
	bool registered[2];			/* true if at least one source/destination has been registered (in and out) */
	bool io_enabled[2];			/* true if access has been allowed (in and out) */
	bool module_input;			/* true when we are about to read inputs to the module (command line) */
	bool usage;				/* Flag when 1-liner modern mode modules just want usage */
	bool allow_reuse;				/* Flag when get_region_from_data can read a file and not flag it as "used" */
	size_t n_objects_alloc;			/* Allocation counter for data objects */
	int error;				/* Error code from latest API call [GMT_OK] */
	int last_error;				/* Error code from previous API call [GMT_OK] */
	int shelf;				/* Place to pass hidden values within API */
	unsigned int log_level;			/* 0 = stderr, 1 = just this module, 2 = set until unset */
	unsigned int io_mode[2];		/* 1 if access as set, 0 if record-by-record */
	struct GMT_CTRL *GMT;			/* Key structure with low-level GMT internal parameters */
	struct GMTAPI_DATA_OBJECT **object;	/* List of registered data objects */
	char *session_tag;			/* Name tag for this session (or NULL) */
	char *session_name;			/* Unique name for modern mode session (NULL for classic) */
	char *tmp_dir;				/* System tmp_dir (NULL if not found) */
	char *session_dir;			/* GMT Session dir (NULL if not running in modern mode) */
	char *gwf_dir;				/* GMT WorkFlow dir (NULL if not running in modern mode) */
	char **tmp_header;			/* Temporary table headers held until we are able to write them to destination */
	char *tmp_segmentheader;		/* Temporary segment header held until we are able to write it to destination */
	char *message;				/* To be allocated by Create_Session and used for messages */
	char error_msg[4096];			/* The cached last error message */
	bool internal;				/* true if session was initiated by gmt.c */
	bool deep_debug;			/* temporary for debugging */
	int (*print_func) (FILE *, const char *);	/* Pointer to fprintf function (may be reset by external APIs like MEX) */
	unsigned int do_not_exit;		/* 0 by default, mieaning it is OK to call exit  (may be reset by external APIs like MEX to call return instead) */
	struct Gmt_libinfo *lib;		/* List of shared libs to consider */
	unsigned int n_shared_libs;		/* How many in lib */
	/* Items used by GMT_Put_Record and sub-functions */
	int (*api_put_record) (struct GMTAPI_CTRL *API, unsigned int, struct GMT_RECORD *);
	/*   Items used by api_put_record_fp */
	FILE *current_fp;
	struct GMTAPI_DATA_OBJECT *current_put_obj;
	uint64_t current_put_n_columns;
	/*   Items used by api_put_record_dataset */
	struct GMT_DATATABLE *current_put_D_table;
	/*   Items used by api_put_record_matrix */
	struct GMT_MATRIX *current_put_M;
	p_func_uint64_t current_put_M_index;
	GMT_putfunction current_put_M_val;
	/*   Items used by api_put_record_vector */
	struct GMT_VECTOR *current_put_V;
	GMT_putfunction *current_put_V_val;
	/* Items used by GMT_Put_Record and sub-functions */
	struct GMT_RECORD * (*api_get_record) (struct GMTAPI_CTRL *, unsigned int, int *);
	struct GMTAPI_DATA_OBJECT *current_get_obj;
	bool get_next_record;
	/*   Items used by api_get_record_dataset */
	struct GMT_DATASET *current_get_D_set;
	/*   Items used by api_get_record_matrix */
	struct GMT_MATRIX *current_get_M;
	p_func_uint64_t current_get_M_index;
	GMT_getfunction current_get_M_val;
	uint64_t current_get_n_columns;
	/*   Items used by api_get_record_vector */
	struct GMT_VECTOR *current_get_V;
	p_func_uint64_t current_get_V_index;
	GMT_getfunction *current_get_V_val;
	/* These are used for -O -K -P -c and set to blank under modern/classic modes */
	char *O_OPT, *K_OPT, *P_OPT, *c_OPT;
};

/* Macro to test if filename is a special name indicating memory location */

#define GMTAPI_PREFIX_LEN 9U		/* The length of the unique leading prefix of virtual filenames */
#define GMTAPI_OBJECT_ID_START 21U	/* Start position of the encoded object ID in the virtual filename */
#define gmt_M_file_is_memory(file) (file && !strncmp (file, "@GMTAPI@-", GMTAPI_PREFIX_LEN))

#ifdef __cplusplus
}
#endif

#endif /* _GMTAPI_PRIVATE_H */
