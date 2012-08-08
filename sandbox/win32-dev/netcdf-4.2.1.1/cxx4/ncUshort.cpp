#include "ncUshort.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcUshort  called netCDF::ncUshort
namespace netCDF {
  NcUshort ncUshort;
}

// constructor
NcUshort::NcUshort() : NcType(NC_USHORT){
}

NcUshort::~NcUshort() {
}


// equivalence operator
bool NcUshort::operator==(const NcUshort & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
