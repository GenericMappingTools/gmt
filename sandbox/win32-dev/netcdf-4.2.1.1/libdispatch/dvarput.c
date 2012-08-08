/*! \file
Functions for writing data to variables.

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/

#include "ncdispatch.h"

/** \internal
\ingroup variables
*/
static int
NC_put_vara(int ncid, int varid, const size_t *start, 
	    const size_t *edges, const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   if(edges == NULL) {
      size_t shape[NC_MAX_VAR_DIMS];
      int ndims;
      stat = nc_inq_varndims(ncid, varid, &ndims); 
      if(stat != NC_NOERR) return stat;
      stat = NC_getshape(ncid, varid, ndims, shape);
      if(stat != NC_NOERR) return stat;
      return ncp->dispatch->put_vara(ncid, varid, start, shape, value, memtype);
   } else
      return ncp->dispatch->put_vara(ncid, varid, start, edges, value, memtype);
}

/** \internal
\ingroup variables
*/
static int
NC_put_var(int ncid, int varid, const void *value, nc_type memtype)
{
   int ndims;
   size_t shape[NC_MAX_VAR_DIMS];
   int stat = nc_inq_varndims(ncid,varid, &ndims);
   if(stat) return stat;
   stat = NC_getshape(ncid,varid, ndims, shape);
   if(stat) return stat;
   return NC_put_vara(ncid, varid, NC_coord_zero, shape, value, memtype);
}

/** \internal
\ingroup variables
*/
static int
NC_put_var1(int ncid, int varid, const size_t *coord, const void* value, 
	    nc_type memtype)
{
   return NC_put_vara(ncid, varid, coord, NC_coord_one, value, memtype);
}

/** \internal
\ingroup variables
*/
int
NCDEFAULT_put_vars(int ncid, int varid, const size_t * start,
	    const size_t * edges, const ptrdiff_t * stride,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->put_varm(ncid,varid,start,edges,stride,NULL,value,memtype);
}

