#include "ncVar.h"
#include "ncVarAtt.h"
#include "ncGroup.h"
#include "ncCheck.h"
#include <netcdf.h>
using namespace std;


namespace netCDF {
  //  Global comparator operator ==============
  // comparator operator 
  bool operator<(const NcVarAtt& lhs,const NcVarAtt& rhs)
  {
    return false;
  }
  
  // comparator operator 
  bool operator>(const NcVarAtt& lhs,const NcVarAtt& rhs)
  {
    return true;
  }
}


using namespace netCDF;


// assignment operator
NcVarAtt& NcVarAtt::operator=(const NcVarAtt & rhs)
{
  NcAtt::operator=(rhs);    // assign base class parts
  return *this;
}

//! The copy constructor.
NcVarAtt::NcVarAtt(const NcVarAtt& rhs): 
  NcAtt(rhs) // invoke base class copy constructor
{}


// Constructor generates a null object.
NcVarAtt::NcVarAtt() :
  NcAtt()  // invoke base class constructor
{}


// Constructor for an existing local attribute.
NcVarAtt::NcVarAtt(const NcGroup& grp, const NcVar& ncVar, const int index):
  NcAtt(false)
{
  groupId =  grp.getId();
  varId = ncVar.getId();
  // get the name of this attribute
  char attName[NC_MAX_NAME+1];
  ncCheck(nc_inq_attname(groupId,varId, index, attName),__FILE__,__LINE__);
  ncCheck(nc_inq_attname(groupId,varId,index,attName),__FILE__,__LINE__);
  myName = attName;
}

// Returns the NcVar parent object.
NcVar NcVarAtt::getParentVar() const {
  return NcVar(groupId,varId);
}
