#include "ncAtt.h"
#include "netcdf.h"

#ifndef NcGroupAttClass
#define NcGroupAttClass

namespace netCDF
{
  class NcGroup;  // forward declaration.

  /*! Class represents a netCDF group attribute */
  class NcGroupAtt : public NcAtt
  {
  public:
    
    /*! assignment operator */
    NcGroupAtt& operator= (const NcGroupAtt& rhs);
   
    /*! Constructor generates a \ref isNull "null object". */
    NcGroupAtt ();
    
    /*! The copy constructor. */
    NcGroupAtt(const NcGroupAtt& rhs) ;
      
    /*! 
      Constructor for an existing global attribute.
      \param  grp        Parent Group object.
      \param  index      The index (id) of the attribute.
    */
    NcGroupAtt(const NcGroup& grp, const int index);
    
    /*! equivalence operator */
    bool operator== (const NcGroupAtt& rhs);
      
    /*! comparator operator */
    friend bool operator<(const NcGroupAtt& lhs,const NcGroupAtt& rhs);
    
    /*! comparator operator */
    friend bool operator>(const NcGroupAtt& lhs,const NcGroupAtt& rhs);
    
  };
  
}

#endif
