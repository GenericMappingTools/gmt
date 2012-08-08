#include "ncVarAtt.h"
#include "ncDim.h"
#include "ncVar.h"
#include "ncGroup.h"
#include "ncCheck.h"
#include "ncException.h"
#include<netcdf.h>
using namespace std;
using namespace netCDF::exceptions;
  
namespace netCDF {
  //  Global comparator operator ==============
  // comparator operator 
  bool operator<(const NcVar& lhs,const NcVar& rhs)
  {
    return false;
  }
  
  // comparator operator 
  bool operator>(const NcVar& lhs,const NcVar& rhs)
  {
    return true;
  }
}

using namespace netCDF;

// assignment operator
NcVar& NcVar::operator=(const NcVar & rhs)
{
  nullObject = rhs.nullObject;
  myId = rhs.myId;
  groupId = rhs.groupId;
  return *this;
}

// The copy constructor.
NcVar::NcVar(const NcVar& rhs) : 
  nullObject(rhs.nullObject),
  myId(rhs.myId),
  groupId(rhs.groupId)
{}


// equivalence operator
bool NcVar::operator==(const NcVar & rhs) const
{
  // simply check the netCDF id.
  return (myId == rhs.myId);
}  

//  !=  operator
bool NcVar::operator!=(const NcVar & rhs) const
{
  return !(*this == rhs);
}  

/////////////////
  
// Constructors and intialization
  
/////////////////
  
// Constructor generates a null object.
NcVar::NcVar() : nullObject(true) {}
  
// Constructor for a variable (must already exist in the netCDF file.)
NcVar::NcVar (const NcGroup& grp, const int& varId) :
  nullObject (false),
  myId (varId),
  groupId(grp.getId())
{}

  
  
// Gets parent group.
NcGroup  NcVar::getParentGroup() const {
  return NcGroup(groupId);
}
  

// Get the variable id.
int  NcVar::getId() const {return myId;}
    
//////////////////////

//  Information about the variable type

/////////////////////


// Gets the NcType object with a given name.
NcType NcVar::getType() const {

  // if this variable has not been defined, return a NULL type
  if(isNull()) return NcType();

  // first get the typeid
  nc_type xtypep;
  ncCheck(nc_inq_vartype(groupId,myId,&xtypep),__FILE__,__LINE__);

  if(xtypep ==  ncByte.getId()    ) return ncByte;
  if(xtypep ==  ncUbyte.getId()   ) return ncUbyte;
  if(xtypep ==  ncChar.getId()    ) return ncChar;
  if(xtypep ==  ncShort.getId()   ) return ncShort;
  if(xtypep ==  ncUshort.getId()  ) return ncUshort;
  if(xtypep ==  ncInt.getId()     ) return ncInt;
  if(xtypep ==  ncUint.getId()    ) return ncUint;  
  if(xtypep ==  ncInt64.getId()   ) return ncInt64; 
  if(xtypep ==  ncUint64.getId()  ) return ncUint64;
  if(xtypep ==  ncFloat.getId()   ) return ncFloat;
  if(xtypep ==  ncDouble.getId()  ) return ncDouble;
  if(xtypep ==  ncString.getId()  ) return ncString;

  multimap<string,NcType>::const_iterator it;
  multimap<string,NcType> types(NcGroup(groupId).getTypes(NcGroup::ParentsAndCurrent));
  for(it=types.begin(); it!=types.end(); it++) {
    if(it->second.getId() == xtypep) return it->second;
  }
  // we will never reach here
  return true;
}







  
/////////////////
  
// Information about Dimensions
  
/////////////////

  
// Gets the number of dimensions.
int NcVar::getDimCount() const
{
  // get the number of dimensions
  int dimCount;
  ncCheck(nc_inq_varndims(groupId,myId, &dimCount),__FILE__,__LINE__);
  return dimCount;
}  

