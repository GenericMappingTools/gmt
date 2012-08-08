/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is an example which reads some surface pressure and
   temperatures. The data file read by this program is produced
   companion program sfc_pres_temp_wr.cxx. It is intended to
   illustrate the use of the netCDF C++ API.

   This program is part of the netCDF tutorial:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

   Full documentation of the netCDF C++ API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx

   $Id$
*/

#include <iostream>
#include <cstring>
#include <netcdfcpp.h>

using namespace std;

// We are reading 2D data, a 6 x 12 lat-lon grid.
static const int NLAT = 6;
static const int NLON = 12;

// These are used to calculate the values we expect to find. 
static const float SAMPLE_PRESSURE = 900;
static const float SAMPLE_TEMP = 9.0;
static const float START_LAT = 25.0;
static const float START_LON = -125.0;

// Return this code to the OS in case of failure.
static const int NC_ERR = 2;

int main(void)
@{
   // These will hold our pressure and temperature data.
   float presIn[NLAT][NLON];
   float tempIn[NLAT][NLON];

   // These will hold our latitudes and longitudes.
   float latsIn[NLAT];
   float lonsIn[NLON];
  
   // Change the error behavior of the netCDF C++ API by creating an
   // NcError object. Until it is destroyed, this NcError object will
   // ensure that the netCDF C++ API silently returns error codes on
   // any failure, and leaves any other error handling to the calling
   // program. In the case of this example, we just exit with an
   // NC_ERR error code.
   NcError err(NcError::silent_nonfatal);

   // Open the file and check to make sure it's valid.
   NcFile dataFile("sfc_pres_temp.nc", NcFile::ReadOnly);
   if(!dataFile.is_valid())
      return NC_ERR;
  
   // There are a number of inquiry functions in netCDF which can be
   // used to learn about an unknown netCDF file. In this case we know
   // that there are 2 netCDF dimensions, 4 netCDF variables, no
   // global attributes, and no unlimited dimension.
   if (dataFile.num_dims() != 2 || dataFile.num_vars() != 4 || 
       dataFile.num_atts() != 0 || dataFile.rec_dim() != 0)  
      return NC_ERR;
     
   // We get back a pointer to each NcVar we request. Get the
   // latitude and longitude coordinate variables.
   NcVar *latVar, *lonVar;
   if (!(latVar = dataFile.get_var("latitude")))
      return NC_ERR;
   if (!(lonVar = dataFile.get_var("longitude")))
      return NC_ERR;
       
   // Read the latitude and longitude coordinate variables into arrays
   // latsIn and lonsIn.
   if (!latVar->get(latsIn, NLAT))
      return NC_ERR;
   if (!lonVar->get(lonsIn, NLON))
      return NC_ERR;
       
   // Check the coordinate variable data. 
   for(int lat = 0; lat < NLAT; lat++)
      if (latsIn[lat] != START_LAT + 5. * lat)
	 return NC_ERR;
   
   // Check longitude values.
   for (int lon = 0; lon < NLON; lon++)
      if (lonsIn[lon] != START_LON + 5. * lon)
	 return NC_ERR;
   
   // We get back a pointer to each NcVar we request. 
   NcVar *presVar, *tempVar;
   if (!(presVar = dataFile.get_var("pressure")))
      return NC_ERR;
   if (!(tempVar = dataFile.get_var("temperature")))
      return NC_ERR;
       
   // Read the data. Since we know the contents of the file we know
   // that the data arrays in this program are the correct size to
   // hold all the data.
   if (!presVar->get(&presIn[0][0], NLAT, NLON))
      return NC_ERR;
   if (!tempVar->get(&tempIn[0][0], NLAT, NLON))
      return NC_ERR;
       
   // Check the data. 
   for (int lat = 0; lat < NLAT; lat++)
      for (int lon = 0; lon < NLON; lon++)
	 if (presIn[lat][lon] != SAMPLE_PRESSURE + (lon * NLAT + lat)
	     || tempIn[lat][lon] != SAMPLE_TEMP + .25 * (lon * NLAT + lat))
	    return NC_ERR;
   
   // Each of the netCDF variables has a "units" attribute. Let's read
   // them and check them.
   NcAtt *att;
   char *units;
   
   if (!(att = latVar->get_att("units")))
      return NC_ERR;
   units = att->as_string(0);
   if (strncmp(units, "degrees_north", strlen("degrees_north")))
      return NC_ERR;
   // Attributes and attribute values should be deleted by the caller
   // when no longer needed, to prevent memory leaks.
   delete units;
   delete att;

   if (!(att = lonVar->get_att("units")))
      return NC_ERR;
   units = att->as_string(0);
   if (strncmp(units, "degrees_east", strlen("degrees_east")))
      return NC_ERR;
   delete units;
   delete att;
   
   if (!(att = presVar->get_att("units")))
      return NC_ERR;
   units = att->as_string(0);
   if (strncmp(units, "hPa", strlen("hPa")))
      return NC_ERR;
   delete units;
   delete att;
   
   if (!(att = tempVar->get_att("units")))
      return NC_ERR;
   units = att->as_string(0);
   if (strncmp(units, "celsius", strlen("celsius")))
      return NC_ERR;
   delete units;
   delete att;

   // The file will be automatically closed by the destructor. This
   // frees up any internal netCDF resources associated with the file,
   // and flushes any buffers.
   cout << "*** SUCCESS reading example file sfc_pres_temp.nc!" << endl;

   return 0;
@}
