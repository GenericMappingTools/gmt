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
 * Include file for list of data grids available via @earth_relief_*.grd references.
 *
 * Author:      Paul Wessel
 * Date:        19-Oct-2019
 * Version:     6 API
 */

#ifndef GMT_REMOTE_H
#define GMT_REMOTE_H

struct GMT_DATA_INFO {
	int id;						/* Running number 0-(n-1) AFTER array is sorted */
	char dir[GMT_LEN64];		/* Here, / means /export/gmtserver/gmt/data */
	char file[GMT_LEN64];		/* E.g., earth_relief_20m_g.grd */
	char ext[GMT_LEN8];			/* E.g., .grd, *tif, etc. */
	char inc[GMT_LEN8];			/* E.g., 30m */
	char reg;					/* E.g., g */
	double d_inc;				/* Grid spacing in degrees (e.g., 0.5) */
	double scale;				/* Scale integers to data units */
	double offset;				/* Offset to shift to data range */
	char size[GMT_LEN8];		/* E.g., 300M */
	double tile_size;			/* Tile size in degrees (0 if no tiles) */
	char tag[GMT_LEN16];		/* E.g., ER, SRTMGL3 */
	char coverage[GMT_LEN64];	/* E.g., ER, earth_relief_tiles_03s.nc or - for none */
	char filler[GMT_LEN64];		/* E.g., ER, earth_relief_tiles_15s.grd or - for none*/
	char remark[GMT_LEN128];	/* What it is */
};

struct GMT_DATA_HASH {	/* Holds file hashes (probably SHA256) */
	char name[GMT_LEN64];	/* File name */
	char hash[GMT_LEN128];	/* The file hash */
	size_t size;			/* File size in bytes */
};

#define GMT_HASH_SERVER_FILE "gmt_hash_server.txt"
#define GMT_INFO_SERVER_FILE "gmt_data_server.txt"

#define GMT_HASH_INDEX	0
#define GMT_INFO_INDEX	1

#endif /* GMT_REMOTE_H */
