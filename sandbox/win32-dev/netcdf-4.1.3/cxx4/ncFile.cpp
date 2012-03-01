#include "ncFile.h"
#include "ncCheck.h"
#include "ncException.h"
#include "ncByte.h"
#include<iostream>
#include<string>
#include<sstream>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// destructor
NcFile::~NcFile()
{
  ncCheck(nc_close(myId),__FILE__,__LINE__);
}


// assignment operator
  NcFile& NcFile::operator=(const NcGroup & rhs)
{
  NcGroup::operator=(rhs);      // assign base class parts
  return *this;
}

//! The copy constructor.
NcFile::NcFile(const NcGroup& rhs):
    NcGroup(rhs)                   // intialize base class parts
{}

// Constructor generates a null object.
NcFile::NcFile() : 
    NcGroup()  // invoke base class constructor
{}

// constructor
NcFile::NcFile(const string& filePath, const FileMode fMode)
{
  switch (fMode) 
    {
    case NcFile::write:
      ncCheck(nc_open(filePath.c_str(), NC_WRITE, &myId),__FILE__,__LINE__);
      break;
    case NcFile::read:
      ncCheck(nc_open(filePath.c_str(), NC_NOWRITE, &myId),__FILE__,__LINE__);
      break;
    case NcFile::newFile:
      ncCheck(nc_create(filePath.c_str(), NC_NETCDF4 | NC_NOCLOBBER, &myId),__FILE__,__LINE__);
      break;
    case NcFile::replace:
      ncCheck(nc_create(filePath.c_str(), NC_NETCDF4 | NC_CLOBBER, &myId),__FILE__,__LINE__);
      break;
    }
  nullObject=false;
}

// constructor with file type specified
NcFile::NcFile(const string& filePath, const FileMode fMode, const FileFormat fFormat )
{
  int format;
  switch (fFormat)
    {
    case NcFile::classic:
	format = 0;
	break;
    case NcFile::classic64:
	format = NC_64BIT_OFFSET;
	break;
    case NcFile::nc4:
	format = NC_NETCDF4;
	break;
    case NcFile::nc4classic:
	format = NC_NETCDF4 | NC_CLASSIC_MODEL;
	break;
    }
  switch (fMode) 
    {
    case NcFile::write:
      ncCheck(NC_EINVAL,__FILE__,__LINE__);
      break;
    case NcFile::read:
      ncCheck(NC_EINVAL,__FILE__,__LINE__);
      break;
    case NcFile::newFile:
      ncCheck(nc_create(filePath.c_str(), format | NC_NOCLOBBER, &myId),__FILE__,__LINE__);
      break;
    case NcFile::replace:
      ncCheck(nc_create(filePath.c_str(), format | NC_CLOBBER, &myId),__FILE__,__LINE__);
      break;
    }
  nullObject=false;
}
