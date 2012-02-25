#include "ncAtt.h"
#include "netcdf.h"

#ifndef NcVarAttClass
#define NcVarAttClass

namespace netCDF
{
  class NcGroup;  // forward declaration.
  class NcVar;    // forward declaration.

  /*! Class represents a netCDF attribute local to a netCDF variable. */
  class NcVarAtt : public NcAtt
  {
  public:
    
    /*! assignment operator */
    NcVarAtt& operator= (const NcVarAtt& rhs);
      
    /*! Constructor generates a \ref isNull "null object". */
    NcVarAtt ();

    /*! The copy constructor. */
    NcVarAtt(const NcVarAtt& rhs) ;
      
    /*! 
      Constructor for an existing local attribute.
      \param  grp        Parent Group object.
      \param  NcVar      Parent NcVar object.
      \param  index      The index (id) of the attribute.
    */
    NcVarAtt(const NcGroup& grp, const NcVar& ncVar, const int index);
    
    /*! Returns the NcVar parent object. */
    NcVar getParentVar() const;

    /*! comparator operator */
    friend bool operator<(const NcVarAtt& lhs,const NcVarAtt& rhs);
    
    /*! comparator operator  */
    friend bool operator>(const NcVarAtt& lhs,const NcVarAtt& rhs);
    
  };
  
}

#endif
