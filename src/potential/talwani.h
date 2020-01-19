/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

/*!
 * \file talwani.h
 * \brief 
 */

#ifndef TALWANI_H
#define TALWANI_H

#define TOL		1.0e-7	/* Gotta leave a bit slack for these calculations */
#define SI_TO_MGAL	1.0e5		/* Convert m/s^2 to mGal */
#define SI_TO_EOTVOS	1.0e9		/* Convert (m/s^2)/m to Eotvos */
#define DEG_TO_KM	111.319490793	/* For flat-Earth scaling of degrees to km on WGS-84 Equator */
#define SI_GAMMA 	6.673e-11	/* Gravitational constant (SI units) */
#define GAMMA 		6.673		/* Gravitational constant for distances in km and mass in kg/m^3 */

#include "../mgd77/mgd77_IGF_coeffs.h"	/* Normal gravity coefficients */

GMT_LOCAL double g_normal (double lat) {
	/* IAG 1980 model */
	double slat2 = sind (lat);
	slat2 *= slat2;
	return (MGD77_IGF80_G0 * ((1.0 + MGD77_IGF80_G1 * slat2) / sqrt (1.0 - MGD77_IGF80_G2 * slat2)) / SI_TO_MGAL);
}

#endif /* TALWANI_H */
