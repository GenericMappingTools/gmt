/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is a very simple example which reads a 2D array of
   sample data produced by simple_xy_wr.cpp.

   This example is part of the netCDF tutorial:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

   Full documentation of the netCDF C++ API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx

   $Id$
*/

#include <iostream>
#include <netcdfcpp.h>

using namespace std;

// We are reading 2D data, a 6 x 12 grid. 
static const int NX = 6;
static const int NY = 12;

// Return this in event of a problem.
static const int NC_ERR = 2;

int main(void)
@{
   // This is the array we will read.
   int dataIn[NX][NY]; 

   // Open the file. The ReadOnly parameter tells netCDF we want
   // read-only access to the file.
   NcFile dataFile("simple_xy.nc", NcFile::ReadOnly);

   // You should always check whether a netCDF file open or creation
   // constructor succeeded.
   if (!dataFile.is_valid())
   @{
      cout << "Couldn't open file!\n";
      return NC_ERR;
   @}
  
   // For other method calls, the default behavior of the C++ API is
   // to exit with a message if there is an error.  If that behavior
   // is OK, there is no need to check return values in simple cases
   // like the following.
      
   // Retrieve the variable named "data"
   NcVar *data = dataFile.get_var("data");

   // Read all the values from the "data" variable into memory. 
   data->get(&dataIn[0][0], NX, NY);

   // Check the values. 
   for (int i = 0; i < NX; i++)
      for (int j = 0; j < NY; j++)
	 if (dataIn[i][j] != i * NY + j)
	    return NC_ERR;
    
   // The netCDF file is automatically closed by the NcFile destructor
   cout << "*** SUCCESS reading example file simple_xy.nc!" << endl;

   return 0;
@}
