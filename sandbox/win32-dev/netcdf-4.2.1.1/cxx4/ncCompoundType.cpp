#include "ncGroup.h"
#include "ncCheck.h"
#include "ncCompoundType.h"
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
NcCompoundType& NcCompoundType::operator=(const NcCompoundType& rhs)
{
  NcType::operator=(rhs);    // assign base class parts
  return *this;
}

// assignment operator
NcCompoundType& NcCompoundType::operator=(const NcType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of a Compound type
    if(getTypeClass() != nc_COMPOUND) 	throw NcException("NcException","The NcType object must be the base of a Compound type.",__FILE__,__LINE__);
    // assign base class parts
    NcType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcCompoundType::NcCompoundType(const NcCompoundType& rhs): 
  NcType(rhs)
{
}


// equivalence operator
bool NcCompoundType::operator==(const NcCompoundType& rhs)
{
  if(nullObject) 
    return nullObject == rhs.nullObject;
  else
    return myId ==rhs.myId && groupId == rhs.groupId;
}  
  
// Constructor generates a null object.
NcCompoundType::NcCompoundType() : 
  NcType()   // invoke base class constructor
{}
  
// constructor
NcCompoundType::NcCompoundType(const NcGroup& grp, const string& name): 
  NcType(grp,name)
{
}
  
//  Inserts a named field.
void NcCompoundType::addMember(const string& memberName, const NcType& newMemberType,size_t offset)
{
  ncCheck(nc_insert_compound(groupId,myId,const_cast<char*>(memberName.c_str()),offset,newMemberType.getId()),__FILE__,__LINE__);
}



//  Inserts a named array field.
void NcCompoundType::addMember(const string& memberName, const NcType& newMemberType, size_t offset, const vector<int>& shape)
{
  ncCheck(nc_insert_array_compound(groupId, myId,const_cast<char*>(memberName.c_str()), offset, newMemberType.getId(), shape.size(), const_cast<int*>(&shape[0])),__FILE__,__LINE__);
}



// Returns number of members in this NcCompoundType object.
size_t  NcCompoundType::getMemberCount() const
{
  size_t nfieldsp;
  ncCheck(nc_inq_compound_nfields(groupId,myId,&nfieldsp),__FILE__,__LINE__);
  return nfieldsp;
}
  

// Returns a NcType object for a single member. */
NcType NcCompoundType::getMember(int memberIndex) const 
{
  nc_type fieldtypeidp;
  ncCheck(nc_inq_compound_fieldtype(groupId,myId,memberIndex,&fieldtypeidp),__FILE__,__LINE__);
  switch (fieldtypeidp) {
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
    return NcType(getParentGroup(),fieldtypeidp);
  }
}

  
// Returns the number of dimensions of a member with the given index.
int NcCompoundType::getMemberDimCount(int memberIndex) const 
{
  int ndimsp;
  ncCheck(nc_inq_compound_fieldndims(groupId,myId,memberIndex, &ndimsp),__FILE__,__LINE__);
  return ndimsp;
}
  
  
// Returns the shape of the given member.
vector<int> NcCompoundType::getMemberShape(int memberIndex) const 
{
  vector<int> dim_size;
  dim_size.resize(getMemberDimCount(memberIndex));
  ncCheck(nc_inq_compound_fielddim_sizes(groupId,myId,memberIndex,&dim_size[0]),__FILE__,__LINE__);
  return dim_size;
}
 

// Returns the offset of the member with given index.
size_t NcCompoundType::getMemberOffset(const int index) const
{
  size_t offsetp;
  ncCheck(nc_inq_compound_fieldoffset(groupId,myId, index,&offsetp),__FILE__,__LINE__);
  return offsetp;
}
