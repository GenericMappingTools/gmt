/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test that HDF5 and NetCDF-4 can read and write the same file.

   $Id$
*/
#include <config.h>
#include <nc_tests.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_interops.h5"
#define DIMSCALE_NAME "dimscale"
#define VAR1_NAME "var1"
#define NDIMS 1
#define DIM1_LEN 3
#define NAME_ATTRIBUTE "dimscale_name_attribute"
#define DIMSCALE_LABEL "dimscale_label"
#define LAT_LEN 3
#define LON_LEN 2
#define DIMS_2 2
#define LAT_NAME "lat"
#define LON_NAME "lon"
#define PRES_NAME "pres"
#define TEMP_NAME "temp"
#define ATT_NAME "song"
#define NEW_FLOAT 1.0

void 
printRes(const char *msg, int ires) 
{
   printf("%s: %d\n", msg, ires);
   if (ires < 0) 
   {
      printf("bad ires: %d\n", ires);
      /*H5Eprint2(ires, stdout);*/
      exit(1);
   }
}

static char song[] = "Oh, better far to live and die\n\
Under the brave black flag I fly,\n\
Than play a sanctimonious part,\n\
With a pirate head and a pirate heart.\n\
Away to the cheating world go you,\n\
Where pirates all are well-to-do;\n\
But I.ll be true to the song I sing,\n\
And live and die a Pirate King.\n\
For I am a Pirate King!\n\
And it is, it is a glorious thing\n\
To be a Pirate King!\n";

