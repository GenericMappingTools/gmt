/* 

Copyright 2006, University Corporation for Atmospheric Research. See
COPYRIGHT file for copying and redistribution conditions.

This is the header file for the NetCDF CF Library. 

For more info on the NetCDF CF Library, see:
http://www.unidata.ucar.edu/software/nccflib/

Ed Hartnett, 8/1/06

$Id$
*/

#include <stdlib.h>
#include <netcdf.h>

#ifndef _NCCFLIB_
#define _NCCFLIB_

#define COORDINATE_AXIS_TYPE "_CoordinateAxisType"
#define COORDINATE_AXES "_CoordinateAxes"
#define COORDINATE_Z_IS_POSITIVE "_CoordinateZisPositive"
#define Z_UP "up"
#define Z_DOWN "down"
#define COORDINATE_SYSTEMS "_CoordinateSystems"
#define TRANSFORM_NAME "transform_name"
#define TRANSFORM_TYPE "_CoordinateTransformType"
#define COORDINATE_TRANSFORMS "_CoordinateTransforms"

#define CF_LONGITUDE_NAME "longitude"
#define CF_LATITUDE_NAME "latitude"
#define CF_LONGITUDE_UNITS "degrees_east"
#define CF_LATITUDE_UNITS "degrees_north"
#define CF_LONGITUDE_AXIS "X"
#define CF_LATITUDE_AXIS "Y"
#define CF_LEVEL_AXIS "Z"
#define CF_TIME_AXIS "T"
#define CF_UNITS "units"
#define CF_FORMULA_TERMS "formula_terms"
#define CF_AXIS "axis"
#define CF_UP "up"
#define CF_DOWN "down"
#define CF_POSITIVE "positive"
#define CF_MAX_LEN 40

/* Some maximums for the "formula_terms" attribute used with
 * dimensionless vertical coordinates. There are (currently) at most 7
 * var names in a formula_terms attribute, and each may be NC_MAX_NAME
 * chars long. */
#define CF_MAX_FT_VARS 7
#define CF_MAX_FT_LEN (NC_MAX_NAME * CF_MAX_FT_VARS + 31)

/* The maximum length of the "coordinates" attribue. */
#define CF_MAX_COORDS 10
#define CF_MAX_COORD_LEN (NC_MAX_NAME * CF_MAX_COORDS)

/* These are in support of the coordinate axis stuff. */
#define NCCF_NOAXISTYPE 0
#define NCCF_LATITUDE 1
#define NCCF_LONGITUDE 2
#define NCCF_GEOX 3
#define NCCF_GEOY 4
#define NCCF_GEOZ 5
#define NCCF_HEIGHT_UP 6
#define NCCF_HEIGHT_DOWN 7
#define NCCF_PRESSURE 8
#define NCCF_TIME 9
#define NCCF_RADAZ 10
#define NCCF_RADEL 11
#define NCCF_RADDIST 12

/* Error codes for libcf. */
#define CF_NOERR NC_NOERR
#define CF_ENOMEM NC_ENOMEM /* Out of memory. */
#define CF_EBADTYPE NC_EBADTYPE /* Bad type. */
#define CF_EINVAL NC_EINVAL /* Invalid input. */
#define CF_ENETCDF -400 /* Error at netCDF layer. */
#define CF_ETIME -401   /* Error getting current time and date. */
#define CF_EMINMAX -402 /* Min and max must both be provided. */
#define CF_EUNKCOORD -403 /* Unknown coordinate type. */
#define CF_EEXISTS -404 /* Attempting to define var, dim, or att that already exists. */
#define CF_ENOTFOUND -405 /* Can't find what you're looking for! */
#define CF_ENODIM -406 /* Can't find dim to go with coordinate variable. */
#define CF_EBADVALUE -407 /* Attribute has incorrect value for CF. */
#define CF_EMAXFT -408 /* formula_terms attribute exceeds max size. */
#define CF_EBADFT -409 /* Couldn't parse formula_terms attribute. */
#define CF_EMAX_NAME -410 /* Name of netCDF object too long. */
#define CF_EMAXCOORDS -411 /* Too many coordinates! */
#define CF_ERECTOOLARGE -412 /* Record number too large. */
#define CF_ENDIMS -413 /* Wrong number of dimensions. */
#define CF_EBADRANGE -414 /* range[0] must be <= range[1]. */

