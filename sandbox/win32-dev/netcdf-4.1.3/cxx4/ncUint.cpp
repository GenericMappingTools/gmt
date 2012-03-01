#include "ncUint.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcUint  called netCDF::ncUint
namespace netCDF {
  NcUint ncUint;
}

// constructor
NcUint::NcUint() : NcType(NC_UINT){
}

NcUint::~NcUint() {
}


// equivalence operator
bool NcUint::operator==(const NcUint & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
