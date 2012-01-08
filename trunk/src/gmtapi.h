/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * The single include file for users who wish to develop applications
 * that require building blocks from the GMT Application Program Interface
 * library (the GMT API), which also depends on the GMT Core library.
 *
 * Author: 	Paul Wessel
 * Date:	30-MAR-2010
 * Version:	5 API
 */

#ifndef _GMTAPI_H
#define _GMTAPI_H

/*=====================================================================================
 *	GMT API STRUCTURE DEFINITIONS
 *===================================================================================*/

struct GMTAPI_DATA_OBJECT {
	/* Information for each input or output data entity, including information
	 * needed while reading/writing from a table (file or array) */
	GMT_LONG close_file;			/* TRUE if we opened source as a file adn thus need to close it when done */
	GMT_LONG alloc_mode;			/* GMTAPI_REFERENCE or GMTAPI_ALLOCATED */
	GMT_LONG ID;				/* Unique identifier which is > 0 */
	GMT_LONG direction;			/* GMT_IN or GMT_OUT */
	GMT_LONG family;			/* One of GMT_IS_{DATASET|TEXTSET|CPT|IMAGE|GMTGRID} */
	GMT_LONG method;			/* One of GMT_IS_{FILE,STREAM,FDESC,ARRAY,GRID,COPY,REF|READONLY} */
	GMT_LONG status;			/* 0 when first registered, 1 after reading/writing has started, 2 when finished */
	GMT_LONG n_columns;			/* Number of columns to process in this dataset [GMT_DATASET only] */
	GMT_LONG n_rows;			/* Number or rows in this array [GMT_DATASET and GMT_TEXTSET only] */
	GMT_LONG n_expected_fields;		/* Number of expected columns for this dataset [GMT_DATASET only] */
	GMT_LONG n_alloc;			/* Number of memory allocated so far if writing to memory */
	GMT_LONG geometry;			/* One of GMT_POINT, GMT_LINE, GMT_POLY, GMT_SURF */
	GMT_LONG region;			/* 1 if wesn was passed, 0 otherwise */
	GMT_LONG level;				/* Nested module level when object was allocated */
	double wesn[GMTAPI_N_GRID_ARGS];	/* Grid domain limits */
	void *resource;				/* Points to registered data container (if appropriate) */
	void *data;				/* Points to container associated with this object [for garbage collection purposes] */
	FILE *fp;				/* Pointer to source/destination stream [For rec-by-rec procession, NULL if memory location] */
	char *filename;				/* Filename, stream, of file handle (otherwise NULL) */
	PFP import;				/* Pointer to input function (for DATASET/TEXTSET only) */
};

struct GMTAPI_CTRL {
	/* Master controller which holds all GMT API related information at run-time for a single session.
	 * Users can run several GMT sessions concurrently; each session requires its own structure.
	 * Use GMTAPI_Create_Session to initialize a new session and GMTAPI_Destroy_Session to end it. */
	
	GMT_LONG error;				/* Error code from latest API call [GMT_OK] */
	GMT_LONG n_objects;			/* Number of currently active input and output data objects */
	GMT_LONG n_objects_alloc;		/* Allocation counter for data objects */
	GMT_LONG unique_ID;			/* Used to create unique IDs for duration of session */
	GMT_LONG session_ID;			/* ID of this session */
	GMT_LONG current_item[2];		/* Array number of current dataset being processed (in and out)*/
	GMT_LONG current_rec[2];		/* Current record number >= 0 in the combined virtual dataset (in and out) */
	GMT_LONG registered[2];			/* TRUE if at least one source/destination has been registered (in and out) */
	GMT_LONG io_enabled[2];			/* TRUE if access has been allowed (in and out) */
	GMT_LONG io_mode[2];			/* 1 if access as set, 0 if record-by-record */
	struct GMT_CTRL *GMT;			/* Key structure with low-level GMT internal parameters */
	struct GMTAPI_DATA_OBJECT **object;	/* List of registered data objects */
	char *session_tag;			/* Name tag for this session (or NULL) */
};

struct GMT_OPTION {	/* Structure for a single GMT command option */
	char option;			/* 1-char command line -<option> (e.g. D in -D) identifying the option (* if file) */
	char *arg;			/* If not NULL, contains the argument for this option */
	struct GMT_OPTION *next;	/* Pointer to next option in a linked list */
	struct GMT_OPTION *previous;	/* Pointer to previous option in a linked list */
};

/*=====================================================================================
 *	GMT API FUNCTION PROTOTYPES
 *=====================================================================================
 */

