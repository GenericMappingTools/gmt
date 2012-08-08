#include "netcdf.h"
#include <ncException.h>
using namespace std;
using namespace netCDF::exceptions;

//  C++ API for netCDF4.
namespace netCDF
{
  // function checks error code and if necessary throws appropriate exception.
  void ncCheck(int retCode,char* file,int line){
    switch(retCode) {
      
    case NC_NOERR           : return; /* No Error */
      
    case NC_EBADID          : throw NcBadId("Not a netcdf id",file,line);
    case NC_ENFILE          : throw NcNFile("Too many netcdfs open",file,line);
    case NC_EEXIST          : throw NcExist("netcdf file exists && NC_NOCLOBBER",file,line);
    case NC_EINVAL          : throw NcInvalidArg("Invalid Argument",file,line);
    case NC_EPERM           : throw NcInvalidWrite("Write to read only",file,line);
    case NC_ENOTINDEFINE    : throw NcNotInDefineMode("Operation not allowed in data mode",file,line);
    case NC_EINDEFINE       : throw NcInDefineMode("Operation not allowed in define mode",file,line);
    case NC_EINVALCOORDS    : throw NcInvalidCoords("Index exceeds dimension bound",file,line);
    case NC_EMAXDIMS        : throw NcMaxDims("NC_MAX_DIMS is exceeded",file,line);
    case NC_ENAMEINUSE      : throw NcNameInUse("String match to name in use",file,line);
    case NC_ENOTATT         : throw NcNotAtt("Attribute not found",file,line);
    case NC_EMAXATTS        : throw NcMaxAtts("NC_MAX_ATTRS exceeded",file,line);
    case NC_EBADTYPE        : throw NcBadType("Not a netcdf data type",file,line);
    case NC_EBADDIM         : throw NcBadDim("Invalid dimension id or name",file,line);
    case NC_EUNLIMPOS       : throw NcUnlimPos("NC_UNLIMITED is in the wrong index",file,line);
    case NC_EMAXVARS        : throw NcMaxVars("NC_MAX_VARS is exceeded",file,line);
    case NC_ENOTVAR         : throw NcNotVar("Variable is not found",file,line);
    case NC_EGLOBAL         : throw NcGlobal("Action prohibited on NC_GLOBAL varid",file,line);
    case NC_ENOTNC          : throw NcNotNCF("Not a netcdf file",file,line);
    case NC_ESTS            : throw NcSts("In Fortran, string too short",file,line);
    case NC_EMAXNAME        : throw NcMaxName("NC_MAX_NAME exceeded",file,line);
    case NC_EUNLIMIT        : throw NcUnlimit("NC_UNLIMITED size already in use",file,line);
    case NC_ENORECVARS      : throw NcNoRecVars("nc_rec op when there are no record vars",file,line);
    case NC_ECHAR           : throw NcChar("Attempt to convert between text & numbers",file,line);
    case NC_EEDGE           : throw NcEdge("Edge+start exceeds dimension bound",file,line);
    case NC_ESTRIDE         : throw NcStride("Illegal stride",file,line);
    case NC_EBADNAME        : throw NcBadName("Attribute or variable name contains illegal characters",file,line);
    case NC_ERANGE          : throw NcRange("Math result not representable",file,line);
    case NC_ENOMEM          : throw NcNoMem("Memory allocation (malloc) failure",file,line);
    case NC_EVARSIZE        : throw NcVarSize("One or more variable sizes violate format constraints",file,line);
    case NC_EDIMSIZE        : throw NcDimSize("Invalid dimension size",file,line);
    case NC_ETRUNC          : throw NcTrunc("File likely truncated or possibly corrupted",file,line);

      // The following are specific netCDF4 errors.
    case NC_EHDFERR         : throw NcHdfErr("An error was reported by the HDF5 layer.",file,line);
    case NC_ECANTREAD       : throw NcCantRead("Cannot Read",file,line);
    case NC_ECANTWRITE      : throw NcCantWrite("Cannott write",file,line);
    case NC_ECANTCREATE     : throw NcCantCreate("Cannot create",file,line);
    case NC_EFILEMETA       : throw NcFileMeta("File  meta",file,line);
    case NC_EDIMMETA        : throw NcDimMeta("dim meta",file,line);
    case NC_EATTMETA        : throw NcAttMeta("att meta",file,line);
    case NC_EVARMETA        : throw NcVarMeta("var meta",file,line);
    case NC_ENOCOMPOUND     : throw NcNoCompound("No compound",file,line);
    case NC_EATTEXISTS      : throw NcAttExists("Attribute exists",file,line);
    case NC_ENOTNC4         : throw NcNotNc4("Attempting netcdf-4 operation on netcdf-3 file.",file,line);
    case NC_ESTRICTNC3      : throw NcStrictNc3("Attempting netcdf-4 operation on strict nc3 netcdf-4 file.",file,line);
    case NC_EBADGRPID       : throw NcBadGroupId("Bad group id.",file,line);
    case NC_EBADTYPID       : throw NcBadTypeId("Bad type id.",file,line);                       // netcdf.h file inconsistent with documentation!!
    case NC_EBADFIELD       : throw NcBadFieldId("Bad field id.",file,line);                     // netcdf.h file inconsistent with documentation!!
      //  case NC_EUNKNAME        : throw NcUnkownName("Cannot find the field id.",file,line);   // netcdf.h file inconsistent with documentation!!

    case NC_ENOGRP          : throw NcEnoGrp("No netCDF group found",file,line);
    case NC_ELATEDEF        : throw NcElateDef("Operation to set the deflation, chunking, endianness, fill, compression, or checksum of a NcVar object has been made after a call to getVar or putVar."
					       ,file,line);

    default:
      throw NcException("NcException","Unknown error",file,line);
    }
  }
}
