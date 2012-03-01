#include "ncGroup.h"
#include "ncVar.h"
#include "ncDim.h"
#include "ncVlenType.h"
#include "ncCompoundType.h"
#include "ncOpaqueType.h"
#include "ncGroupAtt.h"
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
#include <ncException.h>
#include "ncCheck.h"
using namespace std;
using namespace netCDF::exceptions;

namespace netCDF {
  //  Global comparator operator ==============
  // comparator operator 
  bool operator<(const NcGroup& lhs,const NcGroup& rhs)
  {
    return false;
  }
  
  // comparator operator 
  bool operator>(const NcGroup& lhs,const NcGroup& rhs)
  {
    return true;
  }
}

using namespace netCDF;
    
/////////////////////////////////////////////

NcGroup::~NcGroup()
{
}

// Constructor generates a null object.
NcGroup::NcGroup() :
  nullObject(true)
{}

   
// constructor
NcGroup::NcGroup(const int groupId) :
  nullObject(false),
  myId(groupId)
{ }

// assignment operator
NcGroup& NcGroup::operator=(const NcGroup & rhs)
{
  nullObject = rhs.nullObject;
  myId = rhs.myId;
  return *this;
}

// The copy constructor.
NcGroup::NcGroup(const NcGroup& rhs): 
  nullObject(rhs.nullObject),
  myId(rhs.myId)
{}


// equivalence operator
bool NcGroup::operator==(const NcGroup & rhs) const
{
  if(nullObject) 
    return nullObject == rhs.nullObject;
  else
    return myId == rhs.myId;
}  
  
//  !=  operator
bool NcGroup::operator!=(const NcGroup & rhs) const
{
  return !(*this == rhs);
}  
  
  
// /////////////
// NcGroup-related methods
// /////////////
  
// Get the group name.
string NcGroup::getName(bool fullName) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getName on a Null group",__FILE__,__LINE__);
  string groupName;
  if(fullName){
    // return full name of group with foward "/" separarating sub-groups.
    size_t lenp;
    ncCheck(nc_inq_grpname_len(myId,&lenp),__FILE__,__LINE__);
    char* charName= new char[lenp+1];
    ncCheck(nc_inq_grpname_full(myId,&lenp,charName),__FILE__,__LINE__);
    groupName = charName;
    delete charName;
  }
  else {
    // return the (local) name of this group.
    char charName[NC_MAX_NAME+1];
    ncCheck(nc_inq_grpname(myId,charName),__FILE__,__LINE__);
    groupName = charName;
  }
  return groupName;
}

// returns true if this is the root group.
bool NcGroup::isRootGroup()  const{
  bool result = getName() == "/";
  return result;
}
  
// Get the parent group.
NcGroup NcGroup::getParentGroup() const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getParentGroup on a Null group",__FILE__,__LINE__);
  try {
    int parentId;
    ncCheck(nc_inq_grp_parent(myId,&parentId),__FILE__,__LINE__);
    NcGroup ncGroupParent(parentId);
    return ncGroupParent;
  }
  catch (NcEnoGrp& e) {
    // no group found, so return null group
    return NcGroup();
  }
}
  
  
// Get the group id.
int  NcGroup::getId() const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getId on a Null group",__FILE__,__LINE__);
  return myId;
}
  
