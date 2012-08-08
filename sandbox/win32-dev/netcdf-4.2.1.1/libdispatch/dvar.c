/*! \file
Functions for defining and inquiring about variables.

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/

#include "ncdispatch.h"
#include "netcdf_f.h"

/** \defgroup variables Variables

Variables hold multi-dimensional arrays of data.

Variables for a netCDF dataset are defined when the dataset is
created, while the netCDF dataset is in define mode. Other variables
may be added later by reentering define mode. A netCDF variable has a
name, a type, and a shape, which are specified when it is defined. A
variable may also have values, which are established later in data
mode.

Ordinarily, the name, type, and shape are fixed when the variable is
first defined. The name may be changed, but the type and shape of a
variable cannot be changed. However, a variable defined in terms of
the unlimited dimension can grow without bound in that dimension.

A netCDF variable in an open netCDF dataset is referred to by a small
integer called a variable ID.

Variable IDs reflect the order in which variables were defined within
a netCDF dataset. Variable IDs are 0, 1, 2,..., in the order in which
the variables were defined. A function is available for getting the
variable ID from the variable name and vice-versa.

Attributes (see Attributes) may be associated with a variable to
specify such properties as units.

Operations supported on variables are:
- Create a variable, given its name, data type, and shape.
- Get a variable ID from its name.
- Get a variable's name, data type, shape, and number of attributes
  from its ID.
- Put a data value into a variable, given variable ID, indices, and value.
- Put an array of values into a variable, given variable ID, corner
  indices, edge lengths, and a block of values.
- Put a subsampled or mapped array-section of values into a variable,
  given variable ID, corner indices, edge lengths, stride vector,
  index mapping vector, and a block of values.
- Get a data value from a variable, given variable ID and indices.
- Get an array of values from a variable, given variable ID, corner
  indices, and edge lengths.
- Get a subsampled or mapped array-section of values from a variable,
  given variable ID, corner indices, edge lengths, stride vector, and
  index mapping vector.
- Rename a variable. 

\section language_types Language Types Corresponding to netCDF
External Data Types

NetCDF supported six atomic data types through version 3.6.0 (char,
byte, short, int, float, and double). Starting with version 4.0, many
new atomic and user defined data types are supported (unsigned int
types, strings, compound types, variable length arrays, enums,
opaque).

The additional data types are only supported in netCDF-4/HDF5
files. To create netCDF-4/HDF5 files, use the HDF5 flag in
nc_create. (see nc_create).

\section classic_types NetCDF-3 Classic and 64-Bit Offset Data Types

NetCDF-3 classic and 64-bit offset files support 6 atomic data types,
and none of the user defined datatype introduced in NetCDF-4.

The following table gives the netCDF-3 external data types and the
corresponding type constants for defining variables in the C
interface:

<table>
<tr><td>Type</td><td>C define</td><td>Bits</td></tr>
<tr><td>byte</td><td>NC_BYTE</td><td>8</td></tr>
<tr><td>char</td><td>NC_CHAR</td><td>8</td></tr>
<tr><td>short</td><td>NC_SHORT</td><td>16</td></tr>
<tr><td>int</td><td>NC_INT</td><td>32</td></tr>
<tr><td>float</td><td>NC_FLOAT</td><td>32</td></tr>
<tr><td>double</td><td>NC_DOUBLE</td><td>64</td></tr>
</table>

The first column gives the netCDF external data type, which is the
same as the CDL data type. The next column gives the corresponding C
pre-processor macro for use in netCDF functions (the pre-processor
macros are defined in the netCDF C header-file netcdf.h). The last
column gives the number of bits used in the external representation of
values of the corresponding type.

\section netcdf_4_atomic NetCDF-4 Atomic Types

NetCDF-4 files support all of the atomic data types from netCDF-3,
plus additional unsigned integer types, 64-bit integer types, and a
string type.

<table>
<tr><td>Type</td><td>C define</td><td>Bits

<tr><td>byte</td><td>NC_BYTE</td><td>8</td></tr>
<tr><td>unsigned byte </td><td>NC_UBYTE^</td><td> 8</td></tr>
<tr><td>char </td><td>NC_CHAR </td><td>8</td></tr>
<tr><td>short </td><td>NC_SHORT </td><td>16</td></tr>
<tr><td>unsigned short </td><td>NC_USHORT^ </td><td>16</td></tr>
<tr><td>int </td><td>NC_INT </td><td>32</td></tr>
<tr><td>unsigned int </td><td>NC_UINT^ </td><td>32</td></tr>
<tr><td>unsigned long long </td><td>NC_UINT64^ </td><td>64</td></tr>
<tr><td>long long </td><td>NC_INT64^ </td><td>64</td></tr>
<tr><td>float </td><td>NC_FLOAT </td><td>32</td></tr>
<tr><td>double </td><td>NC_DOUBLE </td><td>64</td></tr>
<tr><td>char ** </td><td>NC_STRING^ </td><td>string length + 1</td></tr>
</table>

^This type was introduced in netCDF-4, and is not supported in netCDF
classic or 64-bit offset format files, or in netCDF-4 files if they
are created with the NC_CLASSIC_MODEL flags.
 */

