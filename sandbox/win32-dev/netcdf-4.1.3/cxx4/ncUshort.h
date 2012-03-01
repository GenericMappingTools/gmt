#include "ncType.h"

#ifndef NcUshortClass
#define NcUshortClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Ushort type. */
  class NcUshort : public NcType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcUshort & rhs);
    
    /*! destructor */
    ~NcUshort();
    
    /*! Constructor */
    NcUshort();
  };

  // declare that the class instance ncUshort is known by all....
  extern NcUshort ncUshort;

}
#endif
