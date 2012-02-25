/*
  Copyright 2006, University Corporation for Atmospheric Research. See
  COPYRIGHT file for copying and redistribution conditions.

  This file is part of the NetCDF CF Library. 

  This file handles the libcf file stuff.

  Ed Hartnett, 9/1/06

  $Id$
*/

#include <config.h>
#include <libcf.h>
#include <libcf_int.h>
#include <netcdf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NLAT_UNITS 6
#define NLON_UNITS 6
#define NPRES_UNITS 31
#define NLEVEL_UNITS 35
#define NTIME_UNITS 33
#define CF_POS_LEN 4

#define CF_PTOP "PTOP"

static char *vert_std_name[] = {
   STD_ATM_LN, STD_SIGMA, STD_HYBRID_SIGMA, STD_HYBRID_HEIGHT, STD_SLEVE, 
   STD_OCEAN_SIGMA, STD_OCEAN_S, STD_OCEAN_SIGMA_Z, STD_OCEAN_DBL_SIGMA
};

/* The Common Data Model defines these axis types. */
static char* axis_type_name[] = {"", "Lat", "Lon", "GeoX", "GeoY", "GeoZ", 
				 "Height", "Height", "Pressure", "Time", 
				 "RadialAzimuth", "RadialElevation", 
				 "RadialDistance"};

int
cf_test1(int i)
{
   printf("i = %d", i);
   return 0;
}

/* Define a coordinate dimension and variable with all the CF
 * recomended attribute accessories. */
static int 
def_coord_var(int ncid, const char *name, size_t len, nc_type xtype, 
	      const char *units, const char *axis, int positive_up, 
	      const char *standard_name, const char *formula_terms, 
	      int cdm_axis_type, int *dimidp, int *varidp)
{
   int did, vid;
   int ret;

   /* Name is required. */
   if (!name)
      return CF_EINVAL;

   /* There must be neither a dimension nor a variable of the chosen
    * name. */
   if (nc_inq_dimid(ncid, name, &did) != NC_EBADDIM ||
       nc_inq_varid(ncid, name, &vid) != NC_ENOTVAR)
      return CF_EEXISTS;

   /* Create a dimension. */
   if ((ret = nc_def_dim(ncid, name, len, &did)))
      return ret;

   /* Give the user back the dimid if he wants it. */
   if (dimidp)
      *dimidp = did;

   /* Create a variable. */
   if ((ret = nc_def_var(ncid, name, xtype, 1, &did, &vid)))
      return ret;

   /* Give the user back the varid if he wants it. */
   if (varidp)
      *varidp = vid;

   /* Create required attributes for this coordinate variable. */
   if (units)
      if ((ret = nc_put_att_text(ncid, vid, CF_UNITS, strlen(units) + 1, 
				 units)))
	 return ret;

   /* If an axis was provided, then use it. This is the "axis"
    * described in the CF document, section 4. It is a text string, X,
    * Y, Z or T.*/
   if (axis)
   {
      if ((ret = nc_put_att_text(ncid, vid, CF_AXIS, strlen(axis) + 1, 
				 axis)))
	 return ret;
      
      /* If this is a vertical axis, use the "positive" attribute to
       * indicate whether it is increasing upwards, or downwards. */
      if (!strncmp(axis, CF_LEVEL_AXIS, strlen(CF_LEVEL_AXIS)))
      {
	 if (positive_up)
	 {
	    if ((ret = nc_put_att_text(ncid, vid, CF_POSITIVE, 
				       strlen(CF_UP) + 1, CF_UP)))
	       return ret;
	 }
	 else
	 {
	    if ((ret = nc_put_att_text(ncid, vid, CF_POSITIVE, 
				       strlen(CF_DOWN) + 1, CF_DOWN)))
	       return ret;
	 }
      }
   }

   /* This is the CDM axis, which is somewhat different. */
   if (cdm_axis_type)
   {
      /* Is the axis type valid? */
      if (cdm_axis_type < 0 || cdm_axis_type > NCCF_RADDIST)
	 return NC_EINVAL;
      
      /* Now write the attribute which stores the axis type. */
      if ((ret = nc_put_att_text(ncid, vid, COORDINATE_AXIS_TYPE, 
				 strlen(axis_type_name[cdm_axis_type]) + 1, 
				 axis_type_name[cdm_axis_type])))
	 return ret;
   }

   /* If formula_terms was provided, then use it. */
   if (formula_terms)
      if ((ret = nc_put_att_text(ncid, vid, CF_FORMULA_TERMS, 
				 strlen(formula_terms) + 1, 
				 formula_terms)))
	 return ret;

   /* If standard_name was provided, then use it. */
   if (standard_name)
      if ((ret = nc_put_att_text(ncid, vid, CF_STANDARD_NAME, 
				 strlen(standard_name) + 1, 
				 standard_name)))
	 return ret;

   return CF_NOERR;
}

