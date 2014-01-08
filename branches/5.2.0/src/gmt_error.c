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
 * Library file for internal GMT error codes
 *
 * Author:	Paul Wessel
 * Date:	26-JUN-2012
 * Version:	5 API
 */

#include "gmt_constants.h"

const char *g_error_string[] = {
	/* GMT_NOERROR */			"No internal GMT error",
	/* GMT_GRDIO_UNKNOWN_FORMAT */		"Not a supported grid format",
	/* GMT_GRDIO_UNKNOWN_TYPE */		"Unknown grid data type",
	/* GMT_GRDIO_UNKNOWN_ID */		"Unknown grid format id number",
	/* GMT_GRDIO_PIPE_CODECHECK */		"Cannot guess grid format type if grid is passed via pipe",
	/* GMT_GRDIO_DOMAIN_VIOLATION */	"Tried to read beyond grid domain",
	/* GMT_GRDIO_OPEN_FAILED */		"Could not open file",
	/* GMT_GRDIO_CREATE_FAILED */		"Could not create file",
	/* GMT_GRDIO_READ_FAILED */		"Could not read from file",
	/* GMT_GRDIO_WRITE_FAILED */		"Could not write to file",
	/* GMT_GRDIO_STAT_FAILED */		"Could not stat file",
	/* GMT_GRDIO_SEEK_FAILED */		"Failed to fseek on file",
	/* GMT_GRDIO_FILE_NOT_FOUND */		"Could not find file",
	/* GMT_GRDIO_BAD_VAL */			"Bad value encountered",
	/* GMT_GRDIO_BAD_XINC */		"Grid x increment <= 0.0",
	/* GMT_GRDIO_BAD_XRANGE */		"Subset x range <= 0.0",
	/* GMT_GRDIO_BAD_YINC */		"Grid y increment <= 0.0",
	/* GMT_GRDIO_BAD_YRANGE */		"Subset y range <= 0.0",
	/* GMT_GRDIO_BAD_IMG_LAT */		"Must specify max latitude for img file",
	/* GMT_GRDIO_NO_2DVAR */		"No 2-D variable in file",
	/* GMT_GRDIO_NO_VAR */			"Named variable does not exist in file",
	/* GMT_GRDIO_BAD_DIM */			"Named variable is not 2-, 3-, 4- or 5-D",
	/* GMT_GRDIO_NC_NO_PIPE */		"NetCDF-based I/O does not support piping",
	/* GMT_GRDIO_NOT_RAS */			"Not a Sun raster file",
	/* GMT_GRDIO_NOT_8BIT_RAS */		"Not a standard 8-bit Sun raster file",
	/* GMT_GRDIO_NOT_SURFER */		"Not a valid Surfer 6|7 grid file",
	/* GMT_GRDIO_SURF7_UNSUPPORTED */	"This Surfer 7 format (full extent or with break lines) is not supported",
	/* GMT_GRDIO_GRD98_XINC */		"GRD98 format requires n = 1/x_inc to be an integer",
	/* GMT_GRDIO_GRD98_YINC */		"GRD98 format requires n = 1/y_inc to be an integer",
	/* GMT_GRDIO_GRD98_BADMAGIC */		"GRD98 grid file has wrong magic number",
	/* GMT_GRDIO_GRD98_BADLENGTH */		"GRD98 grid file has wrong length",
	/* GMT_GRDIO_ESRI_NONSQUARE */		"Only square pixels are allowed in ESRI grids"
	/* GMT_GRDIO_RI_OLDBAD */		"Use grdedit -A on your grid file to make region and increments compatible",
	/* GMT_GRDIO_RI_NEWBAD */		"Please select compatible -R and -I values",
	/* GMT_GRDIO_RI_NOREPEAT */		"Pixel format grids do not have repeating rows or columns",
	/* GMT_IO_BAD_PLOT_DEGREE_FORMAT */	"Unacceptable PLOT_DEGREE_FORMAT template. A not allowed",
	/* GMT_CHEBYSHEV_NEG_ORDER */		"GMT_chebyshev given negative degree",
	/* GMT_CHEBYSHEV_BAD_DOMAIN */		"GMT_chebyshev given |x| > 1",
	/* GMT_MAP_EXCEEDS_360 */		"Map region exceeds 360 degrees",
	/* GMT_MAP_BAD_ELEVATION_MIN */		"\"South\" (min elevation) is outside 0-90 degree range",
	/* GMT_MAP_BAD_ELEVATION_MAX */		"\"North\" (max elevation) is outside 0-90 degree range",
	/* GMT_MAP_BAD_LAT_MIN */		"South is outside -90 to +90 degree range",
	/* GMT_MAP_BAD_LAT_MAX */		"North is outside -90 to +90 degree range",
	/* GMT_MAP_NO_REGION */			"No map region selected",
	/* GMT_MAP_NO_PROJECTION */		"No projection selected",
	/* GMT_MAP_BAD_DIST_FLAG */		"Wrong flag passed to GMT_dist_array",
	/* GMT_MAP_BAD_MEASURE_UNIT */		"Bad measurement unit.  Choose among " GMT_DIM_UNITS_DISPLAY,
};
