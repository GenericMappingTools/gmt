#include "ncUbyte.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcUbyte  called netCDF::ncUbyte
namespace netCDF {
  NcUbyte ncUbyte;
}

// constructor
NcUbyte::NcUbyte() : NcType(NC_UBYTE){
}

NcUbyte::~NcUbyte() {
}


// equivalence operator
bool NcUbyte::operator==(const NcUbyte & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
