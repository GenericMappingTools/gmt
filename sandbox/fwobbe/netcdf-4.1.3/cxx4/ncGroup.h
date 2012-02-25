#include <string>
#include <vector>
#include <set>
#include <map>
#include "ncType.h"
#include "ncEnumType.h"
#include "ncGroupAtt.h"



#ifndef NcGroupClass
#define NcGroupClass


namespace netCDF
{
  class NcVar;          // forward declaration.
  class NcDim;          // forward declaration.
  class NcVlenType;     // forward declaration.
  class NcCompoundType; // forward declaration.
  class NcOpaqueType;   // forward declaration.

  /*! Class represents a netCDF group. */
  class NcGroup
  {	
    
  public:
    
    /*!
      The enumeration list contains the options for selecting groups (used for returned set of NcGroup objects).
    */
    enum GroupLocation
      {
	ChildrenGrps,              //!< Select from the set of children in the current group.
	ParentsGrps,               //!< Select from set of parent groups (excludes the current group).
	ChildrenOfChildrenGrps,    //!< Select from set of all children of children in the current group.
	AllChildrenGrps,           //!< Select from set of all children of the current group and beneath.
	ParentsAndCurrentGrps,     //!< Select from set of parent groups(includes the current group).
	AllGrps                    //!< Select from set of parent groups, current groups and all the children beneath.
      };
    
    /*!
      The enumeration list contains the options for selecting groups.
    */
    enum Location
      {
	Current,            //!< Select from contents of current group.
	Parents,            //!< Select from contents of parents groups.
	Children,           //!< Select from contents of children groups.
	ParentsAndCurrent,  //!< Select from contents of current and parents groups.
	ChildrenAndCurrent, //!< Select from contents of current and child groups.
	All                 //!< Select from contents of current, parents and child groups.
      };
    

    /*! assignment operator  */
    NcGroup& operator=(const NcGroup& rhs);
    
    /*! Constructor generates a \ref isNull "null object". */
    NcGroup();

    //* constructor */
    NcGroup(int groupId);

    /*! The copy constructor. */
    NcGroup(const NcGroup& rhs);

    /*! destructor  */
    virtual ~NcGroup();
    
    /*! equivalence operator */
    bool operator==(const NcGroup& rhs) const;     
    
    /*!  != operator */
    bool operator!=(const NcGroup& rhs) const;     

    /*! comparator operator  */
    friend bool operator<(const NcGroup& lhs,const NcGroup& rhs);
    
    /*! comparator operator  */
    friend bool operator>(const NcGroup& lhs,const NcGroup& rhs);
    
    // /////////////
    // NcGroup-related methods
    // /////////////
    
    /*! Gets the group name. */
    /*!
      Method will throw an netCDF::exceptions::NcNullgrp exception if the group is null (ie NcGroup::isNull()=true).
      \param fullName If true then the full name is returned with subgroups separated by a forward slash "/" (default is false)
      \return         The group name.
    */
    std::string getName(bool fullName=false) const;

    /*! 
      Gets the parent group.
      Method will throw an netCDF::exceptions::NcNullgrp exception if the  group is null (ie NcGroup::isNull()=true).
      If the current root is the parent group, then return a null group.
    */
    NcGroup getParentGroup() const ;

    /*! 
      Gets the group id.
      Method will throw an netCDF::exceptions::NcNullgrp exception if the group is null (ie NcGroup::isNull()=true).
    */
    int  getId() const;
    
    /*! 
      Gets the number of  NcGroup objects.
      Method will throw an netCDF::exceptions::NcNullgrp exception if the group is null (ie NcGroup::isNull()=true).
      \param location Enumeration type controlling the groups to search.
      \return         Number of groups.
    */
    int getGroupCount(NcGroup::GroupLocation location=ChildrenGrps) const;

    /*! 
      Gets the collection of NcGroup objects.
      Method will throw an netCDF::exceptions::NcNullgrp exception if the group is null (ie NcGroup::isNull()=true).
      \param location Enumeration type controlling the groups to search.
      \return         A STL multimap object, containing pairs of <attribute name, NcGroup object> entities.
    */
    std::multimap<std::string,NcGroup> getGroups(NcGroup::GroupLocation location=ChildrenGrps) const;


    /*! 
      Gets NcGroup objects with a given name.
      Method will throw an netCDF::exceptions::NcNullgrp exception if the group is null (ie NcGroup::isNull()=true).
      \param name     Name of group.
      \param location Enumeration type controlling the groups to search.
      \return         Set of NcGroup objects with given name.
    */
    std::set<NcGroup> getGroups(const std::string& name,NcGroup::GroupLocation location=ChildrenGrps) const;

