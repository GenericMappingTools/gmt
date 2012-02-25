/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This program deals with HDF5 compound types.

   $Id$
*/
#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_compounds.h5"
#define DIM1_LEN 3
#define OSMONDS "TheOsmonds"
#define WHO "TheWho"
#define BOOZE_VAR "Alcohol_Consumed"
#define BEER_OR_WINE "Beer_or_Wine"
#define LIQUOR "The_Hard_Stuff"
#define COMPOUND_NAME "Booze_Index"
#define ARRAY_LEN 5
#define STR_LEN 255

int
main()
{
   hid_t fileid, osmonds_grpid, who_grpid, spaceid, typeid;
   hid_t datasetid, datasetid1, typeid1;
   hsize_t dims[1];
   struct s1 {
         int i1, i2;
   } data[DIM1_LEN];
   struct s2 {
         int i1[ARRAY_LEN], i2;
   } data2[DIM1_LEN];
   char *dummy;
   int i, j;

   /* REALLY initialize the data (even the gaps in the structs). This
    * is only needed to pass valgrind. */
   if (!(dummy = calloc(sizeof(struct s2), DIM1_LEN))) ERR;
   memcpy((void *)data2, (void *)dummy, sizeof(struct s2) * DIM1_LEN); 
   free(dummy); 
   if (!(dummy = calloc(sizeof(struct s1), DIM1_LEN))) ERR;
   memcpy((void *)data2, (void *)dummy, sizeof(struct s1) * DIM1_LEN); 
   free(dummy); 

   for (i=0; i<DIM1_LEN; i++)
   {
      data[i].i1 = 99;
      data[i].i2 = -99;
      data2[i].i2 = -99;
      for (j=0; j<ARRAY_LEN; j++)
         data2[i].i1[j] = 99;
  }

   printf("\n*** Checking HDF5 compound types.\n");
   printf("*** Checking simple HDF5 compound types...");
   {
   
      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
                              H5P_DEFAULT)) < 0) ERR;
      if ((osmonds_grpid = H5Gcreate(fileid, OSMONDS, 0)) < 0) ERR;

      /* Create a simple compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid, BEER_OR_WINE, HOFFSET(struct s1, i1), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid, LIQUOR, HOFFSET(struct s1, i2), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tcommit(osmonds_grpid, COMPOUND_NAME, typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create a dataset of this compound type. */
      if ((datasetid = H5Dcreate(osmonds_grpid, BOOZE_VAR, typeid, 
                                 spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, 
                   H5P_DEFAULT, data) < 0) ERR;

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(osmonds_grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((osmonds_grpid = H5Gopen(fileid, OSMONDS)) < 0) ERR;
      if ((datasetid = H5Dopen1(osmonds_grpid, BOOZE_VAR)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Tget_nmembers(typeid) != 2) ERR;
      /* This doesn't work because all I have is a reference to the type! 
         if (H5Iget_name(typeid, type_name, STR_LEN) < 0) ERR;
         if (strcmp(type_name, COMPOUND_NAME)) ERR;*/

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(osmonds_grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 compound types and groups...");
   
   {
      /* Open file and create two group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
                              H5P_DEFAULT)) < 0) ERR;
      if ((osmonds_grpid = H5Gcreate(fileid, OSMONDS, 0)) < 0) ERR;
      if ((who_grpid = H5Gcreate(fileid, WHO, 0)) < 0) ERR;

      /* Create a simple compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid, BEER_OR_WINE, HOFFSET(struct s1, i1), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid, LIQUOR, HOFFSET(struct s1, i2), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tcommit(osmonds_grpid, COMPOUND_NAME, typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create a dataset of this compound type in the same group. */
      if ((datasetid = H5Dcreate(osmonds_grpid, BOOZE_VAR, typeid, 
                                 spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, 
                   H5P_DEFAULT, data) < 0) ERR;

      /* Create a dataset of this compound type in a different group. */
      if ((datasetid1 = H5Dcreate(who_grpid, BOOZE_VAR, typeid, 
                                  spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid1, typeid, H5S_ALL, H5S_ALL, 
                   H5P_DEFAULT, data) < 0) ERR;

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Dclose(datasetid1) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(osmonds_grpid) < 0 ||
          H5Gclose(who_grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      {
         hsize_t num_obj;
         int i, obj_type;
         char name[STR_LEN + 1];
         htri_t equal;


         /* Now open the file and read it. */
         if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
         if ((osmonds_grpid = H5Gopen(fileid, OSMONDS)) < 0) ERR;
         if (H5Gget_num_objs(osmonds_grpid, &num_obj) < 0) ERR;
         for (i=0; i<num_obj; i++)
         {
            if (H5Gget_objname_by_idx(osmonds_grpid, i, name, STR_LEN+1) < 0) ERR;
            if ((obj_type = H5Gget_objtype_by_idx(osmonds_grpid, i)) < 0) ERR;
            switch(obj_type)
            {
               case H5G_DATASET:
                  if ((datasetid = H5Dopen1(osmonds_grpid, name)) < 0) ERR;
                  break;
               case H5G_TYPE:
                  if ((typeid = H5Topen(osmonds_grpid, name)) < 0) ERR;
                  if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
                  if (H5Tget_nmembers(typeid) != 2) ERR;
                  if (strcmp(name, COMPOUND_NAME)) ERR;
                  break;
               default:
                  ERR;
            }
         }

         /* Open the other dataset, and learn about its type. */
         if ((who_grpid = H5Gopen(fileid, WHO)) < 0) ERR;
         if ((datasetid1 = H5Dopen1(who_grpid, BOOZE_VAR)) < 0) ERR;
         if ((typeid1 = H5Dget_type(datasetid1)) < 0) ERR;
         if ((equal = H5Tequal(typeid, typeid1)) < 0) ERR;
         if (!equal) ERR;
      }

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Dclose(datasetid1) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Tclose(typeid1) < 0 ||
          H5Gclose(osmonds_grpid) < 0 ||
          H5Gclose(who_grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 compound type which contains an array...");
   
   {
      hsize_t array_dims[] = {ARRAY_LEN};
      hid_t array_typeid;

      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
                              H5P_DEFAULT)) < 0) ERR;
      if ((osmonds_grpid = H5Gcreate(fileid, OSMONDS, 0)) < 0) ERR;

      /* Create an array type. */
      if ((array_typeid = H5Tarray_create(H5T_NATIVE_INT, 1, array_dims, NULL)) < 0) ERR;

      /* Create a compound type containing an array. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct s2))) < 0) ERR;
      if (H5Tinsert(typeid, BEER_OR_WINE, HOFFSET(struct s2, i1), array_typeid) < 0) ERR;
      if (H5Tinsert(typeid, LIQUOR, HOFFSET(struct s2, i2), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tcommit(osmonds_grpid, COMPOUND_NAME, typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create a dataset of this compound type. */
      if ((datasetid = H5Dcreate(osmonds_grpid, BOOZE_VAR, typeid, 
                                 spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, 
                   H5P_DEFAULT, data2) < 0) ERR;
      
      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Tclose(array_typeid) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(osmonds_grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((osmonds_grpid = H5Gopen(fileid, OSMONDS)) < 0) ERR;
      if ((datasetid = H5Dopen1(osmonds_grpid, BOOZE_VAR)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Tget_nmembers(typeid) != 2) ERR;
      /* This doesn't work because all I have is a reference to the type! 
         if (H5Iget_name(typeid, type_name, STR_LEN) < 0) ERR;
         if (strcmp(type_name, COMPOUND_NAME)) ERR;*/

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(osmonds_grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 compound type 6 different types...");
   
   {
#define DAY "day"
#define ELEV "elev"
#define COUNT "count"
#define RELHUM "relhum"
#define TIME "time"
#define OBS_T "obs_t"
#define OBS_VAR "obs_var"
#define DIM6_LEN 3
      
      hid_t fileid, grpid, spaceid, typeid, native_typeid;
      hid_t datasetid, mem_type;
      hsize_t dims[1];
      typedef struct obs_t {
            char day ;
            short elev;
            int count;
            float relhum;
            double time;
      } obs_t ;
      obs_t obsdata[DIM6_LEN];
      obs_t obsdata_in[DIM6_LEN], obsdata2_in[DIM6_LEN];
      char file_in[STR_LEN * 2];
      char *dummy;
      size_t size_in;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct obs_t), DIM6_LEN))) ERR;
      memcpy((void *)obsdata, (void *)dummy, sizeof(struct obs_t) * DIM6_LEN); 
      free(dummy); 

      /* Initialize data. */
      for (i = 0; i < DIM6_LEN; i++)
      {
	 obsdata[i].day = 15 * i + 1;
	 obsdata[i].elev = 2 * i + 1;
	 obsdata[i].count = 2 * i + 1;
	 obsdata[i].relhum = 2.0 * i + 1;
	 obsdata[i].time = 2.0 * i + 1;
      }

      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a compound type containing some different types. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct obs_t))) < 0) ERR;
      if (H5Tinsert(typeid, DAY, HOFFSET(struct obs_t, day), H5T_NATIVE_CHAR) < 0) ERR;
      if (H5Tinsert(typeid, ELEV, HOFFSET(struct obs_t, elev), H5T_NATIVE_SHORT) < 0) ERR;
      if (H5Tinsert(typeid, COUNT, HOFFSET(struct obs_t, count), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid, RELHUM, HOFFSET(struct obs_t, relhum), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid, TIME, HOFFSET(struct obs_t, time), H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, OBS_T, typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM6_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create a dataset of this compound type. */
      if ((datasetid = H5Dcreate(grpid, OBS_VAR, typeid, spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, obsdata) < 0) ERR;
      
      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, OBS_VAR)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if ((size_in = H5Tget_size(typeid)) == 0) ERR;
      if (size_in != sizeof(struct obs_t)) ERR;
      if ((native_typeid = H5Tget_native_type(typeid, H5T_DIR_DEFAULT)) < 0) ERR;
      if ((size_in = H5Tget_size(native_typeid)) == 0) ERR;
      if (size_in != sizeof(struct obs_t)) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Tget_nmembers(typeid) != 5) ERR;

      /* Read all data. */
      if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, obsdata_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DIM6_LEN; i++)
      {
         if (obsdata[i].day != obsdata_in[i].day || obsdata[i].elev != obsdata_in[i].elev ||
             obsdata[i].count != obsdata_in[i].count || obsdata[i].relhum != obsdata_in[i].relhum ||
             obsdata[i].time != obsdata_in[i].time)
         {
            ERR;
            return 1;
         }
      }

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the reference copy of this file and read it. */
#define REF_FILE_NAME "ref_tst_h_compounds.h5"
      if (getenv("srcdir"))
      {
         strcpy(file_in, getenv("srcdir"));
         strcat(file_in, "/");
         strcat(file_in, REF_FILE_NAME);
      }
      else
         strcpy(file_in, REF_FILE_NAME);

      if ((fileid = H5Fopen(file_in, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, OBS_VAR)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if ((mem_type = H5Tget_native_type(typeid, H5T_DIR_DEFAULT)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Tget_nmembers(typeid) != 5) ERR;

      /* Read all data. */
      if (H5Dread(datasetid, mem_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, obsdata2_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DIM6_LEN; i++)
      {
         if (obsdata[i].day != obsdata2_in[i].day || obsdata[i].elev != obsdata2_in[i].elev ||
             obsdata[i].count != obsdata2_in[i].count || obsdata[i].relhum != obsdata2_in[i].relhum ||
             obsdata[i].time != obsdata2_in[i].time)
         {
            ERR;
            return 1;
         }
      }

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 compound variable which contains a compound type...");
   {
#define DATASET_NAME "Enterprise"
      /* This struct will be embeddeded in another. */
      struct s1
      {
            int i1;
            int i2;
      };
      /* StarFleet Human Resources Department has data records for all
       * employees. */
      struct hr_rec
      {
            int starfleet_id;
            struct s1 svc_rec;
            char name[STR_LEN + 1];
            float max_temp, min_temp; /* temperature range */
            double percent_transporter_errosion;
      };
      struct hr_rec hr_data_out[DIM1_LEN], hr_data_in[DIM1_LEN];

      hid_t fileid, grpid, typeid_inner, typeid, spaceid, array1_tid, datasetid;
      hsize_t dims[1];
      char *dummy;
      int i;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct hr_rec), DIM1_LEN))) ERR;
      memcpy((void *)hr_data_out, (void *)dummy, sizeof(struct hr_rec) * DIM1_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
         hr_data_out[i].starfleet_id = i;
         hr_data_out[i].svc_rec.i1 = 95;
         hr_data_out[i].svc_rec.i2 = 90;
         if (sprintf(hr_data_out[i].name, "alien_%d", i) < 0) ERR;
         hr_data_out[i].max_temp = 99.99;
         hr_data_out[i].min_temp = -9.99;
         hr_data_out[i].percent_transporter_errosion = .1;
      }

      /* Open file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a compound type. */
      if ((typeid_inner = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid_inner, "i1", HOFFSET(struct s1, i1), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid_inner, "i2", HOFFSET(struct s1, i2), H5T_NATIVE_INT) < 0) ERR;

      /* Create a compound type containing a compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct hr_rec))) < 0) ERR;
      if (H5Tinsert(typeid, "starfleet_id", HOFFSET(struct hr_rec, starfleet_id), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid, "svc_rec", HOFFSET(struct hr_rec, svc_rec), typeid_inner) < 0) ERR;
      if ((array1_tid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(array1_tid, STR_LEN + 1) < 0) ERR;
      if (H5Tinsert(typeid, "name", HOFFSET(struct hr_rec, name), array1_tid) < 0) ERR;
      if (H5Tinsert(typeid, "max_temp", HOFFSET(struct hr_rec, max_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid, "min_temp", HOFFSET(struct hr_rec, min_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid, "percent_transporter_errosion", HOFFSET(struct hr_rec, percent_transporter_errosion),
                    H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, "hr_rec", typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create a dataset of this compound type. */
      if ((datasetid = H5Dcreate(grpid, DATASET_NAME, typeid, spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, hr_data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(array1_tid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Tclose(typeid_inner) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, DATASET_NAME)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, hr_data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DIM1_LEN; i++)
         if (hr_data_out[i].starfleet_id != hr_data_in[i].starfleet_id ||
             hr_data_out[i].svc_rec.i1 != hr_data_in[i].svc_rec.i1 ||
             hr_data_out[i].svc_rec.i2 != hr_data_in[i].svc_rec.i2 ||
             strcmp(hr_data_out[i].name, hr_data_in[i].name) ||
             hr_data_out[i].max_temp != hr_data_in[i].max_temp ||
             hr_data_out[i].min_temp != hr_data_in[i].min_temp ||
             hr_data_out[i].percent_transporter_errosion != hr_data_in[i].percent_transporter_errosion)
            ERR;

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 variable which contains a compound type with different string handling...");
   {
#define DATASET_NAME "Enterprise"
      /* This struct will be embeddeded in another. */
      struct s1
      {
            int i1;
            int i2;
      };
      /* StarFleet Human Resources Department has data records for all
       * employees. */
      struct hr_rec
      {
            int starfleet_id;
            struct s1 svc_rec;
            char name[STR_LEN + 1];
            float max_temp, min_temp; /* temperature range */
            double percent_transporter_errosion;
      };
      struct hr_rec hr_data_out[DIM1_LEN], hr_data_in[DIM1_LEN];

      hid_t fileid, grpid, typeid_inner, typeid, spaceid, array1_tid, datasetid, str_tid;
      hsize_t dims[1];
      int i;

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
         hr_data_out[i].starfleet_id = i;
         hr_data_out[i].svc_rec.i1 = 95;
         hr_data_out[i].svc_rec.i2 = 90;
         if (sprintf(hr_data_out[i].name, "alien_%d", i) < 0) ERR;
         hr_data_out[i].max_temp = 99.99;
         hr_data_out[i].min_temp = -9.99;
         hr_data_out[i].percent_transporter_errosion = .1;
      }

      /* Open file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a compound type. */
      if ((typeid_inner = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid_inner, "i1", HOFFSET(struct s1, i1), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid_inner, "i2", HOFFSET(struct s1, i2), H5T_NATIVE_INT) < 0) ERR;

      /* Create a compound type containing a compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct hr_rec))) < 0) ERR;
      if (H5Tinsert(typeid, "starfleet_id", HOFFSET(struct hr_rec, starfleet_id), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid, "svc_rec", HOFFSET(struct hr_rec, svc_rec), typeid_inner) < 0) ERR;
      dims[0] = STR_LEN + 1;
      if ((str_tid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_strpad(str_tid, H5T_STR_NULLTERM) < 0) ERR;
      if ((array1_tid = H5Tarray_create2(str_tid, 1, dims)) < 0) ERR;
      if (H5Tinsert(typeid, "name", HOFFSET(struct hr_rec, name), array1_tid) < 0) ERR;
      if (H5Tinsert(typeid, "max_temp", HOFFSET(struct hr_rec, max_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid, "min_temp", HOFFSET(struct hr_rec, min_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid, "percent_transporter_errosion", HOFFSET(struct hr_rec, percent_transporter_errosion),
                    H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, "hr_rec", typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create a dataset of this compound type. */
      if ((datasetid = H5Dcreate(grpid, DATASET_NAME, typeid, spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, hr_data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(array1_tid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Tclose(typeid_inner) < 0 ||
          H5Tclose(str_tid) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, DATASET_NAME)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, hr_data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DIM1_LEN; i++)
         if (hr_data_out[i].starfleet_id != hr_data_in[i].starfleet_id ||
             hr_data_out[i].svc_rec.i1 != hr_data_in[i].svc_rec.i1 ||
             hr_data_out[i].svc_rec.i2 != hr_data_in[i].svc_rec.i2 ||
             strcmp(hr_data_out[i].name, hr_data_in[i].name) ||
             hr_data_out[i].max_temp != hr_data_in[i].max_temp ||
             hr_data_out[i].min_temp != hr_data_in[i].min_temp ||
             hr_data_out[i].percent_transporter_errosion != hr_data_in[i].percent_transporter_errosion)
            ERR;

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 compound attribute which contains a compound type...");
   {
#define ATT_NAME1 "Space_Station_Regula_One"
      /* This struct will be embeddeded in another. */
      struct s1
      {
            int i1;
            int i2;
      };
      /* StarFleet Human Resources Department has data records for all
       * employees. */
      struct hr_rec
      {
            int starfleet_id;
            struct s1 svc_rec;
            char name[STR_LEN + 1];
            float max_temp, min_temp; /* temperature range */
            double percent_transporter_errosion;
      };
      struct hr_rec hr_data_out[DIM1_LEN], hr_data_in[DIM1_LEN];

      hid_t fileid, grpid, typeid_inner, typeid, spaceid, array1_tid, attid, native_typeid;
      hsize_t dims[1];
      int i;

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
         hr_data_out[i].starfleet_id = i;
         hr_data_out[i].svc_rec.i1 = 95;
         hr_data_out[i].svc_rec.i2 = 90;
         if (sprintf(hr_data_out[i].name, "alien_%d", i) < 0) ERR;
         hr_data_out[i].max_temp = 99.99;
         hr_data_out[i].min_temp = -9.99;
         hr_data_out[i].percent_transporter_errosion = .1;
      }

      /* Open file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a compound type. */
      if ((typeid_inner = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid_inner, "i1", HOFFSET(struct s1, i1), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid_inner, "i2", HOFFSET(struct s1, i2), H5T_NATIVE_INT) < 0) ERR;

      /* Create a compound type containing a compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct hr_rec))) < 0) ERR;
      if (H5Tinsert(typeid, "starfleet_id", HOFFSET(struct hr_rec, starfleet_id), H5T_NATIVE_INT) < 0) ERR;
      if (H5Tinsert(typeid, "svc_rec", HOFFSET(struct hr_rec, svc_rec), typeid_inner) < 0) ERR;
      if ((array1_tid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(array1_tid, STR_LEN + 1) < 0) ERR;
      if (H5Tinsert(typeid, "name", HOFFSET(struct hr_rec, name), array1_tid) < 0) ERR;
      if (H5Tinsert(typeid, "max_temp", HOFFSET(struct hr_rec, max_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid, "min_temp", HOFFSET(struct hr_rec, min_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid, "percent_transporter_errosion", HOFFSET(struct hr_rec, percent_transporter_errosion),
                    H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, "hr_rec", typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create an attribute of this compound type. */
      if ((attid = H5Acreate2(grpid, ATT_NAME1, typeid, spaceid, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Write some data to the attribute. */
      if (H5Awrite(attid, typeid, hr_data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Tclose(typeid_inner) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_by_name(grpid, ".", ATT_NAME1, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if ((native_typeid = H5Tget_native_type(typeid, H5T_DIR_DEFAULT)) < 0) ERR;
      if (H5Aread(attid, native_typeid, hr_data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DIM1_LEN; i++)
         if (hr_data_out[i].starfleet_id != hr_data_in[i].starfleet_id ||
             hr_data_out[i].svc_rec.i1 != hr_data_in[i].svc_rec.i1 ||
             hr_data_out[i].svc_rec.i2 != hr_data_in[i].svc_rec.i2 ||
             strcmp(hr_data_out[i].name, hr_data_in[i].name) ||
             hr_data_out[i].max_temp != hr_data_in[i].max_temp ||
             hr_data_out[i].min_temp != hr_data_in[i].min_temp ||
             hr_data_out[i].percent_transporter_errosion != hr_data_in[i].percent_transporter_errosion)
            ERR;

      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 variable of compound type which contains a string...");
   {
#define DATASET_NAME "Enterprise"
      struct hr_rec
      {
            char name[STR_LEN + 1];
            float max_temp;
      };
      struct hr_rec hr_data_out[DIM1_LEN], hr_data_in[DIM1_LEN];

      hid_t fileid, grpid, typeid, spaceid, array1_tid, datasetid, str_tid;
      hsize_t dims[1] = {STR_LEN + 1};
      int i;

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
         if (sprintf(hr_data_out[i].name, "alien_%d", i) < 0) ERR;
         hr_data_out[i].max_temp = 99.99;
      }

      /* Open file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct hr_rec))) < 0) ERR;
/*      printf("sizeof(struct hr_rec)=%d dims[0]=%d\n", sizeof(struct hr_rec), dims[0]);*/
      if ((str_tid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_strpad(str_tid, H5T_STR_NULLTERM) < 0) ERR;
      if ((array1_tid = H5Tarray_create2(str_tid, 1, dims)) < 0) ERR;
/*      printf("sizeof(struct hr_rec)=%d HOFFSET(struct hr_rec, name) = %d HOFFSET(struct hr_rec, max_temp) = %d\n",
        sizeof(struct hr_rec), HOFFSET(struct hr_rec, name), HOFFSET(struct hr_rec, max_temp));*/
      if (H5Tinsert(typeid, "name", HOFFSET(struct hr_rec, name), array1_tid) < 0) ERR;
      if (H5Tinsert(typeid, "max_temp", HOFFSET(struct hr_rec, max_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tcommit(grpid, "hr_rec", typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create a dataset of this compound type. */
      if ((datasetid = H5Dcreate(grpid, DATASET_NAME, typeid, spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, hr_data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(array1_tid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Tclose(str_tid) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, DATASET_NAME)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, hr_data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DIM1_LEN; i++)
         if (strcmp(hr_data_out[i].name, hr_data_in[i].name) ||
             hr_data_out[i].max_temp != hr_data_in[i].max_temp) ERR;

      /* Release all resources. */
      if (H5Dclose(datasetid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 compound attribute which contains an array of char...");
   {
#define DIM2_LEN 1
#define ATT_NAME "HR_Records"
      struct hr_rec
      {
            char name[STR_LEN + 1];
            float max_temp;
      };
      struct hr_rec hr_data_out[DIM2_LEN], hr_data_in[DIM2_LEN];

      hid_t fileid, grpid, typeid, spaceid, array1_tid, attid, str_tid;
      hsize_t dims[1] = {STR_LEN + 1};
      char *dummy;
      int i;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct hr_rec), DIM2_LEN))) ERR;
      memcpy((void *)hr_data_out, (void *)dummy, sizeof(struct hr_rec) * DIM2_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (i = 0; i < DIM2_LEN; i++)
      {
         if (sprintf(hr_data_out[i].name, "alien_%d", i) < 0) ERR;
         hr_data_out[i].max_temp = 99.99;
      }

      /* Open file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct hr_rec))) < 0) ERR;
      if ((str_tid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_strpad(str_tid, H5T_STR_NULLTERM) < 0) ERR;
      if ((array1_tid = H5Tarray_create2(str_tid, 1, dims)) < 0) ERR;
/*      printf("sizeof(struct hr_rec)=%d HOFFSET(struct hr_rec, name) = %d HOFFSET(struct hr_rec, max_temp) = %d\n",
        sizeof(struct hr_rec), HOFFSET(struct hr_rec, name), HOFFSET(struct hr_rec, max_temp));*/
      if (H5Tinsert(typeid, "Name", HOFFSET(struct hr_rec, name), array1_tid) < 0) ERR;
      if (H5Tinsert(typeid, "Max_Temp", HOFFSET(struct hr_rec, max_temp), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tcommit(grpid, "SF_HR_Record", typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM2_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create an attribute of this compound type. */
      if ((attid = H5Acreate2(grpid, ATT_NAME, typeid, spaceid, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Awrite(attid, typeid, hr_data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(array1_tid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Tclose(str_tid) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_by_name(grpid, ".", ATT_NAME, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Aread(attid, typeid, hr_data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DIM2_LEN; i++)
         if (strcmp(hr_data_out[i].name, hr_data_in[i].name) ||
             hr_data_out[i].max_temp != hr_data_in[i].max_temp) ERR;

      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 compound attribute which contains an array of unsigned byte...");
   {
#define DISC_DIM1_LEN 2
#define DISC_ATT_NAME "Discovery"
      struct s1
      {
	    unsigned char x[STR_LEN + 1];
	    float y;
      };
      struct s1 data_out[DISC_DIM1_LEN], data_in[DISC_DIM1_LEN];

      hid_t fileid, grpid, typeid, spaceid, array1_tid, attid;
      hid_t fcpl_id, fapl_id;
      hsize_t dims[1] = {STR_LEN + 1};
      int i, j;

      /* Create some data. */
      for (i = 0; i < DISC_DIM1_LEN; i++)
      {
	 for (j = 0; j < STR_LEN + 1; j++)
	    data_out[i].x[j] = 4;
	 data_out[i].y = 99.99;
      }

      /* Set latest_format in access propertly list and
       * H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Open file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a compound type. */
      if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if ((array1_tid = H5Tarray_create2(H5T_NATIVE_UCHAR, 1, dims)) < 0) ERR;
      if (H5Tinsert(typeid, "x", HOFFSET(struct s1, x), array1_tid) < 0) ERR;
      if (H5Tinsert(typeid, "y", HOFFSET(struct s1, y), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tcommit(grpid, "c", typeid) < 0) ERR;

      /* Create a space. */
      dims[0] = DISC_DIM1_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create an attribute of this compound type. */
      if ((attid = H5Acreate2(grpid, DISC_ATT_NAME, typeid, spaceid, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Write some data. */
      if (H5Awrite(attid, typeid, data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
	  H5Tclose(array1_tid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Now open the file and read it. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_by_name(grpid, ".", DISC_ATT_NAME, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if (H5Tget_class(typeid) != H5T_COMPOUND) ERR;
      if (H5Aread(attid, typeid, data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < DISC_DIM1_LEN; i++)
      {
	 for (j = 0; j < STR_LEN + 1; j++)
	    if (data_in[i].x[j] != data_out[i].x[j]) ERR_RET;
	 if (data_in[i].y != data_out[i].y) ERR;
      }

      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking simple read of HDF5 compound attribute which contains a simple compound type...");
   {
#define DIM_CMP_LEN 1
#define ATT_CMP_NAME1 "The_Nutmeg_of_Consolation"
#define NUM_TYPES 2
#define INNER_TYPE_NAME "s1"
#define OUTER_TYPE_NAME "d"

      /* This struct will be embeddeded in another. */
      struct s1
      {
            float x;
            double y;
      };
      struct s2
      {
            struct s1 s1;
      };
      struct s2 data_out[DIM_CMP_LEN], data_in[DIM_CMP_LEN];
      hid_t fileid, grpid, typeid_inner, typeid_outer, spaceid, attid;
      hid_t att_typeid, att_native_typeid;
      hsize_t dims[1];
      hid_t fapl_id, fcpl_id;
      char *dummy;
      int i;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s2), DIM_CMP_LEN))) ERR;
      memcpy((void *)data_out, (void *)dummy, sizeof(struct s2) * DIM_CMP_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (i = 0; i < DIM_CMP_LEN; i++)
      {
         data_out[i].s1.x = 1.0;
         data_out[i].s1.y = -2.0;
      }

      /* Create file access and create property lists. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create the inner compound type. */
      if ((typeid_inner = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid_inner, "x", HOFFSET(struct s1, x), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid_inner, "y", HOFFSET(struct s1, y), H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, INNER_TYPE_NAME, typeid_inner) < 0) ERR;

      /* Create a compound type containing a compound type. */
      if ((typeid_outer = H5Tcreate(H5T_COMPOUND, sizeof(struct s2))) < 0) ERR;
      if (H5Tinsert(typeid_outer, INNER_TYPE_NAME, HOFFSET(struct s2, s1), typeid_inner) < 0) ERR;
      if (H5Tcommit(grpid, OUTER_TYPE_NAME, typeid_outer) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM_CMP_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create an attribute of this compound type. */
      if ((attid = H5Acreate2(grpid, ATT_CMP_NAME1, typeid_outer, spaceid, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Write some data to the attribute. */
      if (H5Awrite(attid, typeid_outer, data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(typeid_outer) < 0 ||
          H5Tclose(typeid_inner) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and get the type of the attribute. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_by_name(grpid, ".", ATT_CMP_NAME1, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((att_typeid = H5Aget_type(attid)) < 0) ERR;
      if ((att_native_typeid = H5Tget_native_type(att_typeid, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Check the data. */
      if (H5Aread(attid, att_native_typeid, data_in) < 0) ERR;
      for (i = 0; i < DIM_CMP_LEN; i++)
         if (data_out[i].s1.x != data_in[i].s1.x ||
             data_out[i].s1.y != data_in[i].s1.y) ERR;

      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(att_typeid) < 0 ||
          H5Tclose(att_native_typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

   }
   SUMMARIZE_ERR;
   printf("*** Checking simple read of HDF5 compound attribute which contains a simple compound type...");
   {
#define DIM_CMP_LEN 1
#define ATT_CMP_NAME1 "The_Nutmeg_of_Consolation"
#define NUM_TYPES 2
#define INNER_TYPE_NAME "s1"
#define OUTER_TYPE_NAME "d"

      /* This struct will be embeddeded in another. */
      struct s1
      {
            float x;
            double y;
      };
      struct s2
      {
            struct s1 s1;
      };
      struct s2 data_out[DIM_CMP_LEN], data_in[DIM_CMP_LEN];
      hid_t fileid, grpid, typeid_inner, typeid_outer, spaceid, attid;
      hid_t att_typeid, att_native_typeid;
      hsize_t dims[1];
      hid_t fapl_id, fcpl_id;
      int i;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s2), DIM_CMP_LEN))) ERR;
      memcpy((void *)data_out, (void *)dummy, sizeof(struct s2) * DIM_CMP_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (i = 0; i < DIM_CMP_LEN; i++)
      {
         data_out[i].s1.x = 1.0;
         data_out[i].s1.y = -2.0;
      }

      /* Create file access and create property lists. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create the inner compound type. */
      if ((typeid_inner = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid_inner, "x", HOFFSET(struct s1, x), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid_inner, "y", HOFFSET(struct s1, y), H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, INNER_TYPE_NAME, typeid_inner) < 0) ERR;

      /* Create a compound type containing a compound type. */
      if ((typeid_outer = H5Tcreate(H5T_COMPOUND, sizeof(struct s2))) < 0) ERR;
      if (H5Tinsert(typeid_outer, INNER_TYPE_NAME, HOFFSET(struct s2, s1), typeid_inner) < 0) ERR;
      if (H5Tcommit(grpid, OUTER_TYPE_NAME, typeid_outer) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM_CMP_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create an attribute of this compound type. */
      if ((attid = H5Acreate2(grpid, ATT_CMP_NAME1, typeid_outer, spaceid, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Write some data to the attribute. */
      if (H5Awrite(attid, typeid_outer, data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(typeid_outer) < 0 ||
          H5Tclose(typeid_inner) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and get the type of the attribute. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_by_name(grpid, ".", ATT_CMP_NAME1, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((att_typeid = H5Aget_type(attid)) < 0) ERR;
      if ((att_native_typeid = H5Tget_native_type(att_typeid, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Check the data. */
      if (H5Aread(attid, att_native_typeid, data_in) < 0) ERR;
      for (i = 0; i < DIM_CMP_LEN; i++)
         if (data_out[i].s1.x != data_in[i].s1.x ||
             data_out[i].s1.y != data_in[i].s1.y) ERR;

      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(att_typeid) < 0 ||
          H5Tclose(att_native_typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
