/*--------------------------------------------------------------------
 *	$Id: gmtapi.h,v 1.25 2011-03-03 21:02:51 guru Exp $
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
 *===================================================================================*/

#include "gmtapi_errno.h"		/* All error return values are defined here */
#include "gmtapi_define.h"		/* All constant values are defined here */

/*=====================================================================================
 *	GMT API STRUCTURE DEFINITIONS
 *===================================================================================*/

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
	int current_io_session[2];		/* indices into io_session for the current i/o session */
	int GMTAPI_size[GMTAPI_N_TYPES];	/* Size of various data types in bytes */
	PFI GMT_2D_to_index[2];			/* Pointers to the row or column-order index functions */
	PFV GMT_index_to_2D[2];			/* Pointers to the inverse index functions */
};

struct GMT_OPTION {
	char option;			/* 1-char command line -<option> (e.g. D in -D) identifying the option (NULL if file) */
	GMT_LONG common;			/* TRUE if it is one of the common GMT options -R -J etc */
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
EXTERN_MSC int GMT_prog (struct GMTAPI_CTRL *API, char *name, struct GMT_OPTION *options, int *error);
EXTERN_MSC int GMT_prog_cmd (struct GMTAPI_CTRL *API, char *name, int n_args, char *args[], int *error);

/*=====================================================================================
 *	GMT API PROGRAM DEFINITIONS
 *===================================================================================*/

#include "gmtapi_progs.h"		/* All prototypes for GMT programs */

#include "gmtapi_parse.h"

#endif /* _GMTAPI_H */
