/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF varm functions.

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
FCALLSCFUN7(NF_INT, nc_put_varm_text, NF_PUT_VARM_TEXT, nf_put_varm_text,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, CBUF)


/*
 * Read values into a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN7(NF_INT, nc_get_varm_text, NF_GET_VARM_TEXT, nf_get_varm_text,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, CBUF)


/*
 * Write values from a FORTRAN INTEGER*1 variable array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN7(NF_INT, nc_put_varm_schar, NF_PUT_VARM_INT1, nf_put_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INT1VARV)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN7(NF_INT, nc_put_varm_short, NF_PUT_VARM_INT1, nf_put_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INT1VARV)
#elif NF_INT1_IS_C_INT
FCALLSCFUN7(NF_INT, nc_put_varm_int, NF_PUT_VARM_INT1, nf_put_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INT1VARV)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN7(NF_INT, nc_put_varm_long, NF_PUT_VARM_INT1, nf_put_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INT1VARV)
#endif


/*
 * Read values into a FORTRAN INTEGER*1 variable array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN7(NF_INT, nc_get_varm_schar, NF_GET_VARM_INT1, nf_get_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINT1VARV)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN7(NF_INT, nc_get_varm_short, NF_GET_VARM_INT1, nf_get_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINT1VARV)
#elif NF_INT1_IS_C_INT
FCALLSCFUN7(NF_INT, nc_get_varm_int, NF_GET_VARM_INT1, nf_get_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINT1VARV)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN7(NF_INT, nc_get_varm_long, NF_GET_VARM_INT1, nf_get_varm_int1,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINT1VARV)
#endif


/*
 * Write values from a FORTRAN INTEGER*2 variable array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN7(NF_INT, nc_put_varm_short, NF_PUT_VARM_INT2, nf_put_varm_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INT2VARV)
#elif NF_INT2_IS_C_INT
FCALLSCFUN7(NF_INT, nc_put_varm_int, NF_PUT_VARM_INT2, nf_put_varm_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INT2VARV)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN7(NF_INT, nc_put_varm_long, NF_PUT_VARM_INT2, nf_put_varm_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INT2VARV)
#endif


/*
 * Read values into a FORTRAN INTEGER*2 variable array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN7(NF_INT, nc_get_varm_short, NF_GET_VARM_INT2, nf_get_varm_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINT2VARV)
#elif NF_INT2_IS_C_INT
FCALLSCFUN7(NF_INT, nc_get_varm_int, NF_GET_VARM_INT2, nf_get_varm_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINT2VARV)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN7(NF_INT, nc_get_varm_long, NF_GET_VARM_INT2, nf_get_varm_int2,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINT2VARV)
#endif


/*
 * Write values from a FORTRAN INTEGER variable array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN7(NF_INT, nc_put_varm_int, NF_PUT_VARM_INT, nf_put_varm_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INTVARV)
#elif NF_INT_IS_C_LONG
FCALLSCFUN7(NF_INT, nc_put_varm_long, NF_PUT_VARM_INT, nf_put_varm_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, INTVARV)
#endif


/*
 * Read values into a FORTRAN INTEGER variable array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN7(NF_INT, nc_get_varm_int, NF_GET_VARM_INT, nf_get_varm_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINTVARV)
#elif NF_INT_IS_C_LONG
FCALLSCFUN7(NF_INT, nc_get_varm_long, NF_GET_VARM_INT, nf_get_varm_int,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINTVARV)
#endif


/*
 * Write values from a FORTRAN REAL variable array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN7(NF_INT, nc_put_varm_double, NF_PUT_VARM_REAL, nf_put_varm_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, DOUBLEVARV)
#else
FCALLSCFUN7(NF_INT, nc_put_varm_float, NF_PUT_VARM_REAL, nf_put_varm_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, REALVARV)
#endif


/*
 * Read values into a FORTRAN REAL variable array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN7(NF_INT, nc_get_varm_double, NF_GET_VARM_REAL, nf_get_varm_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PDOUBLEVARV)
#else
FCALLSCFUN7(NF_INT, nc_get_varm_float, NF_GET_VARM_REAL, nf_get_varm_real,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PREALVARV)
#endif


/*
 * Write values from a FORTRAN DOUBLEPRECISION variable array.
 */
FCALLSCFUN7(NF_INT, nc_put_varm_double, NF_PUT_VARM_DOUBLE, nf_put_varm_double,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, DOUBLEVARV)


/*
 * Read values into a FORTRAN DOUBLEPRECISION variable array.
 */
FCALLSCFUN7(NF_INT, nc_get_varm_double, NF_GET_VARM_DOUBLE, nf_get_varm_double,
	    NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PDOUBLEVARV)
