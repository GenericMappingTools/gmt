/*--------------------------------------------------------------------
 *
 *  Copyright (c) 2013-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#include "seis_defaults.h"

#define SEIS_EPSILON 0.0001

/* Reading mode values for different formats */
#define SEIS_READ_CMT	0
#define SEIS_READ_AKI	1
#define SEIS_READ_PLANES	2
#define SEIS_READ_AXIS	4
#define SEIS_READ_TENSOR	8

#define SEIS_PLOT_DC		1
#define SEIS_PLOT_AXIS	2
#define SEIS_PLOT_TRACE	4
#define SEIS_PLOT_TENSOR	8

#define SEIS_CART_OFFSET	1	/* Cartesian plot offset in optional|trailing text */
#define SEIS_CART_OFFSET_FIX	2	/* Same, but given as fixed offset with +o on option line */

#define SEiS_EVENT_FILL		0	/* Default is to fill optional symbol with even color */
#define SEiS_FIXED_FILL		1	/* Used fixed color set via +g<fill> */
#define SEiS_NO_FILL		2	/* Skip filling the symbol */

#define squared(x) ((x) * (x))

#define SEIS_LINE_SYNTAX	"[+g[<fill>]][+o[<dx>/<dy>]][+p<pen>][+s[<symbol>]<size>]"

enum Seis_scaletype {
	SEIS_READ_SCALE		= 0,
	SEIS_CONST_SCALE	= 1
};

struct SEIS_OFFSET_LINE { 
	bool active;
	unsigned int mode;	/* 0-3 as above */
	unsigned int symbol;	/* Default to PSL_CIRCLE */
	unsigned int fill_mode;	/* Default to SEiS_EVENT_FILL */
	double size;		/* Circle size if drawn */
	double off[2];		/* Cartesian offsets from actual location [0/0] */
	struct GMT_PEN pen;	/* Pen parameters controlling the line */
	struct GMT_FILL fill;	/* Fill parameters controlling the symbol fill */
};

struct SEIS_AXIS {
	double str;
	double dip;
	double val;
	int e;
};
/* val in 10**e dynes-cm */

struct SEIS_MOMENT {
	double mant;
	int exponent;
};

struct SEIS_NODAL_PLANE {
	double str;
	double dip;
	double rake;
};

struct SEIS_MECHANISM {
	struct SEIS_NODAL_PLANE NP1;
	struct SEIS_NODAL_PLANE NP2;
	struct SEIS_MOMENT moment;
	double magms;
};

struct SEIS_M_TENSOR {
	int expo;
	double f[6];
};
/* mrr mtt mff mrt mrf mtf in 10**expo dynes-cm */

typedef struct SEIS_MOMENT st_mo;
typedef struct SEIS_MECHANISM st_me;
