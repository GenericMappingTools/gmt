/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF var functions.

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
 * Write an entire variable from a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN3(NF_INT, nc_put_var_text, NF_PUT_VAR_TEXT, nf_put_var_text,
	    NCID, VARID, CBUF)


/*
 * Read an entire variable into a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN3(NF_INT, nc_get_var_text, NF_GET_VAR_TEXT, nf_get_var_text,
	    NCID, VARID, CBUF)


/*
 * Write an entire variable from a FORTRAN INTEGER*1 variable.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN3(NF_INT, nc_put_var_schar, NF_PUT_VAR_INT1, nf_put_var_int1,
	    NCID, VARID, INT1VAR)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN3(NF_INT, nc_put_var_short, NF_PUT_VAR_INT1, nf_put_var_int1,
	    NCID, VARID, INT1VAR)
#elif NF_INT1_IS_C_INT
FCALLSCFUN3(NF_INT, nc_put_var_int, NF_PUT_VAR_INT1, nf_put_var_int1,
	    NCID, VARID, INT1VAR)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN3(NF_INT, nc_put_var_long, NF_PUT_VAR_INT1, nf_put_var_int1,
	    NCID, VARID, INT1VAR)
#endif


/*
 * Read an entire variable into a FORTRAN INTEGER*1 variable.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN3(NF_INT, nc_get_var_schar, NF_GET_VAR_INT1, nf_get_var_int1,
	    NCID, VARID, PINT1VAR)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN3(NF_INT, nc_get_var_short, NF_GET_VAR_INT1, nf_get_var_int1,
	    NCID, VARID, PINT1VAR)
#elif NF_INT1_IS_C_INT
FCALLSCFUN3(NF_INT, nc_get_var_int, NF_GET_VAR_INT1, nf_get_var_int1,
	    NCID, VARID, PINT1VAR)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN3(NF_INT, nc_get_var_long, NF_GET_VAR_INT1, nf_get_var_int1,
	    NCID, VARID, PINT1VAR)
#endif


/*
 * Write an entire variable from a FORTRAN INTEGER*2 variable.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN3(NF_INT, nc_put_var_short, NF_PUT_VAR_INT2, nf_put_var_int2,
	    NCID, VARID, INT2VAR)
#elif NF_INT2_IS_C_INT
FCALLSCFUN3(NF_INT, nc_put_var_int, NF_PUT_VAR_INT2, nf_put_var_int2,
	    NCID, VARID, INT2VAR)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN3(NF_INT, nc_put_var_long, NF_PUT_VAR_INT2, nf_put_var_int2,
	    NCID, VARID, INT2VAR)
#endif


/*
 * Read an entire variable into a FORTRAN INTEGER*2 variable.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN3(NF_INT, nc_get_var_short, NF_GET_VAR_INT2, nf_get_var_int2,
	    NCID, VARID, PINT2VAR)
#elif NF_INT2_IS_C_INT
FCALLSCFUN3(NF_INT, nc_get_var_int, NF_GET_VAR_INT2, nf_get_var_int2,
	    NCID, VARID, PINT2VAR)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN3(NF_INT, nc_get_var_long, NF_GET_VAR_INT2, nf_get_var_int2,
	    NCID, VARID, PINT2VAR)
#endif


/*
 * Write an entire variable from a FORTRAN INTEGER variable.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN3(NF_INT, nc_put_var_int, NF_PUT_VAR_INT, nf_put_var_int,
	    NCID, VARID, INTVAR)
#elif NF_INT_IS_C_LONG
FCALLSCFUN3(NF_INT, nc_put_var_long, NF_PUT_VAR_INT, nf_put_var_int,
	    NCID, VARID, INTVAR)
#endif


/*
 * Read an entire variable into a FORTRAN INTEGER variable.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN3(NF_INT, nc_get_var_int, NF_GET_VAR_INT, nf_get_var_int,
	    NCID, VARID, PINTVAR)
#elif NF_INT_IS_C_LONG
FCALLSCFUN3(NF_INT, nc_get_var_long, NF_GET_VAR_INT, nf_get_var_int,
	    NCID, VARID, PINTVAR)
#endif


/*
 * Write an entire variable from a FORTRAN REAL variable.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN3(NF_INT, nc_put_var_double, NF_PUT_VAR_REAL, nf_put_var_real,
	    NCID, VARID, DOUBLEVAR)
#else
FCALLSCFUN3(NF_INT, nc_put_var_float, NF_PUT_VAR_REAL, nf_put_var_real,
	    NCID, VARID, REALVAR)
#endif


/*
 * Read an entire variable into a FORTRAN REAL variable.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN3(NF_INT, nc_get_var_double, NF_GET_VAR_REAL, nf_get_var_real,
	    NCID, VARID, PDOUBLEVAR)
#else
FCALLSCFUN3(NF_INT, nc_get_var_float, NF_GET_VAR_REAL, nf_get_var_real,
	    NCID, VARID, PREALVAR)
#endif


/*
 * Write an entire variable from a FORTRAN DOUBLEPRECISION variable.
 */
FCALLSCFUN3(NF_INT, nc_put_var_double, NF_PUT_VAR_DOUBLE, nf_put_var_double,
	    NCID, VARID, DOUBLEVAR)


/*
 * Read an entire variable into a FORTRAN DOUBLEPRECISION variable.
 */
FCALLSCFUN3(NF_INT, nc_get_var_double, NF_GET_VAR_DOUBLE, nf_get_var_double,
	    NCID, VARID, PDOUBLEVAR)

/* Apparently only defined for netcdf-4 */
#ifdef USE_NETCDF4

/* Handle any type. */
FCALLSCFUN3(NF_INT, nc_put_var, NF_PUT_VAR, nf_put_var, NCID, VARID, PVOID)

/* Handle any type. */
FCALLSCFUN3(NF_INT, nc_get_var, NF_GET_VAR, nf_get_var, NCID, VARID, PVOID)
#endif
