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
	char tag[4];	/* E.g., 30m */
	char size[8];	/* E.g., 300M */
	char remark[GMT_LEN128];	/* What it is */
};

struct GMT_DATA_HASH {	/* Holds file hashes (probably SHA256) */
	char name[GMT_LEN64];	/* File name */
	char hash[GMT_LEN128];	/* The file hash */
	size_t size;		/* File size in bytes */
};

#define GMT_N_DATA_INFO_ITEMS 16

GMT_LOCAL struct GMT_DATA_INFO gmt_data_info[GMT_N_DATA_INFO_ITEMS] = {
	{"60m", "106K", "Earth Relief at 60x60 arc minutes obtained by Gaussian Cartesian filtering (111 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"01d", "106K", "Earth Relief at 1x1 arc degrees obtained by Gaussian Cartesian filtering (111 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"30m", "363K", "Earth Relief at 30x30 arc minutes obtained by Gaussian Cartesian filtering (55 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"20m", "759K", "Earth Relief at 20x20 arc minutes obtained by Gaussian Cartesian filtering (37 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"15m", "1.3M", "Earth Relief at 15x15 arc minutes obtained by Gaussian Cartesian filtering (28 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"10m", "2.8M", "Earth Relief at 10x10 arc minutes obtained by Gaussian Cartesian filtering (18 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"06m", "7.3M", "Earth Relief at 6x6 arc minutes obtained by Gaussian Cartesian filtering (10 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"05m",  "10M", "Earth Relief at 5x5 arc minutes obtained by Gaussian Cartesian filtering (9 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"04m",  "16M", "Earth Relief at 4x4 arc minutes obtained by Gaussian Cartesian filtering (7.5 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"03m",  "27M", "Earth Relief at 3x3 arc minutes obtained by Gaussian Cartesian filtering (5.6 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"02m",  "58M", "Earth Relief at 2x2 arc minutes obtained by Gaussian Cartesian filtering (3.7 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"01m", "214M", "Earth Relief at 1x1 arc minutes obtained by Gaussian Cartesian filtering (1.9 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"30s", "765M", "Earth Relief at 30x30 arc seconds obtained by Gaussian Cartesian filtering (0.9 km fullwidth) of SRTM15+V2 [Tozer et al., 2019]"},
	{"15s", "2.6G", "Earth Relief at 15x15 arc seconds provided by SRTM15+ [Tozer et al., 2019]"},
	{"03s", "6.8G", "Earth Relief at 3x3 arc seconds tiles provided by SRTMGL3 (land only) [NASA/USGS]"},
	{"01s",  "41G", "Earth Relief at 1x1 arc seconds tiles provided by SRTMGL1 (land only) [NASA/USGS]"}
};

#endif /* GMT_REMOTE_H */
