/*--------------------------------------------------------------------
 *      $Id$
 *
 *      Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * Include file for list of data grids available via @earth_relief_*.grd references.
 *
 * Author:      Paul Wessel
 * Date:        11-Sept-2017
 * Version:     6 API
 */

#ifndef GMT_REMOTE_H
#define GMT_REMOTE_H

struct GMT_DATA_INFO {
	char tag[4];	/* E.g., 30m */
	char size[8];	/* E.g., 300M */
	char remark[GMT_LEN128];	/* What it is */
};

#define GMT_N_DATA_INFO_ITEMS 13

GMT_LOCAL struct GMT_DATA_INFO gmt_data_info[GMT_N_DATA_INFO_ITEMS] = {
	{"60m", "112K", "Earth Relief at 60x60 arc minutes obtained by Gaussian spherical filtering (111 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"30m", "377K", "Earth Relief at 30x30 arc minutes obtained by Gaussian spherical filtering (55 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"20m", "783K", "Earth Relief at 20x20 arc minutes obtained by Gaussian spherical filtering (37 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"15m", "1.4M", "Earth Relief at 15x15 arc minutes obtained by Gaussian spherical filtering (28 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"10m", "2.9M", "Earth Relief at 10x10 arc minutes obtained by Gaussian spherical filtering (18 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"06m", "7.5M", "Earth Relief at 6x6 arc minutes obtained by Gaussian spherical filtering (10 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"05m",  "11M", "Earth Relief at 5x5 arc minutes obtained by Gaussian spherical filtering (9 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"04m",  "16M", "Earth Relief at 4x4 arc minutes obtained by Gaussian spherical filtering (7.5 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"03m",  "28M", "Earth Relief at 3x3 arc minutes obtained by Gaussian spherical filtering (5.6 km fullwidth) of ETOPO1 (Ice g) [NOAA]"},
	{"02m",  "58M", "Earth Relief at 2x2 arc minutes provided by ETOPO2v2g_f4 [NOAA]"},
	{"01m", "214M", "Earth Relief at 1x1 arc minutes provided by ETOPO1 (Ice g) [NOAA]"},
	{"30s", "778M", "Earth Relief at 30x30 arc seconds provided by SRTM30+ [Sandwell/SIO]"},
	{"15s", "2.6G", "Earth Relief at 15x15 arc seconds provided by SRTM15+ [Sandwell/SIO]"}
};

#endif /* GMT_REMOTE_H */
