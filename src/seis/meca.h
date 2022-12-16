/*--------------------------------------------------------------------
 *
 *  Copyright (c) 2013-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *  See LICENSE.TXT file for copying and redistribution conditions.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 3 or any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 *  Copyright (c) 1996-2012 by G. Patau
 *  Donated to the GMT project by G. Patau upon her retirement from IGPG
 *--------------------------------------------------------------------*/

/*!
 * \file meca.h
 * \brief
 */

#include <stdio.h>
#include <math.h>

#define EPSIL 0.0001

#define SEIS_MAG_REFERENCE 5.0			/* Reference magnitude for -S */
#define SEIS_MOMENT_MANT_REFERENCE 4.0	/* Mantissa for reference moment for -S */
#define SEIS_MOMENT_EXP_REFERENCE 23	/* Exponent for reference moment for -S */

#define squared(x) ((x) * (x))

struct AXIS {
	double str;
	double dip;
	double val;
	int e;
};
/* val in 10**e dynes-cm */

struct MOMENT {
	double mant;
	int exponent;
};

struct nodal_plane {
	double str;
	double dip;
	double rake;
};

struct MECHANISM {
	struct nodal_plane NP1;
	struct nodal_plane NP2;
	struct MOMENT moment;
	double magms;
};

struct M_TENSOR {
	int expo;
	double f[6];
};
/* mrr mtt mff mrt mrf mtf in 10**expo dynes-cm */

typedef struct MOMENT st_mo;
typedef struct MECHANISM st_me;
