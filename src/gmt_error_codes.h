/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Include file for GMT API error codes
 *
 * Author:	Paul Wessel
 * Date:	22-JUN-2012
 * Version:	5 API
 */

#ifndef GMT_ERROR_CODES_H
#define GMT_ERROR_CODES_H

#include "declspec.h"
/* external array with error descriptions */
EXTERN_MSC const char* g_api_error_string[];

enum Gmt_api_error_code {
	GMT_OK = 0,
	GMT_WRONG_MATRIX_SHAPE,
	GMT_ACCESS_NOT_ENABLED,
	GMT_ARGV_LIST_NULL,
	GMT_ARG_IS_NULL,
	GMT_COUNTER_IS_NEGATIVE,
	GMT_BAD_GEOMETRY,
	GMT_BAD_PERMISSION,
	GMT_CPT_READ_ERROR,
	GMT_DATA_READ_ERROR,
	GMT_DATA_WRITE_ERROR,
	GMT_DIM_TOO_LARGE,
	GMT_DIM_TOO_SMALL,
	GMT_ERROR_ON_FCLOSE,
	GMT_ERROR_ON_FDOPEN,
	GMT_ERROR_ON_FOPEN,
	GMT_FILE_NOT_FOUND,
	GMT_GRID_BC_ERROR,
	GMT_GRID_READ_ERROR,
	GMT_GRID_WRITE_ERROR,
	GMT_ID_TOO_LARGE,
	GMT_IMAGE_BC_ERROR,
	GMT_IMAGE_READ_ERROR,
	GMT_MEMORY_ERROR,
	GMT_FREE_EXTERNAL_NOT_ALLOWED,
	GMT_FREE_WRONG_LEVEL,
	GMT_NOT_A_SESSION,
	GMT_NOT_A_VALID_ARG,
	GMT_NOT_A_VALID_DIRECTION,
	GMT_NOT_A_VALID_FAMILY,
	GMT_NOT_A_VALID_ID,
	GMT_NOT_A_VALID_IO_ACCESS,
	GMT_NOT_A_VALID_IO_MODE,
	GMT_NOT_A_VALID_IO_SESSION,
	GMT_NOT_A_VALID_METHOD,
	GMT_NOT_A_VALID_MODE,
	GMT_NOT_A_VALID_MODULE,
	GMT_NOT_A_VALID_TYPE,
	GMT_NOT_INPUT_OBJECT,
	GMT_NOT_OUTPUT_OBJECT,
	GMT_NO_GRDHEADER,
	GMT_NO_INPUT,
	GMT_NO_OUTPUT,
	GMT_NO_PARAMETERS,
	GMT_NO_RESOURCES,
	GMT_N_COLS_NOT_SET,
	GMT_N_COLS_VARY,
	GMT_N_ROWS_NOT_SET,
	GMT_OBJECT_NOT_FOUND,
	GMT_OGR_ONE_TABLE_ONLY,
	GMT_ONLY_ONE_ALLOWED,
	GMT_OPTION_EXIST,
	GMT_OPTION_HISTORY_ERROR,
	GMT_OPTION_IS_NULL,
	GMT_OPTION_LIST_NULL,
	GMT_OPTION_NOT_FOUND,
	GMT_OPTION_SORT_ERROR,
	GMT_OUTPUT_NOT_SET,
	GMT_PADDING_NOT_ALLOWED,
	GMT_PARSE_ERROR,
	GMT_PROG_NOT_FOUND,
	GMT_PTR_IS_NULL,
	GMT_PTR_NOT_NULL,
	GMT_PTR_NOT_UNIQUE,
	GMT_READ_ONCE,
	GMT_RUNTIME_ERROR,
	GMT_SIZE_IS_ZERO,
	GMT_STREAM_NOT_ALLOWED,
	GMT_SUBSET_NOT_ALLOWED,
	GMT_VALUE_NOT_SET,
	GMT_WRITTEN_ONCE
};
#endif /* GMT_ERROR_CODES_H */
