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
 * Include file for GMT error codes
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

/*!
 * \file gmt_error.h
 * \brief Include file for GMT error codes
 */

#ifndef GMT_ERROR_H
#define GMT_ERROR_H

/* Grid i/o error codes */

/* External array with error descriptions */
EXTERN_MSC const char* gmt_error_string[];

enum Gmt_error_code {
	GMT_NOERROR_UNUSED=0,	/* The real GMT_NOERROR is declared in gmt_resources.h and is part of API */
	GMT_GRDIO_NONUNIQUE_FORMAT,
	GMT_GRDIO_UNKNOWN_FORMAT,
	GMT_GRDIO_UNKNOWN_TYPE,
	GMT_GRDIO_UNKNOWN_ID,
	GMT_GRDIO_PIPE_CODECHECK,
	GMT_GRDIO_DOMAIN_VIOLATION,
	GMT_GRDIO_OPEN_FAILED,
	GMT_GRDIO_CREATE_FAILED,
	GMT_GRDIO_READ_FAILED,
	GMT_GRDIO_WRITE_FAILED,
	GMT_GRDIO_STAT_FAILED,
	GMT_GRDIO_SEEK_FAILED,
	GMT_GRDIO_FILE_NOT_FOUND,
	GMT_GRDIO_BAD_VAL,
	GMT_GRDIO_BAD_XINC,
	GMT_GRDIO_BAD_XRANGE,
	GMT_GRDIO_BAD_YINC,
	GMT_GRDIO_BAD_YRANGE,
	GMT_GRDIO_BAD_IMG_LAT,
	GMT_GRDIO_NO_2DVAR,
	GMT_GRDIO_NO_VAR,
	GMT_GRDIO_BAD_DIM,
	GMT_GRDIO_NC_NO_PIPE,
	GMT_GRDIO_NC_NOT_COARDS,
	GMT_GRDIO_NOT_RAS,
	GMT_GRDIO_NOT_8BIT_RAS,
	GMT_GRDIO_NOT_SURFER,
	GMT_GRDIO_SURF7_UNSUPPORTED,
	GMT_GRDIO_GRD98_XINC,
	GMT_GRDIO_GRD98_YINC,
	GMT_GRDIO_GRD98_BADMAGIC,
	GMT_GRDIO_GRD98_BADLENGTH,
	GMT_GRDIO_ESRI_NONSQUARE,
	GMT_GRDIO_RI_OLDBAD,
	GMT_GRDIO_RI_NEWBAD,
	GMT_GRDIO_RI_NOREPEAT,
	GMT_IO_BAD_PLOT_DEGREE_FORMAT,
	GMT_CHEBYSHEV_NEG_ORDER,
	GMT_CHEBYSHEV_BAD_DOMAIN,
	GMT_MAP_EXCEEDS_360,
	GMT_MAP_BAD_ELEVATION_MIN,
	GMT_MAP_BAD_ELEVATION_MAX,
	GMT_MAP_BAD_LAT_MIN,
	GMT_MAP_BAD_LAT_MAX,
	GMT_MAP_NO_REGION,
	GMT_MAP_NO_PROJECTION,
	GMT_MAP_BAD_DIST_FLAG,
	GMT_MAP_BAD_MEASURE_UNIT
};

EXTERN_MSC const char * GMT_strerror (int err);

#define gmt_M_is_verbose(C,level) (MAX(C->parent->verbose, C->current.setting.verbose) >= level)

/* Check condition and report error if true */
#define gmt_M_check_condition(C,condition,...) ((condition) ? 1+GMT_Report(C->parent,GMT_MSG_NORMAL,__VA_ARGS__) : 0)

/* Set __func__ identifier */
#ifndef HAVE___FUNC__
#	ifdef HAVE___FUNCTION__
#		define __func__ __FUNCTION__
#	else
#		define __func__ "<unknown>"
#	endif
#endif

/* Concatenate __FILE__ and __LINE__ as string */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __SOURCE_LINE __FILE__ ":" TOSTRING(__LINE__)
static inline char* __source_line_func (const char* src_line, const char* func) {
	/* This function is not thread-safe */
	static char str[GMT_LEN256];
	const char *c = src_line;
	size_t len;
	*str = '\0';
	while ((c = strpbrk (c, "/\\")) != NULL) /* get basename of src_line */
		src_line = ++c;
	strncat (str, src_line, GMT_LEN256-1);
	len = strlen (src_line);
	strncat (str, "(", GMT_LEN256 - 2 - len);
	strncat (str, func, GMT_LEN256 - 3 - len);
	strcat (str, ")");
	return str;
}
#define __SOURCE_LINE_FUNC __source_line_func (__FILE__ ":" TOSTRING(__LINE__), __func__)

/* Convenience functions to gmt_err_func */
#ifdef DEBUG
#	define gmt_M_err_pass(C,err,file) gmt_err_func(C,err,false,file,__SOURCE_LINE_FUNC)
#	define gmt_M_err_fail(C,err,file) gmt_err_func(C,err,true,file,__SOURCE_LINE_FUNC)
#else
#	define gmt_M_err_pass(C,err,file) gmt_err_func(C,err,false,file,__func__)
#	define gmt_M_err_fail(C,err,file) gmt_err_func(C,err,true,file,__func__)
#endif

#endif /* GMT_ERROR_H */