/** \name Defining Variables

Use these functions to define variables.
 */
/*! \{ */

/**
\ingroup variables
Define a new variable.

This function adds a new variable to an open netCDF dataset or group. 
It returns (as an argument) a variable ID, given the netCDF ID,
the variable name, the variable type, the number of dimensions, and a
list of the dimension IDs.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param name Variable \ref object_name.

\param xtype \ref data_type of the variable.

\param ndims Number of dimensions for the variable. For example, 2
specifies a matrix, 1 specifies a vector, and 0 means the variable is
a scalar with no dimensions. Must not be negative or greater than the
predefined constant ::NC_MAX_VAR_DIMS.

\param dimidsp Vector of ndims dimension IDs corresponding to the
variable dimensions. For classic model netCDF files, if the ID of the
unlimited dimension is included, it must be first. This argument is
ignored if ndims is 0. For expanded model netCDF4/HDF5 files, there
may be any number of unlimited dimensions, and they may be used in any
element of the dimids array.

\param varidp Pointer to location for the returned variable ID.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTINDEFINE Not in define mode. 
\returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
\returns ::NC_EMAXVARS NC_MAX_VARS exceeded
\returns ::NC_EBADTYPE Bad type.
\returns ::NC_EINVAL Invalid input.
\returns ::NC_ENAMEINUSE Name already in use.
\returns ::NC_EPERM Attempt to create object in read-only file. 

\section Example

Here is an example using nc_def_var to create a variable named rh of
type double with three dimensions, time, lat, and lon in a new netCDF
dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;       
     int  ncid;          
     int  lat_dim, lon_dim, time_dim;  
     int  rh_id;                       
     int  rh_dimids[3];                
        ...
     status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...

     status = nc_def_dim(ncid, "lat", 5L, &lat_dim);
     if (status != NC_NOERR) handle_error(status);
     status = nc_def_dim(ncid, "lon", 10L, &lon_dim);
     if (status != NC_NOERR) handle_error(status);
     status = nc_def_dim(ncid, "time", NC_UNLIMITED, &time_dim);
     if (status != NC_NOERR) handle_error(status);
        ...

     rh_dimids[0] = time_dim;
     rh_dimids[1] = lat_dim;
     rh_dimids[2] = lon_dim;
     status = nc_def_var (ncid, "rh", NC_DOUBLE, 3, rh_dimids, &rh_id);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_def_var(int ncid, const char *name, nc_type xtype, 
	   int ndims,  const int *dimidsp, int *varidp)
{
   NC* ncp;
   int stat = NC_NOERR;

   if ((stat = NC_check_id(ncid, &ncp)))
      return stat;
   return ncp->dispatch->def_var(ncid, name, xtype, ndims,
				 dimidsp, varidp);
}
/*! \} */

/** \name Rename a Variable

Rename a variable.
 */
/*! \{ */

