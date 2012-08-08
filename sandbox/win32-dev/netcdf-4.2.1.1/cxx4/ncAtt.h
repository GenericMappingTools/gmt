#include "ncType.h"
#include "ncException.h"
#include <string>
#include <typeinfo>

#ifndef NcAttClass
#define NcAttClass

namespace netCDF
{

  /*! Abstract base class represents inherited by ncVarAtt and ncGroupAtt. */
  class NcAtt
  {
  public:
    
    /*! destructor */
    virtual ~NcAtt()=0;

    /*! Constructor generates a \ref isNull "null object". */
    NcAtt ();
    
    /*! Constructor for non-null instances. */
    NcAtt(bool nullObject); 

    /*! The copy constructor. */
    NcAtt(const NcAtt& rhs);

    /*! Get the attribute name. */
    std::string getName() const {return myName;}

    /*! Gets attribute length. */
    size_t  getAttLength() const;

    /*! Returns the attribute type. */
    NcType  getType() const;

    /*! Gets parent group. */
    NcGroup  getParentGroup() const;
      
    /*! equivalence operator */
    bool operator== (const NcAtt& rhs) const;
      
    /*!  != operator */
    bool operator!=(const NcAtt& rhs) const;     

    /*! \overload
     */ 
    void getValues(char* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned char* dataValues) const;
    /*! \overload
     */ 
    void getValues(signed char* dataValues) const;
    /*! \overload
     */ 
    void getValues(short* dataValues) const;
    /*! \overload
     */ 
    void getValues(int* dataValues) const;
    /*! \overload
     */ 
    void getValues(long* dataValues) const;
    /*! \overload
     */ 
    void getValues(float* dataValues) const;
    /*! \overload
     */ 
    void getValues(double* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned short* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned int* dataValues) const;
    /*! \overload
     */ 
    void getValues(long long* dataValues) const;
    /*! \overload
     */ 
    void getValues(unsigned long long* dataValues) const;
    /*! \overload
     */ 
    void getValues(char** dataValues) const;

    /*! \overload
      (The string variable does not need preallocating.)
     */ 
    void getValues(std::string& dataValues) const;

    /*! 
      Gets a netCDF attribute.
      The user must ensure that the variable "dataValues" has sufficient space to hold the attribute.
      \param  dataValues On return contains the value of the attribute. 
      If the type of data values differs from the netCDF variable type, type conversion will occur. 
      (However, no type conversion is carried out for variables using the user-defined data types:
      nc_Vlen, nc_Opaque, nc_Compound and nc_Enum.)
    */
    void getValues(void* dataValues) const;

    /*! Returns true if this object is null (i.e. it has no contents); otherwise returns false. */
    bool isNull() const {return nullObject;}

  protected:
    /*! assignment operator */
    NcAtt& operator= (const NcAtt& rhs);
      
    bool nullObject;

    std::string myName;
    
    int groupId;
      
    int varId;
    
  };
  
}

#endif
