/*
Copyright 2006, University Corporation for Atmospheric Research. See
COPYRIGHT file for copying and redistribution conditions.

This file is part of the CF Library. 

This file handles the nc4 coordinate systems.

Ed Hartnett, 10/1/05

$Id$
*/

#include <config.h>
#include <libcf.h>
#include <libcf_int.h>
#include <stdlib.h>
#include <string.h>

static char* axis_type_name[] = {"", "Lat", "Lon", "GeoX", "GeoY", "GeoZ", 
				 "Height", "Height", "Pressure", "Time", 
				 "RadialAzimuth", "RadialElevation", 
				 "RadialDistance"};

/* Here are functions for coordinate axis stuff. */
int
nccf_def_axis_type(int ncid, int varid, int axis_type)
{
   int dimid;
   char var_name[NC_MAX_NAME + 1];
   int ret;

   /* Is the axis type valid? */
   if (axis_type < 0 || axis_type > NCCF_RADDIST)
      return NC_EINVAL;

   /* Make sure there's a dimension with the same name in this
    * ncid. */
   if ((ret = nc_inq_varname(ncid, varid, var_name)))
      return ret;
   if ((ret = nc_inq_dimid(ncid, var_name, &dimid)))
      return ret;

   /* Now write the attribute which stores the axis type. */
   if ((ret = nc_put_att_text(ncid, varid, COORDINATE_AXIS_TYPE, 
				 strlen(axis_type_name[axis_type]) + 1, 
				 axis_type_name[axis_type])))
      return ret;

   /* For height we write another attribute, indicating up
    * vs. down. */
   if (axis_type == NCCF_HEIGHT_UP)
   {
      if ((ret = nc_put_att_text(ncid, varid, COORDINATE_Z_IS_POSITIVE, 
				    strlen(Z_UP) + 1, Z_UP)))
	 return ret;
   }
   else if (axis_type == NCCF_HEIGHT_DOWN)
   {
      if ((ret = nc_put_att_text(ncid, varid, COORDINATE_Z_IS_POSITIVE, 
				    strlen(Z_UP) + 1, Z_UP)))
	 return ret;
   }

   return NC_NOERR;
}

int
nccf_inq_axis_type(int ncid, int varid, int *axis_type)
{
   char value[NC_MAX_NAME + 1];
   int i;
   int ret;

   /* Get the attribute which stores the axis type. */
   if ((ret = nc_get_att_text(ncid, varid, COORDINATE_AXIS_TYPE, 
				 value)))
      return ret;

   /* Which axis type is this? */
   for (i = 1; i <= NCCF_RADDIST; i++)
      if (!strcmp(value, axis_type_name[i]))
	 break;

   /* For height we also have to look at another attribute to find out
    * whether it is up or down. At this point, i will equal
    * NCCF_HEIGHT_UP, find out whether it should be HEIGHT_DOWN. */
   if (i == NCCF_HEIGHT_UP)
   {
      if ((ret = nc_get_att_text(ncid, varid, COORDINATE_Z_IS_POSITIVE, 
				    value)))
	 return ret;
      if (!strcmp(value, Z_DOWN))
	 i = NCCF_HEIGHT_DOWN;
   }

   /* Return a valid type if we found one, otherwise zero. */
   if (i > NCCF_RADDIST)
      return NCCF_NOAXISTYPE;
   
   if (axis_type)
      *axis_type = i;
   
   return NC_NOERR;
}


/* Define a coordinate system. This creates a var with contains a text
 * attribute, which is the names of the (coordinate) vars which make
 * up the axes of a coordinate system. */
