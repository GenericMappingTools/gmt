/* This is part of the netCDF package. Copyright 2005-2007, University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program creates some test files which ncdump will read. This
   is only done if netCDF-4 is enabled.

   $Id$
*/

#include <nc_tests.h>
#include <netcdf.h>
#include <stdlib.h>

#define FILE_NAME_1 "tst_solar_1.nc"
#define FILE_NAME_2 "tst_solar_2.nc"
#define FILE_NAME_CMP "tst_solar_cmp.nc"
#define SOLAR_SYSTEM "solar_system"
#define EARTH "Earth"
#define LUNA "Luna"
#define ATT_NAME "Vogon_Poem"
#define VLEN_TYPE_NAME "unimaginatively_named_vlen_type"

/* http://www.bbc.co.uk/cult/hitchhikers/vogonpoetry/lettergen.shtml */
static char *poem = {
"See, see the netCDF-filled sky\n\
Marvel at its big barf-green depths.\n\
Tell me, Ed do you\n\
Wonder why the yellow-bellied Snert ignores you?\n\
Why its foobly stare\n\
makes you feel ubiquitous obliquity.\n\
I can tell you, it is\n\
Worried by your HDF5-eating facial growth\n\
That looks like\n\
A moldy pile of ASCII data.\n\
What's more, it knows\n\
Your redimensioning potting shed\n\
Smells of booger.\n\
Everything under the big netCDF-filled sky\n\
Asks why, why do you even bother?\n\
You only charm software defects."
};

#define ATT_LEN 3
#define UCHAR_ATT_NAME "Number_of_vogons"
#define ULONGLONG_ATT_NAME "Number_of_vogon_poems"
#define LONGLONG_ATT_NAME "alien_concept_number_which_cannot_be_understood_by_humans"
#define DIM_NAME "length_of_name"
#define VAR_NAME "var_name"
#define DIM_LEN 2