// Gets the set of Ncdim objects.
vector<NcDim> NcVar::getDims() const
{
  // get the number of dimensions
  int dimCount = getDimCount();  
  // create a vector of dimensions.
  vector<NcDim> ncDims;
  vector<int> dimids(dimCount);
  ncCheck(nc_inq_vardimid(groupId,myId, &dimids[0]),__FILE__,__LINE__);
  ncDims.reserve(dimCount);
  for (int i=0; i<dimCount; i++){
    NcDim tmpDim(getParentGroup(),dimids[i]);
    ncDims.push_back(tmpDim);
  }
  return ncDims;
}  

  
// Gets the i'th NcDim object.
NcDim NcVar::getDim(int i) const
{
  vector<NcDim> ncDims = getDims();
  if((size_t)i >= ncDims.size() | i < 0) throw NcException("NcException","Index out of range",__FILE__,__LINE__);
  return ncDims[i];
}


/////////////////
  
// Information about Attributes
  
/////////////////

  
// Gets the number of attributes.
int NcVar::getAttCount() const
{
  // get the number of attributes
  int attCount;
  ncCheck(nc_inq_varnatts(groupId,myId, &attCount),__FILE__,__LINE__);
  return attCount;
}  

// Gets the set of attributes.
map<string,NcVarAtt> NcVar::getAtts() const
{
  // get the number of attributes
  int attCount = getAttCount();  
  // create a container of attributes.
  map<string,NcVarAtt> ncAtts;
  for (int i=0; i<attCount; i++){
    NcVarAtt tmpAtt(getParentGroup(),*this,i);
    ncAtts.insert(pair<const string,NcVarAtt>(tmpAtt.getName(),tmpAtt));
  }
  return ncAtts;
}  

  
// Gets attribute by name.
NcVarAtt NcVar::getAtt(const string& name) const
{
  map<string,NcVarAtt> attributeList = getAtts();
  map<string,NcVarAtt>::iterator myIter;
  myIter = attributeList.find(name);
  if(myIter == attributeList.end())
    throw NcException("NcException","attribute '"+name+"' not found",__FILE__,__LINE__);
  return NcVarAtt(myIter->second);
}