/** \internal
\ingroup variables
*/
int
NCDEFAULT_put_varm(
   int ncid,
   int varid,
   const size_t * start,
   const size_t * edges,
   const ptrdiff_t * stride,
   const ptrdiff_t * imapp,
   const void *value0,
   nc_type memtype)
{
   int status = NC_NOERR;
   nc_type vartype = NC_NAT;
   int varndims = 0;
   int maxidim = 0;
   NC* ncp;
   size_t memtypelen;
   ptrdiff_t cvtmap[NC_MAX_VAR_DIMS];
   const char* value = (char*)value0;

   status = NC_check_id (ncid, &ncp);
   if(status != NC_NOERR) return status;

/*
  if(NC_indef(ncp)) return NC_EINDEFINE;
  if(NC_readonly (ncp)) return NC_EPERM;
*/

   /* mid body */
   status = nc_inq_vartype(ncid, varid, &vartype); 
   if(status != NC_NOERR) return status;
   /* Check that this is an atomic type */
   if(vartype >= NC_MAX_ATOMIC_TYPE)
	return NC_EMAPTYPE;

   status = nc_inq_varndims(ncid, varid, &varndims); 
   if(status != NC_NOERR) return status;

   if(memtype == NC_NAT) {
      if(imapp != NULL && varndims != 0) {
	 /*
	  * convert map units from bytes to units of sizeof(type)
	  */
	 size_t ii;
	 const ptrdiff_t szof = (ptrdiff_t) nctypelen(vartype);
	 for(ii = 0; ii < varndims; ii++) {
	    if(imapp[ii] % szof != 0) {
	       /*free(cvtmap);*/
	       return NC_EINVAL;
	    }
	    cvtmap[ii] = imapp[ii] / szof;
	 }
	 imapp = cvtmap;
      }
      memtype = vartype;
   }

   if(memtype == NC_CHAR && vartype != NC_CHAR)
      return NC_ECHAR;
   else if(memtype != NC_CHAR && vartype == NC_CHAR)  
      return NC_ECHAR;

   memtypelen = nctypelen(memtype);

   maxidim = (int) varndims - 1;

   if (maxidim < 0)
   {
      /*
       * The variable is a scalar; consequently,
       * there s only one thing to get and only one place to put it.
       * (Why was I called?)
       */
      size_t edge1[1] = {1};
      return NC_put_vara(ncid, varid, start, edge1, value, memtype);
   }

   /*
    * else
    * The variable is an array.
    */
   {
      int idim;
      size_t *mystart = NULL;
      size_t *myedges;
      size_t *iocount;    /* count vector */
      size_t *stop;   /* stop indexes */
      size_t *length; /* edge lengths in bytes */
      ptrdiff_t *mystride;
      ptrdiff_t *mymap;
      size_t varshape[NC_MAX_VAR_DIMS];
      int isrecvar;
      size_t numrecs;
      int stride1; /* is stride all ones? */

      /*
       * Verify stride argument.
       */
      stride1 = 1;		/*  assume ok; */
      if(stride != NULL) {
	 for (idim = 0; idim <= maxidim; ++idim) {
            if ((stride[idim] == 0)
		/* cast needed for braindead systems with signed size_t */
                || ((unsigned long) stride[idim] >= X_INT_MAX))
	    {
	       return NC_ESTRIDE;
            }
	    if(stride[idim] != 1) stride1 = 0;
	 }
      }

      /* If stride1 is true, and there is no imap, then call get_vara
         directly
      */
      if(stride1 && imapp == NULL) {
	 return NC_put_vara(ncid, varid, start, edges, value, memtype);
      }

      /* Compute some dimension related values */
      isrecvar = NC_is_recvar(ncid,varid,&numrecs);
      NC_getshape(ncid,varid,varndims,varshape);	

      /* assert(sizeof(ptrdiff_t) >= sizeof(size_t)); */
      mystart = (size_t *)calloc(varndims * 7, sizeof(ptrdiff_t));
      if(mystart == NULL) return NC_ENOMEM;
      myedges = mystart + varndims;
      iocount = myedges + varndims;
      stop = iocount + varndims;
      length = stop + varndims;
      mystride = (ptrdiff_t *)(length + varndims);
      mymap = mystride + varndims;

      /*
       * Initialize I/O parameters.
       */
      for (idim = maxidim; idim >= 0; --idim)
      {
	 mystart[idim] = start != NULL
	    ? start[idim]
	    : 0;

	 if (edges != NULL && edges[idim] == 0)
	 {
	    status = NC_NOERR;    /* read/write no data */
	    goto done;
	 }

	 myedges[idim] = edges != NULL
	    ? edges[idim]
	    : idim == 0 && isrecvar
	    ? numrecs - mystart[idim]
	    : varshape[idim] - mystart[idim];
	 mystride[idim] = stride != NULL
	    ? stride[idim]
	    : 1;
	 mymap[idim] = imapp != NULL
	    ? imapp[idim]
	    : idim == maxidim
	    ? 1
	    : mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1];

	 iocount[idim] = 1;
	 length[idim] = mymap[idim] * myedges[idim];
	 stop[idim] = mystart[idim] + myedges[idim] * mystride[idim];
      }

      /*
       * Check start, edges
       */
      for (idim = isrecvar; idim < maxidim; ++idim)
      {
	 if (mystart[idim] > varshape[idim])
	 {
	    status = NC_EINVALCOORDS;
	    goto done;
	 }
	 if (mystart[idim] + myedges[idim] > varshape[idim])
	 {
	    status = NC_EEDGE;
	    goto done;
	 }
      }

      /* Lower body */
      /*
       * As an optimization, adjust I/O parameters when the fastest 
       * dimension has unity stride both externally and internally.
       * In this case, the user could have called a simpler routine
       * (i.e. ncvar$1()
       */
      if (mystride[maxidim] == 1
	  && mymap[maxidim] == 1)
      {
	 iocount[maxidim] = myedges[maxidim];
	 mystride[maxidim] = (ptrdiff_t) myedges[maxidim];
	 mymap[maxidim] = (ptrdiff_t) length[maxidim];
      }

      /*
       * Perform I/O.  Exit when done.
       */
      for (;;)
      {
	 /* TODO: */
	 int lstatus = NC_put_vara(ncid, varid, mystart, iocount,
				   value, memtype);
	 if (lstatus != NC_NOERR) {
	    if(status == NC_NOERR || lstatus != NC_ERANGE)
	       status = lstatus;
	 }	    

	 /*
	  * The following code permutes through the variable s
	  * external start-index space and it s internal address
	  * space.  At the UPC, this algorithm is commonly
	  * called "odometer code".
	  */
	 idim = maxidim;
        carry:
	 value += (mymap[idim] * memtypelen);
	 mystart[idim] += mystride[idim];
	 if (mystart[idim] == stop[idim])
	 {
	    mystart[idim] = start[idim];
	    value -= (length[idim] * memtypelen);
	    if (--idim < 0)
	       break; /* normal return */
	    goto carry;
	 }
      } /* I/O loop */
     done:
      free(mystart);
   } /* variable is array */
   return status;
}

