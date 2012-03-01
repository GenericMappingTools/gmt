#include "ncType.h"

#ifndef NcCharClass
#define NcCharClass

namespace netCDF
{
  
  /*! Class represents a netCDF atomic Char type. */
  class NcChar : public NcType
  {
  public: 
    
    /*! equivalence operator */
    bool operator==(const NcChar & rhs);
    
    ~NcChar();
    
    /*! Constructor */
    NcChar();
  };

  /*! A global instance  of the NcChar class within the netCDF namespace. */
  extern NcChar ncChar;

}
#endif