/** Rename a variable.
\ingroup variables

This function changes the name of a netCDF variable in an open netCDF
file or group. You cannot rename a variable to have the name of any existing
variable.

For classic format, 64-bit offset format, and netCDF-4/HDF5 with
classic mode, if the new name is longer than the old name, the netCDF
dataset must be in define mode. 

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param name New name of the variable.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_EBADNAME Bad name.
\returns ::NC_EMAXNAME Name is too long.
\returns ::NC_ENAMEINUSE Name in use.
\returns ::NC_ENOMEM Out of memory.

\section Example

Here is an example using nc_rename_var to rename the variable rh to
rel_hum in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status; 
     int  ncid;   
     int  rh_id;  
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid); 
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
     status = nc_rename_var (ncid, rh_id, "rel_hum");
     if (status != NC_NOERR) handle_error(status);
     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode

*/
int
nc_rename_var(int ncid, int varid, const char *name)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->rename_var(ncid, varid, name);
}
/*! \} */

/** \internal
\ingroup variables
 */
int
NC_is_recvar(int ncid, int varid, size_t* nrecs)
{
   int status = NC_NOERR;
   int unlimid;
   int ndims;
   int dimset[NC_MAX_VAR_DIMS];
    
   status = nc_inq_unlimdim(ncid,&unlimid);
   if(status != NC_NOERR) return 0; /* no unlimited defined */
   status = nc_inq_varndims(ncid,varid,&ndims);
   if(status != NC_NOERR) return 0; /* no unlimited defined */
   if(ndims == 0) return 0; /* scalar */
   status = nc_inq_vardimid(ncid,varid,dimset);
   if(status != NC_NOERR) return 0; /* no unlimited defined */
   status = nc_inq_dim(ncid,dimset[0],NULL,nrecs);
   if(status != NC_NOERR) return 0;
   return (dimset[0] == unlimid ? 1: 0);
}

/* Ok to use NC pointers because
   all IOSP's will use that structure,
   but not ok to use e.g. NC_Var pointers
   because they may be different structure
   entirely.
*/

/** \internal
\ingroup variables
Find the length of a type. This is how much space is required by the user, as in
\code
vals = malloc(nel * nctypelen(var.type));
ncvarget(cdfid, varid, cor, edg, vals);
\endcode
 */
int
nctypelen(nc_type type) 
{
   switch(type){
      case NC_CHAR :
	 return((int)sizeof(char));
      case NC_BYTE :
	 return((int)sizeof(signed char));
      case NC_SHORT :
	 return(int)(sizeof(short));
      case NC_INT :
	 return((int)sizeof(int));
      case NC_FLOAT :
	 return((int)sizeof(float));
      case NC_DOUBLE : 
	 return((int)sizeof(double));

	 /* These can occur in netcdf-3 code */ 
      case NC_UBYTE :
	 return((int)sizeof(unsigned char));
      case NC_USHORT :
	 return((int)(sizeof(unsigned short)));
      case NC_UINT :
	 return((int)sizeof(unsigned int));
      case NC_INT64 :
	 return((int)sizeof(signed long long));
      case NC_UINT64 :
	 return((int)sizeof(unsigned long long));
#ifdef USE_NETCDF4
      case NC_STRING :
	 return((int)sizeof(char*));
#endif /*USE_NETCDF4*/

      default:
	 return -1;
   }
}

/** \internal
\ingroup variables
Find the length of a type. Redunant over nctypelen() above. */
int
NC_atomictypelen(nc_type xtype)
{
   int sz = 0;
   switch(xtype) {
      case NC_NAT: sz = 0; break;
      case NC_BYTE: sz = sizeof(signed char); break;
      case NC_CHAR: sz = sizeof(char); break;
      case NC_SHORT: sz = sizeof(short); break;
      case NC_INT: sz = sizeof(int); break;
      case NC_FLOAT: sz = sizeof(float); break;
      case NC_DOUBLE: sz = sizeof(double); break;
      case NC_INT64: sz = sizeof(signed long long); break;
      case NC_UBYTE: sz = sizeof(unsigned char); break;
      case NC_USHORT: sz = sizeof(unsigned short); break;
      case NC_UINT: sz = sizeof(unsigned int); break;
      case NC_UINT64: sz = sizeof(unsigned long long); break;
#ifdef USE_NETCDF4
      case NC_STRING: sz = sizeof(char*); break;
#endif
      default: break;
   }	
   return sz;
}

