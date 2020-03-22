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
 * Date:	11-JUN-2016
 * Version:	6 API
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
 * are imported have to be identified as such.  EXTERN_MSC is used
 * for all functions are is defined in the declspec.h include file.
 */
#include "declspec.h"

/* Include GMT constants and resources definitions */

#include "gmt_resources.h"

/*=====================================================================================
 *	GMT API FUNCTION PROTOTYPES
 *=====================================================================================
 */

/* 33 Primary API functions */
EXTERN_MSC void * GMT_Create_Session   (const char *tag, unsigned int pad, unsigned int mode, int (*print_func) (FILE *, const char *));
EXTERN_MSC void * GMT_Create_Data      (void *API, unsigned int family, unsigned int geometry, unsigned int mode, uint64_t dim[],
                                           double *wesn, double *inc, unsigned int registration, int pad, void *data);
EXTERN_MSC void * GMT_Read_Data        (void *API, unsigned int family, unsigned int method, unsigned int geometry,
                                           unsigned int mode, double wesn[], const char *input, void *data);
EXTERN_MSC void * GMT_Duplicate_Data   (void *API, unsigned int family, unsigned int mode, void *data);
EXTERN_MSC void * GMT_Get_Record       (void *API, unsigned int mode, int *retval);
EXTERN_MSC int GMT_Destroy_Session     (void *API);
EXTERN_MSC int GMT_Register_IO         (void *API, unsigned int family, unsigned int method, unsigned int geometry,
                                           unsigned int direction, double wesn[], void *resource);
EXTERN_MSC int GMT_Init_IO             (void *API, unsigned int family, unsigned int geometry, unsigned int direction,
                                           unsigned int mode, unsigned int n_args, void *args);
EXTERN_MSC int GMT_Begin_IO            (void *API, unsigned int family, unsigned int direction, unsigned int header);
EXTERN_MSC int GMT_Get_Status          (void *API, unsigned int mode);
EXTERN_MSC int GMT_End_IO              (void *API, unsigned int direction, unsigned int mode);
EXTERN_MSC int GMT_Write_Data          (void *API, unsigned int family, unsigned int method, unsigned int geometry,
                                           unsigned int mode, double wesn[], const char *output, void *data);
EXTERN_MSC int GMT_Destroy_Data        (void *API, void *object);
EXTERN_MSC int GMT_Put_Record          (void *API, unsigned int mode, void *record);
EXTERN_MSC int GMT_Get_Row             (void *API, int rec_no, struct GMT_GRID *G, gmt_grdfloat *row);
EXTERN_MSC int GMT_Put_Row             (void *API, int rec_no, struct GMT_GRID *G, gmt_grdfloat *row);
EXTERN_MSC int GMT_Set_Comment         (void *API, unsigned int family, unsigned int mode, void *arg, void *data);
EXTERN_MSC int GMT_Set_Geometry	       (void *API, unsigned int direction, unsigned int geometry);

EXTERN_MSC int GMT_Open_VirtualFile    (void *API, unsigned int family, unsigned int geometry, unsigned int direction, void *data, char *string);
EXTERN_MSC int GMT_Close_VirtualFile   (void *API, const char *string);
EXTERN_MSC void *GMT_Read_VirtualFile  (void *API, const char *string);
EXTERN_MSC int GMT_Init_VirtualFile    (void *API, unsigned int mode, const char *string);
EXTERN_MSC int GMT_Inquire_VirtualFile (void *V_API, const char *string);
EXTERN_MSC void *GMT_Read_Group        (void *API, unsigned int family, unsigned int method, unsigned int geometry,
                                           unsigned int mode, double wesn[], void *sources, unsigned int *n_items, void *data);
