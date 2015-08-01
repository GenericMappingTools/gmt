/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015
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

/**
 * \file gmt.h
 * \brief Include file for users who wish to develop custom applications 
 *
 * Include file for users who wish to develop custom applications that
 * require building blocks from the GMT Application Program Interface
 * library (the GMT API), which also depends on the GMT Core library.
 * For complete documentation, see the GMT API docs.
 */
/*
 * Author: 	Paul Wessel
 * Date:	10-JUN-2013
 * Version:	5.2 API
 */

#ifndef _GMT_H
#define _GMT_H

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

/*
 * We only include the basic include files needed by the API. Users may
 * need to include additional files, such as <math.h>, etc. In order to
 * be portable we use uint64_t and int64_t for unsigned and signed long
 * integers, hence the stdint.h below.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/*
 * When an application links to a DLL in Windows, the symbols that
 * are imported have to be identified as such.  MSC_EXTERN is used
 * for all functions are is defined in the declspec.h include file.
 */
#include "declspec.h"

/* Include GMT constants and resources definitions */

#include "gmt_resources.h"

/*=====================================================================================
 *	GMT API FUNCTION PROTOTYPES
 *=====================================================================================
 */

/* 22 Primary API functions */
EXTERN_MSC void * GMT_Create_Session(char *tag, unsigned int pad, unsigned int mode, int (*print_func) (FILE *, const char *));
EXTERN_MSC void * GMT_Create_Data	(void *API, unsigned int family, unsigned int geometry, unsigned int mode, uint64_t dim[],
                                     double *wesn, double *inc, unsigned int registration, int pad, void *data);
EXTERN_MSC void * GMT_Get_Data		(void *API, int object_ID, unsigned int mode, void *data);
EXTERN_MSC void * GMT_Read_Data		(void *API, unsigned int family, unsigned int method, unsigned int geometry,
                                     unsigned int mode, double wesn[], char *input, void *data);
EXTERN_MSC void * GMT_Retrieve_Data	(void *API, int object_ID);
EXTERN_MSC void * GMT_Duplicate_Data(void *API, unsigned int family, unsigned int mode, void *data);
EXTERN_MSC void * GMT_Get_Record	(void *API, unsigned int mode, int *retval);
EXTERN_MSC int GMT_Destroy_Session	(void *API);
EXTERN_MSC int GMT_Register_IO		(void *API, unsigned int family, unsigned int method, unsigned int geometry,
                                     unsigned int direction, double wesn[], void *resource);
EXTERN_MSC int GMT_Init_IO		(void *API, unsigned int family, unsigned int geometry, unsigned int direction,
                                     unsigned int mode, unsigned int n_args, void *args);
EXTERN_MSC int GMT_Begin_IO		(void *API, unsigned int family, unsigned int direction, unsigned int header);
EXTERN_MSC int GMT_Status_IO		(void *API, unsigned int mode);
EXTERN_MSC int GMT_End_IO		(void *API, unsigned int direction, unsigned int mode);
EXTERN_MSC int GMT_Put_Data		(void *API, int object_ID, unsigned int mode, void *data);
EXTERN_MSC int GMT_Write_Data		(void *API, unsigned int family, unsigned int method, unsigned int geometry,
                                     unsigned int mode, double wesn[], char *output, void *data);
EXTERN_MSC int GMT_Destroy_Data		(void *API, void *object);
EXTERN_MSC int GMT_Put_Record		(void *API, unsigned int mode, void *record);
EXTERN_MSC int GMT_Encode_ID		(void *API, char *string, int object_ID);
EXTERN_MSC int GMT_Get_Row		(void *API, int rec_no, struct GMT_GRID *G, float *row);
EXTERN_MSC int GMT_Put_Row		(void *API, int rec_no, struct GMT_GRID *G, float *row);
EXTERN_MSC int GMT_Set_Comment		(void *API, unsigned int family, unsigned int mode, void *arg, void *data);
EXTERN_MSC int GMT_Get_ID 		(void *API, unsigned int family, unsigned int direction, void *resource);

/* 2 functions to relate (row,col) to a 1-D index and to precompute equidistant coordinates for grids and images */

EXTERN_MSC int64_t GMT_Get_Index	(void *API, struct GMT_GRID_HEADER *header, int row, int col);
EXTERN_MSC double * GMT_Get_Coord	(void *API, unsigned int family, unsigned int dim, void *container);

