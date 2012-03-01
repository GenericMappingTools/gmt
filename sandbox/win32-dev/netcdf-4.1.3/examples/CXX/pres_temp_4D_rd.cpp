/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is an example which reads some 4D pressure and temperature
   values. The data file read by this program is produced by the
   companion program pres_temp_4D_wr.cpp. It is intended to illustrate
   the use of the netCDF C++ API.

   This program is part of the netCDF tutorial:
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
   float lats[NLAT], lons[NLON];
   
   // These arrays will hold the data we will read in. We will only
   // need enough space to hold one timestep of data; one record.
   float pres_in[NLVL][NLAT][NLON];
   float temp_in[NLVL][NLAT][NLON];
   
   // Change the error behavior of the netCDF C++ API by creating an
   // NcError object. Until it is destroyed, this NcError object will
   // ensure that the netCDF C++ API returns error codes on any
   // failure, prints an error message, and leaves any other error
   // handling to the calling program. In the case of this example, we
   // just exit with an NC_ERR error code.
   NcError err(NcError::verbose_nonfatal);

   // Open the file.
   NcFile dataFile("pres_temp_4D.nc", NcFile::ReadOnly);
   
   // Check to see if the file was opened.
   if(!dataFile.is_valid())
      return NC_ERR;

   // Get pointers to the latitude and longitude variables.
   NcVar *latVar, *lonVar;
   if (!(latVar = dataFile.get_var("latitude")))
      return NC_ERR;
   if (!(lonVar = dataFile.get_var("longitude")))
      return NC_ERR;
       
   // Get the lat/lon data from the file.
   if (!latVar->get(lats, NLAT))
      return NC_ERR;
   if (!lonVar->get(lons, NLON))
      return NC_ERR;

   // Check the coordinate variable data. 
   for (int lat = 0; lat < NLAT; lat++)
      if (lats[lat] != START_LAT + 5. * lat)
	 return NC_ERR;
   for (int lon = 0; lon < NLON; lon++)
      if (lons[lon] != START_LON + 5. * lon)
	 return NC_ERR;

   // Get pointers to the pressure and temperature variables.
   NcVar *presVar, *tempVar;
   if (!(presVar = dataFile.get_var("pressure")))
      return NC_ERR;
   if (!(tempVar  = dataFile.get_var("temperature")))
      return NC_ERR;

   // Read the data. Since we know the contents of the file we know
   // that the data arrays in this program are the correct size to
   // hold one timestep. 
   for (int rec = 0; rec < NREC; rec++)
   {
      // Read the data one record at a time.
      if (!presVar->set_cur(rec, 0, 0, 0))
	 return NC_ERR;
      if (!tempVar->set_cur(rec, 0, 0, 0))
	 return NC_ERR;

      // Get 1 record of NLVL by NLAT by NLON values for each variable.
      if (!presVar->get(&pres_in[0][0][0], 1, NLVL, NLAT, NLON))
	 return NC_ERR;
      if (!tempVar->get(&temp_in[0][0][0], 1, NLVL, NLAT, NLON))
	 return NC_ERR;
	   	   
      // Check the data. 
      int  i = 0;
      for (int lvl = 0; lvl < NLVL; lvl++)
	 for (int lat = 0; lat < NLAT; lat++)
	    for (int lon = 0; lon < NLON; lon++)
	       if (pres_in[lvl][lat][lon] != SAMPLE_PRESSURE + i ||
		   temp_in[lvl][lat][lon] != SAMPLE_TEMP + i++) 
		  return NC_ERR;
   } // next record 
       
   // The file is automatically closed by the destructor. This frees
   // up any internal netCDF resources associated with the file, and
   // flushes any buffers.

   cout << "*** SUCCESS reading example file pres_temp_4D.nc!" << endl;
   return 0;
}