/* These are from libsrc/ncx.h. They will never change. */
#define X_SCHAR_MAX	127
#define X_UCHAR_MAX	255U
#define X_SHORT_MAX	32767
#define X_INT_MAX	2147483647
#define X_FLOAT_MAX	3.402823466e+38f
#define X_FLOAT_MIN	(-X_FLOAT_MAX)
#define X_DOUBLE_MAX	1.7976931348623157e+308 
#define X_DOUBLE_MIN	(-X_DOUBLE_MAX)

/* Default valid min and valid max for netCDF-3 types. These values
 * take into account the default fill values, which always will fall
 * out of the range expressed here for each type. */
#define CF_BYTE_MIN (-126)
#define CF_BYTE_MAX X_SCHAR_MAX
#define CF_CHAR_MIN 1
#define CF_CHAR_MAX X_UCHAR_MAX
#define CF_SHORT_MIN (-32766)
#define CF_SHORT_MAX X_SHORT_MAX
#define CF_INT_MIN (-2147483646L)
#define CF_INT_MAX X_INT_MAX
#define CF_FLOAT_MIN X_FLOAT_MIN
#define CF_FLOAT_MAX NC_FILL_FLOAT
#define CF_DOUBLE_MIN NC_FILL_DOUBLE
#define CF_DOUBLE_MAX X_DOUBLE_MIN

#define CF_CONVENTIONS "Conventions"
#define CF_CONVENTION_STRING "CF-1.0"
#define CF_TITLE "title"
#define CF_HISTORY "history"
#define CF_INSTITUTION "institution"
#define CF_SOURCE "source"
#define CF_COMMENT "comment"
#define CF_REFERENCES "references"
#define CF_UNITS "units"
#define CF_LONG_NAME "long_name"
#define CF_STANDARD_NAME "standard_name"
#define CF_FILL_VALUE "_FillValue"
#define CF_VALID_MIN "valid_min"
#define CF_VALID_MAX "valid_max"
#define CF_COORDINATES "coordinates"

/* These are defined in appendix D of the CF standard. They are for
 * dimensionless vertical coordinates. */
#define CF_VERT_ATM_LN 0
#define STD_ATM_LN "atmosphere_ln_pressure_coordinate"
#define FT_ATM_LN_FORMAT "p0: %s lev: %s"
#define FT_ATM_LN_TERMS 2

#define CF_VERT_SIGMA 1
#define STD_SIGMA "atmosphere_sigma_coordinate"
#define FT_SIGMA_FORMAT "sigma: %s ps: %s ptop: %s"
#define FT_SIGMA_TERMS 3

#define CF_VERT_HYBRID_SIGMA 2
#define STD_HYBRID_SIGMA "atmosphere_hybrid_sigma_pressure_coordinate"
#define FT_HYBRID_SIGMA_FORMAT "a: %s b: %s ps: %s p0: %s"
#define FT_HYBRID_SIGMA_TERMS 4

#define CF_VERT_HYBRID_HEIGHT 3
#define STD_HYBRID_HEIGHT "atmosphere_hybrid_height_coordinate"
#define FT_HYBRID_HEIGHT_FORMAT "a: %s b: %s orog: %s"
#define FT_HYBRID_HEIGHT_TERMS 3

#define CF_VERT_SLEVE 4
#define STD_SLEVE "atmosphere_sleve_coordinate"
#define FT_SLEVE_FORMAT "a: %s b1: %s b2: %s ztop: %s zsurf1: %s zsurf2: %s"
#define FT_SLEVE_TERMS 6

#define CF_VERT_OCEAN_SIGMA 5
#define STD_OCEAN_SIGMA "ocean_sigma_coordinate"
#define FT_OCEAN_SIGMA_FORMAT "sigma: %s eta: %s depth: %s"
#define FT_OCEAN_SIGMA_TERMS 3

#define CF_VERT_OCEAN_S 6
#define STD_OCEAN_S "ocean_s_coordinate"
#define FT_OCEAN_S_FORMAT "s: %s eta: %s depth: %s a: %s b: %s depth_c: %s"
#define FT_OCEAN_S_TERMS 6

