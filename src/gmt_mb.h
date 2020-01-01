/*--------------------------------------------------------------------
 *
 *      Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

/*
 * File with definitions that allow compatibility between GMT 6 and
 * the MB-system programs mbcontour.c, mbgrdtiff.c, and mbswath.c.
 *
 * Author:      Paul Wessel
 * Date:        11-Mar-2018
 * Version:     6 API
 */

/* Only included when compiling the MB system */
#ifndef GMT_MB_H
#define GMT_MB_H

#if GMT_MAJOR_VERSION == 6
#define gmt_get_cpt(GMT,file,flag,min,max) gmt_get_palette(GMT,file,flag,min,max,0.0,0)
#define gmt_M_grd_is_global gmt_grd_is_global
#define GMT_c_OPT	"-c<ncopies>"	/* OBSOLETE */
#endif

#endif /* GMT_MB_H */
