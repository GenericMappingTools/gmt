#include "ncInt64.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcInt64  called netCDF::ncInt64
namespace netCDF {
  NcInt64 ncInt64;
}

// constructor
NcInt64::NcInt64() : NcType(NC_INT64){
}

NcInt64::~NcInt64() {
}


// equivalence operator
bool NcInt64::operator==(const NcInt64 & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