/* Inquire about a coordinate variable. */
static int 
inq_coord_var(int ncid, int nvalid, const char valid_units[][CF_MAX_LEN + 1], 
	      const char *valid_standard_name, char *name, size_t *lenp, 
	      nc_type *xtypep, size_t *ft_lenp, char *formula_terms, 
	      int *positive_upp, int *dimidp, int *varidp)
{
   int varid, nvars, dimid;
   nc_type att_type_in, xtype_in;
   size_t att_len_in, target_len_in;
   char *target_end;
   char target_in[NC_MAX_NAME + 1], var_name[NC_MAX_NAME + 1];
   char target[NC_MAX_NAME + 1];
   char pos_in[CF_POS_LEN + 1];
   int positive_found = 0, positive_up, vnum;
   int ret;

   /* How many vars are there? */
   if ((ret = nc_inq_nvars(ncid, &nvars)))
      return ret;

   /* If there are no valid units, and no standard name, I can't find
    * a coodinate variable! */
   if (!valid_units && !valid_standard_name)
      return CF_EINVAL;

   /* Decide what we are looking for. */
   if (valid_units)
      strcpy(target, CF_UNITS);
   else
      strcpy(target, CF_STANDARD_NAME);

   /* Search for a variable with an appropriate "units" or
    * "standard_name" attribute. */
   for (varid = 0; varid < nvars; varid++)
   {
      /* Is there's no units or standard_name attribute for this
       * var, move on. */
      if ((ret = nc_inq_att(ncid, varid, target, &att_type_in, 
			    &att_len_in)) == NC_ENOTATT)
	 continue;
      
      /* If we got some other error, we're totally hosed. */
      if (ret != NC_NOERR)
	 return ret;
      
      /* If the attribute is not a character string, or if it's longer
       * than NC_MAX_NAME, then what the heck is going on? Skip it. */
      if (att_type_in != NC_CHAR || att_len_in > NC_MAX_NAME)
	 continue;

      /* Get the value of the attribute. */
      if ((ret = nc_get_att_text(ncid, varid, target, target_in)))
	 return ret;

      /* We only care about this string before the first space. */
      target_end = index(target_in, ' ');
      if (target_end)
	 target_len_in = target_end - target_in - 1;
      else
	 target_len_in = att_len_in;
      
      if (valid_units)
      {
	 /* If the length and value matches on of our valid strings,
	  * then we have found our coordinate variable. */
	 for (vnum = 0; vnum < nvalid; vnum++)
	    if (target_len_in <= strlen(valid_units[vnum]) + 1 &&
		!strncmp(target_in, valid_units[vnum], strlen(valid_units[vnum])))
	       break;

	 /* If we found the coordinate var, stop looking. */
	 if (vnum < nvalid)
	    break;
      }
      else
      {
	 if (target_len_in <= strlen(valid_standard_name) + 1 &&
	     !strncmp(target_in, valid_standard_name, strlen(valid_standard_name)))
	    break;
      }

   } /* next varid*/
   
   /* Did we still not find our coordinate variable? */
   if (varid == nvars)
   {
      /* Let's look for an attribute named "positve". */
      for (varid = 0; varid < nvars; varid++)
      {
	 /* Is there's no "positive" attribute for this var, move on. */
	 if ((ret = nc_inq_att(ncid, varid, CF_POSITIVE, &att_type_in, 
			       &att_len_in)) == NC_ENOTATT)
	    continue;
      
	 /* If we got some other error, we're totally hosed. */
	 if (ret != NC_NOERR)
	    return ret;
      
	 /* If the "positive" attribute is not a character string,
	  * or it's too long, then what the heck is going on? Skip
	  * it. */
	 if (att_type_in != NC_CHAR || att_len_in > strlen(CF_DOWN) + 1)
	    continue;
      
	 /* Get the value of the "positive" attribute. */
	 if ((ret = nc_get_att_text(ncid, varid, CF_POSITIVE, pos_in)))
	    return ret;
	 if (strncasecmp(pos_in, CF_DOWN, strlen(CF_DOWN)) == 0)
	    positive_up = 0;
	 else if (strncasecmp(pos_in, CF_UP, strlen(CF_UP)) == 0)
	    positive_up = 1;
	 else
	    return CF_EBADVALUE;

	 /* Remember that we have already found and dealt with the
	  * "positive" attribute. */
	 positive_found++;
      
	 /* We found the coordinate var, stop looking. */
	 break;
      
      } /* next varid*/
      
      if (varid == nvars)
	 return CF_ENOTFOUND;
   }

   /* Does the user want the varid of this coordinate variable? */
   if (varidp)
      *varidp = varid;

   /* We need to know some stuff. */
   if ((ret = nc_inq_vartype(ncid, varid, &xtype_in)))
      return ret;

   /* Does the user want the type of this coordinate variable? */
   if (xtypep)
      *xtypep = xtype_in;

   /* Is there an interest in the formula_terms attribute? */
   if (ft_lenp || formula_terms)
   {
      ret = nc_inq_att(ncid, varid, CF_FORMULA_TERMS, &att_type_in, 
		       &att_len_in);
      if (ret == NC_ENOTATT)
      {
	 if (ft_lenp)
	    *ft_lenp = 0;
      }
      else if (ret == CF_NOERR)
      {
	 /* If this is not a character attribute, what the heck is
	  * it? */
	 if (att_type_in != NC_CHAR)
	 {
	    if (ft_lenp)
	       *ft_lenp = 0;
	 }
	 else
	 {
	    if (ft_lenp)
	       *ft_lenp = att_len_in;
	    if (formula_terms)
	       if ((ret = nc_get_att_text(ncid, varid, CF_FORMULA_TERMS, 
					  formula_terms)))
		  return ret;
	 }
      }	 
   }

   if (positive_upp)
      *positive_upp = positive_up;

   if (positive_upp)
   {
      if (!positive_found)
      {
	 /* Is there a "positive" attribute? */
	 ret = nc_inq_att(ncid, varid, CF_POSITIVE, &att_type_in, &att_len_in);
	 if (ret == NC_NOERR && att_len_in <= CF_POS_LEN + 1)
	 {
	    /* Get the contents of the attribute and see if it is "up" or
	     * "down". */
	    if ((ret = nc_get_att_text(ncid, varid, CF_POSITIVE, pos_in)))
	       return ret;
	    if (strncasecmp(pos_in, CF_DOWN, strlen(CF_DOWN)))
	       *positive_upp = 0;
	    else if (strncasecmp(pos_in, CF_UP, strlen(CF_UP)))
	       *positive_upp = 1;
	    else
	       return CF_EBADVALUE;
	 }
	 if (ret == NC_ENOTATT)
	 {
	    /* No "positive" attribute there. What should be done?
	     * According to the CF standards, assume down for pressure
	     * units, up otherwise. */
	    if (vnum < NPRES_UNITS)
	       *positive_upp = 0;
	    else
	       *positive_upp = 1;
	 }
      }
   }

   /* Get the name. */
   if ((ret = nc_inq_varname(ncid, varid, var_name)))
      return ret;

   /* Does the user want the name? */
   if (name)
      strcpy(name, var_name);

   /* Now that we have our coordianate variable, let's find the
    * associated dimension. It will have the same name. */
   if ((ret = nc_inq_dimid(ncid, var_name, &dimid)))
      return CF_ENODIM;

   /* Does the user want the dimid? */
   if (dimidp)
      *dimidp = dimid;

   /* Does the user want to know the length of this coordinate
    * axis? */
   if (lenp)
      if ((ret = nc_inq_dimlen(ncid, dimid, lenp)))
	 return ret;
   return CF_NOERR;
}

/* Define a latitude dimension and variable with all the CF
 * recomended attribute accessories.*/
int 
nccf_def_latitude(int ncid, size_t len, nc_type xtype, 
		  int *lat_dimidp, int *lat_varidp)
{
   return def_coord_var(ncid, CF_LATITUDE_NAME, len,
			xtype, CF_LATITUDE_UNITS, CF_LATITUDE_AXIS,
			0, CF_LATITUDE_NAME, NULL, 
			NCCF_LATITUDE, lat_dimidp, lat_varidp);
}

/* Inquire about a latitude dimension and variable with all the CF
 * recomended attribute accessories.*/
int 
nccf_inq_latitude(int ncid, size_t *lenp, nc_type *xtypep, 
		  int *lat_dimidp, int *lat_varidp)
{
   const char val[NLAT_UNITS][CF_MAX_LEN + 1] = {"degrees_north", "degree_north", 
						 "degree_N", "degrees_N", "degreeN",
						 "degreesN"};
   return inq_coord_var(ncid, NLAT_UNITS, val, NULL, NULL, lenp, xtypep, 
			NULL, NULL, NULL, lat_dimidp, lat_varidp);
}

