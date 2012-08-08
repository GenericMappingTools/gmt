#include <ncException.h>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;


// Default object thrown if a netCDF exception is encountered.
NcException::NcException(const string& name,const string& complaint,const char* file,int line) :
  message(complaint),
  exceptionName(name),
  fileName(file),
  lnumber(line)
{
  if(complaint.length()==0) message = "A netCDF exception has occured";
}

NcException::~NcException()throw() {}


// Thrown if the specified netCDF ID does not refer to an open netCDF dataset. 
NcBadId::NcBadId(const string& complaint,const char* file,int line) :
  NcException("NcBadId",complaint,file,line) { }


// Thrown if too many netcdf files are open.
NcNFile::NcNFile(const string& complaint,const char* file,int line) :
  NcException("NcFile",complaint,file,line) { }

// Thrown if, having set NC_NOCLOBBER, the specified dataset already exists. 
NcExist::NcExist(const string& complaint,const char* file,int line) :
  NcException("NcExist",complaint,file,line) { }

// Thrown if not a netCDF id.
NcInvalidArg::NcInvalidArg(const string& complaint,const char* file,int line) :
  NcException("NcInvalidArg",complaint,file,line) { }

// Thrown if invalid argument.
NcInvalidWrite::NcInvalidWrite(const string& complaint,const char* file,int line) :
  NcException("NcInvalidWrite",complaint,file,line) { }

// Thrown if operation not allowed in data mode.
NcNotInDefineMode::NcNotInDefineMode(const string& complaint,const char* file,int line) :
  NcException("NcNotIndDefineMode",complaint,file,line) { }

// Thrown if operation not allowed in defined mode.
NcInDefineMode::NcInDefineMode(const string& complaint,const char* file,int line) :
  NcException("NcInDefineMode",complaint,file,line) { }

// Index exceeds dimension bound
NcInvalidCoords::NcInvalidCoords(const string& complaint,const char* file,int line) :
  NcException("NcInvalidCoords",complaint,file,line) { }

// Thrown if NC_MAX_DIMS is exceeded.
NcMaxDims::NcMaxDims(const string& complaint,const char* file,int line) :
  NcException("NcMaxDims",complaint,file,line) { }

// Thrown if string match to name is in use.
NcNameInUse::NcNameInUse(const string& complaint,const char* file,int line) :
  NcException("NcNameInUse",complaint,file,line) { }

// Thrown if attribute is not found.
NcNotAtt::NcNotAtt(const string& complaint,const char* file,int line) :
  NcException("NcNotAtt",complaint,file,line) { }

// Thrown if Nc_MAX_ATTRS is exceeded.
NcMaxAtts::NcMaxAtts(const string& complaint,const char* file,int line) :
  NcException("NcMaxAtts",complaint,file,line) { }

// Thrown if not a valid netCDF data type.
NcBadType::NcBadType(const string& complaint,const char* file,int line) :
  NcException("NcBadType",complaint,file,line) { }

// Thrown if an invalid dimension id or name.
NcBadDim::NcBadDim(const string& complaint,const char* file,int line) :
  NcException("NcBadDim",complaint,file,line) { }

// Thrown if Nc_UNLIMITED is in the wrong index.
NcUnlimPos::NcUnlimPos(const string& complaint,const char* file,int line) :
  NcException("NcUnlimPos",complaint,file,line) { }

// Thrown if NC_MAX_VARS is exceeded.
NcMaxVars::NcMaxVars(const string& complaint,const char* file,int line) :
  NcException("NcMaxVars",complaint,file,line) { }

// Thrown if variable is not found.
NcNotVar::NcNotVar(const string& complaint,const char* file,int line) :
  NcException("NcNotVar",complaint,file,line) { }

// Thrown if the action is prohibited on the NC_GLOBAL varid.
NcGlobal::NcGlobal(const string& complaint,const char* file,int line) :
  NcException("NcGlobal",complaint,file,line) { }

// Thrown if not a netCDF file.
NcNotNCF::NcNotNCF(const string& complaint,const char* file,int line) :
  NcException("NcNotNCF",complaint,file,line) { }

// Thrown if in FORTRAN, string is too short.
NcSts::NcSts(const string& complaint,const char* file,int line) :
  NcException("NcSts",complaint,file,line) { }

// Thrown if NC_MAX_NAME is exceeded.
NcMaxName::NcMaxName(const string& complaint,const char* file,int line) :
  NcException("NcMaxName",complaint,file,line) { }

// Thrown if NC_UNLIMITED size is already in use.
NcUnlimit::NcUnlimit(const string& complaint,const char* file,int line) :
  NcException("NcUnlimit",complaint,file,line) { }

// Thrown if nc_rec op when there are no record vars.
NcNoRecVars::NcNoRecVars(const string& complaint,const char* file,int line) :
  NcException("NcNoRecVars",complaint,file,line) { }

// Thrown if attempt to convert between text and numbers.
NcChar::NcChar(const string& complaint,const char* file,int line) :
  NcException("NcChar",complaint,file,line) { }

// Thrown if edge+start exceeds dimension bound.
NcEdge::NcEdge(const string& complaint,const char* file,int line) :
  NcException("NcEdge",complaint,file,line) { }

