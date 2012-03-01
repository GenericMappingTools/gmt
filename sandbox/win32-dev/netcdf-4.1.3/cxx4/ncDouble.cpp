#include "ncDouble.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcDouble  called netCDF::ncDouble
namespace netCDF {
  NcDouble ncDouble;
}

// constructor
NcDouble::NcDouble() : NcType(NC_DOUBLE){
}

NcDouble::~NcDouble() {
}


// equivalence operator
bool NcDouble::operator==(const NcDouble & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