int
nccf_def_coord_system(int ncid, const char *name, int naxes, int *axis_varids, 
		    int *system_varid)
{
   int dimid, a;
   int varid;
   char var_name[NC_MAX_NAME + 1];
   char *coord_axes_str;
   int ret;
   
   /* naxes must be positive and less than max_dims. */
   if (naxes < 0)
      return NC_EINVAL;
   if (naxes > NC_MAX_DIMS)
      return NC_EMAXDIMS;

   /* Create a scalar var, type NC_CHAR, to hold the attribute that
    * defines this system. */
   if ((ret = nc_def_var(ncid, name, NC_CHAR, 0, NULL, &varid)))
      return ret;
   
   /* If the user wants it, return the varid of this system. */
   if (system_varid)
      *system_varid = varid;

   /* malloc space for the string which will hold the names of all the
    * axes. */
   if (!(coord_axes_str = malloc(naxes * NC_MAX_NAME + naxes)))
      return NC_NOERR;
   coord_axes_str[0] = 0;

   /* Store the names of the axes in an attribute. At the same time,
    * make sure that all the axis_varids are actually coordinate
    * variables - that is, there is a dimension withn the same
    * name. */
   for (a = 0; a < naxes; a++)
   {
      if ((ret = nc_inq_varname(ncid, axis_varids[a], var_name)))
	 break;
      if ((ret = nc_inq_dimid(ncid, var_name, &dimid)))
	 break;
      strcat(coord_axes_str, var_name);
      if (a < naxes - 1)
	 strcat(coord_axes_str, " ");
   }
   
   /* If we arrived here without error, then write the coord_axes_str
    * to the appropriate attribute, to save the system in the file. */
   if (!ret)
      ret = nc_put_att_text(ncid, varid, COORDINATE_AXES, 
			       strlen(coord_axes_str) + 1, coord_axes_str);

   /* Free the memory we allocated. */
   free(coord_axes_str);
   
   return ret;
}

/* Both CF and CDM use an attribute to hold a space-separated list of
 * names of coordinate variables. This function reads that attribute
 * and looks up the ID of each coordinate var. */
int
nccf_parse_coords(int ncid, int varid, char *att_name, int *naxes, 
		  int *axis_varids)
{
   size_t len;
   char *coords_str;
   char axis_name[NC_MAX_NAME + 1], *p, *a;
   int num_axes = 0, axes[NC_MAX_DIMS];
   int a1, ret;

   /* Check reasonableness of name. */
   if (!att_name || strlen(att_name) > NC_MAX_NAME)
      return CF_EINVAL;

   /* Find length of the attribute. */
   if ((ret = nc_inq_att(ncid, varid, att_name, NULL, &len)))
      return ret;

   /* Allocate space to read in the string. */
   if (!(coords_str = malloc(len)))
      return CF_ENOMEM;

   /* Read in the system string. */
   ret = nc_get_att_text(ncid, varid, att_name, coords_str);

   /* If we got the coord axes string, parse it to find the names of
    * the coordinate vars which make up this system. */
   for (p = coords_str; !ret && (p - coords_str) < strlen(coords_str);)
   {
      for (a = axis_name; *p && *p != ' '; )
	 *a++ = *p++;
      p++;
      *a = '\0';
      
      ret = nc_inq_varid(ncid, axis_name, &axes[num_axes++]);
   }

   /* Thanks for the memories! */
   free(coords_str);

   /* Give the user what they asked for. */
   if (naxes)
      *naxes = num_axes;
   if (axis_varids)
      for (a1 = 0; a1 < num_axes; a1++)
	 axis_varids[a1] = axes[a1];
   
   return CF_NOERR;
}

/* Inquire about a coordinate system. */
EXTERNL int
nccf_inq_coord_system(int ncid, int system_varid, char *name, 
		      int *naxes, int *axis_varids)
{
   int ret;

   /* Get the name of this coordinate system, which is the same as the
    * name of this variable. */
   if ((ret = nc_inq_varname(ncid, system_varid, name)))
      return ret;

   /* Look up the ids of the vars in the COORDINATE_AXES attribute. */
   if ((ret = nccf_parse_coords(ncid, system_varid, COORDINATE_AXES, 
				naxes, axis_varids)))
      return ret;

   return ret;
}

/* Given the varid of a system, write an attribute to a varid, whcih
 * indicates that it (the var) partakes of the system. Recall that the
 * system is just a collection of var names which are axes of the
 * coordinate system. */
