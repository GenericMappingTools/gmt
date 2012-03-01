/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is an example program which writes some 4D pressure and
   temperatures. This example demonstrates the netCDF C++ API. 

   This is part of the netCDF tutorial:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

   Full documentation of the netCDF C++ API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx

   $Id$
*/

#include <iostream>
#include <netcdfcpp.h>

using namespace std;

// We are writing 4D data, a 2 x 6 x 12 lvl-lat-lon grid, with 2
// timesteps of data.
static const int NLVL = 2;
static const int NLAT = 6;
static const int NLON = 12;
static const int NREC = 2;

// These are used to construct some example data. 
static const float SAMPLE_PRESSURE = 900.0;
static const float SAMPLE_TEMP = 9.0;
static const float START_LAT = 25.0;
static const float START_LON = -125.0;

// Return this code to the OS in case of failure.
static const int NC_ERR = 2;

int main()
{
   // These arrays will store the latitude and longitude values.
   float lats[NLAT],lons[NLON];

   // These arrays will hold the data we will write out. We will
   // only need enough space to hold one timestep of data; one record.
   float pres_out[NLVL][NLAT][NLON];
   float temp_out[NLVL][NLAT][NLON];

   int i = 0;
  
   // Create some pretend data. If this wasn't an example program, we
   // would have some real data to write for example, model output.
   for (int lat = 0; lat < NLAT; lat++)
      lats[lat] = START_LAT + 5. * lat;
   for (int lon = 0; lon < NLON; lon++)
      lons[lon] = START_LON + 5. * lon;

   for (int lvl = 0; lvl < NLVL; lvl++)
      for (int lat = 0; lat < NLAT; lat++)
	 for (int lon = 0; lon < NLON; lon++)
	 {
	    pres_out[lvl][lat][lon] = SAMPLE_PRESSURE + i;
	    temp_out[lvl][lat][lon]  = SAMPLE_TEMP + i++;
	 }
   
   // Change the error behavior of the netCDF C++ API by creating an
   // NcError object. Until it is destroyed, this NcError object will
   // ensure that the netCDF C++ API returns error codes on any
   // failure, prints an error message, and leaves any other error
   // handling to the calling program. In the case of this example, we
   // just exit with an NC_ERR error code.
   NcError err(NcError::verbose_nonfatal);

   // Create the file.
   NcFile dataFile("pres_temp_4D.nc", NcFile::Replace);

   // Check to see if the file was created.
   if(!dataFile.is_valid())
      return NC_ERR;

   // Define the dimensions. NetCDF will hand back an ncDim object for
   // each.
   NcDim *lvlDim, *latDim, *lonDim, *recDim;
   if (!(lvlDim = dataFile.add_dim("level", NLVL)))
      return NC_ERR;
   if (!(latDim = dataFile.add_dim("latitude", NLAT)))
      return NC_ERR;
   if (!(lonDim = dataFile.add_dim("longitude", NLON)))
      return NC_ERR;
   // Add an unlimited dimension...
   if (!(recDim = dataFile.add_dim("time")))
      return NC_ERR;
       
   // Define the coordinate variables.
   NcVar *latVar, *lonVar;
   if (!(latVar = dataFile.add_var("latitude", ncFloat, latDim)))
      return NC_ERR;
   if (!(lonVar = dataFile.add_var("longitude", ncFloat, lonDim)))
      return NC_ERR;
       
   // Define units attributes for coordinate vars. This attaches a
   // text attribute to each of the coordinate variables, containing
   // the units.
   if (!latVar->add_att("units", "degrees_north"))
      return NC_ERR;
   if (!lonVar->add_att("units", "degrees_east"))
      return NC_ERR;
       
   // Define the netCDF variables for the pressure and temperature
   // data.
   NcVar *presVar, *tempVar;
   if (!(presVar = dataFile.add_var("pressure", ncFloat, recDim, 
				    lvlDim, latDim, lonDim)))
      return NC_ERR;
   if (!(tempVar = dataFile.add_var("temperature", ncFloat, recDim, 
				    lvlDim, latDim, lonDim)))
      return NC_ERR;       

   // Define units attributes for data variables.
   if (!presVar->add_att("units", "hPa"))
      return NC_ERR;       
   if (!tempVar->add_att("units", "celsius"))
      return NC_ERR;       

   // Write the coordinate variable data to the file.
   if (!latVar->put(lats, NLAT))
      return NC_ERR;       
   if (!lonVar->put(lons, NLON))
      return NC_ERR;       
            
   // Write the pretend data. This will write our surface pressure and
   // surface temperature data. The arrays only hold one timestep
   // worth of data. We will just rewrite the same data for each
   // timestep. In a real application, the data would change between
   // timesteps.
   for (int rec = 0; rec < NREC; rec++) 
   {
      if (!presVar->put_rec(&pres_out[0][0][0], rec))
	 return NC_ERR;       
      if (!tempVar->put_rec(&temp_out[0][0][0], rec))
	 return NC_ERR;       
   }

   // The file is automatically closed by the destructor. This frees
   // up any internal netCDF resources associated with the file, and
   // flushes any buffers.
   
   cout << "*** SUCCESS writing example file pres_temp_4D.nc!" << endl;
   return 0;
}
