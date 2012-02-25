/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF vars functions.

Copyright 2006, University Corporation for Atmospheric Research. See
the COPYRIGHT file for copying and redistribution conditions.

$Id$
*/

#include <config.h>
#include "netcdf.h"
#include "nfconfig.inc"
#include "ncfortran.h"
#include "fort-lib.h"


/*
 * Write values from a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN6(NF_INT, nc_put_vars_text, NF_PUT_VARS_TEXT, nf_put_vars_text,
	    NCID, VARID, COORDS, COUNTS, STRIDES, CBUF)


/*
 * Read values into a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN6(NF_INT, nc_get_vars_text, NF_GET_VARS_TEXT, nf_get_vars_text,
	    NCID, VARID, COORDS, COUNTS, STRIDES, CBUF)


/*
 * Write values from a FORTRAN INTEGER*1 variable array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN6(NF_INT, nc_put_vars_schar, NF_PUT_VARS_INT1, nf_put_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INT1VARV)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN6(NF_INT, nc_put_vars_short, NF_PUT_VARS_INT1, nf_put_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INT1VARV)
#elif NF_INT1_IS_C_INT
FCALLSCFUN6(NF_INT, nc_put_vars_int, NF_PUT_VARS_INT1, nf_put_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INT1VARV)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_put_vars_long, NF_PUT_VARS_INT1, nf_put_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INT1VARV)
#endif


/*
 * Read values into a FORTRAN INTEGER*1 variable array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN6(NF_INT, nc_get_vars_schar, NF_GET_VARS_INT1, nf_get_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINT1VARV)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN6(NF_INT, nc_get_vars_short, NF_GET_VARS_INT1, nf_get_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINT1VARV)
#elif NF_INT1_IS_C_INT
FCALLSCFUN6(NF_INT, nc_get_vars_int, NF_GET_VARS_INT1, nf_get_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINT1VARV)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_get_vars_long, NF_GET_VARS_INT1, nf_get_vars_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINT1VARV)
#endif


/*
 * Write values from a FORTRAN INTEGER*2 variable array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN6(NF_INT, nc_put_vars_short, NF_PUT_VARS_INT2, nf_put_vars_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INT2VARV)
#elif NF_INT2_IS_C_INT
FCALLSCFUN6(NF_INT, nc_put_vars_int, NF_PUT_VARS_INT2, nf_put_vars_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INT2VARV)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_put_vars_long, NF_PUT_VARS_INT2, nf_put_vars_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INT2VARV)
#endif


/*
 * Read values into a FORTRAN INTEGER*2 variable array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN6(NF_INT, nc_get_vars_short, NF_GET_VARS_INT2, nf_get_vars_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINT2VARV)
#elif NF_INT2_IS_C_INT
FCALLSCFUN6(NF_INT, nc_get_vars_int, NF_GET_VARS_INT2, nf_get_vars_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINT2VARV)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_get_vars_long, NF_GET_VARS_INT2, nf_get_vars_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINT2VARV)
#endif


/*
 * Write values from a FORTRAN INTEGER variable array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN6(NF_INT, nc_put_vars_int, NF_PUT_VARS_INT, nf_put_vars_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INTVARV)
#elif NF_INT_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_put_vars_long, NF_PUT_VARS_INT, nf_put_vars_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INTVARV)
#endif


/*
 * Read values into a FORTRAN INTEGER variable array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN6(NF_INT, nc_get_vars_int, NF_GET_VARS_INT, nf_get_vars_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINTVARV)
#elif NF_INT_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_get_vars_long, NF_GET_VARS_INT, nf_get_vars_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PINTVARV)
#endif


/*
 * Write values from a FORTRAN REAL variable array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN6(NF_INT, nc_put_vars_double, NF_PUT_VARS_REAL, nf_put_vars_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, DOUBLEVARV)
#else
FCALLSCFUN6(NF_INT, nc_put_vars_float, NF_PUT_VARS_REAL, nf_put_vars_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, REALVARV)
#endif


/*
 * Read values into a FORTRAN REAL variable array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN6(NF_INT, nc_get_vars_double, NF_GET_VARS_REAL, nf_get_vars_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PDOUBLEVARV)
#else
FCALLSCFUN6(NF_INT, nc_get_vars_float, NF_GET_VARS_REAL, nf_get_vars_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PREALVARV)
#endif


/*
 * Write values from a FORTRAN DOUBLEPRECISION variable array.
 */
FCALLSCFUN6(NF_INT, nc_put_vars_double, NF_PUT_VARS_DOUBLE, nf_put_vars_double,
	    NCID, VARID, COORDS, COUNTS, STRIDES, DOUBLEVARV)


/*
 * Read values into a FORTRAN DOUBLEPRECISION variable array.
 */
FCALLSCFUN6(NF_INT, nc_get_vars_double, NF_GET_VARS_DOUBLE, nf_get_vars_double,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PDOUBLEVARV)

FCALLSCFUN6(NF_INT, nc_put_vars, NF_PUT_VARS, nf_put_vars,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PVOID)

FCALLSCFUN6(NF_INT, nc_get_vars, NF_GET_VARS, nf_get_vars,
	    NCID, VARID, COORDS, COUNTS, STRIDES, PVOID)



