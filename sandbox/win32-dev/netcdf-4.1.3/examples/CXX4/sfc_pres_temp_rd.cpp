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
#include <string>
#include <netcdf>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

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
{
   // These will hold our pressure and temperature data.
   float presIn[NLAT][NLON];
   float tempIn[NLAT][NLON];

   // These will hold our latitudes and longitudes.
   float latsIn[NLAT];
   float lonsIn[NLON];
  
  try
  {
   // Open the file and check to make sure it's valid.
   NcFile dataFile("sfc_pres_temp.nc", NcFile::read);

   // There are a number of inquiry functions in netCDF which can be
   // used to learn about an unknown netCDF file. In this case we know
   // that there are 2 netCDF dimensions, 4 netCDF variables, no
   // global attributes, and no unlimited dimension.

   //cout<<"there are "<<dataFile.getVarCount()<<" variables"<<endl;
   //cout<<"there are "<<dataFile.getAttCount()<<" attributes"<<endl;
   //cout<<"there are "<<dataFile.getDimCount()<<" dimensions"<<endl;
   //cout<<"there are "<<dataFile.getGroupCount()<<" groups"<<endl;
   //cout<<"there are "<<dataFile.getTypeCount()<<" types"<<endl;
     
   // Get the  latitude and longitude coordinate variables and read data
   NcVar latVar, lonVar;
   latVar = dataFile.getVar("latitude");
   if(latVar.isNull()) return NC_ERR;
   lonVar = dataFile.getVar("longitude");
   if(lonVar.isNull()) return NC_ERR;
   latVar.getVar(latsIn);
   lonVar.getVar(lonsIn);

   // Check the coordinate variable data. 
   for(int lat = 0; lat < NLAT; lat++)
      if (latsIn[lat] != START_LAT + 5. * lat)
	 return NC_ERR;
   
   // Check longitude values.
   for (int lon = 0; lon < NLON; lon++)
      if (lonsIn[lon] != START_LON + 5. * lon)
	 return NC_ERR;

   // Read in presure and temperature variables and read data
   NcVar presVar, tempVar;
   presVar = dataFile.getVar("pressure");
   if(presVar.isNull()) return NC_ERR;
   tempVar = dataFile.getVar("temperature");
   if(tempVar.isNull()) return NC_ERR;
   presVar.getVar(presIn);
   tempVar.getVar(tempIn);
       
   // Check the data. 
   for (int lat = 0; lat < NLAT; lat++)
      for (int lon = 0; lon < NLON; lon++)
	 if (presIn[lat][lon] != SAMPLE_PRESSURE + (lon * NLAT + lat)
	     || tempIn[lat][lon] != SAMPLE_TEMP + .25 * (lon * NLAT + lat))
	    return NC_ERR;
   
   // Each of the netCDF variables has a "units" attribute. Let's read
   // them and check them.
   NcVarAtt att;
   string units;
   
   att = latVar.getAtt("units");
   if(att.isNull()) return NC_ERR;
   
   att.getValues(units);
   if (units != "degrees_north")
     {
       cout<<"getValue returned "<<units<<endl;
       return NC_ERR;
     }
   

   att = lonVar.getAtt("units");
   if(att.isNull()) return NC_ERR;
   
   att.getValues(units);
   if (units != "degrees_east")
     {
       cout<<"getValue returned "<<units<<endl;
       return NC_ERR;
     }
   
   att = presVar.getAtt("units");
   if(att.isNull()) return NC_ERR;
   
   att.getValues(units);
   if (units != "hPa")
     {
       cout<<"getValue returned "<<units<<endl;
       return NC_ERR;
     }
   
   att = tempVar.getAtt("units");
   if(att.isNull()) return NC_ERR;
   
   att.getValues(units);
   if (units != "celsius")
     {
       cout<<"getValue returned "<<units<<endl;
       return NC_ERR;
     }
   
   // The file will be automatically closed by the destructor. This
   // frees up any internal netCDF resources associated with the file,
   // and flushes any buffers.
   //cout << "*** SUCCESS reading example file sfc_pres_temp.nc!" << endl;
   return 0;
  }
  catch(NcException e)
  {
     e.what();
     cout<<"FAILURE********************************8"<<endl;
     return NC_ERR;
  }
}
