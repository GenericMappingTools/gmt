#include "ncShort.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcShort  called netCDF::ncShort
namespace netCDF {
  NcShort ncShort;
}

// constructor
NcShort::NcShort() : NcType(NC_SHORT){
}

NcShort::~NcShort() {
}


// equivalence operator
bool NcShort::operator==(const NcShort & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
