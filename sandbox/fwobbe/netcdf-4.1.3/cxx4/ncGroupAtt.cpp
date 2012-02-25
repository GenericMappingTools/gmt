#include "ncGroupAtt.h"
#include "ncGroup.h"
#include "ncCheck.h"
#include <netcdf.h>
using namespace std;


namespace netCDF {
  //  Global comparator operator ==============
  // comparator operator 
  bool operator<(const NcGroupAtt& lhs,const NcGroupAtt& rhs)
  {
    return false;
  }
  
  // comparator operator 
  bool operator>(const NcGroupAtt& lhs,const NcGroupAtt& rhs)
  {
    return true;
  }
}


using namespace netCDF;

// assignment operator
NcGroupAtt& NcGroupAtt::operator=(const NcGroupAtt & rhs)
{
  NcAtt::operator=(rhs);    // assign base class parts
  return *this;
}

//! The copy constructor.
NcGroupAtt::NcGroupAtt(const NcGroupAtt& rhs): 
  NcAtt(rhs)   // invoke base class copy constructor
{}


// Constructor generates a null object.
NcGroupAtt::NcGroupAtt() : 
  NcAtt()  // invoke base class constructor
{}

// equivalence operator (doesn't bother compaing varid's of each object).
bool NcGroupAtt::operator==(const NcGroupAtt & rhs)
{
  if(nullObject) 
    return nullObject == rhs.isNull();
  else
    return myName == rhs.myName && groupId == rhs.groupId;
}  

// Constructor for an existing global attribute.
NcGroupAtt::NcGroupAtt(const NcGroup& grp, const int index):
  NcAtt(false)
{
  groupId =  grp.getId();
  varId = NC_GLOBAL;
  // get the name of this attribute
  char attName[NC_MAX_NAME+1];
  ncCheck(nc_inq_attname(groupId,varId, index, attName),__FILE__,__LINE__);
  ncCheck(nc_inq_attname(groupId,varId,index,attName),__FILE__,__LINE__);
  myName = attName;
}

