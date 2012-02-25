/*
 * $Id$
 *
 * This file contains support functions for FORTRAN code.  For example,
 * under HP-UX A.09.05, the U77 library doesn't contain the exit()
 * routine -- so we create one here.
 */

#include <config.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

#include "../fortran/ncfortran.h"


#if defined(f2cFortran) && !defined(pgiFortran) && !defined(gFortran)
/*
 * The f2c(1) utility on BSD/OS and Linux systems adds an additional
 * underscore suffix (besides the usual one) to global names that have
 * an embedded underscore.  For example, `nfclose' becomes `nfclose_',
 * but `nf_close' becomes `nf_close__.  Consequently, we have to modify
 * some names.
 */
#define max_uchar	max_uchar_
#define min_schar	min_schar_
#define max_schar	max_schar_
#define min_short	min_short_
#define max_short	max_short_
#define min_int		min_int_
#define max_int		max_int_
#define min_long	min_long_
#define max_long	max_long_
#define max_float	max_float_
#define max_double	max_double_
#endif	/* f2cFortran */


FCALLSCSUB1(exit, UDEXIT, udexit, FINT2CINT)


FCALLSCSUB0(abort, UDABORT, udabort)


static double
myrand(int iflag)
{
    if (iflag != 0)
	srand(iflag);

    /*
     * Return a pseudo-random value between 0.0 and 1.0.
     *
     * We don't use RAND_MAX here because not all compilation
     * environments define it (e.g. gcc(1) under SunOS 4.1.3).
     */
    return (rand() % 32768) / 32767.0;
}
FCALLSCFUN1(DOUBLE, myrand, UDRAND, udrand, FINT2CINT)


static int
myshift(int value, int amount)
{
    if (amount < 0)
	value >>= -amount;
    else
    if (amount > 0)
	value <<= amount;
    return value;
}
FCALLSCFUN2(NF_INT, myshift, UDSHIFT, udshift, FINT2CINT, FINT2CINT)

#include <signal.h>
static void
nc_ignorefpe(int doit)
{
	if(doit)
		(void) signal(SIGFPE, SIG_IGN);
}
FCALLSCSUB1(nc_ignorefpe, IGNOREFPE, ignorefpe, FINT2CINT)

static double cmax_uchar()
{
    return UCHAR_MAX;
}
FCALLSCFUN0(DOUBLE, cmax_uchar, MAX_UCHAR, max_uchar)

static double cmin_schar()
{
    return SCHAR_MIN;
}
FCALLSCFUN0(DOUBLE, cmin_schar, MIN_SCHAR, min_schar)

static double cmax_schar()
{
    return SCHAR_MAX;
}
FCALLSCFUN0(DOUBLE, cmax_schar, MAX_SCHAR, max_schar)

static double cmin_short()
{
    return SHRT_MIN;
}
FCALLSCFUN0(DOUBLE, cmin_short, MIN_SHORT, min_short)

static double cmax_short()
{
    return SHRT_MAX;
}
FCALLSCFUN0(DOUBLE, cmax_short, MAX_SHORT, max_short)

static double cmin_int()
{
    return INT_MIN;
}
FCALLSCFUN0(DOUBLE, cmin_int, MIN_INT, min_int)

static double cmax_int()
{
    return INT_MAX;
}
FCALLSCFUN0(DOUBLE, cmax_int, MAX_INT, max_int)

static double cmin_long()
{
    return LONG_MIN;
}
FCALLSCFUN0(DOUBLE, cmin_long, MIN_LONG, min_long)

static double cmax_long()
{
    return LONG_MAX;
}
FCALLSCFUN0(DOUBLE, cmax_long, MAX_LONG, max_long)

static double cmax_float()
{
    return FLT_MAX;
}
FCALLSCFUN0(DOUBLE, cmax_float, MAX_FLOAT, max_float)

static double cmax_double()
{
    return DBL_MAX;
}
FCALLSCFUN0(DOUBLE, cmax_double, MAX_DOUBLE, max_double)