    /*!
      Gets the named child NcGroup object.
      Method will throw an netCDF::exceptions::NcNullgrp exception if the group is null (ie NcGroup::isNull()=true).
      \param name  Group name.
      \param location   Enumeration type controlling the groups to search.
      \return      An NcGroup object. If there are multiple objects indentied with the same name, 
      the object closest to the current group is returned. If no valid object is found , 
      a \ref NcGroup::isNull "null node" is returned.
    */
    NcGroup getGroup(const std::string& name,NcGroup::GroupLocation location=ChildrenGrps) const;

    /*! 
      Adds a new child netCDF group object.
      Method will throw an netCDF::exceptions::NcNullgrp exception if the group is null (ie NcGroup::isNull()=true).
      \param   name     Variable name.
      \return  NcGroup  The NcGroup object for this new netCDF group.
    */
    NcGroup addGroup(const std::string& name) const;


    /*! Returns true if this object is null (i.e. it has no contents); otherwise returns false. */
    bool isNull() const  {return nullObject;}

    /*! Returns true if this is the root group, otherwise returns false. */
    bool isRootGroup() const;

    // /////////////
    // NcVar-related accessors
    // /////////////

    /*! 
      Gets the number of NcVar objects in this group.
      \param location Enumeration type controlling the groups to search.
      \return         Number of variables.
    */
    int getVarCount(NcGroup::Location location=Current) const;

    /*!
      Get the collection of NcVar objects.
      \param location Enumeration type controlling the groups to search.
      \return         A STL multimap object, containing pairs of <attribute name, NcVar object> entities.
    */
   std::multimap<std::string,NcVar> getVars(NcGroup::Location location=Current) const;

   /*! 
     Gets all NcVar objects with a given name.
      \param name     Name of attribute
      \param location Enumeration type controlling the groups to search.
      \return         Set of NcVar objects.
    */
    std::set<NcVar> getVars(const std::string& name,NcGroup::Location location=Current) const;
    
    /*! 
      Gets the named NcVar object..
      \param name     Variable name.
      \param location Enumeration type controlling the groups to search.
      \return         A NcVar object. If there are multiple objects indentied with the 
      same name, the object closest  to the current group is returned.  
      If no valid object is found , a \ref NcVar::isNull "null node" is returned.
     */
    NcVar getVar(const std::string& name,NcGroup::Location location=Current) const;

    /*!
      Adds a new netCDF variable.
      The NcType and NcDim objects must be non-null, and be defined in either the current group or a parent group.
      An NcNullType exception is thrown if the NcType object is invalid.
      An NcNullDim exception is thrown if the NcDim object is invalid.
      \param    name     Variable name.
      \param   typeName  Type name.
      \param   dimName   Dimension name.
      \return            The NcVar object for this new netCDF variable.
    */
    NcVar addVar(const std::string& name, const std::string& typeName, const std::string& dimName) const;

    /*! 
      Adds a new netCDF variable.
      The NcType and NcDim objects must be non-null, and be defined in either the current group or a parent group.
      An NcNullType exception is thrown if the NcType object is invalid.
      An NcNullDim exception is thrown if the NcDim object is invalid.
      \param    name      Variable name.
      \param    ncType    NcType object.
      \param    ncDim     NcDim object.
      \return             The NcVar object for this new netCDF variable.
    */
    NcVar addVar(const std::string& name, const NcType& ncType, const NcDim& ncDim) const;

    /*! 
      Adds a new netCDF multi-dimensional variable.
      The NcType and NcDim objects must be non-null, and be defined in either the current group or a parent group.
      An NcNullType exception is thrown if the NcType object is invalid.
      An NcNullDim exception is thrown if the NcDim object is invalid.
      \param   name     Variable name.
      \param   typeName Type name.
      \param   dimNames Vector of dimension names.
      \return           The NcVar object for this new netCDF variable.
    */
    NcVar addVar(const std::string& name, const std::string& typeName, const std::vector<std::string>& dimNames) const;
    

    /*! 
      Adds a new multi-dimensional netCDF variable.
      The NcType and NcDim objects must be non-null, and be defined in either the current group or a parent group.
      An NcNullType exception is thrown if the NcType object is invalid.
      An NcNullDim exception is thrown if any of the the NcDim objects are invalid.
      \param    name        Variable name.
      \param    ncType      NcType object.
      \param    ncDimvector Vector of NcDim objects.
      \return               The NcVar object for this new netCDF variable.
    */
    NcVar addVar(const std::string& name, const NcType& ncType, const std::vector<NcDim>& ncDimVector) const;

    // /////////////
    // NcGroupAtt-related methods
    // /////////////

    /*! 
      Gets the number of group attributes.
      \param location Enumeration type controlling the groups to search.
      \return         Number of attributes.
    */
    int getAttCount(NcGroup::Location location=Current) const;