/* 7 functions to show and inquire about GMT common options, GMT default settings, convert strings to doubles, and message and report printing */

EXTERN_MSC int GMT_Option		(void *API, char *options);
EXTERN_MSC int GMT_Get_Common		(void *API, unsigned int option, double *par);
EXTERN_MSC int GMT_Get_Default		(void *API, char *keyword, char *value);
EXTERN_MSC int GMT_Set_Default		(void *API, char *keyword, char *value);
EXTERN_MSC int GMT_Get_Value		(void *API, char *arg, double *par);
EXTERN_MSC int GMT_Report		(void *API, unsigned int level, char *message, ...);
EXTERN_MSC int GMT_Message		(void *API, unsigned int mode, char *format, ...);

/* 1 function to list or call the core, optional supplemental, and custom GMT modules */

EXTERN_MSC int GMT_Call_Module		(void *API, const char *module, int mode, void *args);

/* 12 secondary functions for argument and option parsing */

EXTERN_MSC struct GMT_OPTION * GMT_Create_Options	(void *API, int argc, void *in);
EXTERN_MSC struct GMT_OPTION * GMT_Make_Option		(void *API, char option, char *arg);
EXTERN_MSC struct GMT_OPTION * GMT_Find_Option		(void *API, char option, struct GMT_OPTION *head);
EXTERN_MSC struct GMT_OPTION * GMT_Append_Option	(void *API, struct GMT_OPTION *current, struct GMT_OPTION *head);
EXTERN_MSC char ** GMT_Create_Args			(void *API, int *argc, struct GMT_OPTION *head);
EXTERN_MSC char * GMT_Create_Cmd			(void *API, struct GMT_OPTION *head);
EXTERN_MSC int GMT_Destroy_Options			(void *API, struct GMT_OPTION **head);
EXTERN_MSC int GMT_Destroy_Args				(void *API, int argc, char **argv[]);
EXTERN_MSC int GMT_Destroy_Cmd				(void *API, char **cmd);
EXTERN_MSC int GMT_Update_Option			(void *API, struct GMT_OPTION *current, char *arg);
EXTERN_MSC int GMT_Delete_Option			(void *API, struct GMT_OPTION *current);
EXTERN_MSC int GMT_Parse_Common				(void *API, char *given_options, struct GMT_OPTION *options);

/* 8 GMT_FFT_* functions */
EXTERN_MSC unsigned int GMT_FFT_Option	(void *API, char option, unsigned int dim, char *string);
EXTERN_MSC void * GMT_FFT_Parse		(void *API, char option, unsigned int dim, char *args);
EXTERN_MSC void * GMT_FFT_Create	(void *API, void *X, unsigned int dim, unsigned int mode, void *F);
EXTERN_MSC double GMT_FFT_Wavenumber	(void *API, uint64_t k, unsigned int mode, void *K);
EXTERN_MSC int GMT_FFT			(void *API, void *X, int direction, unsigned int mode, void *K);
EXTERN_MSC int GMT_FFT_Destroy		(void *API, void *K);
EXTERN_MSC int GMT_FFT_1D		(void *API, float *data, uint64_t n, int direction, unsigned int mode);
EXTERN_MSC int GMT_FFT_2D		(void *API, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode);

/* 3 F77 basic grid i/ functions.  These give basic Fortran programs the ability to read and write any GMT-accessible grid */

EXTERN_MSC int GMT_F77_readgrdinfo_	(unsigned int dim[], double wesn[], double inc[], char *title, char *remark, char *file);
EXTERN_MSC int GMT_F77_readgrd_		(float *array, unsigned int dim[], double wesn[], double inc[], char *title, char *remark, char *file);
EXTERN_MSC int GMT_F77_writegrd_	(float *array, unsigned int dim[], double wesn[], double inc[], char *title, char *remark, char *file);

/* 2 for external API developers only */
EXTERN_MSC struct GMT_RESOURCE * GMT_Encode_Options	(void *V_API, char *module, char marker, struct GMT_OPTION **head, unsigned int *n);
EXTERN_MSC int GMT_Expand_Option			(void *API, struct GMT_OPTION *current, char marker, char *txt);

#ifdef __cplusplus
}
#endif

#endif /* _GMT_H */
