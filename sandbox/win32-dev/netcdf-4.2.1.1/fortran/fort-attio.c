#include <config.h>
#include <string.h>
#include <errno.h>

#include "netcdf.h"
#include "netcdf_f.h"
#include "nfconfig.inc"
#include "ncfortran.h"


/*
 * Write an attribute from a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN5(NF_INT, nc_put_att_text, NF_PUT_ATT_TEXT, nf_put_att_text,
	    NCID, VARID, STRING, COUNT, CBUF)


/*
 * Read an attribute into a FORTRAN CHARACTER*(*) variable.
 */
FCALLSCFUN4(NF_INT, nc_get_att_text, NF_GET_ATT_TEXT, nf_get_att_text,
	    NCID, VARID, STRING, CBUF)


/*
 * Write an attribute from an INTEGER*1 array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN6(NF_INT, nc_put_att_schar, NF_PUT_ATT_INT1, nf_put_att_int1,
	    NCID, VARID, STRING, TYPE, COUNT, INT1ATT)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN6(NF_INT, nc_put_att_short, NF_PUT_ATT_INT1, nf_put_att_int1,
	    NCID, VARID, STRING, TYPE, COUNT, INT1ATT)
#elif NF_INT1_IS_C_INT
FCALLSCFUN6(NF_INT, nc_put_att_int, NF_PUT_ATT_INT1, nf_put_att_int1,
	    NCID, VARID, STRING, TYPE, COUNT, INT1ATT)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_put_att_long, NF_PUT_ATT_INT1, nf_put_att_int1,
	    NCID, VARID, STRING, TYPE, COUNT, INT1ATT)
#endif


/*
 * Read an attribute into an INTEGER*1 array.
 */
#if NF_INT1_IS_C_SIGNED_CHAR
FCALLSCFUN4(NF_INT, nc_get_att_schar, NF_GET_ATT_INT1, nf_get_att_int1,
	    NCID, VARID, STRING, PINT1ATT)
#elif NF_INT1_IS_C_SHORT
FCALLSCFUN4(NF_INT, nc_get_att_short, NF_GET_ATT_INT1, nf_get_att_int1,
	    NCID, VARID, STRING, PINT1ATT)
#elif NF_INT1_IS_C_INT
FCALLSCFUN4(NF_INT, nc_get_att_int, NF_GET_ATT_INT1, nf_get_att_int1,
	    NCID, VARID, STRING, PINT1ATT)
#elif NF_INT1_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_get_att_long, NF_GET_ATT_INT1, nf_get_att_int1,
	    NCID, VARID, STRING, PINT1ATT)
#endif


/*
 * Write an attribute from an INTEGER*2 array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN6(NF_INT, nc_put_att_short, NF_PUT_ATT_INT2, nf_put_att_int2,
	    NCID, VARID, STRING, TYPE, COUNT, INT2ATT)
#elif NF_INT2_IS_C_INT
FCALLSCFUN6(NF_INT, nc_put_att_int, NF_PUT_ATT_INT2, nf_put_att_int2,
	    NCID, VARID, STRING, TYPE, COUNT, INT2ATT)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_put_att_long, NF_PUT_ATT_INT2, nf_put_att_int2,
	    NCID, VARID, STRING, TYPE, COUNT, INT2ATT)
#endif


/*
 * Read an attribute into an INTEGER*2 array.
 */
#if NF_INT2_IS_C_SHORT
FCALLSCFUN4(NF_INT, nc_get_att_short, NF_GET_ATT_INT2, nf_get_att_int2,
	    NCID, VARID, STRING, PINT2ATT)
#elif NF_INT2_IS_C_INT
FCALLSCFUN4(NF_INT, nc_get_att_int, NF_GET_ATT_INT2, nf_get_att_int2,
	    NCID, VARID, STRING, PINT2ATT)
#elif NF_INT2_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_get_att_long, NF_GET_ATT_INT2, nf_get_att_int2,
	    NCID, VARID, STRING, PINT2ATT)
#endif


/*
 * Write an attribute from an INTEGER array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN6(NF_INT, nc_put_att_int, NF_PUT_ATT_INT, nf_put_att_int,
	    NCID, VARID, STRING, TYPE, COUNT, INTATT)
#elif NF_INT_IS_C_LONG
FCALLSCFUN6(NF_INT, nc_put_att_long, NF_PUT_ATT_INT, nf_put_att_int,
	    NCID, VARID, STRING, TYPE, COUNT, INTATT)
#endif


/*
 * Read an attribute into an INTEGER array.
 */
#if NF_INT_IS_C_INT
FCALLSCFUN4(NF_INT, nc_get_att_int, NF_GET_ATT_INT, nf_get_att_int,
	    NCID, VARID, STRING, PINTATT)
#elif NF_INT_IS_C_LONG
FCALLSCFUN4(NF_INT, nc_get_att_long, NF_GET_ATT_INT, nf_get_att_int,
	    NCID, VARID, STRING, PINTATT)
#endif


/*
 * Write an attribute from a REAL array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN6(NF_INT, nc_put_att_double, NF_PUT_ATT_REAL, nf_put_att_real,
	    NCID, VARID, STRING, TYPE, COUNT, DOUBLEATT)
#else
FCALLSCFUN6(NF_INT, nc_put_att_float, NF_PUT_ATT_REAL, nf_put_att_real,
	    NCID, VARID, STRING, TYPE, COUNT, REALATT)
#endif


/*
 * Read an attribute into a REAL array.
 */
#if NF_REAL_IS_C_DOUBLE
FCALLSCFUN4(NF_INT, nc_get_att_double, NF_GET_ATT_REAL, nf_get_att_real,
	    NCID, VARID, STRING, PDOUBLEATT)
#else
FCALLSCFUN4(NF_INT, nc_get_att_float, NF_GET_ATT_REAL, nf_get_att_real,
	    NCID, VARID, STRING, PREALATT)
#endif


/*
 * Write an attribute from a DOUBLEPRECISION array.
 */
FCALLSCFUN6(NF_INT, nc_put_att_double, NF_PUT_ATT_DOUBLE, nf_put_att_double,
	    NCID, VARID, STRING, TYPE, COUNT, DOUBLEATT)


/*
 * Read an attribute into a DOUBLEPRECISION array.
 */
FCALLSCFUN4(NF_INT, nc_get_att_double, NF_GET_ATT_DOUBLE, nf_get_att_double,
	    NCID, VARID, STRING, PDOUBLEATT)