EXTERN_MSC void *GMT_Convert_Data      (void *API, void *in, unsigned int family_in, void *out, unsigned int family_out, unsigned int flag[]);
EXTERN_MSC void *GMT_Alloc_Segment     (void *API, unsigned int mode, uint64_t n_rows, uint64_t n_columns, char *header, void *segment);
EXTERN_MSC int GMT_Set_Columns         (void *API, unsigned int direction, unsigned int n_columns, unsigned int mode);
EXTERN_MSC int GMT_Destroy_Group       (void *API, void *object, unsigned int n_items);
EXTERN_MSC int GMT_Change_Layout       (void *API, unsigned int family, char *code, unsigned int mode, void *obj, void *data, void *alpha);
EXTERN_MSC void *GMT_Get_Vector        (void *API, struct GMT_VECTOR *V, unsigned int col);
EXTERN_MSC void *GMT_Get_Matrix        (void *API, struct GMT_MATRIX *M);
EXTERN_MSC int GMT_Put_Vector          (void *API, struct GMT_VECTOR *V, unsigned int col, unsigned int type, void *vector);
EXTERN_MSC int GMT_Put_Matrix          (void *API, struct GMT_MATRIX *M, unsigned int type, int pad, void *matrix);
/* These 2 functions are new in 6.0 and are being considered beta */
EXTERN_MSC int GMT_Put_Strings         (void *API, unsigned int family, void *object, char **array);
EXTERN_MSC char **GMT_Get_Strings      (void *API, unsigned int family, void *object);

/* 4 functions to relate (row,col) to a 1-D index for grids and images and to precompute equidistant coordinates for grids and images */

EXTERN_MSC uint64_t GMT_Get_Index      (void *API, struct GMT_GRID_HEADER *header, int row, int col);
EXTERN_MSC double * GMT_Get_Coord      (void *API, unsigned int family, unsigned int dim, void *container);
EXTERN_MSC int GMT_Set_Index	       (void *API, struct GMT_GRID_HEADER *header, char *code);	/* Experimental */
EXTERN_MSC uint64_t GMT_Get_Pixel      (void *API, struct GMT_GRID_HEADER *header, int row, int col, int layer);	/* Experimental */

/* 11 functions to show and inquire about GMT common options, GMT default settings, object metadata, convert strings to doubles, and message and report printing */

EXTERN_MSC int GMT_Option              (void *API, const char *options);
EXTERN_MSC int GMT_Get_Common          (void *API, unsigned int option, double *par);
EXTERN_MSC int GMT_Get_Default         (void *API, const char *keyword, char *value);
EXTERN_MSC int GMT_Set_Default         (void *API, const char *keyword, const char *value);
EXTERN_MSC int GMT_Get_Enum            (void *API, char *key);
EXTERN_MSC int GMT_Get_Info            (void *API, unsigned int family, void *data, unsigned int *geometry, uint64_t dim[], double *range, double *inc, unsigned int *registration, int *pad);
EXTERN_MSC int GMT_Get_Values          (void *API, const char *arg, double *par, int maxpar);
EXTERN_MSC int GMT_Report              (void *API, unsigned int level, const char *message, ...);
EXTERN_MSC int GMT_Message             (void *API, unsigned int mode, const char *format, ...);
EXTERN_MSC char * GMT_Error_Message    (void *API);
EXTERN_MSC int GMT_Handle_Messages     (void *API, unsigned int mode, unsigned int method, void *dest);

/* 1 function to list or call the core, optional supplemental, and custom GMT modules */

EXTERN_MSC int GMT_Call_Module	       (void *API, const char *module, int mode, void *args);

/* 15 secondary functions for argument and option parsing */