/* Define a longitude dimension and variable with all the CF
 * recomended attribute accessories.*/
int 
nccf_def_longitude(int ncid, size_t len, nc_type xtype, 
		   int *lon_dimidp, int *lon_varidp)
{
   return def_coord_var(ncid, CF_LONGITUDE_NAME, len,
			xtype, CF_LONGITUDE_UNITS, CF_LONGITUDE_AXIS,
			0, CF_LONGITUDE_NAME, NULL, 
			NCCF_LONGITUDE, lon_dimidp, lon_varidp);
}

/* Inquire about a longitude dimension and variable with all the CF
 * recomended attribute accessories.*/
int 
nccf_inq_longitude(int ncid, size_t *lenp, nc_type *xtypep, 
		   int *lon_dimidp, int *lon_varidp)
{
   const char val[NLON_UNITS][CF_MAX_LEN + 1] = {
      "degrees_east", "degree_east", "degree_E", 
      "degrees_E", "degreeE", "degreesE"};

   return inq_coord_var(ncid, NLON_UNITS, val, NULL, NULL, lenp, xtypep, 
			NULL, NULL, NULL, lon_dimidp, lon_varidp);
}

/* Define a verticle level coordinate variable and dimension. */
int 
nccf_def_lvl(int ncid, const char *name, size_t len, nc_type xtype, 
	     const char *units, int positive_up, 
	     const char *standard_name, const char *formula_terms, 
	     int cdm_axis_type, int *lvl_dimidp, int *lvl_varidp)
{
   return def_coord_var(ncid, name, len, xtype, units, CF_LEVEL_AXIS, 
			positive_up, standard_name, formula_terms, 
			cdm_axis_type, lvl_dimidp, lvl_varidp);
}

/* Inquire about a vertical dimension and coordinate variable. */
int 
nccf_inq_lvl(int ncid, char *name, size_t *lenp, nc_type *xtypep, 
	     size_t *ft_lenp, char *formula_terms, int *positive_upp, 
	     int *lon_dimidp, int *lon_varidp)
{
   const char val[NLEVEL_UNITS][CF_MAX_LEN + 1] = 
      {"bar", "standard_atmosphere", "technical_atmosphere", 
       "inch_H2O_39F", "inch_H2O_60F", "inch_Hg_32F", 
       "inch_Hg_60F", "millimeter_Hg_0C", "footH2O", "cmHg", 
       "cmH2O", "Pa", "inch_Hg", "inch_hg", "inHg", "in_Hg", 
       "in_hg", "millimeter_Hg", "mmHg", "mm_Hg", "mm_hg", 
       "torr", "foot_H2O", "ftH2O", "psi", "ksi", "barie", 
       "at", "atmosphere", "atm", "barye", "level", "layer", 
       "sigma_level", "sigma"};

   /* Find a coordinate var and dim with one of the units above. */
   return inq_coord_var(ncid, NLEVEL_UNITS, val, NULL, name, lenp, xtypep, 
			ft_lenp, formula_terms, positive_upp, lon_dimidp, 
			lon_varidp);
}

/* Define a time coordinate variable and dimension. */
int 
nccf_def_time(int ncid, const char *name, size_t len, nc_type xtype, 
	      const char *units, const char *standard_name, 
	      int *time_dimidp, int *time_varidp)
{
   return def_coord_var(ncid, name, len, xtype, units, CF_TIME_AXIS, 0, 
			standard_name, NULL, NCCF_TIME, time_dimidp, 
			time_varidp);
}

/* Inquire about a vertical dimension and coordinate variable. */
int 
nccf_inq_time(int ncid, char *name, size_t *lenp, nc_type *xtypep, 
	      int *time_dimidp, int *time_varidp)
{
   const char val[NTIME_UNITS][CF_MAX_LEN + 1] = {
      "second", "day", "hour", "minute", "s", 
      "sec", "shake", "sidereal_day", "sidereal_hour", 
      "sidereal_minute", "sidereal_second", 
      "sidereal_year", "tropical_year", "lunar_month", 
      "common_year", "leap_year", "Julian_year", 
      "Gregorian_year", "sidereal_month", "tropical_month", 
      "d", "min", "hr", "h", "fortnight", "week", "jiffy", 
      "jiffies", "year", "yr", "a", "eon", "month"};

   return inq_coord_var(ncid, NTIME_UNITS, val, NULL, NULL, lenp, xtypep, NULL, 
			NULL, NULL, time_dimidp, time_varidp);
}

/* Define one of the unitless vertical coordinate variables from
 * appendix D of the CF document. */
static int
def_vert_var(int ncid, const char *name, nc_type xtype, size_t len, 
	     int nterms, int *term_id, const char *ft_format, 
	     int *lvl_dimidp, int *lvl_varidp)
{
   char formula_terms[CF_MAX_FT_LEN + 1];
   char term_name[CF_MAX_FT_VARS][NC_MAX_NAME + 1];
   int v;
   int ret;

   if (strlen(name) > NC_MAX_NAME)
      return CF_EMAX_NAME;

   /* Find names of the vars to put in formula terms.*/
   for (v = 0; v < nterms; v++)
   {
      if (term_id[v] == -99)
	 strcpy(term_name[v], name);
      else
	 if ((ret = nc_inq_varname(ncid, term_id[v], term_name[v])))
	    return ret;
   }

   /* Assemble the formula terms attribute, depending on how many
    * terms are in use. */
   switch (nterms)
   {
      case 1:
	 sprintf(formula_terms, ft_format, term_name[0]);
	 break;
      case 2:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1]);
	 break;
      case 3:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2]);
	 break;
      case 4:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3]);
	 break;
      case 5:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3], term_name[4]);
	 break;
      case 6:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3], term_name[4], term_name[5]);
	 break;
      case 7:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3], term_name[4], term_name[5], 
		 term_name[6]);
	 break;
      default:
	 return CF_EINVAL;
   }
      
   /* Create the coordinate variable and dimension with this
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_SIGMA, formula_terms, NCCF_GEOZ, 
			lvl_dimidp, lvl_varidp);
}

static int
nccf_set_ft(int ncid, int varid, int nterms, int *ft_varids, 
	    const char *ft_format)
{
   char formula_terms[CF_MAX_FT_LEN + 1];
   char term_name[CF_MAX_FT_VARS][NC_MAX_NAME + 1];
   int v;
   int ret;

   /* Find names of the vars to put in formula terms.*/
   for (v = 0; v < nterms; v++)
      if ((ret = nc_inq_varname(ncid, ft_varids[v], term_name[v])))
	 return ret;

   /* Assemble the formula terms attribute, depending on how many
    * terms are in use. */
   switch (nterms)
   {
      case 1:
	 sprintf(formula_terms, ft_format, term_name[0]);
	 break;
      case 2:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1]);
	 break;
      case 3:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2]);
	 break;
      case 4:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3]);
	 break;
      case 5:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3], term_name[4]);
	 break;
      case 6:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3], term_name[4], term_name[5]);
	 break;
      case 7:
	 sprintf(formula_terms, ft_format, term_name[0], term_name[1], 
		 term_name[2], term_name[3], term_name[4], term_name[5], 
		 term_name[6]);
	 break;
      default:
	 return CF_EINVAL;
   }

   /* Attatch the formula_terms attribute to the coordinate variable. */
   if ((ret = nc_put_att_text(ncid, varid, CF_FORMULA_TERMS, 
			      strlen(formula_terms) + 1, formula_terms)))
      return ret;

   return CF_NOERR;
}