// Get the number of NcGroup objects.
int NcGroup::getGroupCount(NcGroup::GroupLocation location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getGroupCount on a Null group",__FILE__,__LINE__);
  // initialize group counter
  int ngroups=0;

  // record this group
  if(location == ParentsAndCurrentGrps || location == AllGrps) {
    ngroups ++;
  }

  // number of children in current group
  if(location == ChildrenGrps || location == AllChildrenGrps || location == AllGrps ) {
    int numgrps;
    int* ncids=NULL;
    ncCheck(nc_inq_grps(getId(), &numgrps,ncids),__FILE__,__LINE__);
    ngroups += numgrps;
  }

  // search in parent groups
  if(location == ParentsGrps || location == ParentsAndCurrentGrps || location == AllGrps ) {
    multimap<string,NcGroup> groups(getGroups(ParentsGrps));
    ngroups += groups.size();
  }


  // get the number of all children that are childreof children
  if(location == ChildrenOfChildrenGrps || location == AllChildrenGrps || location == AllGrps ) {
    multimap<string,NcGroup> groups(getGroups(ChildrenOfChildrenGrps));
    ngroups += groups.size();
  }

  return ngroups;
}

  
// Get the set of child NcGroup objects.
multimap<std::string,NcGroup> NcGroup::getGroups(NcGroup::GroupLocation location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getGroups on a Null group",__FILE__,__LINE__);
  // create a container to hold the NcGroup's.
  multimap<string,NcGroup> ncGroups;

  // record this group
  if(location == ParentsAndCurrentGrps || location == AllGrps) {
    ncGroups.insert(pair<const string,NcGroup>(getName(),*this));
  }

  // the child groups of the current group
  if(location == ChildrenGrps || location == AllChildrenGrps || location == AllGrps ) {
    // get the number of groups
    int groupCount = getGroupCount();
    vector<int> ncids(groupCount);
    int* numgrps=NULL;
    // now get the id of each NcGroup and populate the ncGroups container.
    ncCheck(nc_inq_grps(myId, numgrps,&ncids[0]),__FILE__,__LINE__);
    for(int i=0; i<groupCount;i++){
      NcGroup tmpGroup(ncids[i]);
      ncGroups.insert(pair<const string,NcGroup>(tmpGroup.getName(),tmpGroup));
    }
  }

  // search in parent groups.
  if(location == ParentsGrps || location == ParentsAndCurrentGrps || location == AllGrps ) {
    NcGroup tmpGroup(*this); 
    if(!tmpGroup.isRootGroup()) {
      while(1) {
	const NcGroup parentGroup(tmpGroup.getParentGroup());
	if(parentGroup.isNull()) break;
	ncGroups.insert(pair<const string,NcGroup>(parentGroup.getName(),parentGroup));
	tmpGroup=parentGroup;
      }
    }
  }
 
  // search in child groups of the children
  if(location == ChildrenOfChildrenGrps || location == AllChildrenGrps || location == AllGrps ) {
    map<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(ChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcGroup> childGroups(it->second.getGroups(AllChildrenGrps));
      ncGroups.insert(childGroups.begin(),childGroups.end());
    }
  }
  
  return ncGroups;
}
  
// Get the named child NcGroup object.
NcGroup NcGroup::getGroup(const string& name,NcGroup::GroupLocation location) const{
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getGroup on a Null group",__FILE__,__LINE__);
  multimap<string,NcGroup> ncGroups(getGroups(location));
  pair<multimap<string,NcGroup>::iterator,multimap<string,NcGroup>::iterator> ret;
  ret = ncGroups.equal_range(name);
  if(ret.first == ret.second) 
    return NcGroup();  // null group is returned
  else 
    return ret.first->second;
}

  

// Get all NcGroup objects with a given name.
set<NcGroup> NcGroup::getGroups(const std::string& name,NcGroup::GroupLocation location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getGroups on a Null group",__FILE__,__LINE__);
  // get the set of ncGroups in this group and above.
  multimap<std::string,NcGroup> ncGroups(getGroups(location));
  pair<multimap<string,NcGroup>::iterator,multimap<string,NcGroup>::iterator> ret;
  multimap<string,NcGroup>::iterator it;
  ret = ncGroups.equal_range(name);
  set<NcGroup> tmpGroup;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpGroup.insert(it->second);
  }
  return tmpGroup;
}

// Add a new child NcGroup object.
NcGroup NcGroup::addGroup(const string& name) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::addGroup on a Null group",__FILE__,__LINE__);
  int new_ncid;
  ncCheck(nc_def_grp(myId,const_cast<char*> (name.c_str()),&new_ncid),__FILE__,__LINE__);
  return NcGroup(new_ncid);
}


  
// /////////////
// NcVar-related accessors
// /////////////
  
// Get the number of NcVar objects in this group.
int NcGroup::getVarCount(NcGroup::Location location) const {

  // search in current group.
  NcGroup tmpGroup(*this); 
  int nvars=0;
  // search in current group
  if((location == ParentsAndCurrent || location == ChildrenAndCurrent || location == Current || location ==All) && !tmpGroup.isNull()) {
    ncCheck(nc_inq_nvars(tmpGroup.getId(), &nvars),__FILE__,__LINE__);
  }

  // search recursively in all parent groups.
  if(location == Parents || location == ParentsAndCurrent || location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      int nvarsp;
      ncCheck(nc_inq_nvars(tmpGroup.getId(), &nvarsp),__FILE__,__LINE__);
      nvars += nvarsp;
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recursively in all child groups
  if(location == ChildrenAndCurrent || location == Children || location == All) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      nvars += it->second.getVarCount(ChildrenAndCurrent);
    }
  }
  return nvars;
}