/** \internal
\ingroup variables
*/
static int
NC_put_vars(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->put_vars(ncid,varid,start,edges,stride,value,memtype);
}

/** \internal
\ingroup variables
*/
static int
NC_put_varm(int ncid, int varid, const size_t *start, 
	    const size_t *edges, const ptrdiff_t *stride, const ptrdiff_t* map,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->put_varm(ncid,varid,start,edges,stride,map,value,memtype);
}

/** \name Writing Data to Variables

Functions to write data from variables. */
/*! \{ */ /* All these functions are part of this named group... */

/** \ingroup variables
Write an array of values to a variable. 

The values to be written are associated with the netCDF variable by
assuming that the last dimension of the netCDF variable varies fastest
in the C interface. The netCDF dataset must be in data mode. The array
to be written is specified by giving a corner and a vector of edge
lengths to \ref specify_hyperslab.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc_put_var() function will write a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param op Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_vara(int ncid, int varid, const size_t *startp, 
	    const size_t *countp, const void *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   nc_type xtype;
   if(stat != NC_NOERR) return stat;
   stat = nc_inq_vartype(ncid, varid, &xtype);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, op, xtype);
}

int
nc_put_vara_text(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const char *op)
{
   return NC_put_vara(ncid, varid, startp, countp, 
		      (void*)op, NC_CHAR);
}

int
nc_put_vara_schar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const signed char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, 
		      NC_BYTE);
}

int
nc_put_vara_uchar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const unsigned char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, 
		      T_uchar);
}

int
nc_put_vara_short(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const short *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, 
		      NC_SHORT);
}

int
nc_put_vara_int(int ncid, int varid, const size_t *startp, 
		const size_t *countp, const int *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, 
		      NC_INT);
}

int
nc_put_vara_long(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      T_long);
}

int
nc_put_vara_float(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const float *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      T_float);
}

int
nc_put_vara_double(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const double *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      T_double);
}

int
nc_put_vara_ubyte(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const unsigned char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      T_ubyte);
}

int
nc_put_vara_ushort(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const unsigned short *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      T_ushort);
}

int
nc_put_vara_uint(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const unsigned int *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      T_uint);
}

int
nc_put_vara_longlong(int ncid, int varid, const size_t *startp, 
		     const size_t *countp, const long long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      T_longlong);
}

int
nc_put_vara_ulonglong(int ncid, int varid, const size_t *startp, 
		      const size_t *countp, const unsigned long long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_vara_string(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const char* *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, (void *)op,
		      NC_STRING);
}

#endif /*USE_NETCDF4*/
/**@}*/