    /*! 
      Gets the collection of NcGroupAtt objects.
      \param location Enumeration type controlling the groups to search.
      \return         A STL multimap object, containing pairs of <attribute name, NcGroupAtt object> entities.
    */
    std::multimap<std::string,NcGroupAtt> getAtts(NcGroup::Location location=Current) const;
    
    /*! 
    Gets all NcGroupAtt objects with a given name.
      \param name     Name of attribute
      \param location Enumeration type controlling the groups to search.
      \return         Set of NcGroupAtt objects.
    */
    std::set<NcGroupAtt> getAtts(const std::string& name,NcGroup::Location location=Current) const;
    
    /*! 
      Gets the named NcGroupAtt object.
      \param name     Name of attribute
      \param location Enumeration type controlling the groups to search.
      \return         A NcGroupAtt object. If there are multiple objects indentied with the 
      same name, the object closest  to the current group is returned.  If no valid object is found , 
      a \ref NcGroupAtt::isNull "null node" is returned.
    */
    NcGroupAtt getAtt(const std::string& name,NcGroup::Location location=Current) const;
    

    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, size_t len, const char** dataValues) const ;

    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const std::string& dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, short datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, int datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, long datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, float datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, double datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, unsigned short datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, unsigned int datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, unsigned long long datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, long long datumValue) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const unsigned char* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const signed char* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const short* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const int* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const long* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const float* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const double* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const unsigned short* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const unsigned int* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const unsigned long long* dataValues) const ;
    /*! \overload
     */ 
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const long long* dataValues) const ;
    /*! 
      Creates a new NetCDF group attribute or if already exisiting replaces it.
      If you are writing a _Fill_Value_ attribute, and will tell the HDF5 layer to use 
      the specified fill value for that variable. 
      \par 
      Although it's possible to create attributes of all types, text and double attributes are adequate for most purposes. 
      \param name        Name of attribute.
      \param type    The attribute type.
      \param len         The length of the attribute (number of Nctype repeats).
      \param dataValues  Data Values to put into the new attribute.
      If the type of data values differs from the netCDF variable type, type conversion will occur. 
      (However, no type conversion is carried out for variables using the user-defined data types:
      nc_Vlen, nc_Opaque, nc_Compound and nc_Enum.)
      \return            The NcGroupAtt object for this new netCDF attribute.
    */
    NcGroupAtt putAtt(const std::string& name, const NcType& type, size_t len, const void* dataValues) const ;



    // /////////////
    // NcDim-related methods
    // /////////////

    /*! 
      Gets the number of NcDim objects.
      \param location Enumeration type controlling the groups to search.
      \return         Number of dimensions.
    */
    int getDimCount(NcGroup::Location location=Current) const;
    
    /*! 
      Gets the collection of NcDim objects.
      \param location Enumeration type controlling the groups to search.
      \return         A STL multimap object, containing pairs of <attribute name, NcDim object> entities.
    */
    std::multimap<std::string,NcDim> getDims(NcGroup::Location location=Current) const;
    
    /*! 
      Gets NcDim objects with a given name.
      \param name     Name of dimension.
      \param location Enumeration type controlling the groups to search.
      \return         Set of NcDim objects with given name.
    */
    std::set<NcDim> getDims(const std::string& name,NcGroup::Location location=Current) const;

    /*! 
      Gets the named NcDim object.
      \param name       Name of dimension.
      \param location   Enumeration type controlling the groups to search.
      \return           An NcDim object. If there are multiple objects indentied with the same name, 
      the object closest to the current group is returned. If no valid object is found , a \ref NcDim::isNull "null node" is returned.
    */
    NcDim getDim(const std::string& name,NcGroup::Location location=Current) const;

    /*! 
      Adds a new netCDF dimension.
      \param The name of new dimension.
      \param Length of dimension; that is, number of values for this dimension as an index to variables 
      that use it.
      \return   The NcDim object for this new netCDF dimension.
    */
    NcDim addDim(const std::string& name, size_t dimSize) const;

    /*! 
      Adds a new unlimited netCDF dimension.
      \param The name of new dimension.
      \return   The NcDim object for this new netCDF dimension.
    */
    NcDim addDim(const std::string& name) const;

    // /////////////
    // NcType-related methods
    // /////////////

    /*! 
      Gets the number of type objects.
      \param location Enumeration type controlling the groups to search.
      \return         Number of types.
    */
    int getTypeCount(NcGroup::Location location=Current) const;
    
    
    /*! 
      Gets the number of type objects with a given enumeration type.
      \param enumType The enumeration value of the object type.
      \param location Enumeration type controlling the groups to search.
      \return         Number of types of the given enumeration type.
    */
    int getTypeCount(NcType::ncType enumType, NcGroup::Location location=Current) const;
    
    
    /*! 
      Gets the collection of NcType objects.
      \param location Enumeration type controlling the groups to search.
      \return         A STL multimap object, on return contains pairs of <Type name, NcType object> entities.
                      For atomic types, the type returned is the CDL name.
    */
    std::multimap<std::string,NcType> getTypes(NcGroup::Location location=Current) const;

  
    /*! 
      Gets the collection of NcType objects with a given name.
      \param name     Name of type. For atomic types, the CDL name is expected. This is consistent with the 
                         string returned from NcType::getName().
      \param location Enumeration type controlling the groups to search.
      \return         Set of  NcType objects.
    */
    std::set<NcType> getTypes(const std::string& name, NcGroup::Location location=Current) const;

    /*! 
      Gets the collection of NcType objects with a given data type.
      \param enumType Enumeration type specifying the data type.
      \param location Enumeration type controlling the groups to search.
      \return         Set of Nctype objects.
    */
    std::set<NcType> getTypes(NcType::ncType enumType, NcGroup::Location location=Current) const;


    /*! 
      Gets the collection of NcType objects with a given name and data type.
      \param name     Name of type. For atomic types, the CDL name is expected. This is consistent with the 
                         string returned from NcType::getName().
      \param enumType Enumeration type specifying the data type.
      \param location Enumeration type controlling the groups to search.
      \return         Set of Nctype objects.
    */
    std::set<NcType> getTypes(const std::string& name, NcType::ncType enumType, NcGroup::Location location=Current) const;


    /*! 
      Gets the NcType object with a given name.
      \param name     Name of type. For atomic types, the CDL name is expected. This is consistent with the 
                         string returned from NcType::getName().
      \param location Enumeration type controlling the groups to search.
      \return         NcType object. If there are multiple objects indentied with the same name, 
      the object closest to the current group is returned.  If no valid object is found , a \ref NcType::isNull "null node" is returned.

    */
    NcType getType(const std::string& name, NcGroup::Location location=Current) const;
    
    
    /*! 
      Adds a new netCDF enum type.
      \param name        Name of type. For atomic types, the CDL name is expected. This is consistent with the 
                         string returned from NcType::getName().
      \param enumType    The enumeration value of the object type.
      \return            The NcEnumType object for this new netCDF enum type.
    */
    NcEnumType addEnumType(const std::string& name,NcEnumType::ncEnumType basetype) const;


    /*! 
      Adds a new netCDF Vlen type.
      \param name        Name of type.
      \param basetype    A NcType object to be used for the basetype.
      \return            The NcVlenType object for this new netCDF vlen type.
    */
    NcVlenType addVlenType(const std::string& name,NcType& basetype) const;


    /*! 
      Adds a new netCDF Opaque type.
      \param name     Name of type.
      \param size     The size of the new type in bytes.
      \return         The NcOpaqueType object for this new netCDF opaque type..
    */
    NcOpaqueType addOpaqueType(const std::string& name, size_t size) const;


    /*! 
      Adds a new netCDF UserDefined type.
      \param name     Name of type.
      \param size     The size of the new type in bytes.
      \return         The new NcCompoundType object for this new netCDF userDefined type.
    */
    NcCompoundType addCompoundType(const std::string& name, size_t size) const; 


    /*! 
      Gets a collection of  coordinate variables.
      Coordinate variable have  an NcDim and NcVar object with the same name defined in the same group.
      \par
      The method returns STL map object containing a coordinate variables in the current group  and optionally
      in the parent and child groups. It is expected that within each group, the names of dimensions are unique and
      the the names of variables are unique. However, if this is not the case, this method will still work correctly.
      
      \param location Enumeration type controlling the groups to search.
      \return         The NcVar dimension variable. If no valid object is found , a \ref NcVar::isNull "null node" is returned.
    */
    std::map<std::string,NcGroup> getCoordVars(NcGroup::Location location=Current) const;

    /*! 
      Gets the NcDim and NcVar object pair for a named coordinate variable.
      Coordinate variable have  an NcDim and NcVar object with the same name defined in the same group.
      \par
      The method returns two objects for the named coordinate variable. The method searches first in the current 
      group and optionally in the parent and child group and returns the first instance found.
      \param location Enumeration type controlling the groups to search.
      \return         The set of names of dimension variables. 
    */
    void getCoordVar(std::string& coordVarName, NcDim& ncDim, NcVar& ncVar, NcGroup::Location location=Current) const;


  protected:
    
    /*! assignment operator  */
    /* NcGroup& operator=(const NcGroup& rhs); */
    
    bool nullObject;

    int myId;	
    
  };

}
#endif
