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
 * Include file for GMT error codes
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef GMT_ERROR_H
#define GMT_ERROR_H

/* Grid i/o error codes */

/* external array with error descriptions */
EXTERN_MSC const char* g_error_string[];

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

/* Definition for an error trap */
#ifdef DEBUG
#define GMT_err_trap(func_call) if ((err = (func_call)) != GMT_NOERROR) {GMT_Report(GMT->parent,GMT_MSG_NORMAL,"GMT_err_trap: %d\n", err);return(err);}
#else
#define GMT_err_trap(func_call) if ((err = (func_call)) != GMT_NOERROR) return(err)
#endif

EXTERN_MSC const char * GMT_strerror (int err);

#define GMT_is_verbose(C,level) (C->current.setting.verbose >= level)

/* Check condition and report error if true */
#define GMT_check_condition(C,condition,...) ((condition) ? 1+GMT_Report(C->parent,GMT_MSG_NORMAL,__VA_ARGS__) : 0)

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
	static char str[256];
	const char *c = src_line;
	size_t len;
  *str = '\0';
	while ((c = strpbrk (c, "/\\"))) /* get basename of src_line */
		src_line = ++c;
	strncat (str, src_line, 255);
	len = strlen (src_line);
	strncat (str, "(", 255 - 1 - len);
	strncat (str, func, 255 - 2 - len);
	strcat (str, ")");
	return str;
}
#define __SOURCE_LINE_FUNC __source_line_func (__FILE__ ":" TOSTRING(__LINE__), __func__)

/* Convenience functions to GMT_err_func */
#ifdef DEBUG
#	define GMT_err_pass(C,err,file) GMT_err_func(C,err,false,file,__SOURCE_LINE_FUNC)
#	define GMT_err_fail(C,err,file) GMT_err_func(C,err,true,file,__SOURCE_LINE_FUNC)
#else
#	define GMT_err_pass(C,err,file) GMT_err_func(C,err,false,file,__func__)
#	define GMT_err_fail(C,err,file) GMT_err_func(C,err,true,file,__func__)
#endif

#endif /* GMT_ERROR_H */
