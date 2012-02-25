#include <string>
#include "ncGroup.h"
#include "netcdf.h"

#ifndef NcFileClass
#define NcFileClass


//!  C++ API for netCDF4.
namespace netCDF
{

  /*! 
    Class represents a netCDF root group.
    The Ncfile class is the same as the NcGroup class with the additional functionality for opening 
    and closing files.
   */
  class NcFile : public NcGroup
   {
   public:
    
      enum FileMode 
	 {
	    read,	//!< File exists, open read-only.
	    write,      //!< File exists, open for writing.
	    replace,	//!< Create new file, even if already exists.
	    newFile	//!< Create new file, fail if already exists.
	 };

      enum FileFormat
         {
	    classic,    //!< Classic format, classic data model
	    classic64,  //!< 64-bit offset format, classic data model
	    nc4,        //!< (default) netCDF-4/HDF5 format, enhanced data model
	    nc4classic  //!< netCDF-4/HDF5 format, classic data model
         };
    
      /*! assignment operator  */
      NcFile& operator =(const NcGroup & rhs);

      /*! 
	The copy constructor. */
      NcFile(const NcGroup& rhs);


    
      /*! Constructor generates a \ref isNull "null object". */
      NcFile();

      /*! 
	Opens a netCDF file.
	\param filePath    Name of netCDF optional path.
	\param fMode       The file mode:
	                    - 'read'    File exists, open for read-only.
	                    - 'write'   File exists, open for writing.
	                    - 'replace' Create new file, even it already exists.
	                    - 'newFile' Create new file, fail it exists already.
      */
      NcFile(const std::string& filePath, FileMode fMode);

      /*! 
	Creates a netCDF file of a specified format.
	\param filePath    Name of netCDF optional path.
	\param fMode       The file mode:
	                    - 'replace' Create new file, even it already exists.
	                    - 'newFile' Create new file, fail it exists already.
      */
      NcFile(const std::string& filePath, FileMode fMode, FileFormat fFormat);
    
      /*! destructor */
      virtual ~NcFile(); //closes file and releases all resources
    
   };
  
}

 
#endif

