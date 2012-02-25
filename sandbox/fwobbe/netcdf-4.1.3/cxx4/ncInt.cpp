#include "ncInt.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcInt  called netCDF::ncInt
namespace netCDF {
  NcInt ncInt;
}

// constructor
NcInt::NcInt() : NcType(NC_INT){
}

NcInt::~NcInt() {
}


// equivalence operator
bool NcInt::operator==(const NcInt & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
