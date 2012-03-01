#include "ncByte.h"
#include "netcdf.h"
using namespace netCDF;

// create an instance of NcByte  called netCDF::ncByte
namespace netCDF {
  NcByte ncByte;
}

// constructor
NcByte::NcByte() : NcType(NC_BYTE){
}

NcByte::~NcByte() {
}

int NcByte::sizeoff(){char a;return sizeof(a);};


// equivalence operator
bool NcByte::operator==(const NcByte & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
