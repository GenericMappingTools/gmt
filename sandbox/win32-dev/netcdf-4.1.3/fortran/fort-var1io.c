/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF var1 functions.

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
 * Write a single value from a FORTRAN CHARACTER*(*) variable (the
 * `single value' is the whole string).
 */
FCALLSCFUN4(NF_INT, nc_put_var1_text, NF_PUT_VAR1_TEXT, nf_put_var1_text,
	    NCID, VARID, COORDS, CBUF)


/*
 * Read a single value into a FORTRAN CHARACTER*(*) variable (the
 * `single value' is the whole string).
 */
FCALLSCFUN4(NF_INT, nc_get_var1_text, NF_GET_VAR1_TEXT, nf_get_var1_text,
	    NCID, VARID, COORDS, CBUF)


/*
 * Write a single value from a FORTRAN INTEGER*1 variable.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN4(NF_INT, nc_put_var1_schar, NF_PUT_VAR1_INT1, nf_put_var1_int1,
	    NCID, VARID, COORDS, INT1VAR)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN4(NF_INT, nc_put_var1_short, NF_PUT_VAR1_INT1, nf_put_var1_int1,
	    NCID, VARID, COORDS, INT1VAR)
#elif NF_INT1_IS_C_INT
FCALLSCFUN4(NF_INT, nc_put_var1_int, NF_PUT_VAR1_INT1, nf_put_var1_int1,
	    NCID, VARID, COORDS, INT1VAR)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_put_var1_long, NF_PUT_VAR1_INT1, nf_put_var1_int1,
	    NCID, VARID, COORDS, INT1VAR)
#endif


/*
 * Read a single value into a FORTRAN INTEGER*1 variable.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN4(NF_INT, nc_get_var1_schar, NF_GET_VAR1_INT1, nf_get_var1_int1,
	    NCID, VARID, COORDS, PINT1VAR)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN4(NF_INT, nc_get_var1_short, NF_GET_VAR1_INT1, nf_get_var1_int1,
	    NCID, VARID, COORDS, PINT1VAR)
#elif NF_INT1_IS_C_INT
FCALLSCFUN4(NF_INT, nc_get_var1_int, NF_GET_VAR1_INT1, nf_get_var1_int1,
	    NCID, VARID, COORDS, PINT1VAR)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_get_var1_long, NF_GET_VAR1_INT1, nf_get_var1_int1,
	    NCID, VARID, COORDS, PINT1VAR)
#endif


/*
 * Write a single value from a FORTRAN INTEGER*2 variable.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN4(NF_INT, nc_put_var1_short, NF_PUT_VAR1_INT2, nf_put_var1_int2,
	    NCID, VARID, COORDS, INT2VAR)
#elif NF_INT2_IS_C_INT
FCALLSCFUN4(NF_INT, nc_put_var1_int, NF_PUT_VAR1_INT2, nf_put_var1_int2,
	    NCID, VARID, COORDS, INT2VAR)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_put_var1_long, NF_PUT_VAR1_INT2, nf_put_var1_int2,
	    NCID, VARID, COORDS, INT2VAR)
#endif


/*
 * Read a single value into a FORTRAN INTEGER*2 variable.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN4(NF_INT, nc_get_var1_short, NF_GET_VAR1_INT2, nf_get_var1_int2,
	    NCID, VARID, COORDS, PINT2VAR)
#elif NF_INT2_IS_C_INT
FCALLSCFUN4(NF_INT, nc_get_var1_int, NF_GET_VAR1_INT2, nf_get_var1_int2,
	    NCID, VARID, COORDS, PINT2VAR)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_get_var1_long, NF_GET_VAR1_INT2, nf_get_var1_int2,
	    NCID, VARID, COORDS, PINT2VAR)
#endif


/*
 * Write a single value from a FORTRAN INTEGER variable.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN4(NF_INT, nc_put_var1_int, NF_PUT_VAR1_INT, nf_put_var1_int,
	    NCID, VARID, COORDS, INTVAR)
#elif NF_INT_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_put_var1_long, NF_PUT_VAR1_INT, nf_put_var1_int,
	    NCID, VARID, COORDS, INTVAR)
#endif


/*
 * Read a single value into a FORTRAN INTEGER variable.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN4(NF_INT, nc_get_var1_int, NF_GET_VAR1_INT, nf_get_var1_int,
	    NCID, VARID, COORDS, PINTVAR)
#elif NF_INT_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_get_var1_long, NF_GET_VAR1_INT, nf_get_var1_int,
	    NCID, VARID, COORDS, PINTVAR)
#endif


/*
 * Write a single value from a FORTRAN REAL variable.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN4(NF_INT, nc_put_var1_double, NF_PUT_VAR1_REAL, nf_put_var1_real,
	    NCID, VARID, COORDS, DOUBLEVAR)
#else
FCALLSCFUN4(NF_INT, nc_put_var1_float, NF_PUT_VAR1_REAL, nf_put_var1_real,
	    NCID, VARID, COORDS, REALVAR)
#endif


/*
 * Read a single value into a FORTRAN REAL variable.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN4(NF_INT, nc_get_var1_double, NF_GET_VAR1_REAL, nf_get_var1_real,
	    NCID, VARID, COORDS, PDOUBLEVAR)
#else
FCALLSCFUN4(NF_INT, nc_get_var1_float, NF_GET_VAR1_REAL, nf_get_var1_real,
	    NCID, VARID, COORDS, PREALVAR)
#endif


/*
 * Write a single value from a FORTRAN DOUBLEPRECISION variable.
 */
FCALLSCFUN4(NF_INT, nc_put_var1_double, NF_PUT_VAR1_DOUBLE, nf_put_var1_double,
	    NCID, VARID, COORDS, DOUBLEVAR)


/*
 * Read a single value into a FORTRAN DOUBLEPRECISION variable.
 */
FCALLSCFUN4(NF_INT, nc_get_var1_double, NF_GET_VAR1_DOUBLE, nf_get_var1_double,
	    NCID, VARID, COORDS, PDOUBLEVAR)

FCALLSCFUN4(NF_INT, nc_put_var1, NF_PUT_VAR1, nf_put_var1,
	    NCID, VARID, COORDS, PVOID)

FCALLSCFUN4(NF_INT, nc_get_var1, NF_GET_VAR1, nf_get_var1,
	    NCID, VARID, COORDS, PVOID)