int
main(int argc, char **argv)
{
   printf("\n*** Testing HDF5/NetCDF-4 interoperability...\n");
   printf("*** Creating a HDF5 file with one var with two dimension scales...");
   {
      hid_t fileid, lat_spaceid, lon_spaceid, pres_spaceid;
      hid_t pres_datasetid, lat_dimscaleid, lon_dimscaleid;
      hsize_t dims[DIMS_2];
      hid_t fapl_id = H5P_DEFAULT, fcpl_id = H5P_DEFAULT;

      /* Set latest_format in access propertly list and
       * H5P_CRT_ORDER_TRACKED in the creation property list. This turns
       * on HDF5 creation ordering. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;

      /* Create the spaces that will be used for the dimscales. */
      dims[0] = LAT_LEN;
      if ((lat_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = LON_LEN;
      if ((lon_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create the space for the dataset. */
      dims[0] = LAT_LEN;
      dims[1] = LON_LEN;
      if ((pres_spaceid = H5Screate_simple(DIMS_2, dims, dims)) < 0) ERR;

      /* Create our dimension scales. */
      if ((lat_dimscaleid = H5Dcreate(fileid, LAT_NAME, H5T_NATIVE_INT,
				      lat_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lat_dimscaleid, NULL) < 0) ERR;
      if ((lon_dimscaleid = H5Dcreate(fileid, LON_NAME, H5T_NATIVE_INT,
				      lon_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lon_dimscaleid, NULL) < 0) ERR;

      /* Create a variable which uses these two dimscales. */
      if ((pres_datasetid = H5Dcreate(fileid, PRES_NAME, H5T_NATIVE_FLOAT,
				      pres_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lat_dimscaleid, 0) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lon_dimscaleid, 1) < 0) ERR;

      /* Fold up our tents. */
      if (H5Dclose(lat_dimscaleid) < 0 ||
	  H5Dclose(lon_dimscaleid) < 0 ||
	  H5Dclose(pres_datasetid) < 0 ||
	  H5Sclose(lat_spaceid) < 0 ||
	  H5Sclose(lon_spaceid) < 0 ||
	  H5Sclose(pres_spaceid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking that HDF5 file can be read by netCDF-4, and adding an att...");
   {
      int ncid;
      char name_in[NC_MAX_NAME + 1];
      int natts_in, ndims_in, nvars_in, varid_in, dimids_in[5], unlimdimid_in;
      size_t len_in;
      nc_type xtype_in;
      size_t index[2];
      float new_float = NEW_FLOAT;

      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

      /* Check it out. Can't count on creation order until HDF5 1.8.0,
       * so the following code doesn't depend on it. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 2 || nvars_in != 3 || natts_in != 0 || unlimdimid_in != -1) ERR;
      if (nc_inq_varid(ncid, PRES_NAME, &varid_in)) ERR;
      if (nc_inq_var(ncid, varid_in, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, PRES_NAME) || xtype_in != NC_FLOAT || ndims_in != 2 ||
	  natts_in != 0) ERR;
      if (nc_inq_dim(ncid, dimids_in[0], name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dim(ncid, dimids_in[1], name_in, &len_in)) ERR;
      if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;

      /* Change some data. */
      index[0] = index[1] = 0;
      if (nc_put_var1_float(ncid, varid_in, index, &new_float)) ERR;

      /* Just for swank, add an attribute. Now we're talking
       * interoperabity, dude! */
      if (nc_put_att_text(ncid, NC_GLOBAL, ATT_NAME, strlen(song)+1,
			  song)) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking that one var, two dimscales, one att file can still be read by HDF5...");

   {
      hid_t fileid, spaceid, datasetid, attid, typeid, grpid;
      char song_in[1024];
      float pres_in[LAT_LEN][LON_LEN];

      /* Open the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Check it out. */
      if ((datasetid = H5Dopen1(grpid, PRES_NAME)) < 0) ERR;
      if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
      if (H5Dread(datasetid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
		  H5P_DEFAULT, pres_in) < 0) ERR;
      if (pres_in[0][0] != NEW_FLOAT) ERR;
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if (H5Aread(attid, typeid, song_in) < 0) ERR;
      if (strcmp(song, song_in)) ERR;
      
      /* Close up the shop. */
      if (H5Tclose(typeid) < 0 ||
	  H5Aclose(attid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Dclose(datasetid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Creating a HDF5 file with one var and no dimension scales...");
   {
      hid_t fileid, pres_spaceid, pres_datasetid;
      hsize_t dims[DIMS_2];
      hid_t fapl_id, fcpl_id;
      int ncid, nvars_in, ndims_in, natts_in, unlimdim_in;
      size_t len_in;

      /* Set latest_format in access propertly list and
       * H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;

      /* Create the space for the dataset. */
      dims[0] = LAT_LEN;
      dims[1] = LON_LEN;
      if ((pres_spaceid = H5Screate_simple(DIMS_2, dims, dims)) < 0) ERR;

      /* Create a variable. It will not have dimension scales. */
      if ((pres_datasetid = H5Dcreate(fileid, PRES_NAME, H5T_NATIVE_FLOAT, 
				      pres_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Ring down the curtain. */
      if (H5Dclose(pres_datasetid) < 0 ||
	  H5Sclose(pres_spaceid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Read the data with netCDF. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdim_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, NULL, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_inq_dim(ncid, 1, NULL, &len_in)) ERR;
      if (len_in != LON_LEN) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Creating a HDF5 file with one var and no dimension scales, without creation ordering...");
   {
      hid_t fileid, pres_spaceid, pres_datasetid;
      hsize_t dims[DIMS_2];
      int ncid, nvars_in, ndims_in, natts_in, unlimdim_in;
      size_t len_in;

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;

      /* Create the space for the dataset. */
      dims[0] = LAT_LEN;
      dims[1] = LON_LEN;
      if ((pres_spaceid = H5Screate_simple(DIMS_2, dims, dims)) < 0) ERR;

      /* Create a variable. It will not have dimension scales. */
      if ((pres_datasetid = H5Dcreate(fileid, PRES_NAME, H5T_NATIVE_FLOAT, 
				      pres_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Ring down the curtain. */
      if (H5Dclose(pres_datasetid) < 0 ||
	  H5Sclose(pres_spaceid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Read the data with netCDF. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdim_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, NULL, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_inq_dim(ncid, 1, NULL, &len_in)) ERR;
      if (len_in != LON_LEN) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Creating a HDF5 file with two vars and no dimension scales, without creation ordering...");
   {
      hid_t fileid, spaceid, pres_datasetid, temp_datasetid;
      hsize_t dims[DIMS_2];
      int ncid, nvars_in, ndims_in, natts_in, unlimdim_in;
      size_t len_in;

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;

      /* Create the space for the datasets. */
      dims[0] = LAT_LEN;
      dims[1] = LON_LEN;
      if ((spaceid = H5Screate_simple(DIMS_2, dims, dims)) < 0) ERR;

      /* Create two datasets. They will not have dimension scales. */
      if ((pres_datasetid = H5Dcreate(fileid, PRES_NAME, H5T_NATIVE_FLOAT, 
				      spaceid, H5P_DEFAULT)) < 0) ERR;
      if ((temp_datasetid = H5Dcreate(fileid, TEMP_NAME, H5T_NATIVE_FLOAT, 
				      spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Ring down the curtain. */
      if (H5Dclose(pres_datasetid) < 0 ||
	  H5Dclose(temp_datasetid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Read the data with netCDF. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != 2 || nvars_in != 2 || natts_in != 0 || unlimdim_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, NULL, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_inq_dim(ncid, 1, NULL, &len_in)) ERR;
      if (len_in != LON_LEN) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Creating a HDF5 file with fixed length string...");
   {
#define ATT_LEN 3
#define ATT_NAME2 "genius"
#define MAX_LEN 6
      hid_t fileid, spaceid, typeid, attid, fapl_id;
      hid_t class;
      int ncid, nvars_in, ndims_in, natts_in, unlimdim_in;
      char data[ATT_LEN][MAX_LEN + 1] = {"Larry_", "Curley", "Moe111"};
      char *data_in, *cur, *data_in2[ATT_LEN];
      hsize_t dims[1] = {ATT_LEN};
      nc_type type_in;
      size_t size_in;
      int i;

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;


      if ((typeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, MAX_LEN + 1) < 0) ERR;
   
      /* Write an attribute of this (string) type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(fileid, ATT_NAME2, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, data) < 0) ERR;

      /* Close HDF5 objects. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* Open the file and read the attribute of fixed length
       * strings. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((attid = H5Aopen_name(fileid, ATT_NAME2)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;

      /* Given this type id, how would we know this is a string
       * attribute? */
      if ((class = H5Tget_class(typeid)) < 0)
	 return NC_EHDFERR;
      if (class != H5T_STRING) ERR;
/*      if (!(type_size = H5Tget_size(typeid))) ERR;
	if ((is_str = H5Tis_variable_str(typeid)) < 0) ERR;*/

      /* How many strings are in the string array? */
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      if (H5Sget_simple_extent_dims(spaceid, dims, NULL) < 0) ERR;
      if (!(data_in = malloc(dims[0] * sizeof(char *) * (MAX_LEN + 1)))) ERR;

      /* Now read the array of strings. The HDF5 library will allocate
       * space for each string. */
      if (H5Aread(attid, typeid, data_in) < 0) ERR;

      /* Compare the values to make sure it worked... */
      for (cur = data_in, i = 0; i < ATT_LEN; i++)
      {
	 if (strcmp(data[i], cur)) ERR;
	 cur += MAX_LEN + 1;
      }

      /* Free the memory that we allocated. */
      free(data_in);

      /* Close everything up. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      
      /* Read the data with netCDF. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != 0 || nvars_in != 0 || natts_in != 1 || unlimdim_in != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME2, &type_in, &size_in)) ERR;
      if (type_in != NC_STRING || size_in != ATT_LEN) ERR;
      if (nc_get_att_string(ncid, NC_GLOBAL, ATT_NAME2, data_in2)) ERR;
      for (i = 0; i < size_in; i++)
	 if (strcmp(data[i], data_in2[i])) ERR;
      if (nc_free_string(size_in, data_in2)) ERR;

      /* Open the file with HDF5 while netcdf still has it open. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      /* Turn this off for*/
#ifdef EXTRA_TESTS
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI)) ERR;
#else
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
#endif
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
/*    printf("**** testing 2D coordinate variable..."); */

/*    { */
/* #define VAR_NAME "Britany" */
/* #define NDIMS 2 */
/* #define TEXT_LEN 15 */
/* #define D0_NAME "time" */
/* #define D1_NAME "tl" */
/*       int ncid, nvars_in, varids_in[1]; */
/*       int time_dimids[NDIMS], time_id; */
/*       size_t time_count[NDIMS], time_index[NDIMS] = {0, 0}; */
/*       const char ttext[TEXT_LEN]="20051224.150000"; */
/*       int nvars, ndims, ngatts, unlimdimid; */
/*       int ndims_in, natts_in, dimids_in[NDIMS]; */
/*       char var_name_in[NC_MAX_NAME + 1]; */
/*       nc_type xtype_in; */

/*       /\* Create a netcdf-4 file with 2D coordinate var. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */

/*       if (nc_def_dim(ncid, D0_NAME, NC_UNLIMITED, &time_dimids[0])) ERR; */
/*       if (nc_def_dim(ncid, D1_NAME, TEXT_LEN, &time_dimids[1])) ERR; */
/*       if (nc_def_var(ncid, D0_NAME, NC_CHAR, NDIMS, time_dimids, &time_id)) ERR; */

/*       /\* Write one time to the coordinate variable. *\/ */
/*       time_count[0] = 1; */
/*       time_count[1] = TEXT_LEN; */
/*       if (nc_put_vara_text(ncid, time_id, time_index, time_count, ttext)) ERR; */
/*       if (nc_close(ncid)) ERR; */

/*       /\* Open the file and check. *\/ */
/*       if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR; */
/*       if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR; */
/*       if (nvars != 1 || ndims != 2 || ngatts != 0 || unlimdimid != 0) ERR; */
/*       if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR; */
/*       if (nvars_in != 1 || varids_in[0] != 0) ERR; */
/*       if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR; */
/*       if (strcmp(var_name_in, D0_NAME) || xtype_in != NC_CHAR || ndims_in != 2 || */
/* 	dimids_in[0] != 0 || dimids_in[1] != 1 || natts_in != 0) ERR; */
/*       if (nc_close(ncid)) ERR; */
/*    } */
/*    SUMMARIZE_ERR; */
   FINAL_RESULTS;
}

