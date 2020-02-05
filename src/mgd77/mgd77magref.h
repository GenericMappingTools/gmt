/* -------------------------------------------------------------------
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *    Copyright (c) 2005-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html) and M. T. Chandler
 *	File:	mgd77magref.h
 *
 *	MGD77 Magnetic Reference Fields for mgd77sniffer
 *
 *	Authors:
 *		Michael Chandler and Paul Wessel
 *		School of Ocean and Earth Science and Technology
 *		University of Hawaii
 *
 *	Date:	1-Aug-2005
 *
 *  References:
 *    Langel, R. A., 1987. The Main Field. In Geomagnetism, Volume I,
 *      (Ed. J. A. Jacobs). London: Academic Press, pp249-512.
 *
 *    National Geophysical Data Center, 2005. IAGA V-MOD Geomagnetic
 *    Field Modeling: International Geomagnetic Reference Field IGRF-10.
 *    30 June 2008. <http://www.ngdc.noaa.gov/IAGA/vmod/igrf.html>.
 * ------------------------------------------------------------------*/
/*		  Field,		Code,	Start,		End */
		{"Unused",      0,      9999,       9999},
		{"AWC 70",		1,		1965,		1970},
		{"AWC 75",		2,		1967,		1974},
		{"IGRF-65",		3,		1955,		1970},
		{"IGRF-75",		4,		1955,		1980},
		{"GSFC-1266",	5,		1900,		1966},
		{"GSFC-0674",	6,		9999,		9999},
		{"UK 75",		7,		1970,		1975},
		{"POGO 0368",	8,		1965,		1968},
		{"POGO 1068",	9,		1965,		1968},
		{"POGO 0869",	10,		1965,		1968},
		{"IGRF-80",		11,		1965,		1985},
		{"IGRF-85",		12,		1945,		1990},
		{"IGRF-90",		13,		1945,		1995},
		{"IGRF-95",		14,		1900,		2000},
		{"IGRF-00",     15,     1900,       2005},
		{"IGRF-05",     16,     1900,       2010},
		{"IGRF-10",     17,     1900,       2015},
		{"IGRF-15",     18,     1900,       2020},
		{"Other",       88,     9999,       9999}
/* When updating this list, also update in mgd77.h:
   MGD77_IGRF_LAST_ID = highest number of field code (not counting 88)
   MGD77_N_MAG_RF = total number of field codes = MGD77_IGRF_LAST_ID + 2
*/
