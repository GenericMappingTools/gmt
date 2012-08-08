#include "ncOpaqueType.h"
#include "ncGroup.h"
#include "ncCheck.h"
#include "ncException.h"
#include <netcdf.h>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// Class represents a netCDF variable.
using namespace netCDF;
  
// assignment operator
NcOpaqueType& NcOpaqueType::operator=(const NcOpaqueType& rhs)
{
  // assign base class parts
  NcType::operator=(rhs);    
  return *this;
}
  
// assignment operator
NcOpaqueType& NcOpaqueType::operator=(const NcType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of an Opaque type
    if(getTypeClass() != NC_OPAQUE) 	throw NcException("NcException","The NcType object must be the base of an Opaque type.",__FILE__,__LINE__);
    // assign base class parts
    NcType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcOpaqueType::NcOpaqueType(const NcOpaqueType& rhs): 
  NcType(rhs)
{
}


// Constructor generates a null object.
NcOpaqueType::NcOpaqueType() :
  NcType()   // invoke base class constructor
{}


// constructor
NcOpaqueType::NcOpaqueType(const NcGroup& grp, const string& name) :
  NcType(grp,name)
{}
  
  
// constructor
NcOpaqueType::NcOpaqueType(const NcType& ncType) :
  NcType(ncType)
{
  // check the nctype object is the base of a Opaque type
  if(getTypeClass() != NC_OPAQUE) 	throw NcException("NcException","The NcType object must be the base of an Opaque type.",__FILE__,__LINE__);
}
  
// Returns the size of the opaque type in bytes.
size_t  NcOpaqueType::getTypeSize() const
{
  char* charName;
  charName=NULL;
  size_t sizep;
  ncCheck(nc_inq_opaque(groupId,myId,charName,&sizep),__FILE__,__LINE__);
  return sizep;
}