EXTERN_MSC struct GMT_OPTION *GMT_Create_Options    (void *API, int argc, const void *in);
EXTERN_MSC struct GMT_OPTION *GMT_Duplicate_Options (void *API, struct GMT_OPTION *head);
EXTERN_MSC struct GMT_OPTION *GMT_Make_Option       (void *API, char option, const char *arg);
EXTERN_MSC struct GMT_OPTION *GMT_Find_Option       (void *API, char option, struct GMT_OPTION *head);
EXTERN_MSC struct GMT_OPTION *GMT_Append_Option     (void *API, struct GMT_OPTION *current, struct GMT_OPTION *head);
EXTERN_MSC char **GMT_Create_Args                   (void *API, int *argc, struct GMT_OPTION *head);
EXTERN_MSC char  *GMT_Create_Cmd                    (void *API, struct GMT_OPTION *head);
EXTERN_MSC int    GMT_Destroy_Options               (void *API, struct GMT_OPTION **head);
EXTERN_MSC int    GMT_Destroy_Args                  (void *API, int argc, char **argv[]);
EXTERN_MSC int    GMT_Destroy_Cmd                   (void *API, char **cmd);
EXTERN_MSC int    GMT_Update_Option                 (void *API, struct GMT_OPTION *current, const char *arg);
EXTERN_MSC int    GMT_Free_Option                   (void *API, struct GMT_OPTION **current);
EXTERN_MSC int    GMT_Delete_Option                 (void *API, struct GMT_OPTION *current, struct GMT_OPTION **head);
EXTERN_MSC int    GMT_Parse_Common                  (void *API, const char *given_options, struct GMT_OPTION *options);
EXTERN_MSC char  *GMT_Duplicate_String              (void *API, const char* string);

/* 8 GMT_FFT_* functions */
EXTERN_MSC unsigned int GMT_FFT_Option     (void *API, char option, unsigned int dim, const char *string);
EXTERN_MSC void        *GMT_FFT_Parse      (void *API, char option, unsigned int dim, const char *args);
EXTERN_MSC void        *GMT_FFT_Create     (void *API, void *data, unsigned int dim, unsigned int mode, void *info);
EXTERN_MSC double       GMT_FFT_Wavenumber (void *API, uint64_t k, unsigned int mode, void *info);
EXTERN_MSC int          GMT_FFT            (void *API, void *data, int direction, unsigned int mode, void *info);
EXTERN_MSC int          GMT_FFT_Destroy    (void *API, void *info);
EXTERN_MSC int          GMT_FFT_1D         (void *API, gmt_grdfloat *data, uint64_t n, int direction, unsigned int mode);
EXTERN_MSC int          GMT_FFT_2D         (void *API, gmt_grdfloat *data, unsigned int n_columns, unsigned int n_rows, int direction, unsigned int mode);

/* 3 F77 basic grid i/o functions.  These give basic Fortran programs the ability to read and write any GMT-accessible grid */

EXTERN_MSC int gmt_f77_readgrdinfo_ (unsigned int dim[], double wesn[], double inc[], char *title, char *remark, const char *file, int ltitle, int lremark, int lfile);
EXTERN_MSC int gmt_f77_readgrd_	    (gmt_grdfloat *array, unsigned int dim[], double wesn[], double inc[], char *title, char *remark, const char *file, int ltitle, int lremark, int lfile);
EXTERN_MSC int gmt_f77_writegrd_    (gmt_grdfloat *array, unsigned int dim[], double wesn[], double inc[], const char *title, const char *remark, const char *file, int ltitle, int lremark, int lfile);

/* 7 for external API developers only */
EXTERN_MSC struct GMT_RESOURCE *GMT_Encode_Options (void *API, const char *module, int n_in, struct GMT_OPTION **head, unsigned int *n);
EXTERN_MSC int GMT_Expand_Option   (void *API, struct GMT_OPTION *current, const char *txt);
EXTERN_MSC int GMT_Set_AllocMode   (void *API, unsigned int family, void *object);
EXTERN_MSC int GMT_Extract_Region  (void *API, char *file, double wesn[]);
EXTERN_MSC float GMT_Get_Version   (void *API, unsigned int *major, unsigned int *minor, unsigned int *patch);
EXTERN_MSC void *GMT_Get_Ctrl	   (void *API);

#ifdef __cplusplus
}
#endif

#endif /* _GMT_H */