/** \ingroup variables
Write one datum. 

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param indexp Index vector with one element for each dimension.

\param op Pointer from where the data will be copied.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_var1(int ncid, int varid, const size_t *indexp, const void *op)
{
   return NC_put_var1(ncid, varid, indexp, op, NC_NAT);
}

int
nc_put_var1_text(int ncid, int varid, const size_t *indexp, const char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_CHAR);
}

int
nc_put_var1_schar(int ncid, int varid, const size_t *indexp, const signed char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_BYTE);
}

int
nc_put_var1_uchar(int ncid, int varid, const size_t *indexp, const unsigned char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UBYTE);
}

int
nc_put_var1_short(int ncid, int varid, const size_t *indexp, const short *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_SHORT);
}

int
nc_put_var1_int(int ncid, int varid, const size_t *indexp, const int *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_INT);
}

int
nc_put_var1_long(int ncid, int varid, const size_t *indexp, const long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void*)op, longtype);
}

int
nc_put_var1_float(int ncid, int varid, const size_t *indexp, const float *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void*)op, NC_FLOAT);
}

int
nc_put_var1_double(int ncid, int varid, const size_t *indexp, const double *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_DOUBLE);
}

int
nc_put_var1_ubyte(int ncid, int varid, const size_t *indexp, const unsigned char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UBYTE);
}

int
nc_put_var1_ushort(int ncid, int varid, const size_t *indexp, const unsigned short *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_USHORT);
}

int
nc_put_var1_uint(int ncid, int varid, const size_t *indexp, const unsigned int *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UINT);
}

int
nc_put_var1_longlong(int ncid, int varid, const size_t *indexp, const long long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_INT64);
}

int
nc_put_var1_ulonglong(int ncid, int varid, const size_t *indexp, const unsigned long long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_var1_string(int ncid, int varid, const size_t *indexp, const char* *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var1(ncid, varid, indexp, (void*)op, NC_STRING);
}
#endif /*USE_NETCDF4*/
/**@}*/

/** \ingroup variables
Write an entire variable with one call. 

The nc_put_var_ type family of functions write all the values of a
variable into a netCDF variable of an open netCDF dataset. This is the
simplest interface to use for writing a value in a scalar variable or
whenever all the values of a multidimensional variable can all be
written at once. The values to be written are associated with the
netCDF variable by assuming that the last dimension of the netCDF
variable varies fastest in the C interface. The values are converted
to the external data type of the variable, if necessary.

Take care when using this function with record variables (variables
that use the ::NC_UNLIMITED dimension). If you try to write all the
values of a record variable into a netCDF file that has no record data
yet (hence has 0 records), nothing will be written. Similarly, if you
try to write all the values of a record variable but there are more
records in the file than you assume, more in-memory data will be
accessed than you supply, which may result in a segmentation
violation. To avoid such problems, it is better to use the nc_put_vara
interfaces for variables that use the ::NC_UNLIMITED dimension. 

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc_put_var() function will write a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param op Pointer from where the data will be copied.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_var(int ncid, int varid, const void *op)
{
   return NC_put_var(ncid, varid, op, NC_NAT);
}

int
nc_put_var_text(int ncid, int varid, const char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,NC_CHAR);
}

int
nc_put_var_schar(int ncid, int varid, const signed char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,NC_BYTE);
}

int
nc_put_var_uchar(int ncid, int varid, const unsigned char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_uchar);
}

int
nc_put_var_short(int ncid, int varid, const short *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,NC_SHORT);
}

int
nc_put_var_int(int ncid, int varid, const int *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,NC_INT);
}

int
nc_put_var_long(int ncid, int varid, const long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_long);
}

int
nc_put_var_float(int ncid, int varid, const float *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_float);
}

int
nc_put_var_double(int ncid, int varid, const double *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_double);
}

int
nc_put_var_ubyte(int ncid, int varid, const unsigned char *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_ubyte);
}

int
nc_put_var_ushort(int ncid, int varid, const unsigned short *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_ushort);
}

int
nc_put_var_uint(int ncid, int varid, const unsigned int *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_uint);
}

int
nc_put_var_longlong(int ncid, int varid, const long long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,T_longlong);
}

int
nc_put_var_ulonglong(int ncid, int varid, const unsigned long long *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_var_string(int ncid, int varid, const char* *op)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_var(ncid,varid,(void*)op,NC_STRING);
}
#endif /*USE_NETCDF4*/
/**\} */

