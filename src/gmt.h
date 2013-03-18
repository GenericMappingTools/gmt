/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013
 *	P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * The single include file for users who wish to develop applications
 * that require building blocks from the GMT Application Program Interface
 * library (the GMT API), which also depends on the GMT Core library.
 *
 * Author: 	Paul Wessel
 * Date:	20-FEB-2013
 * Version:	5 API
 */

#ifndef _GMT_H
#define _GMT_H

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "declspec.h"

/*
 * When an application links to a DLL in Windows, the symbols that
 * are imported have to be identified as such.
 */
#ifndef EXTERN_MSC
#	ifdef _WIN32
#		ifdef LIBRARY_EXPORTS
#			define EXTERN_MSC extern __declspec(dllexport)
#		else
#			define EXTERN_MSC extern __declspec(dllimport)
#		endif /* !LIBRARY_EXPORTS */
#	else /* !_WIN32 */
#		define EXTERN_MSC extern
#	endif /* _WIN32 */
#endif /* EXTERN_MSC */

/* Include GMT constants, resources, and module prototypes */

#include "gmt_define.h"
#include "gmt_resources.h"
#include "gmt_module.h"

/*=====================================================================================
 *	GMT API FUNCTION PROTOTYPES
 *=====================================================================================
 */

/* 22 Primary API functions */
EXTERN_MSC void * GMT_Create_Session	(char *tag, unsigned int pad, unsigned int mode);
EXTERN_MSC void * GMT_Create_Data	(void *C, unsigned int family, unsigned int geometry, unsigned int mode, uint64_t dim[], double *wesn, double *inc, unsigned int registration, int pad, void *data);
EXTERN_MSC void * GMT_Get_Data		(void *C, int object_ID, unsigned int mode, void *data);
EXTERN_MSC void * GMT_Read_Data		(void *C, unsigned int family, unsigned int method, unsigned int geometry, unsigned int mode, double wesn[], char *input, void *data);
EXTERN_MSC void * GMT_Retrieve_Data	(void *C, int object_ID);
EXTERN_MSC void * GMT_Duplicate_Data	(void *C, unsigned int family, unsigned int mode, void *data);
EXTERN_MSC void * GMT_Get_Record	(void *C, unsigned int mode, int *retval);
EXTERN_MSC int GMT_Destroy_Session	(void *C);
EXTERN_MSC int GMT_Register_IO		(void *C, unsigned int family, unsigned int method, unsigned int geometry, unsigned int direction, double wesn[], void *resource);
EXTERN_MSC int GMT_Init_IO		(void *C, unsigned int family, unsigned int geometry, unsigned int direction, unsigned int mode, unsigned int n_args, void *args);
EXTERN_MSC int GMT_Begin_IO		(void *C, unsigned int family, unsigned int direction, unsigned int header);
EXTERN_MSC int GMT_Status_IO		(void *C, unsigned int mode);
EXTERN_MSC int GMT_End_IO		(void *C, unsigned int direction, unsigned int mode);
EXTERN_MSC int GMT_Report_Error		(void *C, int error);
EXTERN_MSC int GMT_Put_Data		(void *C, int object_ID, unsigned int mode, void *data);
EXTERN_MSC int GMT_Write_Data		(void *C, unsigned int family, unsigned int method, unsigned int geometry, unsigned int mode, double wesn[], char *output, void *data);
EXTERN_MSC int GMT_Destroy_Data		(void *C, unsigned int mode, void *object);
EXTERN_MSC int GMT_Put_Record		(void *C, unsigned int mode, void *record);
EXTERN_MSC int GMT_Encode_ID		(void *C, char *string, int object_ID);
EXTERN_MSC int GMT_Get_Row		(void *C, int rec_no, struct GMT_GRID *G, float *row);
EXTERN_MSC int GMT_Put_Row		(void *C, int rec_no, struct GMT_GRID *G, float *row);
EXTERN_MSC int GMT_Set_Comment		(void *C, unsigned int family, unsigned int mode, void *arg, void *data);

/* 2 convenience functions to relate (row,col) to a 1-D index and to precompute equidistant coordinates for grids, images */

EXTERN_MSC double * GMT_Get_Coord	(void *C, unsigned int family, unsigned int dim, void *container);
EXTERN_MSC int64_t GMT_Get_Index	(struct GMT_GRID_HEADER *header, int row, int col);

/* 4 functions to show and inquire about GMT common options, GMT default settings, convert a string value to double, and a message printer */

EXTERN_MSC void GMT_Option		(void *C, unsigned int mode, char *options);
EXTERN_MSC int GMT_Get_Common		(void *C, unsigned int option, double *par);
EXTERN_MSC int GMT_Get_Default		(void *C, char *keyword, char *value);
EXTERN_MSC int GMT_Get_Value		(void *C, char *arg, double *par);
EXTERN_MSC int GMT_Report_Message	(void *C, unsigned int level, char *message);

/* 12 secondary functions for argument and option parsing */

EXTERN_MSC struct GMT_OPTION * GMT_Create_Options	(void *C, int argc, void *in);
EXTERN_MSC struct GMT_OPTION * GMT_Prep_Options		(void *C, int mode, void *in);
EXTERN_MSC struct GMT_OPTION * GMT_Make_Option		(void *C, char option, char *arg);
EXTERN_MSC struct GMT_OPTION * GMT_Find_Option		(void *C, char option, struct GMT_OPTION *head);
EXTERN_MSC struct GMT_OPTION * GMT_Append_Option	(void *C, struct GMT_OPTION *current, struct GMT_OPTION *head);
EXTERN_MSC char ** GMT_Create_Args			(void *C, int *argc, struct GMT_OPTION *head);
EXTERN_MSC char * GMT_Create_Cmd			(void *C, struct GMT_OPTION *head);
EXTERN_MSC int GMT_Destroy_Options			(void *C, struct GMT_OPTION **head);
EXTERN_MSC int GMT_Destroy_Args				(void *C, int argc, char *argv[]);
EXTERN_MSC int GMT_Update_Option			(void *C, char option, char *arg, struct GMT_OPTION *head);
EXTERN_MSC int GMT_Delete_Option			(void *C, struct GMT_OPTION *current);
EXTERN_MSC int GMT_Parse_Common				(void *C, char *given_options, struct GMT_OPTION *options);

#ifdef GMT_FFT_EXTENSION
/* Also make available the following 6 GMT_FFT_* functions */
EXTERN_MSC unsigned int GMT_FFT_Option	(void *C, char option, unsigned int dim, char *string);
EXTERN_MSC void * GMT_FFT_Parse		(void *C, char option, unsigned int dim, char *args);
EXTERN_MSC void * GMT_FFT_Create	(void *C, void *X, unsigned int dim, unsigned int subdivide, unsigned int mode, void *F);
EXTERN_MSC double GMT_FFT_Wavenumber	(void *C, uint64_t k, unsigned int mode, void *v_K);
EXTERN_MSC int GMT_FFT			(void *C, void *X, int direction, unsigned int mode, void *v_K);
EXTERN_MSC int GMT_FFT_Destroy		(void *C, void *v_K);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _GMT_H */