// Get the collection of NcVar objects.
multimap<std::string,NcVar> NcGroup::getVars(NcGroup::Location location) const {

  // create a container to hold the NcVar's.
  multimap<string,NcVar> ncVars;

  // search in current group.
  NcGroup tmpGroup(*this); 
  if((location == ParentsAndCurrent || location == ChildrenAndCurrent || location == Current || location ==All) && !tmpGroup.isNull()) {
    // get the number of variables.
    int varCount = getVarCount();
    // now get the name of each NcVar object and populate the ncVars container.
    int* nvars=NULL;
    vector<int> varids(varCount);
    ncCheck(nc_inq_varids(myId, nvars,&varids[0]),__FILE__,__LINE__);
    for(int i=0; i<varCount;i++){
      NcVar tmpVar(*this,varids[i]);
      ncVars.insert(pair<const string,NcVar>(tmpVar.getName(),tmpVar));
    }
  }

  
  // search recursively in all parent groups.
  if(location == Parents || location == ParentsAndCurrent || location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      // get the number of variables
      int varCount = tmpGroup.getVarCount();
      // now get the name of each NcVar object and populate the ncVars container.
      int* nvars=NULL;
      vector<int> varids(varCount);
      ncCheck(nc_inq_varids(tmpGroup.getId(), nvars,&varids[0]),__FILE__,__LINE__);
      for(int i=0; i<varCount;i++){
	NcVar tmpVar(tmpGroup,varids[i]);
	ncVars.insert(pair<const string,NcVar>(tmpVar.getName(),tmpVar));
      }
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recusively in all child groups.
  if(location == ChildrenAndCurrent || location == Children  || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcVar> vars=it->second.getVars(ChildrenAndCurrent);
      ncVars.insert(vars.begin(),vars.end());
    }
  }

  return ncVars;
}
  

// Get all NcVar objects with a given name.
set<NcVar> NcGroup::getVars(const string& name,NcGroup::Location location) const {
  // get the set of ncVars in this group and above.
  multimap<std::string,NcVar> ncVars(getVars(location));
  pair<multimap<string,NcVar>::iterator,multimap<string,NcVar>::iterator> ret;
  multimap<string,NcVar>::iterator it;
  ret = ncVars.equal_range(name);
  set<NcVar> tmpVar;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpVar.insert(it->second);
  }
  return tmpVar;
}



// Get the named NcVar object.
NcVar NcGroup::getVar(const string& name,NcGroup::Location location) const {
  multimap<std::string,NcVar> ncVars(getVars(location));
  pair<multimap<string,NcVar>::iterator,multimap<string,NcVar>::iterator> ret;
  ret = ncVars.equal_range(name);
  if(ret.first == ret.second) 
    // no matching netCDF variable found so return null object.
    return NcVar();
  else 
    return ret.first->second;
}


// Add a new netCDF variable.
NcVar NcGroup::addVar(const string& name, const string& typeName, const string& dimName) const {
    
  // get an NcType object with the given type name.
  NcType tmpType(getType(typeName,NcGroup::ParentsAndCurrent));
  if(tmpType.isNull()) throw NcNullType("Attempt to invoke NcGroup::addVar failed: typeName must be defined in either the current group or a parent group",__FILE__,__LINE__);

  // get a NcDim object with the given dimension name
  NcDim tmpDim(getDim(dimName,NcGroup::ParentsAndCurrent));
  if(tmpDim.isNull()) throw NcNullDim("Attempt to invoke NcGroup::addVar failed: dimName must be defined in either the current group or a parent group",__FILE__,__LINE__);

  // finally define a new netCDF  variable
  int varId;
  int dimId(tmpDim.getId());
  ncCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),1,&dimId,&varId),__FILE__,__LINE__);
  // return an NcVar object for this new variable
  return NcVar(*this,varId);
}


// Add a new netCDF variable.
NcVar NcGroup::addVar(const string& name, const NcType& ncType, const NcDim& ncDim) const {
    
  // check NcType object is valid
  if(ncType.isNull()) throw NcNullType("Attempt to invoke NcGroup::addVar with a Null NcType object",__FILE__,__LINE__);
  NcType tmpType(getType(ncType.getName(),NcGroup::ParentsAndCurrent));
  if(tmpType.isNull()) throw NcNullType("Attempt to invoke NcGroup::addVar failed: NcType must be defined in either the current group or a parent group",__FILE__,__LINE__);
  
  // check NcDim object is valid
  if(ncDim.isNull()) throw NcNullDim("Attempt to invoke NcGroup::addVar with a Null NcDim object",__FILE__,__LINE__);
  NcDim tmpDim(getDim(ncDim.getName(),NcGroup::ParentsAndCurrent));
  if(tmpDim.isNull()) throw NcNullDim("Attempt to invoke NcGroup::addVar failed: NcDim must be defined in either the current group or a parent group",__FILE__,__LINE__);
  
  // finally define a new netCDF variable
  int varId;
  int dimId(tmpDim.getId());
  ncCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),1,&dimId,&varId),__FILE__,__LINE__);
  // return an NcVar object for this new variable
  return NcVar(*this,varId);
}