/** \ingroup variables
Write a strided array of values to a variable. 

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param stridep Stride vector with one element for each dimension to
\ref specify_hyperslab.

\param op Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_vars (int ncid, int varid, const size_t *startp,
	     const size_t *countp, const ptrdiff_t *stridep,
	     const void *op)
{
   NC *ncp;
   int stat = NC_NOERR;

   if ((stat = NC_check_id(ncid, &ncp)))
       return stat;
   return ncp->dispatch->put_vars(ncid, varid, startp, countp, 
				  stridep, op, NC_NAT);
}

int
nc_put_vars_text(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep,
		 const char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep,(void*)op,NC_CHAR);
}

int
nc_put_vars_schar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const signed char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp, 
		      stridep,(void*)op,NC_BYTE);
}

int
nc_put_vars_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const unsigned char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_uchar);
}

int
nc_put_vars_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const short *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, NC_SHORT);
}

int
nc_put_vars_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t *stridep,
		const int *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, NC_INT);
}

int
nc_put_vars_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep,
		 const long *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_long);
}

int
nc_put_vars_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const float *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_float);
}

int
nc_put_vars_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep,
		   const double *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_double);
}

int
nc_put_vars_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const unsigned char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_ubyte);
}

int
nc_put_vars_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep,
		   const unsigned short *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_ushort);
}

int
nc_put_vars_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep,
		 const unsigned int *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_uint);
}

int
nc_put_vars_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t *stridep,
		     const long long *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, T_longlong);
}

int
nc_put_vars_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t *stridep,
		      const unsigned long long *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp,
		      stridep, (void *)op, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_vars_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep,
		   const char**op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_vars(ncid, varid, startp, countp, stridep,
		      (void *)op, NC_STRING);
}
#endif /*USE_NETCDF4*/
/**\} */

/** \ingroup variables
Write a mapped array of values to a variable. 

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param stridep Stride vector with one element for each dimension to
\ref specify_hyperslab.

\param imapp Mapping vector with one element for each dimension to
\ref specify_hyperslab.

\param op Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_varm (int ncid, int varid, const size_t *startp,
	     const size_t *countp, const ptrdiff_t *stridep,
	     const ptrdiff_t *imapp, const void *op)
{
   NC *ncp;
   int stat = NC_NOERR;

   if ((stat = NC_check_id(ncid, &ncp)))
       return stat;
   return ncp->dispatch->put_varm(ncid, varid, startp, countp, 
				  stridep, imapp, op, NC_NAT);
}

int
nc_put_varm_text(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep, 
		 const ptrdiff_t *imapp, const char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, NC_CHAR);
}

int
nc_put_varm_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const signed char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, NC_BYTE);
}

int
nc_put_varm_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const unsigned char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_uchar);
}

int
nc_put_varm_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const short *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, NC_SHORT);
}

int
nc_put_varm_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		const int *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, NC_INT);
}

int
nc_put_varm_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		 const long *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_long);
}

int
nc_put_varm_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const float *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_float);
}

int
nc_put_varm_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   const double *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_double);
}

int
nc_put_varm_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const unsigned char *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_ubyte);
}

int
nc_put_varm_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   const unsigned short *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_ushort);
}

int
nc_put_varm_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		 const unsigned int *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_uint);
}

int
nc_put_varm_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		     const long long *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, T_longlong);
}

int
nc_put_varm_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		      const unsigned long long *op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_varm_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   const char**op)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)op, NC_STRING);
}
#endif /*USE_NETCDF4*/
/**\} */


/*! \} */ /*End of named group... */

