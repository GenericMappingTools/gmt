/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * gmt_dwc.h contains definitions for using the DCW in GMT.
 *
 * Author:	Paul Wessel
 * Date:	10-MAR-2013
 * Version:	5 API
 */

#ifndef _GMT_DCW_H
#define _GMT_DCW_H

#define GMT_DCW_DIR			"DCW"
#define GMT_DCW_COUNTRIES		230
#define GMT_DCW_STATES			97
#define GMT_DCW_N_CONTINENTS		8
#define GMT_DCW_N_COUNTRIES_WITH_STATES	4

struct GMT_DCW_COUNTRY {
	char continent[3];
	char code[3];
	char name[80];
};

struct GMT_DCW_STATE {
	char country[3];
	char code[3];
	char name[80];
};

struct GMT_DCW_COUNTRY GMT_DCW_country[GMT_DCW_COUNTRIES] = {
#include "gmt_dcw_countries.h"
};
struct GMT_DCW_STATE GMT_DCW_states[GMT_DCW_STATES] = {
#include "gmt_dcw_states.h"
};

char *GMT_DCW_continents[GMT_DCW_N_CONTINENTS] = {"Africa", "Antarctica", "Asia", "Europe", "Oceania", "North America", "South America", "Miscellaneous"};
char *GMT_DCW_country_with_states[GMT_DCW_N_COUNTRIES_WITH_STATES] = {"AU", "BR", "CA", "US"};

#endif /* _GMT_DCW_H */