/* 17 Primary API functions */
EXTERN_MSC struct GMTAPI_CTRL * GMT_Create_Session	(char *tag, GMT_LONG mode);
EXTERN_MSC void * GMT_Create_Data			(struct GMTAPI_CTRL *C, GMT_LONG type, GMT_LONG par[]);
EXTERN_MSC void * GMT_Get_Data				(struct GMTAPI_CTRL *C, GMT_LONG ID, GMT_LONG mode, void *data);
EXTERN_MSC void * GMT_Read_Data				(struct GMTAPI_CTRL *C, GMT_LONG family, GMT_LONG method, GMT_LONG geometry, double wesn[], GMT_LONG mode, char *input, void *data);
EXTERN_MSC void * GMT_Retrieve_Data			(struct GMTAPI_CTRL *API, GMT_LONG ID);
EXTERN_MSC void * GMT_Get_Record			(struct GMTAPI_CTRL *C, GMT_LONG mode, GMT_LONG *retval);
EXTERN_MSC GMT_LONG GMT_Destroy_Session			(struct GMTAPI_CTRL **C);
EXTERN_MSC GMT_LONG GMT_Register_IO			(struct GMTAPI_CTRL *C, GMT_LONG family, GMT_LONG method, GMT_LONG geometry, GMT_LONG direction, void *resource, double wesn[]);
EXTERN_MSC GMT_LONG GMT_Init_IO				(struct GMTAPI_CTRL *C, GMT_LONG family, GMT_LONG geometry, GMT_LONG direction, GMT_LONG mode, struct GMT_OPTION *head);
EXTERN_MSC GMT_LONG GMT_Begin_IO			(struct GMTAPI_CTRL *API, GMT_LONG family, GMT_LONG direction);
EXTERN_MSC GMT_LONG GMT_End_IO				(struct GMTAPI_CTRL *API, GMT_LONG direction, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_Report_Error			(struct GMTAPI_CTRL *C, GMT_LONG error);
EXTERN_MSC GMT_LONG GMT_Put_Data			(struct GMTAPI_CTRL *C, GMT_LONG ID, GMT_LONG mode, void *data);
EXTERN_MSC GMT_LONG GMT_Write_Data			(struct GMTAPI_CTRL *C, GMT_LONG family, GMT_LONG method, GMT_LONG geometry, double wesn[], GMT_LONG mode, char *output, void *data);
EXTERN_MSC GMT_LONG GMT_Destroy_Data			(struct GMTAPI_CTRL *C, GMT_LONG mode, void *X);
EXTERN_MSC GMT_LONG GMT_Put_Record			(struct GMTAPI_CTRL *C, GMT_LONG mode, void *record);
EXTERN_MSC GMT_LONG GMT_Encode_ID			(struct GMTAPI_CTRL *C, char *string, GMT_LONG object_ID);

/* 12 functions argument and option parsing */

EXTERN_MSC struct GMT_OPTION * GMT_Create_Options	(struct GMTAPI_CTRL *C, GMT_LONG argc, void *in);
EXTERN_MSC struct GMT_OPTION * GMT_Prep_Options		(struct GMTAPI_CTRL *C, GMT_LONG mode, void *in);
EXTERN_MSC struct GMT_OPTION * GMT_Make_Option		(struct GMTAPI_CTRL *C, char option, char *arg);
EXTERN_MSC struct GMT_OPTION * GMT_Find_Option		(struct GMTAPI_CTRL *C, char option, struct GMT_OPTION *head);
EXTERN_MSC struct GMT_OPTION * GMT_Append_Option	(struct GMTAPI_CTRL *C, struct GMT_OPTION *current, struct GMT_OPTION *head);
EXTERN_MSC char ** GMT_Create_Args			(struct GMTAPI_CTRL *C, GMT_LONG *argc, struct GMT_OPTION *head);
EXTERN_MSC char * GMT_Create_Cmd			(struct GMTAPI_CTRL *C, struct GMT_OPTION *head);
EXTERN_MSC GMT_LONG GMT_Destroy_Options			(struct GMTAPI_CTRL *C, struct GMT_OPTION **head);
EXTERN_MSC GMT_LONG GMT_Destroy_Args			(struct GMTAPI_CTRL *C, GMT_LONG argc, char *argv[]);
EXTERN_MSC GMT_LONG GMT_Update_Option			(struct GMTAPI_CTRL *C, char option, char *arg, struct GMT_OPTION *head);
EXTERN_MSC GMT_LONG GMT_Delete_Option			(struct GMTAPI_CTRL *C, struct GMT_OPTION *current);
EXTERN_MSC GMT_LONG GMT_Parse_Common			(struct GMTAPI_CTRL *C, char *sorted, char *unsorted, struct GMT_OPTION *options);

#ifdef DEBUG
/* This function is available for testing purposes if --enable-debug was used during configuration */
EXTERN_MSC GMT_LONG GMT_List_Args			(struct GMTAPI_CTRL *API, struct GMT_OPTION *head);
#endif

/* Macro to test if filename is a special name indicating memory location */

#define GMT_File_Is_Memory(file) (file && !strncmp (file, "@GMTAPI@-", 9))

#endif /* _GMTAPI_H */
