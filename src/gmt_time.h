/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * gmt_time.h contains definitions of structures dealing with time.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	6 API
 */

/*!
 * \file gmt_time.h
 * \brief Definitions of structures dealing with time.
 */

#ifndef GMT_TIME_H
#define GMT_TIME_H

/*--------------------------------------------------------------------
 *			GMT TIME STRUCTURES
 *--------------------------------------------------------------------*/

struct GMT_TIME_SYSTEM {
	double epoch_t0;		/* Rata_die fraction (in days since epoch, 0 <= t0 < 1) */
	double scale;			/* Converts user units to seconds */
	double i_scale;			/* Converts seconds to user units (= 1.0/scale) */
	int64_t rata_die;		/* Rata die number of epoch */
	char epoch[GMT_LEN64];	/* User-defined epoch for time */
	char unit;			/* User-defined time unit */
};

#endif  /* GMT_TIME_H */
