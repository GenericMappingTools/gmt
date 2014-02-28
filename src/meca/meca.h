/*	$Id$
 *    Copyright (c) 1996-2012 by G. Patau
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

#include <stdio.h>
#include <math.h>

#define EPSIL 0.0001

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#ifndef M_PI_4
#define M_PI_4          0.78539816339744830962
#endif
#ifndef M_PI_2
#define M_PI_2          1.57079632679489661923
#endif

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