// Add a new netCDF multi-dimensional variable.
NcVar NcGroup::addVar(const string& name, const string& typeName, const vector<string>& dimNames) const {
    
  // get an NcType object with the given name.
  NcType tmpType(getType(typeName,NcGroup::ParentsAndCurrent));
  if(tmpType.isNull()) throw NcNullType("Attempt to invoke NcGroup::addVar failed: typeName must be defined in either the current group or a parent group",__FILE__,__LINE__);

  // get a set of NcDim objects corresponding to the given dimension names.
  vector<int> dimIds;
  dimIds.reserve(dimNames.size());
  for (size_t i=0; i<dimNames.size();i++){
    NcDim tmpDim(getDim(dimNames[i],NcGroup::ParentsAndCurrent));
    if(tmpDim.isNull()) throw NcNullDim("Attempt to invoke NcGroup::addVar failed: dimNames must be defined in either the current group or a parent group",__FILE__,__LINE__);
    dimIds.push_back(tmpDim.getId());
  }

  // finally define a new netCDF variable
  int varId;
  ncCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),dimIds.size(),&dimIds[0],&varId),__FILE__,__LINE__);
  // return an NcVar object for this new variable
  return NcVar(*this,varId);
}

// Add a new netCDF multi-dimensional variable.
NcVar NcGroup::addVar(const string& name, const NcType& ncType, const vector<NcDim>& ncDimVector) const {
    
  // check NcType object is valid
  if(ncType.isNull()) throw NcNullType("Attempt to invoke NcGroup::addVar with a Null NcType object",__FILE__,__LINE__);
  NcType tmpType(getType(ncType.getName(),NcGroup::ParentsAndCurrent));
  if(tmpType.isNull()) throw NcNullType("Attempt to invoke NcGroup::addVar failed: NcType must be defined in either the current group or a parent group",__FILE__,__LINE__);
  
  // check NcDim objects are valid
  vector<NcDim>::const_iterator iter;
  vector<int> dimIds;
  dimIds.reserve(ncDimVector.size());
  for (iter=ncDimVector.begin();iter < ncDimVector.end(); iter++) {
    if(iter->isNull()) throw NcNullDim("Attempt to invoke NcGroup::addVar with a Null NcDim object",__FILE__,__LINE__);
    NcDim tmpDim(getDim(iter->getName(),NcGroup::ParentsAndCurrent));
    if(tmpDim.isNull()) throw NcNullDim("Attempt to invoke NcGroup::addVar failed: NcDim must be defined in either the current group or a parent group",__FILE__,__LINE__);
    dimIds.push_back(tmpDim.getId());
  }

  // finally define a new netCDF variable
  int varId;
  ncCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),dimIds.size(),&dimIds[0],&varId),__FILE__,__LINE__);
  // return an NcVar object for this new variable
  return NcVar(*this,varId);
}


// /////////////
// NcAtt-related methods
// /////////////
  
// Get the number of group attributes.
int NcGroup::getAttCount(NcGroup::Location location) const {

  // search in current group.
  NcGroup tmpGroup(*this); 
  int ngatts=0;
  // search in current group
  if((location == ParentsAndCurrent || location == ChildrenAndCurrent || location == Current || location ==All) && !tmpGroup.isNull()) {
    ncCheck(nc_inq_natts(tmpGroup.getId(), &ngatts),__FILE__,__LINE__);
  }

  // search recursively in all parent groups.
  if(location == Parents || location == ParentsAndCurrent || location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      int ngattsp;
      ncCheck(nc_inq_natts(tmpGroup.getId(), &ngattsp),__FILE__,__LINE__);
      ngatts += ngattsp;
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recursively in all child groups
  if(location == ChildrenAndCurrent || location == Children || location == All) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      ngatts += it->second.getAttCount(ChildrenAndCurrent);
    }
  }

  return ngatts;
}
  