/* Find out about one of the unitless vertical coordinates in appendix
 * D of the CF document. */
static int 
inq_vert_var(int ncid, const char *standard_name, int nterms, 
	     const char *ft_format, int *termids, char *name, 
	     nc_type *xtypep, size_t *lenp, int *lvl_dimidp, 
	     int *lvl_varidp)
{
   char formula_terms[CF_MAX_FT_LEN + 1];
   char term_name[FT_MAX_TERMS][CF_MAX_FT_LEN + 1];
   size_t ft_len;
   int t;
   int ret;
   
   /* Get some of the info, including the formula_terms attribute. */
   if ((ret = inq_coord_var(ncid, 0, NULL, standard_name, name, lenp, 
			    xtypep, &ft_len, formula_terms, NULL, 
			    lvl_dimidp, lvl_varidp)))
      return ret;
   
   /* Was the formula_terms attribute too big? */
   if (ft_len > CF_MAX_FT_LEN)
      return CF_EMAXFT;

   /* Parse the formula_terms attribute, depending on the number of
    * terms in use. */
   switch (nterms)
   {
      case 1:
	 if (sscanf(formula_terms, ft_format, term_name[0]) != nterms)
	    return CF_EBADFT;
	 break;
      case 2:
	 if (sscanf(formula_terms, ft_format, term_name[0], 
		    term_name[1]) != nterms)
	    return CF_EBADFT;
	 break;
      case 3:
	 if (sscanf(formula_terms, ft_format, term_name[0], term_name[1], 
		    term_name[2]) != nterms)
	    return CF_EBADFT;
	 break;
      case 4:
	 if (sscanf(formula_terms, ft_format, term_name[0], term_name[1], 
		    term_name[2], term_name[3]) != nterms)
	    return CF_EBADFT;
	 break;
      case 5:
	 if (sscanf(formula_terms, ft_format, term_name[0], term_name[1], 
		    term_name[2], term_name[3], term_name[4]) != nterms)
	    return CF_EBADFT;
	 break;
      case 6:
	 if (sscanf(formula_terms, ft_format, term_name[0], term_name[1], 
		    term_name[2], term_name[3], term_name[4], 
		    term_name[5]) != nterms)
	    return CF_EBADFT;
	 break;
      case 7:
	 if (sscanf(formula_terms, ft_format, term_name[0], term_name[1], 
		    term_name[2], term_name[3], term_name[4], term_name[5], 
		    term_name[6]) != nterms)
	    return CF_EBADFT;
	 break;
      default:
	 return CF_EINVAL;
   }

   /* Convert the names of the terms into variable ids, and return
    * them in the termids array. */
   if (termids)
      for (t = 0; t < nterms; t++)
	 if ((ret = nc_inq_varid(ncid, term_name[t], &termids[t])))
	    return ret;

   return CF_NOERR;
}

