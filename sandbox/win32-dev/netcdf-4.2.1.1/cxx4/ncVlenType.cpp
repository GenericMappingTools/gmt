#include "ncVlenType.h"
#include "ncGroup.h"
#include "ncCheck.h"
#include "ncException.h"
#include "ncByte.h"
#include "ncUbyte.h"
#include "ncChar.h"
#include "ncShort.h"
#include "ncUshort.h"
#include "ncInt.h"
#include "ncUint.h"
#include "ncInt64.h"
#include "ncUint64.h"
#include "ncFloat.h"
#include "ncDouble.h"
#include "ncString.h"
#include <netcdf.h>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// Class represents a netCDF variable.
using namespace netCDF;

// assignment operator
NcVlenType& NcVlenType::operator=(const NcVlenType& rhs)
{
  NcType::operator=(rhs);    // assign base class parts
  return *this;
}

// assignment operator
NcVlenType& NcVlenType::operator=(const NcType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of an Opaque type
    if(getTypeClass() != NC_VLEN) 	throw NcException("NcException","The NcType object must be the base of an Vlen type.",__FILE__,__LINE__);
    // assign base class parts
    NcType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcVlenType::NcVlenType(const NcVlenType& rhs):   
  NcType(rhs)
{
}


// Constructor generates a null object.
NcVlenType::NcVlenType() :
  NcType()   // invoke base class constructor
{}

// constructor
NcVlenType::NcVlenType(const NcGroup& grp, const string& name) :
  NcType(grp,name)
{}
  
// constructor
NcVlenType::NcVlenType(const NcType& ncType): 
  NcType(ncType)
{
  // check the nctype object is the base of a Vlen type
  if(getTypeClass() != NC_VLEN) throw NcException("NcException","The NcType object must be the base of a Vlen type.",__FILE__,__LINE__);
}

// Returns the base type.
NcType NcVlenType::getBaseType() const
{
  char charName[NC_MAX_NAME+1];
  nc_type base_nc_typep;
  size_t datum_sizep;
  ncCheck(nc_inq_vlen(groupId,myId,charName,&datum_sizep,&base_nc_typep),__FILE__,__LINE__);
  switch (base_nc_typep) {
  case NC_BYTE    : return ncByte;
  case NC_UBYTE   : return ncUbyte;
  case NC_CHAR    : return ncChar;
  case NC_SHORT   : return ncShort;
  case NC_USHORT  : return ncUshort;
  case NC_INT     : return ncInt;
  case NC_UINT    : return ncUint;  
  case NC_INT64   : return ncInt64; 
  case NC_UINT64  : return ncUint64;
  case NC_FLOAT   : return ncFloat;
  case NC_DOUBLE  : return ncDouble;
  case NC_STRING  : return ncString;
  default:  
    // this is a user defined type
    return NcType(getParentGroup(),base_nc_typep);
  }
}
