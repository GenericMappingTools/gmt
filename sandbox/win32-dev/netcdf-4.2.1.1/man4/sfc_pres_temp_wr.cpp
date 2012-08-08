/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This example writes some surface pressure and temperatures. It is
   intended to illustrate the use of the netCDF C++ API. The companion
   program sfc_pres_temp_rd.cpp shows how to read the netCDF data file
   created by this program.
   
   This program is part of the netCDF tutorial:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

   Full documentation of the netCDF C++ API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx

   $Id$
*/

#include <iostream>
#include <netcdfcpp.h>

using namespace std;

// We are writing 2D data, a 6 x 12 lat-lon grid. We will need two
// netCDF dimensions.
static const int NLAT = 6;
static const int NLON = 12;

// These are used to construct some example data. 
static const float SAMPLE_PRESSURE = 900;
static const float SAMPLE_TEMP = 9.0;
static const float START_LAT = 25.0;
static const float START_LON = -125.0;

// Return this to OS if there is a failure.
static const int NC_ERR = 2;

int main(void)
@{
   // These will hold our pressure and temperature data.
   float presOut[NLAT][NLON];
   float tempOut[NLAT][NLON];

   // These will hold our latitudes and longitudes.
   float lats[NLAT];
   float lons[NLON];

   // Create some pretend data. If this wasn't an example program, we
   // would have some real data to write, for example, model
   // output. 
   for(int lat = 0; lat < NLAT; lat++)
      lats[lat] = START_LAT + 5. * lat;
   
   for(int lon = 0; lon < NLON; lon++)
      lons[lon] = START_LON + 5. * lon;

   for (int lat = 0; lat < NLAT; lat++)
      for(int lon = 0; lon < NLON; lon++)
      @{
	 presOut[lat][lon] = SAMPLE_PRESSURE + (lon * NLAT + lat);
	 tempOut[lat][lon] = SAMPLE_TEMP + .25 * (lon * NLAT + lat);
      @}
  
   // Change the error behavior of the netCDF C++ API by creating an
   // NcError object. Until it is destroyed, this NcError object will
   // ensure that the netCDF C++ API silently returns error codes
   // on any failure, and leaves any other error handling to the
   // calling program. In the case of this example, we just exit with
   // an NC_ERR error code.
   NcError err(NcError::silent_nonfatal);

   // Create the file. The Replace parameter tells netCDF to overwrite
   // this file, if it already exists.
   NcFile dataFile("sfc_pres_temp.nc", NcFile::Replace);
   
   // Check to see if the file was created.
   if(!dataFile.is_valid())
      return NC_ERR;

   // Define the dimensions. NetCDF will hand back an ncDim object for
   // each.
   NcDim *latDim, *lonDim;
   if (!(latDim = dataFile.add_dim("latitude", NLAT)))
      return NC_ERR;
   if (!(lonDim = dataFile.add_dim("longitude", NLON)))
      return NC_ERR;
       
   // In addition to the latitude and longitude dimensions, we will
   // also create latitude and longitude netCDF variables which will
   // hold the actual latitudes and longitudes. Since they hold data
   // about the coordinate system, the netCDF term for these is:
   // "coordinate variables."
   NcVar *latVar, *lonVar;
   if (!(latVar = dataFile.add_var("latitude", ncFloat, latDim)))
      return NC_ERR;
   if (!(lonVar = dataFile.add_var("longitude", ncFloat, lonDim)))
      return NC_ERR;      
 
   // Define units attributes for coordinate vars. This attaches a
   // text attribute to each of the coordinate variables, containing
   // the units.
   if (!lonVar->add_att("units", "degrees_east"))
      return NC_ERR;            
   if (!latVar->add_att("units", "degrees_north"))
      return NC_ERR;          

   // Define the netCDF data variables.
   NcVar *presVar, *tempVar;
   if (!(presVar = dataFile.add_var("pressure", ncFloat, latDim, lonDim)))
      return NC_ERR;          
   if (!(tempVar = dataFile.add_var("temperature", ncFloat, latDim, lonDim)))
      return NC_ERR;          

   // Define units attributes for variables. 
   if (!presVar->add_att("units", "hPa"))
      return NC_ERR;          
   if (!tempVar->add_att("units", "celsius"))
      return NC_ERR;          
    
   // Write the coordinate variable data. This will put the latitudes
   // and longitudes of our data grid into the netCDF file.
   if (!latVar->put(lats, NLAT))
      return NC_ERR;      
   if (!lonVar->put(lons, NLON))
      return NC_ERR;      

   // Write the pretend data. This will write our surface pressure and
   // surface temperature data. The arrays of data are the same size
   // as the netCDF variables we have defined, and below we write them
   // each in one step.
   if (!presVar->put(&presOut[0][0], NLAT, NLON))
      return NC_ERR;          
   if (!tempVar->put(&tempOut[0][0], NLAT, NLON))
      return NC_ERR;          
       
   // The file is automatically closed by the destructor. This frees
   // up any internal netCDF resources associated with the file, and
   // flushes any buffers.
   cout << "*** SUCCESS writing example file sfc_pres_temp.nc!" << endl;

   return 0;
@}
