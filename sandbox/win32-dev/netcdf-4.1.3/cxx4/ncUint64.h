#include "ncType.h"

#ifndef NcUint64Class
#define NcUint64Class

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Uint64 type.*/
  class NcUint64 : public NcType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcUint64 & rhs);
    
    /*! destructor */
    ~NcUint64();
    
    /*! Constructor */
    NcUint64();
  };

  /*! A global instance  of the NcUint64 class within the netCDF namespace. */
  extern NcUint64 ncUint64;

}
#endif