#define CF_VERT_OCEAN_SIGMA_Z 7
#define STD_OCEAN_SIGMA_Z "ocean_sigma_z_coordinate"
#define FT_OCEAN_SIGMA_Z_FORMAT "sigma: %s eta: %s depth: %s depth_c: %s nsigma: %s zlev: %s"
#define FT_OCEAN_SIGMA_Z_TERMS 6

#define CF_VERT_OCEAN_DBL_SIGMA 8
#define STD_OCEAN_DBL_SIGMA "ocean_double_sigma_coordinate"
#define FT_OCEAN_DBL_SIGMA_FORMAT "sigma: %s depth: %s z1: %s z2: %s a: %s href: %s k_c: %s"
#define FT_OCEAN_DBL_SIGMA_TERMS 7

#define CF_NUM_VERT 9
#define FT_MAX_TERMS 7

#if defined(__cplusplus)
extern "C" {
#endif

   /* Get the version string for the CF Library (max length:
    * NC_MAX_NAME + 1). */
   int nccf_inq_libvers(char *version_string);

   /* Mark the file as following CF-1.0 conventions. */
   int nccf_def_convention(int ncid);

   /* Determine if the file follows CF-1.0 conventions. */
   int nccf_inq_convention(int ncid, int *cf_convention);

   /* Add some or all of the CF recomended text attributes to a
    * file. Timestamp will be added to history. If history attribute
    * already exists, this history value will be appended. */
   int nccf_def_file(int ncid, const char *title, const char *history);

   /* Read any existing CF recomended text attributes from the
    * file. The history will get a timestamp. */
   int nccf_inq_file(int ncid, size_t *title_lenp, char *title, 
		     size_t *history_lenp, char *history);

   /* Append to the global "history" attribute, with a timestamp. */
   int nccf_add_history(int ncid, const char *history);

   /* Add some or all of the CF recomended text attributes to a variable. */
   int nccf_def_var(int ncid, int varid, const char *units, 
		    const char *long_name, const char *standard_name, 
		    int ncoord_vars, int *coord_varids);

   /* Read any existing CF recomended text attributes from a variable. */
   int nccf_inq_var(int ncid, int varid, size_t *units_lenp, char *units, 
		    size_t *long_name_lenp, char *long_name, 
		    size_t *standard_name_lenp, char *standard_name,
		    int *ncoord_vars, int *coord_varids);

   /* Set attributes to define missing data information. */
   int nccf_def_var_missing(int ncid, int varid, const void *fill_value, 
			    const void *valid_min, const void *valid_max);

   /* Get attributes which define missing data information. */
   int nccf_inq_var_missing(int ncid, int varid, void *fill_value, 
			    void *valid_min, void *valid_max);

   /* Add any or all of these four attributes to a file or variable. */
   int nccf_def_notes(int ncid, int varid, const char *institution, 
		      const char *source, const char *comment, 
		      const char *references);

   /* Read any or all of these four attributes of a file or
    * variable. */
   int nccf_inq_notes(int ncid, int varid,
		      size_t *institution_lenp, char *institution, 
		      size_t *source_lenp, char *source, 
		      size_t *comment_lenp, char *comment, 
		      size_t *references_lenp, char *references);

   /* Set attributes to define missing data information. */
   int nccf_def_var_missing(int ncid, int varid, const void *fill_valuep, 
			    const void *valid_minp, const void *valid_maxp);

   /* Get attributes which define missing data information. If the
    * attributes are not there, then provide the valid data anyway, based
    * on netCDF defaults. */
   int nccf_inq_var_missing(int ncid, int varid, void *fill_valuep, 
			    void *valid_minp, void *valid_maxp);

   /* Define a latitude dimension and variable with all the CF
    * recomended attribute accessories.*/
   int nccf_def_latitude(int ncid, size_t len, nc_type xtype, 
			 int *lat_dimidp, int *lat_varidp);

   /* Inquire about a latitude dimension and coordinate variable. */
   int nccf_inq_latitude(int ncid, size_t *lenp, nc_type *xtypep, 
			 int *lat_dimidp, int *lat_varidp);

   /* Define a longitude dimension and variable with all the CF
    * recomended attribute accessories.*/
   int nccf_def_longitude(int ncid, size_t len, nc_type xtype, 
			  int *lon_dimidp, int *lon_varidp);

   /* Inquire about a longitude dimension and coordinate variable.*/
   int nccf_inq_longitude(int ncid, size_t *lenp, nc_type *xtypep, 
			  int *lon_dimidp, int *lon_varidp);

   int nccf_def_lvl(int ncid, const char *name, size_t len, nc_type xtype, 
		    const char *units, int positive_up, 
		    const char *standard_name, const char *formula_terms, 
		    int cdm_axis_type, int *lvl_dimidp, int *lvl_varidp);

   /* Inquire about a vertical dimension and coordinate variable. */
   int nccf_inq_lvl(int ncid, char *name, size_t *lenp, nc_type *xtypep, 
		    size_t *ft_lenp, char *formula_terms, int *positive_upp, 
		    int *lvl_dimidp, int *lvl_varidp);

   /* Define a time coordinate variable and dimension. */
   int nccf_def_time(int ncid, const char *name, size_t len, nc_type xtype, 
		     const char *units, const char *standard_name, 
		     int *time_dimidp, int *time_varidp);

   /* Inquire about a time dimension and coordinate variable. */
   int nccf_inq_time(int ncid, char *name, size_t *lenp, nc_type *xtypep, 
		     int *time_dimidp, int *time_varidp);

   /* Define a unitless vertical coordinate. */
   int nccf_def_lvl_vert(int ncid, int lvl_type, const char *name, nc_type xtype, 
			 size_t len, int *lvl_dimidp, int *lvl_varidp);

   /* Learn about a unitless vertical coordinate. */
   int nccf_inq_lvl_vert(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
			 int *lvl_typep, int *lvl_dimidp, int *lvl_varidp);

   /* Define atmosphere sigma coordinate. */
   int nccf_def_lvl_sigma(int ncid, const char *name, nc_type xtype, 
			  size_t len, int *lvl_dimidp, int *lvl_varidp);

   /* Set the formula_terms to contain information about this level. */
   int nccf_def_ft_sigma(int ncid, int lvl_vid, int ps_vid, int p0_vid);
   
   /* Inquire about atmosphere sigma coordinate. */
   int nccf_inq_lvl_sigma(int ncid, char *name, nc_type *xtypep, size_t *lenp, 
			  int *ps_varidp, int *ptop_varidp, int *lvl_dimidp, 
			  int *lvl_varidp);

   /* Define hybrid_sigma coordinate. */
   int nccf_def_lvl_hybrid_sigma(int ncid, const char *name, nc_type xtype, 
				 size_t len, int *lvl_dimidp, int *lvl_varidp);

   /* Set the formula_terms to contain information about this level. */
   int nccf_def_ft_hybrid_sigma(int ncid, int lvl_vid, int a_vid, int b_vid, int ps_vid, 
				int p0_vid);

   /* Inquire about hybrid_sigma coordinate. */
   int nccf_inq_lvl_hybrid_sigma(int ncid, char *name, nc_type *xtypep, 
				 size_t *lenp, int *a_varidp, int *b_varidp, 
				 int *ps_varidp, int *p0_varidp, int *lvl_dimidp, 
				 int *lvl_varidp);

   /* Define hybrid_sigma coordinate. */
   int nccf_def_lvl_hybrid_sigma_full(int ncid, const char *name, nc_type xtype, 
				      size_t len, int *lvl_dimidp, int *lvl_varidp);

   /* Define hybrid_sigma coordinate. */
   int nccf_def_ft_hybrid_sigma_full(int ncid, int varid, int a_varid, int b_varid, 
				     int ps_varid, int p0_varid);

   /* Define hybrid_height coordinate. */
   int nccf_def_lvl_hybrid_height(int ncid, const char *name, nc_type xtype, size_t len,
				  int *lvl_dimidp, int *lvl_varidp);

   /* Define hybrid_height coordinate. */
   int nccf_def_ft_hybrid_height(int ncid, int varid, int a_varid, int b_varid, 
				 int orog_varid);

   /* Inquire about hybrid_height coordinate. */
   int nccf_inq_lvl_hybrid_height(int ncid, char *name, nc_type *xtypep, size_t *lenp,
				  int *a_varidp, int *b_varidp, int *orog_varidp,
				  int *lvl_dimidp, int *lvl_varidp);

   /* Define sleve coordinate. */
   int nccf_def_lvl_sleve(int ncid, const char *name, nc_type xtype, size_t len, 
			  int *lvl_dimidp, int *lvl_varidp);

   /* Define sleve coordinate. */
   int nccf_def_ft_sleve(int ncid, int varid, int a_varid, int b1_varid, int b2_varid, 
			 int ztop_varid, int zsurf1_varid, int zsurf2_varid);

   /* Inquire about sleve coordinate. */
   int nccf_inq_lvl_sleve(int ncid, char *name, nc_type *xtypep, size_t *lenp,
			  int *a_varidp, int *b1_varidp, int *b2_varidp, int *ztop_varidp,
			  int *zsurf1_varidp, int *zsurf2_varidp, int *lvl_dimidp,
			  int *lvl_varidp);

   

   int nccf_def_lvl_ocean_sigma(int ncid, const char *name, nc_type xtype, size_t len,
				int *lvl_dimidp, int *lvl_varidp);

   int nccf_def_ft_ocean_sigma(int ncid, int varid, int eta_varid, int depth_varid);

   int nccf_inq_lvl_ocean_sigma(int ncid, char *name, nc_type *xtypep, size_t *lenp,
				int *eta_varidp, int *depth_varidp, int *lvl_dimidp,
				int *lvl_varidp);

   int nccf_def_lvl_ocean_s(int ncid, const char *name, nc_type xtype, size_t len,
			    int *lvl_dimidp, int *lvl_varidp);

   int nccf_def_ft_ocean_s(int ncid, int varid, int eta_varid, int depth_varid, 
			   int a_varid, int b_varid, int depth_c_varid);

   int nccf_inq_lvl_ocean_s(int ncid, char *name, nc_type *xtypep, size_t *lenp,
			    int *eta_varidp, int *depth_varidp, int *a_varidp, int *b_varidp,
			    int *depth_c_varidp, int *lvl_dimidp, int *lvl_varidp);

   /* Define ocean_sigma_z coordinate. */
   int nccf_def_lvl_ocean_sigma_z(int ncid, const char *name, nc_type xtype,
				  size_t len, int *lvl_dimidp, int *lvl_varidp);

   int nccf_def_ft_ocean_sigma_z(int ncid, int varid, int eta_varid, int depth_varid,
				  int depth_c_varid, int nsigma_varid, int zlev_varid);

   /* Inquire about ocean_sigma_z coordinate. */
   int nccf_inq_lvl_ocean_sigma_z(int ncid, char *name, nc_type *xtypep, size_t *lenp,
				  int *eta_varidp, int *depth_varidp, int *depth_c_varidp,
				  int *nsigma_varidp, int *zlev_varidp, int *lvl_dimidp,
				  int *lvl_varidp);

   int nccf_def_lvl_ocean_dbl_sigma(int ncid, const char *name, nc_type xtype, size_t len,
				    int *lvl_dimidp, int *lvl_varidp);

   int nccf_def_ft_ocean_dbl_sigma(int ncid, int varid, int depth_varid, int z1_varid, 
				   int z2_varid, int a_varid, int href_varid, int k_c_varid);


   /* Inquire about atmosphere ocean_dbl_sigma coordinate. */
   int nccf_inq_lvl_ocean_dbl_sigma(int ncid, char *name, nc_type *xtypep, size_t *lenp,
				    int *depth_varidp, int *z1_varidp, int *z2_varidp,
				    int *a_varidp, int *href_varidp, int *k_c_varidp,
				    int *lvl_dimidp, int *lvl_varidp);

   /* Here are functions for coordinate axis stuff. */

   /* Label the axis type of a coordinate var. */
   int nccf_def_axis_type(int ncid, int varid, int axis_type);

   /* Find out the axis type o a coordinate var. */
   int nccf_inq_axis_type(int ncid, int varid, int *axis_type);

   /* Define a coordinate system consisting of naxes axes, each axis
    * represented by a coordinate varid in the axis_varids array. This
    * create a new (scalar, NC_CHAR) var, whose varid is returned in
    * system_varid. */
   int nccf_def_coord_system(int ncid, const char *name, int naxes, int *axis_varids, 
			     int *system_varid);

   /* Find out about a coordinate system, it's name, number of axes, and
    * the varid of each axis coordinate var. */
   int nccf_inq_coord_system(int ncid, int system_varid, char *name, 
			     int *naxes, int *axis_varids);

   /* Assign a coordinate system to a var. This adds an attriibute to the
    * var. */
   int nccf_assign_coord_system(int ncid, int varid, int system_varid);

   /* Define a coordinate transform. This adds a (scalar, NC_CHAR) var,
    * which contains some attributes. The varid of this new variable is
    * returned in transform_varid. */
   int nccf_def_transform(int ncid, const char *name, const char *transform_type, 
			  const char *transform_name, int *transform_varid);

   /* Find out about a coordinate transform, it's name, and the contents
    * of the transform_type and transform_name attributes. Pass NULL for
    * any that you're not interested in. Pass NULL for transform_type and
    * transform_name to get their lengths with type_len and name_len. */
   int nccf_inq_transform(int ncid, int transform_varid, char *name, size_t *type_len, 
			  char *transform_type, size_t *name_len, char *transform_name);

   /* Assign a coordinate transform to a coordinate system. This adds an
    * attribute to the variable that holds the coordinate system
    * attributes. */
   int nccf_assign_transform(int ncid, int system_varid, int transform_varid);

   /* 4D geographic subsetting. */
   int nccf_get_vara(int ncid, int varid, float *lat_range, int *nlat, float *lon_range, 
		     int *nlon, int lvl_index, int timestep, void *data);

   /* Calandar and time. */
#define cdStandardCal   0x11
#define cdClimCal        0x0
#define cdHasLeap      0x100
#define cdHasNoLeap    0x000
#define cd365Days     0x1000
#define cd360Days     0x0000
#define cdJulianCal  0x10000
#define cdMixedCal   0x20000

   typedef enum cdCalenType {
      cdStandard    = ( cdStandardCal | cdHasLeap   | cd365Days),
      cdJulian      = ( cdStandardCal | cdHasLeap   | cd365Days | cdJulianCal),
      cdNoLeap      = ( cdStandardCal | cdHasNoLeap | cd365Days),
      cd360         = ( cdStandardCal | cdHasNoLeap | cd360Days),
      cdClim        = ( cdClimCal     | cdHasNoLeap | cd365Days),
      cdClimLeap    = ( cdClimCal     | cdHasLeap   | cd365Days),
      cdClim360     = ( cdClimCal     | cdHasNoLeap | cd360Days),
      cdMixed       = ( cdStandardCal | cdHasLeap   | cd365Days | cdMixedCal)
   }  cdCalenType;

   typedef enum cdType {cdInvalidType = -1,
			cdByte = NC_BYTE,
			cdChar = NC_CHAR,
			cdShort = NC_SHORT,
			cdInt = NC_INT,
			cdLong = NC_INT,
			cdFloat = NC_FLOAT,
			cdDouble = NC_DOUBLE,
			cdLongDouble = NC_DOUBLE,
			cdCharTime
   } cdType;

   typedef struct {
	 long 		year;		     /* Year */
	 short 		month;		     /* Numerical month (1..12) */
	 short 		day;		     /* Day of month (1..31) */
	 double 		hour;		     /* Hour and fractional hours */
   } cdCompTime;

   int cdAbs2Comp(char* absunits, void* abstime, cdType abstimetype, cdCompTime* comptime, double* frac);
   void cdChar2Comp(cdCalenType timetype, char* chartime, cdCompTime* comptime);
   void cdChar2Rel(cdCalenType timetype, char* chartime, char* relunits, double* reltime);
   int cdComp2Abs(cdCompTime comptime, char* absunits, cdType abstimetype, double frac, void* abstime);
   void cdComp2Char(cdCalenType timetype, cdCompTime comptime, char* time);
   void cdComp2Rel(cdCalenType timetype, cdCompTime comptime, char* relunits, double* reltime);
   void cdRel2Char(cdCalenType timetype, char* relunits, double reltime, char* chartime);
   void cdRel2Comp(cdCalenType timetype, char* relunits, double reltime, cdCompTime* comptime);
   void cdRel2Rel(cdCalenType timetype, char* inunits, double intime, char* newunits, double* outtime);

#if defined(__cplusplus)
}
#endif

#endif /* NCCFLIB */
