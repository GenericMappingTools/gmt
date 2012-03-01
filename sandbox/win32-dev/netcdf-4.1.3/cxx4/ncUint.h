#include "ncType.h"

#ifndef NcUintClass
#define NcUintClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Uint type. */
  class NcUint : public NcType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcUint & rhs);
    
    /*! destructor */
    ~NcUint();
    
    /*! Constructor */
    NcUint();
  };

  /*! A global instance  of the NcUint class within the netCDF namespace. */
  extern NcUint ncUint;

}
#endif
