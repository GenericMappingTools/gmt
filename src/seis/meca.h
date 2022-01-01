/*
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
 *    Distributed under the Lesser GNU Public License
 *    See README file for copying and redistribution conditions.
 */

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
