#include "ncAtt.h"
#include "ncGroup.h"
#include "ncCheck.h"
#include <vector>

using namespace std;
using namespace netCDF;
  

// destructor  (defined even though it is virtual)
NcAtt::~NcAtt() {}

// assignment operator
NcAtt& NcAtt::operator=(const NcAtt& rhs)
{
  nullObject = rhs.nullObject;
  myName = rhs.myName;
  groupId = rhs.groupId;
  varId =rhs.varId;
  return *this;
}

// Constructor generates a null object.
NcAtt::NcAtt() : 
  nullObject(true) 
{}

// Constructor for non-null instances.
NcAtt::NcAtt(bool nullObject): 
  nullObject(nullObject)
{}

// The copy constructor.
NcAtt::NcAtt(const NcAtt& rhs) :
  nullObject(rhs.nullObject),
  myName(rhs.myName),
  groupId(rhs.groupId),
   varId(rhs.varId)
{}


// equivalence operator
bool NcAtt::operator==(const NcAtt & rhs) const
{
  if(nullObject) 
    return nullObject == rhs.nullObject;
  else
    return myName == rhs.myName && groupId == rhs.groupId && varId == rhs.varId;
}  

//  !=  operator
bool NcAtt::operator!=(const NcAtt & rhs) const
{
  return !(*this == rhs);
}  

// Gets parent group.
netCDF::NcGroup  NcAtt::getParentGroup() const {
  return netCDF::NcGroup(groupId);
}
      

// Returns the attribute type.
NcType  NcAtt::getType() const{
  // get the identifier for the netCDF type of this attribute.
  nc_type xtypep;
  ncCheck(nc_inq_atttype(groupId,varId,myName.c_str(),&xtypep),__FILE__,__LINE__);
  if(xtypep <= 12)
    // This is an atomic type
    return NcType(xtypep);
  else
    // this is a user-defined type
    {
      // now get the set of NcType objects in this file.
      multimap<string,NcType> typeMap(getParentGroup().getTypes(NcGroup::ParentsAndCurrent));
      multimap<string,NcType>::iterator iter;
      // identify the Nctype object with the same id as this attribute.
      for (iter=typeMap.begin(); iter!= typeMap.end();iter++) {
	if(iter->second.getId() == xtypep) return iter->second;
      }
      // return a null object, as no type was identified.
      return NcType();
    }
}

// Gets attribute length.
size_t  NcAtt::getAttLength() const{
  size_t lenp;
  ncCheck(nc_inq_attlen(groupId, varId, myName.c_str(), &lenp),__FILE__,__LINE__);
  return lenp;
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(string& dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());

  size_t att_len=getAttLength();
  char* tmpValues;
  tmpValues = (char *) malloc(att_len + 1);  /* + 1 for trailing null */

  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),tmpValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_text(groupId,varId,myName.c_str(),tmpValues),__FILE__,__LINE__);
  dataValues=string(tmpValues,att_len);
  free(tmpValues);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_text(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}


// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_uchar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_schar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_short(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_int(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_long(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_float(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_double(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_ushort(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_uint(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_longlong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_ulonglong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_att_string(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(void* dataValues) const {
  ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