// Thrown if illegal stride.
NcStride::NcStride(const string& complaint,const char* file,int line) :
  NcException("NcStride",complaint,file,line) { }

// Thrown if attribute or variable name contains illegal characters.
NcBadName::NcBadName(const string& complaint,const char* file,int line) :
  NcException("NcBadName",complaint,file,line) { }

// Thrown if math result not representable.
NcRange::NcRange(const string& complaint,const char* file,int line) :
  NcException("NcRange",complaint,file,line) { }

// Thrown if memory allocation (malloc) failure.
NcNoMem::NcNoMem(const string& complaint,const char* file,int line) :
  NcException("NcNoMem",complaint,file,line) { }

// Thrown if one or more variable sizes violate format constraints
NcVarSize::NcVarSize(const string& complaint,const char* file,int line) :
  NcException("NcVarSize",complaint,file,line) { }

// Thrown if invalid dimension size.
NcDimSize::NcDimSize(const string& complaint,const char* file,int line) :
  NcException("NcDimSize",complaint,file,line) { }

// Thrown if file likely truncated or possibly corrupted.
NcTrunc::NcTrunc(const string& complaint,const char* file,int line) :
  NcException("NcTrunc",complaint,file,line) { }

// Thrown if an error was reported by the HDF5 layer.
NcHdfErr::NcHdfErr(const string& complaint,const char* file,int line) :
  NcException("NcHdfErr",complaint,file,line) { }

// Thrown if cannot read.
NcCantRead::NcCantRead(const string& complaint,const char* file,int line) :
  NcException("NcCantRead",complaint,file,line) { }

// Thrown if cannot write.
NcCantWrite::NcCantWrite(const string& complaint,const char* file,int line) :
  NcException("NcCantWrite",complaint,file,line) { }

// Thrown if cannot create.
NcCantCreate::NcCantCreate(const string& complaint,const char* file,int line) :
  NcException("NcCantCreate",complaint,file,line) { }

// Thrown if file meta.
NcFileMeta::NcFileMeta(const string& complaint,const char* file,int line) :
  NcException("NcFileMeta",complaint,file,line) { }

// Thrown if dim meta.
NcDimMeta::NcDimMeta(const string& complaint,const char* file,int line) :
  NcException("NcDimMeta",complaint,file,line) { }

// Thrown if attribute meta.
NcAttMeta::NcAttMeta(const string& complaint,const char* file,int line) :
  NcException("NcAttMeta",complaint,file,line) { }

// Thrown if variable meta.
NcVarMeta::NcVarMeta(const string& complaint,const char* file,int line) :
  NcException("NcVarMeta",complaint,file,line) { }

// Thrown if no compound.
NcNoCompound::NcNoCompound(const string& complaint,const char* file,int line) :
  NcException("NcNoCompound",complaint,file,line) { }

// Thrown if attribute exists.
NcAttExists::NcAttExists(const string& complaint,const char* file,int line) :
  NcException("NcAttExists",complaint,file,line) { }

// Thrown if attempting netcdf-4 operation on netcdf-3 file.
NcNotNc4::NcNotNc4(const string& complaint,const char* file,int line) :
  NcException("NcNotNc4",complaint,file,line) { }

// Thrown if attempting netcdf-4 operation on strict nc3 netcdf-4 file.
NcStrictNc3::NcStrictNc3(const string& complaint,const char* file,int line) :
  NcException("NcStrictNc3",complaint,file,line) { }

// Thrown if bad group id.
NcBadGroupId::NcBadGroupId(const string& complaint,const char* file,int line) :
  NcException("NcBadGroupId",complaint,file,line) { }

// Thrown if bad type id.
NcBadTypeId::NcBadTypeId(const string& complaint,const char* file,int line) :
  NcException("NcBadTypeId",complaint,file,line) { }

// Thrown if bad field id.
NcBadFieldId::NcBadFieldId(const string& complaint,const char* file,int line) :
  NcException("NcBadFieldId",complaint,file,line) { }

// Thrown if cannot find the field id.
NcUnknownName::NcUnknownName(const string& complaint,const char* file,int line) :
  NcException("NcUnknownName",complaint,file,line) { }

// Thrown if cannot find the field id.
NcEnoGrp::NcEnoGrp(const string& complaint,const char* file,int line) :
  NcException("NcEnoGrp",complaint,file,line) { }

// Thrown if cannot find the field id.
NcNullGrp::NcNullGrp(const string& complaint,const char* file,int line) :
  NcException("NcNullGrp",complaint,file,line) { }

// Thrown if cannot find the field id.
NcNullDim::NcNullDim(const string& complaint,const char* file,int line) :
  NcException("NcNullDim",complaint,file,line) { }

// Thrown if cannot find the field id.
NcNullType::NcNullType(const string& complaint,const char* file,int line) :
  NcException("NcNullType",complaint,file,line) { }

// Thrown if an operation to set the deflation, chunking, endianness, fill, compression, or checksum of a NcVar object is issued after a call to NcVar::getVar or NcVar::putVar.
NcElateDef::NcElateDef(const string& complaint,const char* file,int line) :
  NcException("NcElateDef",complaint,file,line) { }