// Get the collection of NcGroupAtt objects.
multimap<std::string,NcGroupAtt> NcGroup::getAtts(NcGroup::Location location) const {

  // create a container to hold the NcAtt's.
  multimap<string,NcGroupAtt> ncAtts;

  // search in current group.
  NcGroup tmpGroup(*this); 
  if((location == ParentsAndCurrent || location == ChildrenAndCurrent || location == Current || location ==All) && !tmpGroup.isNull()) {
    // get the number of attributes
    int attCount = tmpGroup.getAttCount();
    // now get the name of each NcAtt and populate the ncAtts container.
    for(int i=0; i<attCount;i++){
      char charName[NC_MAX_NAME+1];
      ncCheck(nc_inq_attname(tmpGroup.getId(),NC_GLOBAL,i,charName),__FILE__,__LINE__);
      NcGroupAtt tmpAtt(tmpGroup.getId(),i);
      ncAtts.insert(pair<const string,NcGroupAtt>(string(charName),tmpAtt));
    }
  }
  
  // search recursively in all parent groups.
  if(location == Parents || location == ParentsAndCurrent || location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      // get the number of attributes
      int attCount = tmpGroup.getAttCount();
      // now get the name of each NcAtt and populate the ncAtts container.
      for(int i=0; i<attCount;i++){
	char charName[NC_MAX_NAME+1];
	ncCheck(nc_inq_attname(tmpGroup.getId(),NC_GLOBAL,i,charName),__FILE__,__LINE__);
	NcGroupAtt tmpAtt(tmpGroup.getId(),i);
	ncAtts.insert(pair<const string,NcGroupAtt>(string(charName),tmpAtt));
      }
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recusively in all child groups.
  if(location == ChildrenAndCurrent || location == Children  || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcGroupAtt> atts=it->second.getAtts(ChildrenAndCurrent);
      ncAtts.insert(atts.begin(),atts.end());
    }
  }

  return ncAtts;
}
  
// Get the named NcGroupAtt object.
NcGroupAtt NcGroup::getAtt(const std::string& name,NcGroup::Location location) const {
  multimap<std::string,NcGroupAtt> ncAtts(getAtts(location));
  pair<multimap<string,NcGroupAtt>::iterator,multimap<string,NcGroupAtt>::iterator> ret;
  ret = ncAtts.equal_range(name);
  if(ret.first == ret.second) 
    // no matching groupAttribute so return null object.
    return NcGroupAtt();
  else 
    return ret.first->second;
}

// Get all NcGroupAtt objects with a given name.
set<NcGroupAtt> NcGroup::getAtts(const string& name,NcGroup::Location location) const {
  // get the set of ncGroupAtts in this group and above.
  multimap<std::string,NcGroupAtt> ncAtts(getAtts(location));
  pair<multimap<string,NcGroupAtt>::iterator,multimap<string,NcGroupAtt>::iterator> ret;
  multimap<string,NcGroupAtt>::iterator it;
  ret = ncAtts.equal_range(name);
  set<NcGroupAtt> tmpAtt;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpAtt.insert(it->second);
  }
  return tmpAtt;
}