/////////////////////////


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const string& dataValues) const {
  ncCheck(nc_put_att_text(groupId,myId,name.c_str(),dataValues.size(),dataValues.c_str()),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const unsigned char* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_uchar(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const signed char* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_schar(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}



/////////////////////////////////
// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, short datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_short(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, int datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_int(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, long datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_long(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, float datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_float(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, double datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_double(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, unsigned short datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ushort(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, unsigned int datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_uint(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, long long datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_longlong(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, unsigned long long datumValue) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ulonglong(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


/////////////////////////////////











// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const short* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_short(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const int* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_int(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const long* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_long(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const float* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_float(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const double* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_double(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const unsigned short* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ushort(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const unsigned int* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_uint(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const long long* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_longlong(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const unsigned long long* dataValues) const {
  NcType::ncType typeClass(type.getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_att_ulonglong(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, size_t len, const char** dataValues) const {
  ncCheck(nc_put_att_string(groupId,myId,name.c_str(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcVarAtt NcVar::putAtt(const string& name, const NcType& type, size_t len, const void* dataValues) const {
  ncCheck(nc_put_att(groupId,myId ,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}




////////////////////
  
// Other Basic variable info
  
////////////////////
  
// The name of this variable.
string NcVar::getName() const{
  char charName[NC_MAX_NAME+1];
  ncCheck(nc_inq_varname(groupId, myId, charName),__FILE__,__LINE__);
  return string(charName);
}


////////////////////
  
// Chunking details
  
////////////////////


// Sets chunking parameters.
void NcVar::setChunking(ChunkMode chunkMode, vector<size_t>& chunksizes) const {
  ncCheck(nc_def_var_chunking(groupId,myId,static_cast<int> (chunkMode), &chunksizes[0]),__FILE__,__LINE__);
}
  
  
// Gets the chunking parameters
void NcVar::getChunkingParameters(ChunkMode& chunkMode, vector<size_t>& chunkSizes) const {
  int chunkModeInt;
  chunkSizes.reserve(getDimCount());
    
  ncCheck(nc_inq_var_chunking(groupId,myId,&chunkModeInt, &chunkSizes[0]),__FILE__,__LINE__);
  chunkMode = static_cast<ChunkMode> (chunkModeInt);
}
  
  
  
  
////////////////////
  
// Fill details
  
////////////////////


// Sets the fill parameters
void NcVar::setFill(bool fillMode, void* fillValue) const {
  // If fillMode is enabled, check that fillValue has a legal pointer.
  if(fillMode && fillValue == NULL)
    throw NcException("NcException","FillMode was set to zero but fillValue has invalid pointer",__FILE__,__LINE__);
  
  ncCheck(nc_def_var_fill(groupId,myId,static_cast<int> (!fillMode),fillValue),__FILE__,__LINE__);
}

// Sets the fill parameters
void NcVar::setFill(bool fillMode,const void* fillValue) const {
  setFill(fillMode,const_cast<void*>(fillValue));
}



// Gets the fill parameters
void NcVar::getFillModeParameters(bool& fillMode, void* fillValue)const{
    
  int fillModeInt;
  ncCheck(nc_inq_var_fill(groupId,myId,&fillModeInt,fillValue),__FILE__,__LINE__);
  fillMode= static_cast<bool> (fillModeInt == 0);
}
  

////////////////////
  
// Compression details
  
////////////////////


// Sets the compression parameters
void NcVar::setCompression(bool enableShuffleFilter, bool enableDeflateFilter, int deflateLevel) const {
    
  // Check that the deflate level is legal
  if(enableDeflateFilter & (deflateLevel < 0 | deflateLevel >9))
    throw NcException("NcException","The deflateLevel must be set between 0 and 9.",__FILE__,__LINE__);
    
  ncCheck(nc_def_var_deflate(groupId,myId,
			     static_cast<int> (enableShuffleFilter),
			     static_cast<int> (enableDeflateFilter),
			     deflateLevel),__FILE__,__LINE__);
}
  

// Gets the compression parameters
void NcVar::getCompressionParameters(bool& shuffleFilterEnabled, bool& deflateFilterEnabled, int& deflateLevel) const {
    
  int enableShuffleFilterInt;
  int enableDeflateFilterInt;
  ncCheck(nc_inq_var_deflate(groupId,myId,
			     &enableShuffleFilterInt,
			     &enableDeflateFilterInt,
			     &deflateLevel),__FILE__,__LINE__);
  shuffleFilterEnabled =  static_cast<bool> (enableShuffleFilterInt);
  deflateFilterEnabled =  static_cast<bool> (enableDeflateFilterInt);
}
  


////////////////////
  
// Endianness details
  
////////////////////
      

// Sets the endianness of the variable.
void NcVar::setEndianness(EndianMode endianMode) const {
    
  ncCheck(nc_def_var_endian(groupId,myId,static_cast<int> (endianMode)),__FILE__,__LINE__);
}
  
  
// Gets the endianness of the variable.
NcVar::EndianMode NcVar::getEndianness() const {
    
  int endianInt;
  ncCheck(nc_inq_var_endian(groupId,myId,&endianInt),__FILE__,__LINE__);
  return static_cast<EndianMode> (endianInt);
}
  
      
  
////////////////////
  
// Checksum details
  
////////////////////
  
  
// Sets the checksum parameters of a variable.
void NcVar::setChecksum(ChecksumMode checksumMode) const {
  ncCheck(nc_def_var_fletcher32(groupId,myId,static_cast<int> (checksumMode)),__FILE__,__LINE__);
}
  
  
// Gets the checksum parameters of the variable.
NcVar::ChecksumMode NcVar::getChecksum() const {
  int checksumInt;
  ncCheck(nc_inq_var_fletcher32(groupId,myId,&checksumInt),__FILE__,__LINE__);
  return static_cast<ChecksumMode> (checksumInt);
}
  
  

  
////////////////////
  
//  renaming the variable
  
////////////////////
  
void NcVar::rename( const string& newname ) const 
{
  ncCheck(nc_rename_var(groupId,myId,newname.c_str()),__FILE__,__LINE__);
}





////////////////////

//  data writing

////////////////////

// Writes the entire data into the netCDF variable.
void NcVar::putVar(const char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_text(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_uchar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_schar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_short(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_int(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_long(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_float(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_double(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_ushort(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_uint(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_longlong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_ulonglong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcVar::putVar(const char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var_string(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable with no data conversion.
void NcVar::putVar(const void* dataValues) const {
    ncCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
}


///////////////////
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const char* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    throw NcException("NcException","user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_text(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const unsigned char* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    throw NcException("NcException","user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_uchar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const signed char* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    throw NcException("NcException","user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_schar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const short datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_short(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const int datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_int(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const long datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_long(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const float datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_float(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const double datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_double(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const unsigned short datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_ushort(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const unsigned int datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_uint(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const long long datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_longlong(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const unsigned long long datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_ulonglong(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcVar::putVar(const vector<size_t>& index, const char** datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    throw NcException("NcException","user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncCheck(nc_put_var1_string(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable with no data conversion.
void NcVar::putVar(const vector<size_t>& index, const void* datumValue) const {
    ncCheck(nc_put_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}


////////////////////

// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_text(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_uchar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_schar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_short(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_int(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_long(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_float(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_double(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_ushort(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_uint(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_longlong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_ulonglong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vara_string(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable with no data conversion.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const void* dataValues) const {
    ncCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}



////////////////////

// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_text(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_short(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_int(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_long(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_float(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_double(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_vars_string(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable with no data conversion.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const void* dataValues) const {
    ncCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}


////////////////////
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_text(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_short(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_int(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_long(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_float(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_double(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_put_varm_string(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable with no data conversion.
void NcVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const void* dataValues) const {
    ncCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}



     

// Data reading



// Reads the entire data of the netCDF variable.
void NcVar::getVar(char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_text(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_uchar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_schar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_short(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_int(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_long(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_float(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_double(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_ushort(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_uint(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_longlong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_ulonglong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcVar::getVar(char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var_string(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable with no data conversion.
void NcVar::getVar(void* dataValues) const {
    ncCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
}



///////////

// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, char* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_text(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, unsigned char* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_uchar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, signed char* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_schar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, short* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_short(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, int* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_int(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, long* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_long(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, float* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_float(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, double* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_double(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, unsigned short* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_ushort(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, unsigned int* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_uint(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, long long* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_longlong(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable
void NcVar::getVar(const vector<size_t>& index, unsigned long long* datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_ulonglong(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcVar::getVar(const vector<size_t>& index, char** datumValue) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncCheck(nc_get_var1_string(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable with no data conversion.
void NcVar::getVar(const vector<size_t>& index, void* datumValue) const {
    ncCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}



///////////

// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_text(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_uchar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_schar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_short(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_int(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_long(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_float(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_double(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_ushort(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_uint(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_longlong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_ulonglong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vara_string(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable with no data conversion.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, void* dataValues) const {
    ncCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}


///////////

// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_text(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_short(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_int(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_long(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_float(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_double(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_vars_string(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable with no data conversion.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, void* dataValues) const {
    ncCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}


///////////

// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_text(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, signed char* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_short(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_int(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_long(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, float* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_float(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, double* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_double(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned short* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned int* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned long long* dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, char** dataValues) const {
  NcType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcType::nc_VLEN | typeClass == NcType::nc_OPAQUE | typeClass == NcType::nc_ENUM | typeClass == NcType::nc_COMPOUND) 
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncCheck(nc_get_varm_string(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable with no data conversion.
void NcVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, void* dataValues) const {
    ncCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}

