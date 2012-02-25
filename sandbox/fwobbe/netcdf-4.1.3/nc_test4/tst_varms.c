/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test netcdf-4 mapped var operations. 

   Remember, in C, last dimension of an array varies fastest in memory. 
   int data[2][3] is order in memory (0,0), (0,1), (0,2), (1,0), (1,1), (1,2).
   applying map of (1,2) will reorder them in the following way:
   (0,0), (1,0), (0,1), (1,1), (0,2), (1,2)

   I wrote this in Jenny Lake Campground near Jackson, WY. This is a
   bit of a change from my usual workplace. Given total mobility the
   ability to work anywhere, I usually choose to work at home. Not
   sure what this says about me or the modern world, but camping makes
   a nice change of pace! Ed 8/19/5

   $Id$
*/

#include <nc_tests.h>

#define FILE_NAME "tst_varms.nc"
#define DIM1_NAME "i"
#define DIM1_LEN 2
#define DIM2_NAME "j"
#define DIM2_LEN 3
#define VAR_NAME "Little_Jenny_Campground"

int
main(int argc, char **argv)
{
   int ncid, varid, dimids[2];
   int data[DIM1_LEN][DIM2_LEN], data_in[DIM1_LEN][DIM2_LEN];
   int ndims_in, dimids_in[10], natts_in;
   size_t start[2], count[2];
   ptrdiff_t stride[2], map[2];
   char name_in[NC_MAX_NAME+1];
   nc_type xtype_in;
   int nvars, natts, ndims, unlimdimid;
   int i, j, k = 0;

   printf("\n*** Testing netcdf-4 mapped variable functions.\n");
   {
      int data_2d[2][2], data_2d_in[2][2];

      printf("*** testing mapping with 2x2 variable...");

      /* Create phoney data. */
      for (i = 0; i < 2; i++)
	 for (j = 0; j < 2; j++)
	    data_2d[i][j] = k++;
      
      /* Create a file with one 2D variable of type int and write our
       * data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, "i", 2, &dimids[0])) ERR;
      if (nc_def_dim(ncid, "j", 2, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_int(ncid, varid, (int *)data_2d)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Open the file and check. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_get_var_int(ncid, varid, (int *)data_2d_in)) ERR;   
      for (i = 0; i < 2; i++)
	 for (j = 0; j < 2; j++)
	    if (data_2d_in[i][j] != data_2d[i][j]) ERR;

      /* Get a transpose of the array. I have no idea how to figure
       * out the map array, but my first guess worked. */
      start[0] = start[1] = 0;
      count[0] = count[1] = 2;
      stride[0] = stride[1] = 1;
      map[0] = 1;
      map[1] = 2;
      if (nc_get_varm_int(ncid, varid, start, count, stride, map, 
			  (int *)data_2d_in)) ERR;   
      for (i = 0; i < 2; i++)
	 for (j = 0; j < 2; j++)
	    if (data_2d_in[j][i] != data_2d[i][j]) ERR;

      /* Now read the untransposed array. I still haven't much idea
       * what these numbers mean. */
      map[0] = 2;
      map[1] = 1;
      if (nc_get_varm_int(ncid, varid, start, count, stride, map, 
			  (int *)data_2d_in)) ERR;   
      for (i = 0; i < 2; i++)
	 for (j = 0; j < 2; j++)
	    if (data_2d_in[i][j] != data_2d[i][j]) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing mapping with 2x3 variable...");

   {
      int data_in_t[DIM2_LEN][DIM1_LEN];

      /* Create some phoney data. */
      k = 0;
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    data[i][j] = k++;
      
      /* Create a file with one variable of type int. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_int(ncid, varid, (int *)data)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Open the file and check. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, 
		     &natts_in)) ERR;
      if (strcmp(name_in, VAR_NAME) || xtype_in != NC_INT || 
	  ndims_in != 2 || natts_in != 0 || dimids_in[0] != dimids[0] ||
	  dimids_in[1] != dimids[1]) ERR;
      if (nc_get_var_int(ncid, varid, (int *)data_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (data_in[i][j] != data[i][j]) ERR;

      /* Get a transpose of the array. */
      start[0] = start[1] = 0;
      count[0] = DIM1_LEN;
      count[1] = DIM2_LEN;
      stride[0] = stride[1] = 1;
      map[0] = 1;
      map[1] = 2;
      if (nc_get_varm_int(ncid, varid, start, count, stride, map, 
			  (int *)data_in_t)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (data_in_t[j][i] != data[i][j]) ERR;

      if (nc_close(ncid)) ERR;

   }

   SUMMARIZE_ERR;
   printf("*** testing simple example from C Users' Guide...");

   {
#define D0 4
#define D1 3
#define D2 2     

      /* netCDF dimension       inter-element distance */
      /* ----------------       ---------------------- */
      /* most rapidly varying       1                  */
      /* intermediate               2 (=imap[2]*2)     */
      /* most slowly varying        6 (=imap[1]*3)     */
      ptrdiff_t imap[3] = {6, 2, 1}, stride[3] = {1, 1, 1};
      size_t start[3] = {0, 0, 0}, count[3] = {D0, D1, D2};
      int ncid, varid, dimids[3];
      float data[D0][D1][D2], data_in[D0][D1][D2];
   
      for (i = 0; i < D0; i++)
	 for (j = 0; j < D1; j++)
	    for (k = 0; k < D2; k++)
	       data[i][j][k] = i + j + k;

      /* Create a file with one variable of type float. */
      if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, "D0", D0, &dimids[0])) ERR;
      if (nc_def_dim(ncid, "D1", D1, &dimids[1])) ERR;
      if (nc_def_dim(ncid, "D2", D2, &dimids[2])) ERR;
      if (nc_def_var(ncid, "Jackson_Hole", NC_FLOAT, 3, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_float(ncid, varid, (float *)data)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 3 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_get_var_float(ncid, 0, (float *)data_in)) ERR;   
      for (i = 0; i < D0; i++)
	 for (j = 0; j < D1; j++)
	    for (k = 0; k < D2; k++)
	       if (data_in[i][j][k] != data[i][j][k]) ERR;

      /* Using the imap above I should get identical results
       * (according to the manual). */
      if (nc_get_varm_float(ncid, 0, start, count, stride, imap, 
			    (float *)data_in)) ERR;   
      for (i = 0; i < D0; i++)
	 for (j = 0; j < D1; j++)
	    for (k = 0; k < D2; k++)
	       if (data_in[i][j][k] != data[i][j][k]) ERR;

      /* Now let's mess things around a bit. */
/*       imap[0] = 0; */
/*       imap[1] = 1; */
/*       imap[2] = 2; */
/*       if (nc_get_varm_float(ncid, 0, start, count, stride, imap,  */
/* 			    (float *)data_in)) ERR;    */
/*       for (i = 0; i < D0; i++) */
/* 	 for (j = 0; j < D1; j++) */
/* 	    for (k = 0; k < D2; k++) */
/* 	       if (data_in[i][j][k] != data[i][j][k]) ERR; */

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;

   printf("*** testing transposed rh example from C Users' Guide...");
   {
      /*nc_set_log_level(2);*/

      ptrdiff_t imap[2] = {1, 6}, stride[2] = {1, 1};
      size_t start[2] = {0, 0}, count[2] = {6, 4};
      int ncid, varid, dimids[3];
      float data[4][6], data_in[6][4];
      int k=0;
   
      /* Phoney data. */
      for (i = 0; i < 4; i++)
	 for (j = 0; j < 6; j++)
	 {
	    data[i][j] = k;
	    data_in[j][i] = k;
	    k++;
	 }

      /* Create a file with one variable of type float, writing a transposed array. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, "lat", 6, &dimids[0])) ERR;
      if (nc_def_dim(ncid, "lon", 4, &dimids[1])) ERR;
      if (nc_def_var(ncid, "rh", NC_FLOAT, 2, dimids, &varid)) ERR;
      dimids[0] = 1;
      dimids[1] = 0;
      if (nc_def_var(ncid, "rh2", NC_FLOAT, 2, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_varm_float(ncid, 0, start, count, stride, imap, 
			    (float *)data)) ERR;
      count[0] = 4;
      count[1] = 6;
      if (nc_put_vara_float(ncid, 1, start, count, 
			    (float *)data_in)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_get_var_float(ncid, 0, (float *)data_in)) ERR;   
/*       for (i = 0; i < 4; i++) */
/* 	 for (j = 0; j < 6; j++) */
/* 	    if (data_in[i][j] != data[j][i]) ERR; */

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


