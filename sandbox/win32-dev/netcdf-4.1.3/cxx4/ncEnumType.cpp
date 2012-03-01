#include "ncEnumType.h"
#include "ncGroup.h"
#include "ncCheck.h"
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
#include "ncException.h"
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// Class represents a netCDF variable.

// assignment operator
NcEnumType& NcEnumType::operator=(const NcEnumType& rhs)
{
  NcType::operator=(rhs);    // assign base class parts
  return *this;
}

// assignment operator
NcEnumType& NcEnumType::operator=(const NcType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of an Enum type
    if(getTypeClass() != NC_ENUM) throw NcException("NcException","The NcType object must be the base of an Enum type.",__FILE__,__LINE__);
    // assign base class parts
    NcType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcEnumType::NcEnumType(const NcEnumType& rhs): 
  NcType(rhs)
{
}


// Constructor generates a null object.
NcEnumType::NcEnumType() :
  NcType()   // invoke base class constructor
{}
  
// constructor
NcEnumType::NcEnumType(const NcGroup& grp, const string& name):
  NcType(grp,name)
{}
  

// constructor
NcEnumType::NcEnumType(const NcType& ncType): 
  NcType(ncType)
{
  // check the nctype object is the base of an Enum type
  if(getTypeClass() != NC_ENUM) throw NcException("NcException","The NcType object must be the base of an Enum type.",__FILE__,__LINE__);
}

// Returns the base type.
NcType NcEnumType::getBaseType() const
{
  char charName[NC_MAX_NAME+1];
  nc_type base_nc_typep;
  size_t *base_sizep=NULL;
  size_t *num_membersp=NULL;
  ncCheck(nc_inq_enum(groupId,myId,charName,&base_nc_typep,base_sizep,num_membersp),__FILE__,__LINE__);
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

  
// Returns number of members in this NcEnumType object.
size_t   NcEnumType::getMemberCount() const{
  char charName[NC_MAX_NAME+1];
  nc_type* base_nc_typep=NULL;
  size_t* base_sizep=NULL;
  size_t num_membersp;
  ncCheck(nc_inq_enum(groupId,myId,charName,base_nc_typep,base_sizep,&num_membersp),__FILE__,__LINE__);
  return num_membersp;
};
  
// Returns the member name for the given zero-based index.
string NcEnumType::getMemberNameFromIndex(int index) const{
  void* value=NULL;
  char charName[NC_MAX_NAME+1];
  ncCheck(nc_inq_enum_member(groupId,myId,index,charName,value),__FILE__,__LINE__);
  return static_cast<string> (charName);
};
