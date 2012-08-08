#include "ncFloat.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcFloat  called netCDF::ncFloat
namespace netCDF {
  NcFloat ncFloat;
}

// constructor
NcFloat::NcFloat() : NcType(NC_FLOAT){
}

NcFloat::~NcFloat() {
}


// equivalence operator
bool NcFloat::operator==(const NcFloat & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
