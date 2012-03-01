/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables. 
   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <hdf5.h>
#include <H5DSpublic.h>

#define MAX_LEN 30
#define TMP_FILE_NAME "tst_files2_tmp.out"
#define FILE_NAME "tst_files2_1.nc"
#define MILLION 1000000

void *last_sbrk;

void
get_mem_used2(int *mem_used)
{
   char buf[30];
   FILE *pf;
   
   snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
   pf = fopen(buf, "r");
   if (pf) {
      unsigned size; /*       total program size */
      unsigned resident;/*   resident set size */
      unsigned share;/*      shared pages */
      unsigned text;/*       text (code) */
      unsigned lib;/*        library */
      unsigned data;/*       data/stack */
      /*unsigned dt;          dirty pages (unused in Linux 2.6)*/
      fscanf(pf, "%u %u %u %u %u %u", &size, &resident, &share, 
	     &text, &lib, &data);
      *mem_used = data;
   }
   else
      *mem_used = -1;
  fclose(pf);
}

int
main(int argc, char **argv)
{

   printf("\n*** Testing netcdf-4 file functions, some more.\n");
   last_sbrk = sbrk(0);
/*    printf("Test for memory consumption of simple HDF5 file read...\n"); */
/*    { */
/* #define NUM_TRIES 200000 */
/* #define CHUNK_CACHE_NELEMS_1 1009 */
/* #define CHUNK_CACHE_SIZE_1 1000000 */
/* #define CHUNK_CACHE_PREEMPTION_1 .75 */
/* #define MAX_OBJ 2       */
/* #define FILE_NAME2 "ref_tst_kirk.nc" */
/*       int mem_used, mem_used1, mem_used2; */
/*       hid_t fapl_id, fileid, grpid, datasetid; */
/*       int try; */
/*       int num_scales; */

/*       printf("\t\t\tbef_open\taft_open\taft_close\tused_open\tused_closed\n"); */
/*       for (try = 0; try < NUM_TRIES; try++) */
/*       { */
/* 	 char obj_name2[] = "Captain_Kirk"; */
/* 	 get_mem_used2(&mem_used); */

/* 	 /\* Reopen the file. *\/ */
/*  	 if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR; */
/* 	 if ((fileid = H5Fopen(FILE_NAME2, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR; */
/* 	 if ((grpid = H5Gopen(fileid, "/")) < 0) ERR; */
	 
/* 	 if ((datasetid = H5Dopen2(grpid, obj_name2, H5P_DEFAULT)) < 0) ERR; */
/* 	 num_scales = H5DSget_num_scales(datasetid, 0); */

/* 	 get_mem_used2(&mem_used1); */

/* 	 /\* Close everything. *\/ */
/* 	 if (H5Dclose(datasetid)) ERR_RET; */
/* 	 if (H5Pclose(fapl_id)) ERR_RET; */
/* 	 if (H5Gclose(grpid) < 0) ERR_RET; */
/* 	 if (H5Fclose(fileid) < 0) ERR_RET; */

/* 	 get_mem_used2(&mem_used2); */

/* 	 if (mem_used2 - mem_used) */
/* 	 { */
/* 	    printf("try %d - \t\t%d\t\t%d\t\t%d\t\t%d\t\t%d \n", try,  */
/* 		   mem_used, mem_used1, mem_used2, mem_used1 - mem_used,  */
/* 		   mem_used2 - mem_used); */
/* 	    /\*if (try > 1) */
/* 	      ERR_RET;*\/ */
/* 	 } */
/*       } */
/*    } */
/*    SUMMARIZE_ERR; */
/*    printf("Test for memory consumption of HDF5 file read...\n"); */
/*    { */
/* #define NUM_TRIES 2000 */
/* #define CHUNK_CACHE_NELEMS_1 1009 */
/* #define CHUNK_CACHE_SIZE_1 1000000 */
/* #define CHUNK_CACHE_PREEMPTION_1 .75 */
/* #define MAX_OBJ 2       */
/* #define FILE_NAME2 "ref_tst_kirk.nc" */
/*       hsize_t num_obj, i; */
/*       int mem_used, mem_used1, mem_used2; */
/*       hid_t fapl_id, fileid, grpid, datasetid[MAX_OBJ]; */
/*       hid_t access_pid, spaceid; */
/*       char obj_name[NC_MAX_NAME + 1]; */
/*       int try; */
/*       H5O_info_t obj_info; */
/*       H5_index_t idx_field = H5_INDEX_CRT_ORDER; */
/*       ssize_t size; */
/*       int ndims; */
/*       hsize_t dims[NC_MAX_DIMS], max_dims[NC_MAX_DIMS]; */
/*       int is_scale = 0; */

/*       get_mem_used2(&mem_used); */
/*       mem_used1 = mem_used; */
/*       mem_used2 = mem_used; */
/*       printf("start: memuse= %d\t%d\t%d \n",mem_used, mem_used1,   */
/* 	     mem_used2); */

/* /\*      if (H5Eset_auto(NULL, NULL) < 0) ERR;*\/ */

/*       printf("bef_open\taft_open\taft_close\tused_open\tused_closed\n"); */
/*       for (try = 0; try < NUM_TRIES; try++) */
/*       { */
/* 	 get_mem_used2(&mem_used); */

/* 	 /\* Reopen the file. *\/ */
/* 	 if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR; */
/* 	 if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI)) ERR; */
/* 	 if (H5Pset_cache(fapl_id, 0, CHUNK_CACHE_NELEMS_1, CHUNK_CACHE_SIZE_1, */
/* 			  CHUNK_CACHE_PREEMPTION_1) < 0) ERR; */
/* 	 if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST,  */
/* 				  H5F_LIBVER_LATEST) < 0) ERR; */
/* 	 if ((fileid = H5Fopen(FILE_NAME2, H5F_ACC_RDONLY, fapl_id)) < 0) ERR; */
/* 	 if ((grpid = H5Gopen(fileid, "/")) < 0) ERR; */
	 
/* 	 if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR; */
/* 	 if (num_obj > MAX_OBJ) ERR; */
/* 	 for (i = 0; i < num_obj; i++) */
/* 	 { */
/* 	    if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, */
/* 				   i, &obj_info, H5P_DEFAULT) < 0) ERR; */
/* 	    if ((size = H5Lget_name_by_idx(grpid, ".", idx_field, H5_ITER_INC, i, */
/* 					   NULL, 0, H5P_DEFAULT)) < 0) ERR; */
/* 	    if (H5Lget_name_by_idx(grpid, ".", idx_field, H5_ITER_INC, i, */
/* 				   obj_name, size+1, H5P_DEFAULT) < 0) ERR; */
/* 	    if ((datasetid[i] = H5Dopen2(grpid, obj_name, H5P_DEFAULT)) < 0) ERR; */
/* 	    if ((access_pid = H5Dget_access_plist(datasetid[i])) < 0) ERR; */
/* 	    if ((spaceid = H5Dget_space(datasetid[i])) < 0) ERR; */
/* 	    if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR; */
/* 	    if (H5Sget_simple_extent_dims(spaceid, dims, max_dims) < 0) ERR; */
/* 	    if ((is_scale = H5DSis_scale(datasetid[i])) < 0) ERR; */
/* 	    if (is_scale) */
/* 	    { */
/* 	       char dimscale_name_att[NC_MAX_NAME + 1]; */
/* 	       int natts, a; */
/* 	       hid_t attid = 0; */
/* 	       char att_name[NC_MAX_HDF5_NAME + 1]; */

/* 	       if ((natts = H5Aget_num_attrs(datasetid[i])) < 0) ERR; */
/* 	       for (a = 0; a < natts; a++) */
/* 	       { */
/* 		  if ((attid = H5Aopen_idx(datasetid[i], (unsigned int)a)) < 0) ERR; */
/* 		  if (H5Aget_name(attid, NC_MAX_HDF5_NAME, att_name) < 0) ERR; */
/* 		  if (H5Aclose(attid) < 0) ERR; */
/* 	       } */
/* 	       if (H5DSget_scale_name(datasetid[i], dimscale_name_att, NC_MAX_NAME) < 0) ERR; */
/* 	    } */
/* 	    else */
/* 	    { */
/* 	       int num_scales; */
/* 	       size_t chunk_cache_size, chunk_cache_nelems; */
/* 	       double rdcc_w0; */
/* 	       hid_t propid; */

/* 	       num_scales = H5DSget_num_scales(datasetid[i], 0); */
/* 	       if ((H5Pget_chunk_cache(access_pid, &chunk_cache_nelems, */
/* 				       &chunk_cache_size, &rdcc_w0)) < 0) ERR; */
/* 	       if ((propid = H5Dget_create_plist(datasetid[i])) < 0) ERR; */

/* 	       if (H5Pclose(propid)) ERR; */

/* 	    } */

/* 	    if (H5Pclose(access_pid)) ERR; */
/* 	    if (H5Sclose(spaceid)) ERR; */
/* 	 } */
	 
/* 	 get_mem_used2(&mem_used1); */

/* 	 /\* Close everything. *\/ */
/* 	 for (i = 0; i < num_obj; i++) */
/* 	    if (H5Dclose(datasetid[i])) ERR; */
/* 	 if (H5Pclose(fapl_id)) ERR; */
/* 	 if (H5Gclose(grpid) < 0) ERR; */
/* 	 if (H5Fclose(fileid) < 0) ERR; */

/* 	 get_mem_used2(&mem_used2); */

/* 	 if (mem_used2 - mem_used) */
/* 	 { */
/* 	    printf("try %d - %d\t\t%d\t\t%d\t\t%d\t\t%d \n", try,  */
/* 		   mem_used, mem_used1, mem_used2, mem_used1 - mem_used,  */
/* 		   mem_used2 - mem_used); */
/* 	    if (try > 1) */
/* 	       ERR_RET; */
/* 	 } */
/*       } */
/*    } */
/*    SUMMARIZE_ERR; */
   FINAL_RESULTS;
}
