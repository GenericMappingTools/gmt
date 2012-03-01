/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF vara functions.

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
FCALLSCFUN5(NF_INT, nc_put_vara_text, NF_PUT_VARA_TEXT, nf_put_vara_text,
	    NCID, VARID, COORDS, COUNTS, CBUF)


/*
 * Read values into a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN5(NF_INT, nc_get_vara_text, NF_GET_VARA_TEXT, nf_get_vara_text,
	    NCID, VARID, COORDS, COUNTS, CBUF)


/*
 * Write values from a FORTRAN INTEGER*1 variable array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN5(NF_INT, nc_put_vara_schar, NF_PUT_VARA_INT1, nf_put_vara_int1,
	    NCID, VARID, COORDS, COUNTS, INT1VARV)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN5(NF_INT, nc_put_vara_short, NF_PUT_VARA_INT1, nf_put_vara_int1,
	    NCID, VARID, COORDS, COUNTS, INT1VARV)
#elif NF_INT1_IS_C_INT
FCALLSCFUN5(NF_INT, nc_put_vara_int, NF_PUT_VARA_INT1, nf_put_vara_int1,
	    NCID, VARID, COORDS, COUNTS, INT1VARV)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN5(NF_INT, nc_put_vara_long, NF_PUT_VARA_INT1, nf_put_vara_int1,
	    NCID, VARID, COORDS, COUNTS, INT1VARV)
#endif


/*
 * Read values into a FORTRAN INTEGER*1 variable array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN5(NF_INT, nc_get_vara_schar, NF_GET_VARA_INT1, nf_get_vara_int1,
	    NCID, VARID, COORDS, COUNTS, PINT1VARV)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN5(NF_INT, nc_get_vara_short, NF_GET_VARA_INT1, nf_get_vara_int1,
	    NCID, VARID, COORDS, COUNTS, PINT1VARV)
#elif NF_INT1_IS_C_INT
FCALLSCFUN5(NF_INT, nc_get_vara_int, NF_GET_VARA_INT1, nf_get_vara_int1,
	    NCID, VARID, COORDS, COUNTS, PINT1VARV)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN5(NF_INT, nc_get_vara_long, NF_GET_VARA_INT1, nf_get_vara_int1,
	    NCID, VARID, COORDS, COUNTS, PINT1VARV)
#endif


/*
 * Write values from a FORTRAN INTEGER*2 variable array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN5(NF_INT, nc_put_vara_short, NF_PUT_VARA_INT2, nf_put_vara_int2,
	    NCID, VARID, COORDS, COUNTS, INT2VARV)
#elif NF_INT2_IS_C_INT
FCALLSCFUN5(NF_INT, nc_put_vara_int, NF_PUT_VARA_INT2, nf_put_vara_int2,
	    NCID, VARID, COORDS, COUNTS, INT2VARV)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN5(NF_INT, nc_put_vara_long, NF_PUT_VARA_INT2, nf_put_vara_int2,
	    NCID, VARID, COORDS, COUNTS, INT2VARV)
#endif


/*
 * Read values into a FORTRAN INTEGER*2 variable array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN5(NF_INT, nc_get_vara_short, NF_GET_VARA_INT2, nf_get_vara_int2,
	    NCID, VARID, COORDS, COUNTS, PINT2VARV)
#elif NF_INT2_IS_C_INT
FCALLSCFUN5(NF_INT, nc_get_vara_int, NF_GET_VARA_INT2, nf_get_vara_int2,
	    NCID, VARID, COORDS, COUNTS, PINT2VARV)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN5(NF_INT, nc_get_vara_long, NF_GET_VARA_INT2, nf_get_vara_int2,
	    NCID, VARID, COORDS, COUNTS, PINT2VARV)
#endif


/*
 * Write values from a FORTRAN INTEGER variable array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN5(NF_INT, nc_put_vara_int, NF_PUT_VARA_INT, nf_put_vara_int,
	    NCID, VARID, COORDS, COUNTS, INTVARV)
#elif NF_INT_IS_C_LONG
FCALLSCFUN5(NF_INT, nc_put_vara_long, NF_PUT_VARA_INT, nf_put_vara_int,
	    NCID, VARID, COORDS, COUNTS, INTVARV)
#endif


/*
 * Read values into a FORTRAN INTEGER variable array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN5(NF_INT, nc_get_vara_int, NF_GET_VARA_INT, nf_get_vara_int,
	    NCID, VARID, COORDS, COUNTS, PINTVARV)
#elif NF_INT_IS_C_LONG
FCALLSCFUN5(NF_INT, nc_get_vara_long, NF_GET_VARA_INT, nf_get_vara_int,
	    NCID, VARID, COORDS, COUNTS, PINTVARV)
#endif


/*
 * Write values from a FORTRAN REAL variable array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN5(NF_INT, nc_put_vara_double, NF_PUT_VARA_REAL, nf_put_vara_real,
	    NCID, VARID, COORDS, COUNTS, DOUBLEVARV)
#else
FCALLSCFUN5(NF_INT, nc_put_vara_float, NF_PUT_VARA_REAL, nf_put_vara_real,
	    NCID, VARID, COORDS, COUNTS, REALVARV)
#endif


/*
 * Read values into a FORTRAN REAL variable array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN5(NF_INT, nc_get_vara_double, NF_GET_VARA_REAL, nf_get_vara_real,
	    NCID, VARID, COORDS, COUNTS, PDOUBLEVARV)
#else
FCALLSCFUN5(NF_INT, nc_get_vara_float, NF_GET_VARA_REAL, nf_get_vara_real,
	    NCID, VARID, COORDS, COUNTS, PREALVARV)
#endif


/*
 * Write values from a FORTRAN DOUBLEPRECISION variable array.
 */
FCALLSCFUN5(NF_INT, nc_put_vara_double, NF_PUT_VARA_DOUBLE, nf_put_vara_double,
	    NCID, VARID, COORDS, COUNTS, DOUBLEVARV)


/*
 * Read values into a FORTRAN DOUBLEPRECISION variable array.
 */
FCALLSCFUN5(NF_INT, nc_get_vara_double, NF_GET_VARA_DOUBLE, nf_get_vara_double,
	    NCID, VARID, COORDS, COUNTS, PDOUBLEVARV)

FCALLSCFUN5(NF_INT, nc_put_vara, NF_PUT_VARA, nf_put_vara,
	    NCID, VARID, COORDS, COUNTS, PVOID)

FCALLSCFUN5(NF_INT, nc_get_vara, NF_GET_VARA, nf_get_vara,
	    NCID, VARID, COORDS, COUNTS, PVOID)
