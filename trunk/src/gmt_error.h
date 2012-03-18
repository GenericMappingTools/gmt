/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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

/* Verbosity levels */
enum GMT_enum_verbose {GMT_MSG_SILENCE = 0,	/* No messages whatsoever */
	GMT_MSG_FATAL		 = 1,	/* Fatal messages */
	GMT_MSG_NORMAL		 = 2,	/* Warnings level -V */
	GMT_MSG_VERBOSE		 = 3,	/* Longer verbose, -Vl in some programs */
	GMT_MSG_DEBUG		 = 4};	/* Debug messages for developers mostly */

/* Grid i/o error codes */

#define GMT_NOERROR			0
#define GMT_GRDIO_UNKNOWN_FORMAT	-128
#define GMT_GRDIO_UNKNOWN_TYPE		-129
#define GMT_GRDIO_UNKNOWN_ID		-130
#define GMT_GRDIO_UNKNOWN_CODE		-131
#define GMT_GRDIO_PIPE_CODECHECK	-132
#define GMT_GRDIO_DOMAIN_VIOLATION	-133
#define GMT_GRDIO_OPEN_FAILED		-134
#define GMT_GRDIO_CREATE_FAILED		-135
#define GMT_GRDIO_READ_FAILED		-136
#define GMT_GRDIO_WRITE_FAILED		-137
#define GMT_GRDIO_STAT_FAILED		-138
#define GMT_GRDIO_SEEK_FAILED		-139
#define GMT_GRDIO_FILE_NOT_FOUND	-140
#define GMT_GRDIO_BAD_VAL		-141
#define GMT_GRDIO_BAD_XINC		-142
#define GMT_GRDIO_BAD_XRANGE		-143
#define GMT_GRDIO_BAD_XMATCH		-144
#define GMT_GRDIO_BAD_YINC		-145
#define GMT_GRDIO_BAD_YRANGE		-146
#define GMT_GRDIO_BAD_YMATCH		-147
#define GMT_GRDIO_BAD_IMG_LAT		-148
#define GMT_GRDIO_NO_2DVAR		-149
#define GMT_GRDIO_NO_VAR		-150
#define GMT_GRDIO_BAD_DIM		-151
#define GMT_GRDIO_NOT_NC		-152
#define GMT_GRDIO_NC_NO_PIPE		-153
#define GMT_GRDIO_NOT_RAS		-154
#define GMT_GRDIO_NOT_8BIT_RAS		-155
#define GMT_GRDIO_NOT_SURFER		-156
#define GMT_GRDIO_SURF7_UNSUPPORTED	-157
#define GMT_GRDIO_GRD98_XINC		-158
#define GMT_GRDIO_GRD98_YINC		-159
#define GMT_GRDIO_GRD98_BADMAGIC	-160
#define GMT_GRDIO_GRD98_BADLENGTH	-161
#define GMT_GRDIO_ESRI_NONSQUARE	-164
#define GMT_GRDIO_RI_OLDBAD		-165
#define GMT_GRDIO_RI_NEWBAD		-166
#define GMT_GRDIO_RI_NOREPEAT		-167
#define GMT_IO_BAD_PLOT_DEGREE_FORMAT	-168
#define GMT_CHEBYSHEV_NEG_ORDER		-169
#define GMT_CHEBYSHEV_BAD_DOMAIN	-170
#define GMT_MAP_EXCEEDS_360		-171
#define GMT_MAP_BAD_ELEVATION_MIN	-172
#define GMT_MAP_BAD_ELEVATION_MAX	-173
#define GMT_MAP_BAD_LAT_MIN		-174
#define GMT_MAP_BAD_LAT_MAX		-175
#define GMT_MAP_NO_REGION		-176
#define GMT_MAP_NO_PROJECTION		-177
#define GMT_MAP_BAD_DIST_FLAG		-178
#define GMT_MAP_BAD_MEASURE_UNIT	-179
#define GMT_PSL_INIT_FAILED		-180

/* Definition for an error trap */
#define GMT_err_trap(func_call) if ((err = (func_call)) != GMT_NOERROR) return(err)

EXTERN_MSC const char * GMT_strerror (GMT_LONG err);

#define GMT_is_verbose(C,level) (C->current.setting.verbose >= level)

/* Check condition and report error if true */
#define GMT_check_condition(C,condition,...) ((condition) ? GMT_report(C,GMT_MSG_FATAL,__VA_ARGS__) : 0)

/* Convenience functions to GMT_err_func */
#ifdef DEBUG
#define GMT_err_pass(C,err,file) GMT_err_func (C,err,FALSE,file,__FILE__,__LINE__)
#define GMT_err_fail(C,err,file) GMT_err_func (C,err,TRUE,file,__FILE__,__LINE__)
#else
#define GMT_err_pass(C,err,file) GMT_err_func (C,err,FALSE,file,"",0)
#define GMT_err_fail(C,err,file) GMT_err_func (C,err,TRUE,file,"",0)
#endif

#endif /* GMT_ERROR_H */
