#include <string>
#include "netcdf.h"

#ifndef NcTypeClass
#define NcTypeClass


namespace netCDF
{
  class NcGroup; // forward declaration to avoid cyclic reference.

  /*! Base class inherited by NcOpaque, NcVlen, NcCompound and NcEnum classes. */
  class NcType
  {	
    
  public:
    
    /*!
      List of netCDF types that can be represented.
      The enumeration list contains the complete set of netCDF variable types. In addition, the type NC_TYPE
      is included. This enables the user to instantiate a netCDF type object without explcitly needing to know
      it precise type.
    */
    enum ncType
    {
      nc_BYTE     = NC_BYTE, 	//!< signed 1 byte integer
      nc_CHAR     = NC_CHAR,	//!< ISO/ASCII character
      nc_SHORT    = NC_SHORT, 	//!< signed 2 byte integer
      nc_INT      = NC_INT,	//!< signed 4 byte integer
      nc_FLOAT    = NC_FLOAT, 	//!< single precision floating point number
      nc_DOUBLE   = NC_DOUBLE, 	//!< double precision floating point number
      nc_UBYTE    = NC_UBYTE,	//!< unsigned 1 byte int
      nc_USHORT   = NC_USHORT,	//!< unsigned 2-byte int
      nc_UINT     = NC_UINT,	//!< unsigned 4-byte int
      nc_INT64    = NC_INT64,	//!< signed 8-byte int
      nc_UINT64   = NC_UINT64,	//!< unsigned 8-byte int
      nc_STRING   = NC_STRING, 	//!< string
      nc_VLEN     = NC_VLEN,   	//!< "NcVlen type"
      nc_OPAQUE   = NC_OPAQUE, 	//!< "NcOpaque type"
      nc_ENUM     = NC_ENUM, 	//!< "NcEnum type"
      nc_COMPOUND = NC_COMPOUND //!< "NcCompound type"
    };
    
    /*! Constructor generates a \ref isNull "null object". */
    NcType();

    /*! 
      Constructor for a non-global type.
      This object describes the "essential" information for all netCDF types required by NcVar, NcAtt objects.
      New netCDF types can be added using the appropriate "add" method in the NcGroup object.
      \param grp    Parent NcGroup object.
      \param name   Name of this type.
    */
    NcType(const netCDF::NcGroup& grp, const std::string& name);


    /*! 
      Constructor for a non-global type.
      This object describes the "essential" information for all netCDF types required by NcVar, NcAtt objects.
      New netCDF types can be added using the appropriate "add" method in the NcGroup object.
      \param grp    Parent NcGroup object.
      \param id     type id
    */
    NcType(const netCDF::NcGroup& grp, nc_type id);

    /*! 
      Constructor for a global type
      This object describes the "essential" information for a netCDF global type.
      \param id     type id
    */
    NcType(nc_type id);

    /*! The copy constructor. */
    NcType(const NcType& rhs);

    /*! destructor  */
    virtual ~NcType() {}
    
    /*! equivalence operator */
    bool operator==(const NcType&) const;     
    
    /*!  != operator */
    bool operator!=(const NcType &) const;

    // accessors to private data.
    /*! The netCDF Id of this type. */
    nc_type getId() const {return myId;} 

    /*! Gets parent group. For an atomic type, returns a Null object.*/
    netCDF::NcGroup getParentGroup() const;

    /*! 
      The name of this type. For atomic types, the CDL type names are returned. These are as follows: 
        - NcByte   String returned is "byte".
        - NcUbyte  String returned is "ubyte".
        - NcChar   String returned is "char".   
        - NcShort  String returned is "short".  
        - NcUshort String returned is "ushort". 
        - NcInt    String returned is "int".    
        - NcUint   String returned is "uint".   
        - NcInt64  String returned is "int64".  
        - NcUint64 String returned is "uint64". 
        - NcFloat  String returned is "float".  
        - NcDouble String returned is "double". 
        - NcString String returned is "string".
     */
    std::string getName() const;                        

    /*! 
      The size in bytes. 
      This function will work on any type, including atomic and any user defined type, whether 
      compound, opaque, enumeration, or variable length array. 
     */
    size_t getSize() const;                         

    /*! 
      The type class returned as enumeration type.
      Valid for all types, whether atomic or user-defined. User-defined types are returned as one of the following
      enumeration types: nc_VLEN, nc_OPAQUE, nc_ENUM, or nc_COMPOUND. 
     */
    ncType getTypeClass() const;                       

    /*! 
      Return a string containing the name of the enumerated type.  (ie one of the following strings:
      "nc_BYTE", "nc_CHAR", "nc_SHORT", "nc_INT", "nc_FLOAT", "nc_DOUBLE", "nc_UBYTE", "nc_USHORT", 
      "nc_UINT", "nc_INT64", "nc_UINT64", "nc_STRING", "nc_VLEN", "nc_OPAQUE", "nc_ENUM", "nc_COMPOUND"
     */
    std::string getTypeClassName() const;
    
    /*! Returns true if this object is null (i.e. it has no contents); otherwise returns false. */
    bool isNull() const  {return nullObject;}

    /*! comparator operator  */
    friend bool operator<(const NcType& lhs,const NcType& rhs);
    
    /*! comparator operator  */
    friend bool operator>(const NcType& lhs,const NcType& rhs);
    
  protected:

    /*! assignment operator  */
    NcType& operator=(const NcType& rhs);
    
    bool nullObject;

    /*! the type Id */
    nc_type myId;
    
    /*! the group Id */
    int groupId;

  };

}
#endif

