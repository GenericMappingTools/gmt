#include "ncString.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcString  called netCDF::ncString
namespace netCDF {
  NcString ncString;
}

// constructor
NcString::NcString() : NcType(NC_STRING){
}

NcString::~NcString() {
}


// equivalence operator
bool NcString::operator==(const NcString & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
