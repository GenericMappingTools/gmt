/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id $
 *********************************************************************/

/* initialize a dimmap for n dimension ids */
extern int
dimmap_init(size_t n);

/* store association between an input dimid and an output dimid, and whether unlimited */
extern int
dimmap_store(int idimid, int odimid, int iunlim, int ounlim);

/* return odimid associated with specified idimid */
extern int
dimmap_odimid(int idimid);

/* return idimid associated with specified odimid */
extern int
dimmap_idimid(int odimid);

/* return whether odimid dimension is unlimited */
extern int
dimmap_ounlim(int odimid);

/* return whether idimid dimension is unlimited */
extern int
dimmap_iunlim(int idimid);