/*
  Atmosphere natural log pressure coordinate

  standard_name = "atmosphere_ln_pressure_coordinate"

  Definition:

  p(k) = p0 * exp(-lev(k))

  where p(k) is the pressure at gridpoint (k), p0 is a reference
  pressure, lev(k) is the dimensionless coordinate at vertical gridpoint
  (k).

  The format for the formula_terms attribute is

  formula_terms = "p0: var1 lev: var2"
*/
int 
nccf_def_lvl_atm_ln(int ncid, const char *name, nc_type xtype, size_t len, 
		    int *lvl_dimidp, int *lvl_varidp)
{
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_ATM_LN, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

int 
nccf_def_ft_atm_ln(int ncid, int varid, int pref_varid)
{
   int ft_varids[FT_MAX_TERMS];
   
   ft_varids[0] = pref_varid;

   return nccf_set_ft(ncid, varid, FT_ATM_LN_TERMS, ft_varids, FT_ATM_LN_FORMAT);
}

/* Atmosphere natural log pressure coordinate. */
int 
nccf_inq_lvl_atm_ln(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
		    int *pref_varidp, int *lvl_dimidp, int *lvl_varidp)
{
   int termids[FT_ATM_LN_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_ATM_LN, FT_ATM_LN_TERMS, FT_ATM_LN_FORMAT, 
			   termids, name, xtypep, lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for atm_ln level coordinate formula terms. */
   if (pref_varidp)
      *pref_varidp = termids[0];

   return CF_NOERR;
}

/* Create a unitless vertical dimension/variable from appendix D of
 * the CF standard. */
int 
nccf_def_lvl_vert(int ncid, int lvl_type, const char *name, nc_type xtype, 
		  size_t len, int *lvl_dimidp, int *lvl_varidp)
{
   /* Must be a known level type... */
   if (lvl_type < 0 || lvl_type >= CF_NUM_VERT)
      return CF_EINVAL;

   /* Do the deed! */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 0, 
			vert_std_name[lvl_type], NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

/* Define a vertical sigma coordinate dimension. 
   From the CF doc:

   Atmosphere sigma coordinate:  
   float lev(lev) ;
   lev:long_name = "sigma at layer midpoints" ;
   lev:positive = "down" ;
   lev:standard_name = "atmosphere_sigma_coordinate" ;
   lev:formula_terms = "sigma: lev ps: PS ptop: PTOP" ;
*/
int 
nccf_def_lvl_sigma(int ncid, const char *name, nc_type xtype, size_t len, 
		   int *lvl_dimidp, int *lvl_varidp)
{
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_SIGMA, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

/* Set the formula_terms to contain information about this level. */
int
nccf_def_ft_sigma(int ncid, int lvl_vid, int ps_vid, int p0_vid)
{
   int ft_varids[FT_MAX_TERMS];
   
   ft_varids[0] = lvl_vid;
   ft_varids[1] = ps_vid;
   ft_varids[2] = p0_vid;
   return nccf_set_ft(ncid, lvl_vid, FT_SIGMA_TERMS, ft_varids, FT_SIGMA_FORMAT);
}

int 
nccf_inq_lvl_vert(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
		  int *lvl_typep, int *lvl_dimidp, int *lvl_varidp)
{
   char dim_name[NC_MAX_NAME + 1];
   int varid, d, ndims, v;
   size_t name_len;
   char standard_name[NC_MAX_NAME + 1], var_name[NC_MAX_NAME + 1];
   size_t dim_len;
   nc_type var_type;
   int ret;

   /* Loop thru vars in file to see if any has a standard name that
    * matches one of the standard names of the appendix D vertical
    * dimensions.*/
   if ((ret = nc_inq_ndims(ncid, &ndims)))
      return ret;
   for (d = 0; d < ndims; d++)
   {
      /* Any var with same name as this dim? */
      if ((ret = nc_inq_dimname(ncid, d, dim_name)))
	 return ret;
      ret = nc_inq_varid(ncid, dim_name, &varid);
      if (ret == NC_ENOTVAR)
	 continue; 
      else if (ret)
	 return ret;
      else
      {
	 /* Find length of standard_name attribute, if it exists. */
	 ret = nc_inq_attlen(ncid, varid, CF_STANDARD_NAME, &name_len);
	 if (ret == NC_ENOTATT)
	    continue;
	 else if (ret)
	    return ret;

	 /* If the name is not too long, read it in. */
	 if (name_len > NC_MAX_NAME)
	    return CF_EMAX_NAME;
	 if ((ret = nc_get_att_text(ncid, varid, CF_STANDARD_NAME, standard_name)))
	    return ret;

	 /* Compare the standard name to our list of unitless vert
	  * dimension standard names. */
	 for (v = 0; v < CF_NUM_VERT; v++)
	 {
	    if (!strncmp(vert_std_name[v], standard_name, strlen(vert_std_name[v])))
	       break;
	 }
	 if (v < CF_NUM_VERT)
	    break;
      }
   }

   /* If we went thru all dimensions, I guess we didn't find a
    * unitless vertical coordinate variable. Boo hoo! */
   if (d == ndims)
      return CF_ENOTFOUND;

   /* Learn the type and name from the coordinate variable. */
   if ((ret = nc_inq_var(ncid, v, var_name, &var_type, NULL, NULL, NULL)))
      return ret;

   /* Learn the length from the coordinate dimension. */
   if ((ret = nc_inq_dim(ncid, d, NULL, &dim_len)))
      return ret;

   /* Give the user the results they are interested in. */
   if (name)
      strcpy(name, var_name);

   if (xtypep)
      *xtypep = var_type;

   if (lenp)
      *lenp = dim_len;

   if (lvl_dimidp)
      *lvl_dimidp = d;

   if (lvl_typep)
      *lvl_typep = v;

   if (lvl_varidp)
      *lvl_varidp = varid;

   return CF_NOERR;
}

/* Inquire about atmosphere sigma coordinate. */
int 
nccf_inq_lvl_sigma(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
		   int *ps_varidp, int *ptop_varidp, int *lvl_dimidp, 
		   int *lvl_varidp)
{
   int termids[FT_SIGMA_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_SIGMA, FT_SIGMA_TERMS, FT_SIGMA_FORMAT, 
			   termids, name, xtypep, lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for sigma level coordinate formula terms. */
   if (ps_varidp)
      *ps_varidp = termids[1];
   if (ptop_varidp)
      *ptop_varidp = termids[2];

   return CF_NOERR;
}

/*
  Atmosphere hybrid sigma pressure coordinate

  standard_name = "atmosphere_hybrid_sigma_pressure_coordinate"

  Definition:

  p(n,k,j,i) = a(k)*p0 + b(k)*ps(n,j,i)

  or

  p(n,k,j,i) = ap(k) + b(k)*ps(n,j,i)

  where p(n,k,j,i) is the pressure at gridpoint (n,k,j,i), a(k) or ap(k)
  and b(k) are components of the hybrid coordinate at level k, p0 is a
  reference pressure, and ps(n,j,i) is the surface pressure at
  horizontal gridpoint (j,i) and time (n). The choice of whether a(k) or
  ap(k) is used depends on model formulation; the former is a
  dimensionless fraction, the latter a pressure value. In both
  formulations, b(k) is a dimensionless fraction.

  The format for the formula_terms attribute is

  formula_terms = "a: var1 b: var2 ps: var3 p0: var4"

  where a is replaced by ap if appropriate.

  The hybrid sigma-pressure coordinate for level k is defined as
  a(k)+b(k) or ap(k)/p0+b(k), as appropriate.

*/
int 
nccf_def_lvl_hybrid_sigma(int ncid, const char *name, nc_type xtype, size_t len, 
			  int *lvl_dimidp, int *lvl_varidp)
{
   /* Create the coordinate variable and dimension with no
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_HYBRID_SIGMA, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

/* Set the formula_terms atribute to contain all this information. */
int
nccf_def_ft_hybrid_sigma(int ncid, int lvl_vid, int a_vid, int b_vid, int ps_vid, 
			 int p0_vid)
{
   int ft_varids[FT_MAX_TERMS];
   
   ft_varids[0] = a_vid;
   ft_varids[1] = b_vid;
   ft_varids[2] = ps_vid;
   ft_varids[3] = p0_vid;
   return nccf_set_ft(ncid, lvl_vid, FT_HYBRID_SIGMA_TERMS, ft_varids, 
		      FT_HYBRID_SIGMA_FORMAT);
}

/* #define CF_4D 4 */
/* int */
/* nccf_def_hybrid_sigma(int ncid, char names[CF_4D][NC_MAX_NAME+1], int *lens,  */
/* 		      nc_type *types, int *dids, int *vids, int *term_vids)  */
/* { */
/*    return nccf_def_coord_system(ncid, CF_VERT_HYBRID_SIGMA, names, lens, types, time_units,  */
/* 				FT_HYBRID_SIGMA_TERMS, term_names, term_types, term_ndims, */
/* 				term_dimids, did, vids, term_vids); */
/* } */

/* int */
/* nccf_def_coord_system(int ncid, int vert_axis_type, char name[CF_4D][NC_MAX_NAME+1],  */
/* 		      int *lens, nc_type *types, char *time_units, int nterms,  */
/* 		      char *term_names, nc_type *term_types, int *term_ndims,  */
/* 		      int *term_dimds, int *dids, int *vids, int *term_vids)  */
/* { */
/*    int t; */
/*    int ret; */

/*    /\* Create the latitude and longitude dimensions and variables, with */
/*     * appropriate attributes. *\/ */
/*    if ((ret = nccf_def_latitude(ncid, lens[0], types[0], &did[0], &vid[0]))) */
/*       return ret; */
/*    if ((ret = nccf_def_longitude(ncid, lens[1], types[1], &did[1], &vid[1]))) */
/*       return ret; */

/*    /\* Create the vertical axis dimension and variable. *\/ */
/*    if (def_coord_var(ncid, names[2], lens[2], types[2], NULL, CF_LEVEL_AXIS,  */
/* 		     0, vert_axis_type, NULL, NCCF_GEOZ, &did[2], &vid[2]))  */
/*       return ret; */

/*    /\* Now define the time coordinate variable and dimension. *\/ */
/*    if ((ret = nccf_def_time(ncid, names[3], lens[3], types[3], time_units,  */
/* 			    "time", &did[3], &vid[3]))) */
/*       return ret; */

/*    /\* Before we can set up the formula_terms attribute, we need to */
/*     * define some additional variables specific to this type of */
/*     * vertical dimension. *\/ */
/*    for (t = 0; t < nterms; t++) */
/*    { */
/*       if ((ret = nc_def_var(ncid, term_names[t], term_types[t], 1, did, &a_vid))) */
/* 	 return ret; */
/*    } */

/*    if ((ret = nc_def_var(ncid, "b", NC_FLOAT, 1, did, &b_vid))) */
/*       return ret; */
/*    if ((ret = nc_def_var(ncid, "ps", NC_FLOAT, 2, did, &ps_vid))) */
/*       return ret; */
/*    if ((ret = nc_def_var(ncid, "P0", NC_FLOAT, 0, NULL, &p0_vid))) */
/*       return ret; */

/*    if ((ret = nccf_def_ft_hybrid_sigma(ncid, lvl_vid, a_vid, b_vid, ps_vid,  */
/* 				       p0_vid))) */
/*       return ret; */

/*    return CF_NOERR; */
/* } */

/* #define A_NDIMS 1 */
/* #define B_NDIMS 1 */
/* #define P0_NDIMS 0 */
/* #define PS_NDIMS 3 */
/* int  */
/* nccf_def_lvl_hybrid_sigma_full(int ncid, const char *name, nc_type xtype, size_t len,  */
/* 			       const char *a_name, nc_type a_type, int *a_varid, const char *a_units, */
/* 			       const char *b_name, nc_type b_type, int *b_varid, const char *b_units, */
/* 			       const char *ps_name, nc_type ps_type, int *ps_varid, const char *ps_units, */
/* 			       const char *p0_name, nc_type p0_type, int *p0_varid, const char *p0_units, */
/* 			       int *lvl_dimidp, int *lvl_varidp) */
/* { */
/*    int termids[FT_HYBRID_SIGMA_TERMS] = {a_varid, b_varid, ps_varid, p0_varid}; */
/*    int ret; */

/*    /\* Create the coordinate variable and dimension without */
/*     * formula_terms. *\/ */
/*    if ((ret = def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS,  */
/* 			    0, STD_SIGMA, NULL, NCCF_GEOZ,  */
/* 			    lvl_dimidp, lvl_varidp))) */
/*       return ret; */

/*    if ((ret = nc_def_var(ncid, a_name, a_type, A_NDIMS,  */
   
/* } */

/* Inquire about atmosphere hybrid_sigma coordinate. */
int 
nccf_inq_lvl_hybrid_sigma(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
			  int *a_varidp, int *b_varidp, int *ps_varidp, 
			  int *p0_varidp, int *lvl_dimidp, int *lvl_varidp)
{
   int termids[FT_HYBRID_SIGMA_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_HYBRID_SIGMA, FT_HYBRID_SIGMA_TERMS, 
			   FT_HYBRID_SIGMA_FORMAT, termids, name, xtypep, 
			   lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for hybrid_sigma level coordinate formula terms. */
   if (a_varidp)
      *a_varidp = termids[0];
   if (b_varidp)
      *b_varidp = termids[1];
   if (ps_varidp)
      *ps_varidp = termids[2];
   if (p0_varidp)
      *p0_varidp = termids[3];

   return CF_NOERR;
}

/* Atmosphere hybrid height coordinate

   standard_name = "atmosphere_hybrid_height_coordinate"

   Definition:
   z(n,k,j,i) = a(k) + b(k)*orog(n,j,i)

   where z(n,k,j,i) is the height above the geoid (approximately mean
   sea level) at gridpoint (k,j,i) and time (n), orog(n,j,i) is the
   height of the surface above the geoid at (j,i) and time (n), and
   a(k) and b(k) are the coordinates which define hybrid height level
   k. a(k) has the dimensions of height and b(k) is dimensionless.

   The format for the formula_terms attribute is
   formula_terms = "a: var1 b: var2 orog: var3"

   There is no dimensionless hybrid height coordinate. The hybrid
   height is best approximated as a(k) if a level-dependent constant
   is needed.
*/
int 
nccf_def_lvl_hybrid_height(int ncid, const char *name, nc_type xtype, size_t len, 
			   int *lvl_dimidp, int *lvl_varidp)
{
   /* Create the coordinate variable and dimension with no
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_HYBRID_HEIGHT, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

int 
nccf_def_ft_hybrid_height(int ncid, int varid, int a_varid, int b_varid, 
			  int orog_varid)
{
   int ft_varids[FT_HYBRID_HEIGHT_TERMS];

   ft_varids[0] = a_varid;
   ft_varids[1] = b_varid;
   ft_varids[2] = orog_varid;
   return nccf_set_ft(ncid, varid, FT_HYBRID_HEIGHT_TERMS, ft_varids, 
		      FT_HYBRID_HEIGHT_FORMAT);
}

/* Inquire about atmosphere hybrid_height coordinate. */
int 
nccf_inq_lvl_hybrid_height(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
			   int *a_varidp, int *b_varidp, int *orog_varidp, 
			   int *lvl_dimidp, int *lvl_varidp)
{
   int termids[FT_HYBRID_HEIGHT_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_HYBRID_HEIGHT, FT_HYBRID_HEIGHT_TERMS, 
			   FT_HYBRID_HEIGHT_FORMAT, termids, name, xtypep, 
			   lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for hybrid_height level coordinate formula terms. */
   if (a_varidp)
      *a_varidp = termids[1];
   if (b_varidp)
      *b_varidp = termids[2];
   if (orog_varidp)
      *orog_varidp = termids[3];

   return CF_NOERR;
}

/* Atmosphere smooth level vertical (SLEVE) coordinate

   standard_name = "atmosphere_sleve_coordinate"

   Definition:
   z(n,k,j,i) = a(k)*ztop + b1(k)*zsurf1(n,j,i) + b2(k)*zsurf2(n,j,i)

   where z(n,k,j,i) is the height above the geoid (approximately mean
   sea level) at gridpoint (k,j,i) and time (n), ztop is the height of
   the top of the model, and a(k), b1(k) and b2(k) are the
   dimensionless coordinates which define hybrid height level
   k. zsurf1(n,j,i) and zsurf2(n,j,i) are respectively the large and
   small scale parts of the topography. See Schaer et al [SCH02] for
   details.

   The format for the formula_terms attribute is

   formula_terms = "a: var1 b1: var2 b2: var3 ztop: var4 zsurf1: var5
   zsurf2: var6"

   The hybrid height coordinate for level k is defined as a(k)*ztop.
*/
int 
nccf_def_lvl_sleve(int ncid, const char *name, nc_type xtype, size_t len, 
		   int *lvl_dimidp, int *lvl_varidp)
{
   /* Create the coordinate variable and dimension with no
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_SLEVE, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

int 
nccf_def_ft_sleve(int ncid, int varid, int a_varid, int b1_varid, int b2_varid, 
		  int ztop_varid, int zsurf1_varid, int zsurf2_varid)
{
   int ft_varids[FT_SLEVE_TERMS];

   ft_varids[0] = a_varid;
   ft_varids[1] = b1_varid;
   ft_varids[2] = b2_varid;
   ft_varids[3] = ztop_varid;
   ft_varids[4] = zsurf1_varid;
   ft_varids[5] = zsurf2_varid;
   return nccf_set_ft(ncid, varid, FT_SLEVE_TERMS, ft_varids, FT_SLEVE_FORMAT);
}

/* Inquire about atmosphere sleve coordinate. */
int 
nccf_inq_lvl_sleve(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
		   int *a_varidp, int *b1_varidp, int *b2_varidp, int *ztop_varidp, 
		   int *zsurf1_varidp, int *zsurf2_varidp, int *lvl_dimidp, 
		   int *lvl_varidp)
{
   int termids[FT_SLEVE_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_SLEVE, FT_SLEVE_TERMS, FT_SLEVE_FORMAT, 
			   termids, name, xtypep, lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for sleve level coordinate formula terms. */
   if (a_varidp)
      *a_varidp = termids[0];
   if (b1_varidp)
      *b1_varidp = termids[1];
   if (b2_varidp)
      *b2_varidp = termids[2];
   if (ztop_varidp)
      *ztop_varidp = termids[3];
   if (zsurf1_varidp)
      *zsurf1_varidp = termids[4];
   if (zsurf2_varidp)
      *zsurf2_varidp = termids[5];

   return CF_NOERR;
}

/* Ocean sigma coordinate

   standard_name = "ocean_sigma_coordinate"

   Definition:
   z(n,k,j,i) = eta(n,j,i) + sigma(k) * (depth(j,i)+eta(n,j,i))

   where z(n,k,j,i) is height, positive upwards, relative to ocean
   datum (e.g. mean sea level) at gridpoint (n,k,j,i), eta(n,j,i) is
   the height of the ocean surface, positive upwards, relative to
   ocean datum at gridpoint (n,j,i), sigma(k) is the dimensionless
   coordinate at vertical gridpoint (k), and depth(j,i) is the
   distance from ocean datum to sea floor (positive value) at
   horizontal gridpoint (j,i).

   The format for the formula_terms attribute is
   formula_terms = "sigma: var1 eta: var2 depth: var3"
*/
int 
nccf_def_lvl_ocean_sigma(int ncid, const char *name, nc_type xtype, size_t len, 
			 int *lvl_dimidp, int *lvl_varidp)
{
   /* Create the coordinate variable and dimension with no
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_OCEAN_SIGMA, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

int 
nccf_def_ft_ocean_sigma(int ncid, int varid, int eta_varid, int depth_varid)
{
   int ft_varids[FT_OCEAN_SIGMA_TERMS];

   ft_varids[0] = varid;
   ft_varids[1] = eta_varid;
   ft_varids[2] = depth_varid;
   return nccf_set_ft(ncid, varid, FT_OCEAN_SIGMA_TERMS, ft_varids, 
		      FT_OCEAN_SIGMA_FORMAT);
}

/* Inquire about atmosphere ocean_sigma coordinate. */
int 
nccf_inq_lvl_ocean_sigma(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
			 int *eta_varidp, int *depth_varidp, int *lvl_dimidp, 
			 int *lvl_varidp)
{
   int termids[FT_OCEAN_SIGMA_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_OCEAN_SIGMA, FT_OCEAN_SIGMA_TERMS, FT_OCEAN_SIGMA_FORMAT, 
			   termids, name, xtypep, lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for ocean_sigma level coordinate formula terms. */
   if (eta_varidp)
      *eta_varidp = termids[1];
   if (depth_varidp)
      *depth_varidp = termids[2];

   return CF_NOERR;
}

/* Ocean s-coordinate

   standard_name = "ocean_s_coordinate"

   z(n,k,j,i) = eta(n,j,i)*(1+s(k)) + depth_c*s(k) +
   (depth(j,i)-depth_c)*C(k)

   C(k) = (1-b)*sinh(a*s(k))/sinh(a) + 
   b*[tanh(a*(s(k)+0.5))/(2*tanh(0.5*a)) - 0.5]

   where z(n,k,j,i) is height, positive upwards, relative to ocean
   datum (e.g. mean sea level) at gridpoint (n,k,j,i), eta(n,j,i) is
   the height of the ocean surface, positive upwards, relative to
   ocean datum at gridpoint (n,j,i), s(k) is the dimensionless
   coordinate at vertical gridpoint (k), and depth(j,i) is the
   distance from ocean datum to sea floor (positive value) at
   horizontal gridpoint (j,i). The constants a, b, and depth_c control
   the stretching.

   The format for the formula_terms attribute is

   formula_terms = "s: var1 eta: var2 depth: var3 a: var4 b: var5
   depth_c: var6"
*/
int 
nccf_def_lvl_ocean_s(int ncid, const char *name, nc_type xtype, size_t len, 
		     int *lvl_dimidp, int *lvl_varidp)
{
   /* Create the coordinate variable and dimension with no
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_OCEAN_S, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

int 
nccf_def_ft_ocean_s(int ncid, int varid, int eta_varid, int depth_varid, 
		    int a_varid, int b_varid, int depth_c_varid)
{
   int ft_varids[FT_OCEAN_S_TERMS];

   ft_varids[0] = varid;
   ft_varids[1] = eta_varid;
   ft_varids[2] = depth_varid;
   ft_varids[3] = a_varid;
   ft_varids[4] = b_varid;
   ft_varids[5] = depth_c_varid;
   return nccf_set_ft(ncid, varid, FT_OCEAN_S_TERMS, ft_varids, 
		      FT_OCEAN_S_FORMAT);
}

/* Inquire about atmosphere ocean_s coordinate. */
int 
nccf_inq_lvl_ocean_s(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
		     int *eta_varidp, int *depth_varidp, int *a_varidp, int *b_varidp, 
		     int *depth_c_varidp, int *lvl_dimidp, int *lvl_varidp)
{
   int termids[FT_OCEAN_S_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_OCEAN_S, FT_OCEAN_S_TERMS, FT_OCEAN_S_FORMAT, 
			   termids, name, xtypep, lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for ocean_s level coordinate formula terms. */
   if (eta_varidp)
      *eta_varidp = termids[1];
   if (depth_varidp)
      *depth_varidp = termids[2];
   if (a_varidp)
      *a_varidp = termids[3];
   if (b_varidp)
      *b_varidp = termids[4];
   if (depth_c_varidp)
      *depth_c_varidp = termids[5];

   return CF_NOERR;
}

/* Ocean sigma over z coordinate

   standard_name = "ocean_sigma_z_coordinate"

   for k <= nsigma:
   z(n,k,j,i) = eta(n,j,i) + sigma(k)*(min(depth_c,depth(j,i))+eta(n,j,i))
 
   for k > nsigma:
   z(n,k,j,i) = zlev(k)

   where z(n,k,j,i) is height, positive upwards, relative to ocean
   datum (e.g. mean sea level) at gridpoint (n,k,j,i), eta(n,j,i) is
   the height of the ocean surface, positive upwards, relative to
   ocean datum at gridpoint (n,j,i), sigma(k) is the dimensionless
   coordinate at vertical gridpoint (k) for k <= nsigma, and
   depth(j,i) is the distance from ocean datum to sea floor (positive
   value) at horizontal gridpoint (j,i). Above depth depth_c there are
   nsigma layers.

   The format for the formula_terms attribute is

   formula_terms = "sigma: var1 eta: var2 depth: var3 depth_c: var4
   nsigma: var5 zlev: var6"
*/
int 
nccf_def_lvl_ocean_sigma_z(int ncid, const char *name, nc_type xtype, size_t len, 
			   int *lvl_dimidp, int *lvl_varidp)
{
   /* Create the coordinate variable and dimension with no
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_OCEAN_SIGMA_Z, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);

}

int 
nccf_def_ft_ocean_sigma_z(int ncid, int varid, int eta_varid, int depth_varid, 
			  int depth_c_varid, int nsigma_varid, int zlev_varid)
{
   int ft_varids[FT_OCEAN_SIGMA_Z_TERMS];

   ft_varids[0] = varid;
   ft_varids[1] = eta_varid;
   ft_varids[2] = depth_varid;
   ft_varids[3] = depth_c_varid;
   ft_varids[4] = nsigma_varid;
   ft_varids[5] = zlev_varid;
   return nccf_set_ft(ncid, varid, FT_OCEAN_SIGMA_Z_TERMS, ft_varids, 
		      FT_OCEAN_SIGMA_Z_FORMAT);
}

/* Inquire about atmosphere ocean_sigma_z coordinate. */
int 
nccf_inq_lvl_ocean_sigma_z(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
			   int *eta_varidp, int *depth_varidp, int *depth_c_varidp, 
			   int *nsigma_varidp, int *zlev_varidp, int *lvl_dimidp, 
			   int *lvl_varidp)
{
   int termids[FT_OCEAN_SIGMA_Z_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_OCEAN_SIGMA_Z, FT_OCEAN_SIGMA_Z_TERMS, 
			   FT_OCEAN_SIGMA_Z_FORMAT, termids, name, xtypep, 
			   lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for ocean_sigma_z level coordinate formula terms. */
   if (eta_varidp)
      *eta_varidp = termids[1];
   if (depth_varidp)
      *depth_varidp = termids[2];
   if (depth_c_varidp)
      *depth_c_varidp = termids[3];
   if (nsigma_varidp)
      *nsigma_varidp = termids[4];
   if (zlev_varidp)
      *zlev_varidp = termids[5];

   return CF_NOERR;
}

/*
  Ocean double sigma coordinate

  standard_name = "ocean_double_sigma_coordinate"

  Definition:

  for k <= k_c

  z(k,j,i)= sigma(k)*f(j,i)

  for k > k_c

  z(k,j,i)= f(j,i) + (sigma(k)-1)*(depth(j,i)-f(j,i))

  f(j,i)= 0.5*(z1+ z2) + 0.5*(z1-z2)* tanh(2*a/(z1-z2)*(depth(j,i)-href))

  where z(k,j,i) is height, positive upwards, relative to ocean datum
  (e.g. mean sea level) at gridpoint (k,j,i), sigma(k) is the
  dimensionless coordinate at vertical gridpoint (k) for k <= k_c, and
  depth(j,i) is the distance from ocean datum to sea floor (positive
  value) at horizontal gridpoint (j,i).  z1, z2, a, and href are
  constants.

  The format for the formula_terms attribute is:

  formula_terms = "sigma: var1 depth: var2 z1: var3 z2: var4 a: var5 
  href: var6 k_c: var7"
*/

int 
nccf_def_lvl_ocean_dbl_sigma(int ncid, const char *name, nc_type xtype, size_t len, 
			     int *lvl_dimidp, int *lvl_varidp)
{
   /* Create the coordinate variable and dimension with no
    * formula_terms. */
   return def_coord_var(ncid, name, len, xtype, NULL, CF_LEVEL_AXIS, 
			0, STD_OCEAN_DBL_SIGMA, NULL, NCCF_GEOZ, lvl_dimidp, 
			lvl_varidp);
}

int 
nccf_def_ft_ocean_dbl_sigma(int ncid, int varid, int depth_varid, int z1_varid, 
			    int z2_varid, int a_varid, int href_varid, int k_c_varid)
{
   int ft_varids[FT_OCEAN_DBL_SIGMA_TERMS];

   ft_varids[0] = varid;
   ft_varids[1] = depth_varid;
   ft_varids[2] = z1_varid;
   ft_varids[3] = z2_varid;
   ft_varids[4] = a_varid;
   ft_varids[5] = href_varid;
   ft_varids[6] = k_c_varid;
   return nccf_set_ft(ncid, varid, FT_OCEAN_DBL_SIGMA_TERMS, ft_varids, 
		      FT_OCEAN_DBL_SIGMA_FORMAT);
}

/* Inquire about atmosphere ocean_dbl_sigma coordinate. */
int 
nccf_inq_lvl_ocean_dbl_sigma(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
			     int *depth_varidp, int *z1_varidp, int *z2_varidp, 
			     int *a_varidp, int *href_varidp, int *k_c_varidp, 
			     int *lvl_dimidp, int *lvl_varidp)
{
   int termids[FT_OCEAN_DBL_SIGMA_TERMS];
   int ret;

   /* Get most of the info from inq_vert_var. */
   if ((ret = inq_vert_var(ncid, STD_OCEAN_DBL_SIGMA, FT_OCEAN_DBL_SIGMA_TERMS, 
			   FT_OCEAN_DBL_SIGMA_FORMAT, termids, name, xtypep, 
			   lenp, lvl_dimidp, lvl_varidp)))
      return ret;

   /* Pull out the varids for ocean_dbl_sigma level coordinate formula
    * terms. */
   if (depth_varidp)
      *depth_varidp = termids[1];
   if (z1_varidp)
      *z1_varidp = termids[2];
   if (z2_varidp)
      *z2_varidp = termids[3];
   if (a_varidp)
      *a_varidp = termids[4];
   if (href_varidp)
      *href_varidp = termids[5];
   if (k_c_varidp)
      *k_c_varidp = termids[6];

   return CF_NOERR;
}

