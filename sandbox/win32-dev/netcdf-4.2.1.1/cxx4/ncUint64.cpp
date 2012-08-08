#include "ncUint64.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcUint64  called netCDF::ncUint64
namespace netCDF {
  NcUint64 ncUint64;
}

// constructor
NcUint64::NcUint64() : NcType(NC_UINT64){
}

NcUint64::~NcUint64() {
}


// equivalence operator
bool NcUint64::operator==(const NcUint64 & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
