/*--------------------------------------------------------------------
 *	$Id: gmtapi_errno.h,v 1.3 2011-03-03 21:02:51 guru Exp $
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

/* Misc GMTAPI error codes; can be passed to GMT_Error_Message */
	
#define GMTAPI_OK			  0
#define GMTAPI_ERROR			 -1
#define GMTAPI_NOT_A_SESSION		 -2
#define GMTAPI_NOT_A_VALID_ID		 -3
#define GMTAPI_FILE_NOT_FOUND		 -4
#define GMTAPI_BAD_PERMISSION		 -5
#define GMTAPI_GRID_READ_ERROR		 -6
#define GMTAPI_GRID_WRITE_ERROR		 -7
#define GMTAPI_DATA_READ_ERROR		 -8
#define GMTAPI_DATA_WRITE_ERROR		 -9
#define GMTAPI_N_COLS_VARY		-10
#define GMTAPI_NO_INPUT			-11
#define GMTAPI_NO_OUTPUT		-12
#define GMTAPI_NOT_A_VALID_IO		-13
#define GMTAPI_NOT_A_VALID_IO_SESSION	-14
#define GMTAPI_NOT_INPUT_OBJECT		-15
#define GMTAPI_NOT_OUTPUT_OBJECT	-16
#define GMTAPI_ERROR_ON_FOPEN		-17
#define GMTAPI_ERROR_ON_FCLOSE		-18
#define GMTAPI_NO_PARAMETERS		-19
#define GMTAPI_NOT_A_VALID_METHOD	-20
#define GMTAPI_PARSE_ERROR		-21
#define GMTAPI_PROG_NOT_FOUND		-22

#define GMTAPI_RUNTIME_ERROR		-99
