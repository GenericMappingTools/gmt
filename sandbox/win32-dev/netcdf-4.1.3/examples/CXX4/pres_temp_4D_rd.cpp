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
#include <netcdf>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

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
   
   try
   {
   // Open the file.
     NcFile dataFile("pres_temp_4D.nc", NcFile::read);

   // Get the latitude and longitude variables and read data.
   NcVar latVar, lonVar;
   latVar = dataFile.getVar("latitude");
   if(latVar.isNull()) return NC_ERR;
   lonVar = dataFile.getVar("longitude");
   if(lonVar.isNull()) return NC_ERR;
   lonVar.getVar(lons);
   latVar.getVar(lats);

   // Check the coordinate variable data. 
   for (int lat = 0; lat < NLAT; lat++)
       if (lats[lat] != START_LAT + 5. * lat)
	 return NC_ERR;

   for (int lon = 0; lon < NLON; lon++)
      if (lons[lon] != START_LON + 5. * lon)
 	return NC_ERR;
  
   // Get the pressure and temperature variables and read data one time step at a time
   NcVar presVar, tempVar;
   presVar = dataFile.getVar("pressure");
   if(presVar.isNull()) return NC_ERR;
   tempVar = dataFile.getVar("temperature");
   if(tempVar.isNull()) return NC_ERR;

   vector<size_t> startp,countp;
   startp.push_back(0);
   startp.push_back(0);
   startp.push_back(0);
   startp.push_back(0);
   countp.push_back(1);
   countp.push_back(NLVL);
   countp.push_back(NLAT);
   countp.push_back(NLON);
   for (size_t rec = 0; rec < NREC; rec++)
   {
     // Read the data one record at a time.
     startp[0]=rec;
     presVar.getVar(startp,countp,pres_in);
     tempVar.getVar(startp,countp,temp_in);
     
     int i=0;  //used in the data generation loop
     for (int lvl = 0; lvl < NLVL; lvl++)
       for (int lat = 0; lat < NLAT; lat++)
	 for (int lon = 0; lon < NLON; lon++)
	   {
	     if(pres_in[lvl][lat][lon] != (float) (SAMPLE_PRESSURE + i)) return NC_ERR;
	     if(temp_in[lvl][lat][lon] != (float)(SAMPLE_TEMP + i++)) return NC_ERR;
	   }
     
   } // next record 
       
   // The file is automatically closed by the destructor. This frees
   // up any internal netCDF resources associated with the file, and
   // flushes any buffers.

   // cout << "*** SUCCESS reading example file pres_temp_4D.nc!" << endl;
   return 0;

   }
   catch(NcException& e)
   {
      e.what();
      cout<<"FAILURE**************************"<<endl;
      return NC_ERR;
   }
  
}