int
nccf_assign_coord_system(int ncid, int varid, int system_varid)
{
   char system_name[NC_MAX_NAME + 1];
   char *systems_str;
   size_t len, new_len = 0;
   int ret;

   /* Find the name of this system. */
   if ((ret = nc_inq_varname(ncid, system_varid, system_name)))
      return ret;

   /* Is the att already here? If so, get it's length, then it's
    * contents, then append a space and the new system name. */
   ret = nc_inq_att(ncid, varid, COORDINATE_SYSTEMS, NULL, &len);
   if (ret != NC_ENOTATT && ret != NC_NOERR)
      return ret;
   if (!ret)
      new_len += len;
   new_len += strlen(system_name) + 1;
   if (!(systems_str = malloc(new_len)))
      return NC_NOERR;
   systems_str[0] = 0;
   if (!ret)
   {
      if ((ret = nc_get_att_text(ncid, varid, COORDINATE_SYSTEMS, 
				    systems_str)))
	 return ret;
      strcat(systems_str, " ");
   }
   strcat(systems_str, system_name);

   /* Write an att, called _CoordinateSystems, which
    * contains a list of the system names related to this variable. */
   ret = nc_put_att_text(ncid, varid, COORDINATE_SYSTEMS, 
			    strlen(systems_str) + 1, systems_str);
      

   /* Free the memory we allocated. */
   free(systems_str);
   
   return ret;
}

/* Define a transform. This will create a scalar var of type NC_CHAR
 * to hold some attributes. */
int
nccf_def_transform(int ncid, const char *name, const char *transform_type,
		 const char *transform_name, int *transform_varid)
{
   int varid;
   int ret;
   
   /* Create a scalar var, type NC_CHAR, to hold the attribute that
    * defines this transform. */
   if ((ret = nc_def_var(ncid, name, NC_CHAR, 0, NULL, &varid)))
      return ret;
   
   /* If the user wants it, return the varid of this system. */
   if (transform_varid)
      *transform_varid = varid;

   /* Store the transform_type, if provided by the user. */
   if (transform_type)
      if ((ret = nc_put_att_text(ncid, varid, TRANSFORM_TYPE, 
				    strlen(transform_type) + 1, transform_type)))
	 return ret;

   /* Store the transform name, if provided by the user. */
   if (transform_type)
      if ((ret = nc_put_att_text(ncid, varid, TRANSFORM_NAME, 
				    strlen(transform_name) + 1, transform_name)))
	 return ret;

   return NC_NOERR;
}

EXTERNL int
nccf_inq_transform(int ncid, int transform_varid, char *name, size_t *type_len, 
		 char *transform_type, size_t *name_len, char *transform_name)
{
   int ret;

   /* Find the name of the transform var, if desired. */
   if (name)
      if ((ret = nc_inq_varname(ncid, transform_varid, name)))
	 return ret;

   /* If the user wants the length of the transform_type, find it. */
   if (type_len)
      if ((ret = nc_inq_attlen(ncid, transform_varid, TRANSFORM_TYPE, 
				  type_len)))
	 return ret;

   /* If the user wants the transform type string, get it. */
   if (transform_type)
      if ((ret = nc_get_att_text(ncid, transform_varid, 
				    TRANSFORM_TYPE, transform_type)))
	 return ret;

   /* If the user wants the length of the transform_name, find it. */
   if (type_len)
      if ((ret = nc_inq_attlen(ncid, transform_varid, TRANSFORM_NAME, 
				  name_len)))
	 return ret;

   /* If the user wants the transform name string, get it. */
   if (transform_name)
      if ((ret = nc_get_att_text(ncid, transform_varid, 
				   TRANSFORM_NAME, transform_name)))
	 return ret;

   return NC_NOERR;
}

int
nccf_assign_transform(int ncid, int system_varid, int transform_varid)
{
   char transform_name[NC_MAX_NAME + 1];
   int ret;

   /* Find the name of the transform. */
   if ((ret = nc_inq_varname(ncid, transform_varid, transform_name)))
      return ret;

   /* Write the transform name as an att of the system to which it
    * applies. */
   if ((ret = nc_put_att_text(ncid, system_varid, COORDINATE_TRANSFORMS, 
				 strlen(transform_name) + 1, transform_name)))
      return ret;
   
   return NC_NOERR;
}

