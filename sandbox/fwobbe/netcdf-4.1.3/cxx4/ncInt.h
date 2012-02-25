#include "ncType.h"

#ifndef NcIntClass
#define NcIntClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Int type. */
  class NcInt : public NcType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcInt & rhs);
    
    /*!  destructor */
    ~NcInt();
    
    /*! Constructor */
    NcInt();
  };

  /*! A global instance  of the NcInt class within the netCDF namespace. */
  extern NcInt ncInt;

}
#endif
