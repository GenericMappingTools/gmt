#include <string>
#include "ncType.h"
#include "netcdf.h"
#include "ncCheck.h"

#ifndef NcEnumTypeClass
#define NcEnumTypeClass


namespace netCDF
{
  class NcGroup;  // forward declaration.

  /*! Class represents a netCDF enum type */
  class NcEnumType : public NcType
    {
    public:
      
      /*! List of NetCDF-4 Enumeration types.*/
      enum ncEnumType	{
	nc_BYTE     = NC_BYTE, 	//!< signed 1 byte integer
	nc_SHORT    = NC_SHORT, 	//!< signed 2 byte integer
	nc_INT      = NC_INT,	//!< signed 4 byte integer
	nc_UBYTE    = NC_UBYTE,	//!< unsigned 1 byte int
	nc_USHORT   = NC_USHORT,	//!< unsigned 2-byte int
	nc_UINT     = NC_UINT,	//!< unsigned 4-byte int
	nc_INT64    = NC_INT64,	//!< signed 8-byte int
	nc_UINT64   = NC_UINT64	//!< unsigned 8-byte int
      };
      
      /*! Constructor generates a \ref isNull "null object". */
      NcEnumType();

      /*! 
	Constructor.
	The enum Type must already exist in the netCDF file. New netCDF enum types can 
	be added using NcGroup::addNcEnumType();
	\param grp        The parent group where this type is defined.
	\param name       Name of new type.
      */
      NcEnumType(const NcGroup& grp, const std::string& name);

      /*! 
	Constructor.
	Constructs from the base type NcType object. Will throw an exception if the NcType is not the base of an Enum type.
	\param ncType     A Nctype object.
      */
      NcEnumType(const NcType& ncType);

      /*! assignment operator */
      NcEnumType& operator=(const NcEnumType& rhs);
      
      /*! 
	Assignment operator.
       This assigns from the base type NcType object. Will throw an exception if the NcType is not the base of an Enum type.
      */
      NcEnumType& operator=(const NcType& rhs);
      
      /*! The copy constructor. */
      NcEnumType(const NcEnumType& rhs);
      
      /*! Destructor */
      ~NcEnumType(){}
      
      
      /*! 
	Adds a new member to this NcEnumType type.
	\param name         Name for this new Enum memebr.
	\param memberValue  Member value, must be of the correct NcType.
      */
      template <class T> void addMember(const std::string& name, T memberValue)
      {
	ncCheck(nc_insert_enum(groupId, myId, name.c_str(), (void*) &memberValue),__FILE__,__LINE__);
      }

      /*! Returns number of members in this NcEnumType object. */
      size_t  getMemberCount() const;
      
      /*! Returns the member name for the given zero-based index. */
      std::string  getMemberNameFromIndex(int index) const;

      /*! Returns the member name for the given NcEnumType value. */
      template <class T>  std::string  getMemberNameFromValue(const T memberValue) const {
	char charName[NC_MAX_NAME+1];
	ncCheck(nc_inq_enum_ident(groupId,myId,static_cast<long long>(memberValue),charName),__FILE__,__LINE__);
	return std::string(charName);
      }
	
      /*! 
	Returns the value of a member with the given zero-based index.
	\param name         Name for this new Enum member.
	\param memberValue  Member value, returned by this routine.
      */
      template <class T> void getMemberValue(int index, T& memberValue) const
	{
	  char* charName=NULL;
	  ncCheck(nc_inq_enum_member(groupId,myId,index,charName,&memberValue),__FILE__,__LINE__);
	}

      /*! Returns the base type. */
      NcType  getBaseType() const;
      
  };
  
}

#endif
