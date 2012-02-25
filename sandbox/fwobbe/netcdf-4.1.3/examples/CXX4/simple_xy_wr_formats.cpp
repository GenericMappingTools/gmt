/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is a very simple example which writes a 2D array of
   sample data. To handle this in netCDF we create two shared
   dimensions, "x" and "y", and a netCDF variable, called "data".

   This example is part of the netCDF tutorial:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

   Full documentation of the netCDF C++ API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx

   $Id$
*/

#include <iostream>
#include <netcdf>
#include <vector>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// We are writing 2D data, a 6 x 12 grid. 
static const int NX = 6;
static const int NY = 12;

// Return this in event of a problem.
static const int NC_ERR = 2;

int create_file(string filename, NcFile::FileFormat format) {
  // This is the data array we will write. It will just be filled
  // with a progression of numbers for this example.
  int dataOut[NX][NY];
  
  // Create some pretend data. If this wasn't an example program, we
  // would have some real data to write, for example, model output.
  for(int i = 0; i < NX; i++)
    for(int j = 0; j < NY; j++)
      dataOut[i][j] = i * NY + j;
  // The default behavior of the C++ API is to throw an exception i
  // an error occurs. A try catch block is necessary.
  try
    {  
      // Create the file. The Replace parameter tells netCDF to overwrite
      // this file, if it already exists.  The classic parameter specifies
      // that the file ceated should be in classic format, rather than the
      // default netCDF-4 format used by the cxx4 interface.
      NcFile dataFile(filename, NcFile::replace, format);
      
      // Create netCDF dimensions
      NcDim xDim = dataFile.addDim("x", NX);
      NcDim yDim = dataFile.addDim("y", NY);
      
      // Define the variable. The type of the variable in this case is
      // ncInt (32-bit integer).
      vector<NcDim> dims;
      dims.push_back(xDim);
      dims.push_back(yDim);
      NcVar data = dataFile.addVar("data", ncInt, dims);
   
      // In the classic model, must explicitly leave define mode
      // before writing data.  Need a method that calls nc_enddef().
//       if(format != NcFile::nc4)
// 	  dataFile.

      // Write the data to the file. Although netCDF supports
      // reading and writing subsets of data, in this case we write all
      // the data in one operation.
      data.putVar(dataOut);
      
      // The file will be automatically closed when the NcFile object goes
      // out of scope. This frees up any internal netCDF resources
      // associated with the file, and flushes any buffers.
      
      //cout << "*** SUCCESS writing example file simple_xy.nc!" << endl;
      return 0; 
    }
  catch(NcException& e)
    {e.what();
      return NC_ERR;
    }
}

int main()
{
    int ret;
    if(ret = create_file("simple_xy_nc4.nc", NcFile::nc4))
	return ret;
    cout << "*** SUCCESS creating nc4 file" << endl;

// The following three tests don't work yet.  Uncomment when can write
// files of nc4classic, classic, and classic64 formats.

//     if(ret = create_file("simple_xy_nc4classic.nc", NcFile::nc4classic))
// 	return ret;
//     cout << "*** SUCCESS creating nc4classic file" << endl;

//     if(ret = create_file("simple_xy_classic.nc", NcFile::classic))
// 	return ret;
//     cout << "*** SUCCESS creating classic file" << endl;

//     if(ret = create_file("simple_xy_classic64.nc", NcFile::classic64))
// 	return ret;
//     cout << "*** SUCCESS creating classic64 file" << endl;

    return 0;
}