//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const string& dataValues) const {
  ncCheck(nc_put_att_text(myId,NC_GLOBAL,name.c_str(),dataValues.size(),dataValues.c_str()),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const unsigned char* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_uchar(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const signed char* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_schar(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, short datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_short(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, int datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_int(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, long datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_long(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, float datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_float(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, double datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_double(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, unsigned short datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ushort(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, unsigned int datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_uint(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, long long datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_longlong(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, unsigned long long datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ulonglong(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const short* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_short(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const int* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_int(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const long* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_long(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const float* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_float(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const double* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_double(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const unsigned short* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ushort(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const unsigned int* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_uint(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const long long* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_longlong(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const unsigned long long* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ulonglong(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcGroupAtt NcGroup::putAtt(const string& name, size_t len, const char** dataValues) const {
  ncCheck(nc_put_att_string(myId,NC_GLOBAL,name.c_str(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
 NcGroupAtt NcGroup::putAtt(const string& name, const NcType& type, size_t len, const void* dataValues) const {
  ncCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


  
// /////////////
// NcDim-related methods
// /////////////
  
// Get the number of NcDim objects.
int NcGroup::getDimCount(NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getDimCount on a Null group",__FILE__,__LINE__);

  // intialize counter
  int ndims=0;

  // search in current group
  if(location == Current || location == ParentsAndCurrent || location == ChildrenAndCurrent || location == All ) {
    int ndimsp;
    ncCheck(nc_inq_ndims(getId(), &ndimsp),__FILE__,__LINE__);
    ndims += ndimsp;
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ndims += it->second.getDimCount();
    }
  }
  
  // search in child groups.
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ndims += it->second.getDimCount();
    }
  }
  return ndims;
}


// Get the set of NcDim objects.
multimap<string,NcDim> NcGroup::getDims(NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getDims on a Null group",__FILE__,__LINE__);
  // create a container to hold the NcDim's.
  multimap<string,NcDim> ncDims;

  // search in current group
  if(location == Current || location == ParentsAndCurrent || location == ChildrenAndCurrent || location == All ) {
    int dimCount = getDimCount();
    vector<int> dimids(dimCount);
    ncCheck(nc_inq_dimids(getId(),&dimCount,&dimids[0],0),__FILE__,__LINE__);
    // now get the name of each NcDim and populate the nDims container.
    for(int i=0; i<dimCount;i++){
      NcDim tmpDim(*this,dimids[i]); 
      ncDims.insert(pair<const string,NcDim>(tmpDim.getName(),tmpDim));
    }
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcDim> dimTmp(it->second.getDims());
      ncDims.insert(dimTmp.begin(),dimTmp.end());
    }
  }

  // search in child groups (makes recursive calls).
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcDim> dimTmp(it->second.getDims());
      ncDims.insert(dimTmp.begin(),dimTmp.end());
    }
  }

  return ncDims;
}



// Get the named NcDim object.
NcDim NcGroup::getDim(const string& name,NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getDim on a Null group",__FILE__,__LINE__);
  multimap<string,NcDim> ncDims(getDims(location));
  pair<multimap<string,NcDim>::iterator,multimap<string,NcDim>::iterator> ret;
  ret = ncDims.equal_range(name);
  if(ret.first == ret.second) 
    return NcDim(); // null group is returned
  else 
    return ret.first->second;
}


// Get all NcDim objects with a given name.
set<NcDim> NcGroup::getDims(const string& name,NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getDims on a Null group",__FILE__,__LINE__);
  // get the set of ncDims in this group and above.
  multimap<string,NcDim> ncDims(getDims(location));
  pair<multimap<string,NcDim>::iterator,multimap<string,NcDim>::iterator> ret;
  multimap<string,NcDim>::iterator it;
  ret = ncDims.equal_range(name);
  set<NcDim> tmpDim;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpDim.insert(it->second);
  }
  return tmpDim;
}

// Add a new NcDim object.
NcDim NcGroup::addDim(const string& name, size_t dimSize) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::addDim on a Null group",__FILE__,__LINE__);
  int dimId;
  ncCheck(nc_def_dim(myId,name.c_str(),dimSize,&dimId),__FILE__,__LINE__);
  // finally return NcDim object for this new variable
  return NcDim(*this,dimId);
}

// Add a new NcDim object with unlimited size..
NcDim NcGroup::addDim(const string& name) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::addDim on a Null group",__FILE__,__LINE__);
  int dimId;
  ncCheck(nc_def_dim(myId,name.c_str(),NC_UNLIMITED,&dimId),__FILE__,__LINE__);
  // finally return NcDim object for this new variable
  return NcDim(*this,dimId);
}





// /////////////
// type-object related methods
// /////////////

// Gets the number of type objects.
int NcGroup::getTypeCount(NcGroup::Location location) const {

  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getTypeCount on a Null group",__FILE__,__LINE__);

  // intialize counter
  int ntypes=0;

  // search in current group
  if(location == Current || location == ParentsAndCurrent || location == ChildrenAndCurrent || location == All ) {
    int ntypesp;
    int* typeidsp=NULL;
    ncCheck(nc_inq_typeids(getId(), &ntypesp,typeidsp),__FILE__,__LINE__);
    ntypes+= ntypesp;
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount();
    }
  }
  
  // search in child groups.
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount();
    }
  }
  return ntypes;
}


  
// Gets the number of type objects with a given enumeration type.
int NcGroup::getTypeCount(NcType::ncType enumType, NcGroup::Location location) const {

  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getTypeCount on a Null group",__FILE__,__LINE__);

  // intialize counter
  int ntypes=0;

  // search in current group
  if(location == Current || location == ParentsAndCurrent || location == ChildrenAndCurrent || location == All ) {
    int ntypesp;
    int* typeidsp=NULL;
    ncCheck(nc_inq_typeids(getId(), &ntypesp,typeidsp),__FILE__,__LINE__);
    vector<int> typeids(ntypesp);
    ncCheck(nc_inq_typeids(getId(), &ntypesp,&typeids[0]),__FILE__,__LINE__);
    for (int i=0; i<ntypesp;i++){
      NcType tmpType(*this,typeids[i]);
      if(tmpType.getTypeClass() == enumType) ntypes++;
    }
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount(enumType);
    }
  }
  
  // search in child groups.
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount(enumType);
    }
  }
  return ntypes;
}


// Gets the collection of NcType objects.
multimap<string,NcType> NcGroup::getTypes(NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getTypes on a Null group",__FILE__,__LINE__);
  // create a container to hold the NcType's.
  multimap<string,NcType> ncTypes;

  // search in current group
  if(location == Current || location == ParentsAndCurrent || location == ChildrenAndCurrent || location == All ) {
    int typeCount = getTypeCount();
    vector<int> typeids(typeCount);
    ncCheck(nc_inq_typeids(getId(), &typeCount,&typeids[0]),__FILE__,__LINE__);
    // now get the name of each NcType and populate the nTypes container.
    for(int i=0; i<typeCount;i++){
      NcType tmpType(*this,typeids[i]); 
      ncTypes.insert(pair<const string,NcType>(tmpType.getName(),tmpType));
    }
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcType> typeTmp(it->second.getTypes());
      ncTypes.insert(typeTmp.begin(),typeTmp.end());
    }
  }

  // search in child groups (makes recursive calls).
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcType> typeTmp(it->second.getTypes());
      ncTypes.insert(typeTmp.begin(),typeTmp.end());
    }
  }

  return ncTypes;
}


// Gets the collection of NcType objects with a given name.
set<NcType> NcGroup::getTypes(const string& name, NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getTypes on a Null group",__FILE__,__LINE__);
  // iterator for the multimap container.
  multimap<string,NcType>::iterator it;
  // return argument of equal_range: iterators to lower and upper bounds of the range.
  pair<multimap<string,NcType>::iterator,multimap<string,NcType>::iterator> ret;
  // get the entire collection of types.
  multimap<string,NcType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcType> tmpType;
  // get the set of NcType objects with a given name
  ret=types.equal_range(name);
  for (it=ret.first;it!=ret.second;it++) {
    tmpType.insert(it->second);
  }
  return tmpType;
}


// Gets the collection of NcType objects with a given data type.
set<NcType> NcGroup::getTypes(NcType::ncType enumType, NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getTypes on a Null group",__FILE__,__LINE__);
  // iterator for the multimap container.
  multimap<string,NcType>::iterator it;
  // get the entire collection of types.
  multimap<string,NcType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcType> tmpType;
  // get the set of NcType objects with a given data type
  for (it=types.begin();it!=types.end();it++) {
    if(it->second.getTypeClass() == enumType) {
      tmpType.insert(it->second);
    }
  }
  return(tmpType);
}


// Gets the collection of NcType objects with a given name and data type.
set<NcType> NcGroup::getTypes(const string& name, NcType::ncType enumType, NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getTypes on a Null group",__FILE__,__LINE__);
  // iterator for the multimap container.
  multimap<string,NcType>::iterator it;
  // return argument of equal_range: iterators to lower and upper bounds of the range.
  pair<multimap<string,NcType>::iterator,multimap<string,NcType>::iterator> ret;
  // get the entire collection of types.
  multimap<string,NcType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcType> tmpType;
  // get the set of NcType objects with a given name
  ret=types.equal_range(name);
  for (it=ret.first;it!=ret.second;it++) {
    if((*it).second.getTypeClass() == enumType) {
      tmpType.insert(it->second);
    }
  }
  return(tmpType);
}


// Gets the NcType object with a given name.
NcType NcGroup::getType(const string& name, NcGroup::Location location) const {
  if(isNull()) throw NcNullGrp("Attempt to invoke NcGroup::getType on a Null group",__FILE__,__LINE__);
  if(name ==  "byte"    ) return ncByte;
  if(name ==  "ubyte"   ) return ncUbyte;
  if(name ==  "char"    ) return ncChar;
  if(name ==  "short"   ) return ncShort;
  if(name ==  "ushort"  ) return ncUshort;
  if(name ==  "int"     ) return ncInt;
  if(name ==  "uint"    ) return ncUint;  
  if(name ==  "int64"   ) return ncInt64; 
  if(name ==  "uint64"  ) return ncUint64;
  if(name ==  "float"   ) return ncFloat;
  if(name ==  "double"  ) return ncDouble;
  if(name ==  "string"  ) return ncString;

  // this is a user defined type
  // iterator for the multimap container.
  multimap<string,NcType>::iterator it;
  // return argument of equal_range: iterators to lower and upper bounds of the range.
  pair<multimap<string,NcType>::iterator,multimap<string,NcType>::iterator> ret;
  // get the entire collection of types.
  multimap<string,NcType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcType> tmpType;
    // get the set of NcType objects with a given name
  ret=types.equal_range(name);
  if(ret.first == ret.second) 
    return NcType();
  else
    return ret.first->second;
}


// Adds a new netCDF Enum type.
NcEnumType NcGroup::addEnumType(const string& name,NcEnumType::ncEnumType baseType) const {
  nc_type typeId;
  ncCheck(nc_def_enum(myId, baseType, name.c_str(), &typeId),__FILE__,__LINE__);
  NcEnumType ncTypeTmp(*this,name);
  return ncTypeTmp;
}


// Adds a new netCDF Vlen type.
NcVlenType NcGroup::addVlenType(const string& name,NcType& baseType) const {
  nc_type typeId;
  ncCheck(nc_def_vlen(myId,  const_cast<char*>(name.c_str()),baseType.getId(),&typeId),__FILE__,__LINE__);
  NcVlenType ncTypeTmp(*this,name);
  return ncTypeTmp;
}


// Adds a new netCDF Opaque type.
NcOpaqueType NcGroup::addOpaqueType(const string& name, size_t size) const {
  nc_type typeId;
  ncCheck(nc_def_opaque(myId, size,const_cast<char*>(name.c_str()), &typeId),__FILE__,__LINE__);
  NcOpaqueType ncTypeTmp(*this,name);
  return ncTypeTmp;
}
    
// Adds a new netCDF UserDefined type.
NcCompoundType NcGroup::addCompoundType(const string& name, size_t size) const {
  nc_type typeId;
  ncCheck(nc_def_compound(myId, size,const_cast<char*>(name.c_str()),&typeId),__FILE__,__LINE__);
  NcCompoundType ncTypeTmp(*this,name);
  return ncTypeTmp;
}
  
  
// Get the collection of coordinate variables.
map<string,NcGroup> NcGroup::getCoordVars(NcGroup::Location location) const {
  map<string,NcGroup> coordVars;

  // search in current group and parent groups.
  NcGroup tmpGroup(*this); 
  multimap<string,NcDim>::iterator itD;
  map<string,NcVar>::iterator itV;
  while(1) {
    // get the collection of NcDim objects defined in this group.
    multimap<string,NcDim> dimTmp(tmpGroup.getDims());
    multimap<string,NcVar> varTmp(tmpGroup.getVars());
    for (itD=dimTmp.begin();itD!=dimTmp.end();itD++) {
      string coordName(itD->first);
      itV = varTmp.find(coordName);
      if(itV != varTmp.end()) {
	coordVars.insert(pair<const string,NcGroup>(string(coordName),tmpGroup));
      }
    }
    if(location != ParentsAndCurrent || location != All || tmpGroup.isRootGroup()) {
      break;
    }
    // continue loop with the parent.
    tmpGroup=tmpGroup.getParentGroup();
  }

  // search in child groups (makes recursive calls).
  if(location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      map<string,NcGroup> coordVarsTmp=getCoordVars(ChildrenAndCurrent);
      coordVars.insert(coordVarsTmp.begin(),coordVarsTmp.end());
    }
  }
  
  return coordVars;
}

// Get the NcDim and NcVar object pair for a named coordinate variables.
void NcGroup::getCoordVar(string& coordVarName, NcDim& ncDim, NcVar& ncVar, NcGroup::Location location) const {

  // search in current group and parent groups.
  multimap<string,NcDim>::iterator itD;
  NcGroup tmpGroup(*this); 
  map<string,NcVar>::iterator itV;
  while(1) {
    // get the collection of NcDim objects defined in this group.
    multimap<string,NcDim> dimTmp(tmpGroup.getDims());
    multimap<string,NcVar> varTmp(tmpGroup.getVars());
    itD=dimTmp.find(coordVarName);
    itV=varTmp.find(coordVarName);
    if(itD != dimTmp.end() && itV != varTmp.end()) {
      ncDim=itD->second;
      ncVar=itV->second;
      return;
    }
    if(location != ParentsAndCurrent || location != All || tmpGroup.isRootGroup()) {
      break;
    }
    // continue loop with the parent.
    tmpGroup=tmpGroup.getParentGroup();
  }

  // search in child groups (makes recursive calls).
  if(location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcGroup>::iterator it;
    multimap<string,NcGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      getCoordVar(coordVarName,ncDim,ncVar,ChildrenAndCurrent);
      if(!ncDim.isNull()) break;
    }
  }

  if(ncDim.isNull()) {
    // return null objects as no coordinates variables were obtained.
    NcDim dimTmp;
    NcVar varTmp;
    ncDim=dimTmp;
    ncVar=varTmp;
    return;
  }

}
