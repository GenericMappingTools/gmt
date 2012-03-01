/*********************************************************************
 *   Copyright 2007, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncdump/indent.h,v 1.1 2007/05/20 20:42:30 russ Exp $
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Handle nested group indentation */
extern void indent_init();	/* initialize indent to zero */
extern void indent_out();	/* output current indent */
extern void indent_more();	/* increment current indent */
extern void indent_less();	/* decrement current indent */
extern int  indent_get();	/* return current indent */

#ifdef __cplusplus
}
#endif