/** \internal
\ingroup variables
    Get the type name. */
char *
NC_atomictypename(nc_type xtype)
{
   char* nm = NULL;
   switch(xtype) {
      case NC_NAT: nm = "undefined"; break;
      case NC_BYTE: nm = "byte"; break;
      case NC_CHAR: nm = "char"; break;
      case NC_SHORT: nm = "short"; break;
      case NC_INT: nm = "int"; break;
      case NC_FLOAT: nm = "float"; break;
      case NC_DOUBLE: nm = "double"; break;
      case NC_INT64: nm = "int64"; break;
      case NC_UBYTE: nm = "ubyte"; break;
      case NC_USHORT: nm = "ushort"; break;
      case NC_UINT: nm = "uint"; break;
      case NC_UINT64: nm = "uint64"; break;
#ifdef USE_NETCDF4
      case NC_STRING: nm = "string"; break;
#endif
      default: break;
   }	
   return nm;
}

/** \internal
\ingroup variables
Get the shape of a variable.    
 */
int
NC_getshape(int ncid, int varid, int ndims, size_t* shape)
{
   int dimids[NC_MAX_VAR_DIMS];
   int i;
   int status = NC_NOERR;

   if ((status = nc_inq_vardimid(ncid, varid, dimids)))
      return status;
   for(i = 0; i < ndims; i++) 
      if ((status = nc_inq_dimlen(ncid, dimids[i], &shape[i])))
	 break;

   return status;
}

#ifdef USE_NETCDF4
/** \ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param size The total size of the raw data chunk cache, in bytes.

\param nelems The number of chunk slots in the raw data chunk cache.

\param preemption The preemption, a value between 0 and 1 inclusive
that indicates how much chunks that have been fully read are favored
for preemption. A value of zero means fully read chunks are treated no
differently than other chunks (the preemption is strictly LRU) while a
value of one means fully read chunks are always preempted before other
chunks.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
\returns ::NC_EINVAL Invalid input
 */
int
nc_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, 
		       float preemption)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->set_var_chunk_cache(ncid, varid, size, 
					      nelems, preemption);
}

/** \ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID

\param sizep The total size of the raw data chunk cache, in bytes,
will be put here. \ref ignored_if_null.

\param nelemsp The number of chunk slots in the raw data chunk cache
hash table will be put here. \ref ignored_if_null.

\param preemptionp The preemption will be put here. The preemtion
value is between 0 and 1 inclusive and indicates how much chunks that
have been fully read are favored for preemption. A value of zero means
fully read chunks are treated no differently than other chunks (the
preemption is strictly LRU) while a value of one means fully read
chunks are always preempted before other chunks. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
\returns ::NC_EINVAL Invalid input
*/
int
nc_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp, 
		       float *preemptionp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_var_chunk_cache(ncid, varid, sizep,
					      nelemsp, preemptionp);
}

/** \ingroup variables
Free string space allocated by the library.

When you read string type the library will allocate the storage space
for the data. This storage space must be freed, so pass the pointer
back to this function, when you're done with the data, and it will
free the string memory. 

\param len The number of character arrays in the array. 
\param data The pointer to the data array. 

\returns ::NC_NOERR No error.
*/
int
nc_free_string(size_t len, char **data)
{
   int i;
   for (i = 0; i < len; i++)
      free(data[i]);
   return NC_NOERR;
}

int
nc_def_var_deflate(int ncid, int varid, int shuffle, int deflate, int deflate_level)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_deflate(ncid,varid,shuffle,deflate,deflate_level);
}

int
nc_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_fletcher32(ncid,varid,fletcher32);
}

int
nc_def_var_chunking(int ncid, int varid, int storage, 
		    const size_t *chunksizesp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_chunking(ncid, varid, storage, 
					   chunksizesp);
}

int
nc_def_var_fill(int ncid, int varid, int no_fill, const void *fill_value)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_fill(ncid,varid,no_fill,fill_value);
}

int
nc_def_var_endian(int ncid, int varid, int endian)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_endian(ncid,varid,endian);
}

#endif /* USE_NETCDF4 */
