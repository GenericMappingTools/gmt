#include "ncChar.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcChar  called netCDF::ncChar
namespace netCDF {
  NcChar ncChar;
}

// constructor
NcChar::NcChar() : NcType(NC_CHAR){
}

NcChar::~NcChar() {
}


// equivalence operator
bool NcChar::operator==(const NcChar & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
