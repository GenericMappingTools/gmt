#include "ncType.h"

#ifndef NcInt64Class
#define NcInt64Class

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Int64 type. */
  class NcInt64 : public NcType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcInt64 & rhs);
    
    /*!  destructor */
    ~NcInt64();
    
    /*! Constructor */
    NcInt64();
  };

  /*! A global instance  of the NcInt64 class within the netCDF namespace. */
  extern NcInt64 ncInt64;

}
#endif
