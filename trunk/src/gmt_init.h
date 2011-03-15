/*--------------------------------------------------------------------
 *	$Id: gmt_init.h,v 1.88 2011-03-15 02:06:36 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Include file for gmt_init.c
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef GMT_INIT_H
#define GMT_INIT_H

/* Macro to do conversion to inches with PROJ_LENGTH_UNIT as default */

#define GMT_to_inch(C,value) GMT_convert_units (C, value, C->current.setting.proj_length_unit, GMT_INCH)
#define GMT_to_points(C,value) GMT_convert_units (C, value, C->current.setting.proj_length_unit, GMT_PT)

#endif /* GMT_INIT_H */
