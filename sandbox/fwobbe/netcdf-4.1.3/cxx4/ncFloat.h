#include "ncType.h"

#ifndef NcFloatClass
#define NcFloatClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Float type. */
  class NcFloat : public NcType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcFloat & rhs);
    
    /*!  destructor */
    ~NcFloat();
    
    /*! Constructor */
    NcFloat();
  };

  /*! A global instance  of the NcFloat class within the netCDF namespace. */
  extern NcFloat ncFloat;

}
#endif
