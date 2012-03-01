/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen3/getfill.c,v 1.5 2009/11/24 22:09:09 dmh Exp $
 *********************************************************************/

#include "netcdf.h"
#include "generic.h"
#include "ncgen.h"
#include "genlib.h"


/*
 * Given netCDF type, return a default fill_value appropriate for
 * that type.
 */
void
nc_getfill(
     nc_type type,
     union generic *gval)
{
    switch(type) {
      case NC_CHAR:
	gval->charv = NC_FILL_CHAR;
	return;
      case NC_BYTE:
	gval->charv = NC_FILL_BYTE;
	return;
      case NC_SHORT:
	gval->shortv = NC_FILL_SHORT;
	return;
      case NC_INT:
	gval->intv = NC_FILL_INT;
	return;
      case NC_FLOAT:
	gval->floatv = NC_FILL_FLOAT;
	return;
      case NC_DOUBLE:
	gval->doublev = NC_FILL_DOUBLE;
	return;
      default:
	derror("nc_getfill: unrecognized type");
    }
}


void
nc_fill(
     nc_type type,		/* netcdf type code  */
     size_t num,		/* number of values to fill */
     void *datp,		/* where to start filling */
     union generic fill_val)	/* value to use */
{
    char *char_valp;		/* pointers used to accumulate data values */
    short *short_valp;
    int *long_valp;
    float *float_valp;
    double *double_valp;

    switch (type) {
      case NC_CHAR:
      case NC_BYTE:
	char_valp = (char *) datp;
	break;
      case NC_SHORT:
	short_valp = (short *) datp;
	break;
      case NC_INT:
	long_valp = (int *) datp;
	break;
      case NC_FLOAT:
	float_valp = (float *) datp;
	break;
      case NC_DOUBLE:
	double_valp = (double *) datp;
	break;
      default: break;
    }
    while (num--) {
	switch (type) {
	  case NC_CHAR:
	  case NC_BYTE:
	    *char_valp++ = fill_val.charv;
	    break;
	  case NC_SHORT:
	    *short_valp++ = fill_val.shortv;
	    break;
	  case NC_INT:
	    *long_valp++ = fill_val.intv;
	    break;
	  case NC_FLOAT:
	    *float_valp++ = fill_val.floatv;
	    break;
	  case NC_DOUBLE:
	    *double_valp++ = fill_val.doublev;
	    break;
	  default: break;
	}
    }
}


/*
 * Given netCDF type, put a value of that type into a fill_value
 */
void
nc_putfill(
     nc_type type,
     void *val,			/* value of type to be put */
     union generic *gval)	/* where the value is to be put */
{
    switch(type) {
      case NC_CHAR:
      case NC_BYTE:
	gval->charv = *(char *)val;
	return;
      case NC_SHORT:
	gval->shortv = *(short *)val;
	return;
      case NC_INT:
	gval->intv = *(int *)val;
	return;
      case NC_FLOAT:
	gval->floatv = *(float *)val;
	return;
      case NC_DOUBLE:
	gval->doublev = *(double *)val;
	return;
      default:
	derror("nc_putfill: unrecognized type");
    }
}