int
main(int argc, char **argv)
{
   /* These values will be written in various places. */
   unsigned char num_vogons[ATT_LEN] = {2, 23, 230};
   unsigned long long num_poems[ATT_LEN] = {23232244LL, 1214124123423LL, 2353424234LL};
   long long alien[ATT_LEN] = {-23232244LL, 1214124123423LL, -2353424234LL};
   long long data[DIM_LEN] = {42LL, -42LL};

   printf("\n*** Creating test files for ncdump.\n");
   /*nc_set_log_level(4);*/

   printf("*** creating nested group file %s...", FILE_NAME_1);
   {
      int ncid, solar_system_id, dimid, varid;
      int earth_id, luna_id;

      /* Create a file with nested groups. */
      if (nc_create(FILE_NAME_1, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, SOLAR_SYSTEM, &solar_system_id)) ERR;
      if (nc_def_grp(solar_system_id, EARTH, &earth_id)) ERR;
      if (nc_def_grp(earth_id, LUNA, &luna_id)) ERR;
      
      /* Put some attributes in the root group. */
      if (nc_put_att_uchar(ncid, NC_GLOBAL, UCHAR_ATT_NAME, NC_UBYTE,
			   ATT_LEN, num_vogons)) ERR;
      if (nc_put_att_ulonglong(ncid, NC_GLOBAL, ULONGLONG_ATT_NAME, 
			       NC_UINT64, ATT_LEN, num_poems)) ERR;
      
      /* Put a dimension in the root group. */
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      
      /* Put an attribute in the Earth group. */
      if (nc_put_att_longlong(earth_id, NC_GLOBAL, LONGLONG_ATT_NAME, NC_INT64,
			      ATT_LEN, alien)) ERR;
      
      /* Put an attribute in the bottom group. */
      if (nc_put_att_text(luna_id, NC_GLOBAL, ATT_NAME, 
			  strlen(poem) + 1, poem)) ERR;
      
      /* Put a variable in the bottom group. */
      if (nc_def_var(luna_id, VAR_NAME, NC_INT64, 1, &dimid, &varid)) ERR;
      if (nc_put_var_longlong(luna_id, varid, data)) ERR;
      
      if (nc_close(ncid)) ERR;

   }
   
   SUMMARIZE_ERR;
   printf("*** checking nested group file %s...", FILE_NAME_1);
#define CHAR_ATT_MAX 3000
   
   {
      int ncid, solar_system_id;
      int earth_id, luna_id;
      int numgrps_in;
      unsigned char uchar_in[ATT_LEN];
      unsigned long long ulonglong_in[ATT_LEN];
      long long longlong_in[ATT_LEN], data_in[DIM_LEN];
      char char_in[CHAR_ATT_MAX], name_in[NC_MAX_NAME + 1];
      int varid_in, dimid_in, ndims_in, natts_in, dimid_in_2, nvars_in;
      size_t len_in;
      nc_type xtype_in;
      int i;

      /* Oh well, might as well check this file. It will also be
       * checked by ncdump tests. */
      if (nc_open(FILE_NAME_1, NC_NOWRITE, &ncid)) ERR;      
      
      /* Check nested groups. */
      if (nc_inq_grps(ncid, &numgrps_in, &solar_system_id)) ERR;
      if (numgrps_in != 1) ERR;
      if (nc_inq_grps(solar_system_id, &numgrps_in, &earth_id)) ERR;
      if (numgrps_in != 1) ERR;
      if (nc_inq_grps(earth_id, &numgrps_in, &luna_id)) ERR;
      if (numgrps_in != 1) ERR;
      if (nc_inq_grps(luna_id, &numgrps_in, NULL)) ERR;
      if (numgrps_in != 0) ERR;

      /* Check some attributes in the root group. */
      if (nc_inq_att(ncid, NC_GLOBAL, UCHAR_ATT_NAME, &xtype_in,
		     &len_in)) ERR;
      if (xtype_in != NC_UBYTE || len_in != ATT_LEN) ERR;
      if (nc_get_att_uchar(ncid, NC_GLOBAL, UCHAR_ATT_NAME, uchar_in)) ERR;
      for (i = 0; i < ATT_LEN; i++)
	 if (uchar_in[i] != num_vogons[i]) ERR;

      if (nc_inq_att(ncid, NC_GLOBAL, ULONGLONG_ATT_NAME, &xtype_in,
		     &len_in)) ERR;
      if (xtype_in != NC_UINT64 || len_in != ATT_LEN) ERR;
      if (nc_get_att_ulonglong(ncid, NC_GLOBAL, ULONGLONG_ATT_NAME, ulonglong_in)) ERR;
      for (i = 0; i < ATT_LEN; i++)
	 if (ulonglong_in[i] != num_poems[i]) ERR;

      /* Check a dimension in the root group. */
      if (nc_inq_dimids(ncid, &ndims_in, &dimid_in, 0)) ERR;
      if (ndims_in != 1) ERR;
      if (nc_inq_dim(ncid, dimid_in, name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM_NAME) || len_in != DIM_LEN) ERR;
      
      /* Check an attribute in the Earth group. */
      if (nc_inq_att(earth_id, NC_GLOBAL, LONGLONG_ATT_NAME, &xtype_in,
		     &len_in)) ERR;
      if (xtype_in != NC_INT64 || len_in != ATT_LEN) ERR;
      if (nc_get_att_longlong(earth_id, NC_GLOBAL, LONGLONG_ATT_NAME, 
			      longlong_in)) ERR;
      for (i = 0; i < ATT_LEN; i++)
	 if (longlong_in[i] != alien[i]) ERR;

      /* Check an attribute in the bottom group. */
      if (nc_inq_att(luna_id, NC_GLOBAL, ATT_NAME, &xtype_in,
		     &len_in)) ERR;
      if (xtype_in != NC_CHAR || len_in != strlen(poem) + 1 || 
	  len_in > CHAR_ATT_MAX) ERR;
      if (nc_get_att_text(luna_id, NC_GLOBAL, ATT_NAME, char_in)) ERR;
      char_in[len_in] = '\0';	/* null terminate, because nc_get_att_text doesn't */
      if (strcmp(char_in, poem)) ERR;
      
      /* Check a variable in the bottom group. */
      if (nc_inq_varids(luna_id, &nvars_in, &varid_in)) ERR;
      if (nc_inq_var(luna_id, varid_in, name_in, &xtype_in, &ndims_in, 
		     &dimid_in_2, &natts_in)) ERR;
      if (strcmp(name_in, VAR_NAME) || xtype_in != NC_INT64 || 
	  ndims_in != 1 || dimid_in_2 != dimid_in || natts_in != 0) ERR;
      if (nc_get_var_longlong(luna_id, varid_in, data_in)) ERR;
      for (i = 0; i < DIM_LEN; i++)
	 if (data_in[i] != data[i]) ERR;

      if (nc_close(ncid)) ERR;

   }
   
   SUMMARIZE_ERR;
   printf("*** creating file with VLEN %s...", FILE_NAME_2);
#define ATT_NAME2 "equally_unimaginatively_named_attribute_YAWN"

   {
      int ncid;
      int i, j;
      nc_type typeid;
      nc_vlen_t data[DIM_LEN];
      int *phoney;

      /* Create phoney data. */
      for (i=0; i<DIM_LEN; i++)
      {
	 if (!(phoney = (int *)malloc(sizeof(int) * (i+1))))
	    return NC_ENOMEM;
	 for (j=0; j<i+1; j++)
	    phoney[j] = -99;
	 data[i].p = phoney;
	 data[i].len = i+1;
      }

      /* Create a file with a VLEN attribute. */
      if (nc_create(FILE_NAME_2, NC_NETCDF4, &ncid)) ERR;
      
      if (nc_def_vlen(ncid, VLEN_TYPE_NAME, NC_INT, &typeid)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME2, typeid, DIM_LEN, data)) ERR;

      if (nc_close(ncid)) ERR;

      /* Free the memory used in our phoney data. */
      for (i=0; i<DIM_LEN; i++)
	 free(data[i].p);
   }
   
   SUMMARIZE_ERR;
   printf("*** creating file with compound type %s...", FILE_NAME_CMP);
#define ATT_NAME_CMP "my_favorite_wind_speeds"
#define COMPOUND_NAME "wind_vector"
#define NUM_FAVS 3
#define U_VALUE 13.3
#define V_VALUE 12.2

   {
      int ncid;

      /* Store winds as two floats: the u and v components of the wind. */
      struct wind_vector 
      {
	    float u, v;
      } favs[NUM_FAVS];
      nc_type typeid;
      int fav;

      /* Create some fake data... */
      for (fav = 0; fav < NUM_FAVS; fav++)
      {
	 favs[fav].u = U_VALUE;
	 favs[fav].v = V_VALUE;
      } 

      /* Create a file with a compound attribute. */
      if (nc_create(FILE_NAME_CMP, NC_NETCDF4, &ncid)) ERR;
      
      if (nc_def_compound(ncid, sizeof(struct wind_vector), COMPOUND_NAME, 
			  &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, "u", NC_COMPOUND_OFFSET(struct wind_vector, u), 
			     NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, typeid, "v", NC_COMPOUND_OFFSET(struct wind_vector, v), 
			     NC_FLOAT)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME_CMP, typeid, NUM_FAVS, favs)) ERR;

      if (nc_close(ncid)) ERR;
   }
   
   SUMMARIZE_ERR;

   FINAL_RESULTS;
}

