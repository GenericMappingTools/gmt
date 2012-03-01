/*
  Copyright 2006, University Corporation for Atmospheric Research. See
  COPYRIGHT file for copying and redistribution conditions.

  This file is part of the NetCDF CF Library. 

  This file handles the libcf subsetting.

  Ed Hartnett, 5/1/7

  $Id$
*/

#include <config.h>
#include <cferror.h>
#include <libcf.h>
#include <libcf_int.h>
#include <netcdf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NDIMS 4
#define TIME 0
#define LVL 1
#define LON 2
#define LAT 3
#define START 0
#define END 1
#define UNSET -1
#define MAX_DIMS 7

static int
find_coord_idx(int ncid, int vid, size_t len, float *range, int *nidxp,  
	       size_t *startp, size_t *endp)
{
   int i;
   float *coord;
   int start = UNSET, end = UNSET, nidx;
   int ret;

   if (!range)
   {
      /* If lat range are not provided, take all lats. */
      nidx = len;
      start = 0;
      end = len - 1;
   }
   else
   {
      /* The end must be greater than or equal to the start. */
      if (range[END] < range[START])
	 return CF_EBADRANGE;

      /* Get memory for the coordinate data. */
      if (!(coord = malloc(sizeof(float) * len)))
	 return CF_ENOMEM;

      /* Open the coordinate varid and learn the values. */
      if ((ret = nc_get_var_float(ncid, vid, coord)))
      {
	 free(coord);
	 return ret;
      }

      /* Is this increasing or decreasing? */
      if (len > 1 && coord[0] < coord[1])
      {
	 /* Increasing coord. */
	 /* Find the start and end indexes. */
	 LOG((1, "range: %f  %f", range[0], range[1]));
	 for (i = 0; i < len; i++)
	 {
	    LOG((3, "coord[%d]=%f", i, coord[i]));
	    if (start == UNSET && coord[i] >= range[START] &&
		coord[i] <= range[END])
	       start = i;

	    if (start != UNSET)
	    {	       
	       if (coord[i] == range[END])
	       {
		  end = i;
		  break;
	       }
	       else if (coord[i] > range[END])
	       {
		  end = i - 1;
		  break;
	       }
	    }
	 }
      }
      else
      {
	 /* Decreasing coord. */
	 /* Find the start and end indexes. */
	 LOG((1, "range: %f  %f", range[0], range[1]));
	 for (i = 0; i < len; i++)
	 {
	    LOG((3, "coord[%d]=%f", i, coord[i]));
	    if (start == UNSET && coord[i] >= range[START] &&
		coord[i] <= range[END])
	       start = i;
	    
	    if (start != UNSET && coord[i] == range[START])
	    {
	       end = i;
	       break;
	    }
	 }
      }
      LOG((1, "start %d end %d", start, end));

      /* If start is unset, no data are in the range. If we found a
       * start but not an end, take all the data we can get. */
      if (start == UNSET)
	 nidx = 0;
      else if (end == UNSET)
      {
	 end = len - 1;
	 nidx = abs(abs(end) - abs(start)) + 1;
      }
      else
	 nidx = abs(abs(end) - abs(start)) + 1;
      free(coord);
   }

   if (startp)
      *startp = start;
   if (endp)
      *endp = end;
   if (nidxp)
      *nidxp = nidx;

   return CF_NOERR;
}

int 
nccf_get_vara(int ncid, int varid, float *lat_range, int *nlat, float *lon_range, 
	      int *nlon, int lvl_index, int timestep, void *data)
{
   nc_type xtype[NDIMS];
   char name[NDIMS][NC_MAX_NAME];
   int did[NDIMS], vid[NDIMS];
   size_t len[NDIMS];
   size_t start[NDIMS], end[NDIMS];
   int ndims, dimids[MAX_DIMS];
   size_t dstart[NDIMS], dcount[NDIMS];
   int num[NDIMS];
   int found_lvl = 0, found_time = 0;
   int d;
   int ret;

   /* Get information about each coordinate axis. */
   if ((ret = nccf_inq_latitude(ncid, &len[LAT], &xtype[LAT], &did[LAT], &vid[LAT])))
      return ret;
   if ((ret = nccf_inq_longitude(ncid, &len[LON], &xtype[LON], &did[LON], &vid[LON])))
      return ret;

   /* Get the vertical level info, if there is one. */
   ret = nccf_inq_lvl(ncid, name[LVL], &len[LVL], &xtype[LVL], NULL, NULL, NULL, 
		      &did[LVL], &vid[LVL]);
   if (ret && ret != CF_ENOTFOUND)
      return ret;
   else if (ret == CF_NOERR)
      found_lvl++;

   /* Get the time info, if there is a time axis. */
   ret = nccf_inq_time(ncid, name[TIME], &len[TIME], &xtype[TIME], &did[TIME], 
		       &vid[TIME]);
   if (ret && ret != CF_ENOTFOUND)
      return ret;
   else if (ret == CF_NOERR)
      found_time++;

   /* Find the indixies along the latitude coordinate values to subset
    * along this dimension. */
   if ((ret = find_coord_idx(ncid, vid[LAT], len[LAT], lat_range, &num[LAT], 
			     &start[LAT], &end[LAT])))
      return ret;
   if (nlat)
      *nlat = num[LAT];

   /* Find the indixies along the longitude coordinate values to subset
    * along this dimension. */
   if ((ret = find_coord_idx(ncid, vid[LON], len[LON], lon_range, &num[LON], 
			     &start[LON], &end[LON])))
      return ret;
   if (nlon)
      *nlon = num[LON];

   /* If level indicies are not provided, take all levels. */
/*    if (!lvl_index) */
/*       num[LVL] = len[LVL]; */
/*    else */
/*       num[LVL] = 1; */
/*    if (nlvl) */
/*       *nlvl = num[LVL]; */

   if (found_time && timestep >= len[TIME])
      return CF_ERECTOOLARGE;

   /* If the user wants the data, get it for him. */
   if (lat_range && lon_range && data && num[LAT] && num[LON])
   {
      /* Find out about the dimensions of this data variable. */
      if ((ret = nc_inq_varndims(ncid, varid, &ndims)))
	 return ret;
      if (ndims > MAX_DIMS)
	 return CF_ENDIMS;
      if ((ret = nc_inq_vardimid(ncid, varid, dimids)))
	 return ret;

      /* Set the lat info. */
      for (d = 0; d < ndims; d++)
	 if (dimids[d] == did[LAT])
	    break;
      if (d == ndims)
	 return CF_ENODIM;
      dstart[d] = start[LAT];
      dcount[d] = num[LAT];

      /* Set the lon info. */
      for (d = 0; d < ndims; d++)
	 if (dimids[d] == did[LON])
	    break;
      if (d == ndims)
	 return CF_ENODIM;
      dstart[d] = start[LON];
      dcount[d] = num[LON];

      /* Set the level info. */
      if (found_lvl)
      {
	 for (d = 0; d < ndims; d++)
	    if (dimids[d] == did[LVL])
	       break;
	 if (d != ndims)
	 {
	    dstart[d] = lvl_index;
	    dcount[d] = 1;
	 }
      }

      /* Set the time info. */
      if (found_time)
      {
	 for (d = 0; d < ndims; d++)
	    if (dimids[d] == did[TIME])
	       break;
	 if (d != ndims)
	 {
	    dstart[d] = timestep;
	    dcount[d] = 1;
	 }
      }

      /* Now I've got everything needed to read a record of this data! */
      if ((ret = nc_get_vara(ncid, varid, dstart, dcount, data)))
	 return ret;
   }

   return CF_NOERR;
}
