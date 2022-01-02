/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Include file for list of data grids available via @earth_relief_*.grd references.
 *
 * Author:      Paul Wessel
 * Date:        19-Oct-2019
 * Version:     6 API
 */

#ifndef GMT_REMOTE_H
#define GMT_REMOTE_H

struct GMT_RESOLUTION {	/* Struct to hold information about the various resolutions for a remote data set family */
	char inc[GMT_LEN8];	/* Grid spacing in text format. E.g., 30m, 03s, etc. */
	char reg;			/* Grid/Image registration (g or p). E.g., g */
	double resolution;	/* In number of nodes per degree. E.g, for 01m that is 60 */
};

struct GMT_DATA_INFO {
	int id;						/* Running number 0-(n-1) AFTER array is sorted */
	bool used;					/* If true then do not repeat the attribution details */
	char dir[GMT_LEN64];		/* Directory of file.  Here, / (root) means /export/gmtserver/gmt/data */
	char file[GMT_LEN64];		/* Full file (or tile directory) name. E.g., earth_relief_20m_g.grd or earth_relief_01m_g/ */
	char ext[GMT_LEN8];			/* Data file extension. E.g., .grd, *tif, etc. */
	char inc[GMT_LEN8];			/* Grid spacing in text format. E.g., 30m */
	char reg;					/* Grid/Image registration (g or p). E.g., g */
	double d_inc;				/* Grid spacing in floating point degrees (e.g., 0.5) */
	double scale;				/* Scale to convert integers to data units */
	double offset;				/* Offset to shift to original data range */
	char size[GMT_LEN8];		/* Total file/tile set size in text format. E.g., 300M */
	double tile_size;			/* Tile size in integer degrees (0 if no tiling) */
	char date[GMT_LEN16];		/* Creation date in yyyy-mm-dd (e.g., 2020-06-01) */
	char tag[GMT_LEN64];		/* Tag for tiling.  E.g., earth_relief_01m_g, SRTMGL3 */
	char coverage[GMT_LEN64];	/* File with tile coverage. E.g., srtm_tiles.nc or - for none */
	char filler[GMT_LEN64];		/* File with background filler. E.g., earth_relief_tiles_15s.grd or - for none */
	char CPT[GMT_LEN64];		/* Name of default master CPT. E.g., geo or - for none */
	char remark[GMT_LEN256];	/* Attribution and information about this data set */
};

struct GMT_DATA_HASH {			/* Holds file hashes (probably SHA256) */
	char name[GMT_LEN64];		/* File name (no leading directory) */
	char hash[GMT_LEN128];		/* The file hash */
	size_t size;				/* File size in bytes */
};

enum GMT_tile_coverage {	/* Values in any tile coverage grid (e.g., srtm_tiles.nc) */
	GMT_NO_TILE      = 0,	/* No high-resolution data for this tile */
	GMT_PARTIAL_TILE = 1,	/* There is data, but part of tile is ocean */
	GMT_FULL_TILE    = 2	/* There is complete coverage on land */
};

#define GMT_SRTM_ONLY	1	/* Mode so that when srtm_relief* is used we do not blend in earth_relief_15s */

#define GMT_HASH_SERVER_FILE "gmt_hash_server.txt"
#define GMT_INFO_SERVER_FILE "gmt_data_server.txt"

#define GMT_HASH_TIME_OUT		10L	/* Not waiting longer than this to time out on getting the hash file */
#define GMT_CONNECT_TIME_OUT	10L	/* Not waiting longer than this to time out on getting a response from the server */

#define GMT_TILE_EXTENSION_REMOTE  		"jp2"	/* Tile extension of JPEG2000 files to be downloaded */
#define GMT_TILE_EXTENSION_REMOTE_LEN	3U		/* Length of JPEG2000 file extension */
#define GMT_TILE_EXTENSION_LOCAL		"nc"	/* Tile extension of netCDF nc short int files to be saved */
#define GMT_TILE_EXTENSION_LOCAL_LEN	2U		/* Length of nc short int file extension */

#define GMT_IMAGE_DPU_VALUE	300	/* 300 dots per inch */
#define GMT_IMAGE_DPU_UNIT	'i'	/* 300 dpts per inch */

#endif /* GMT_REMOTE_H */
