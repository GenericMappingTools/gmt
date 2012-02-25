/* This is part of the netCDF package. Copyright 2008 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 dimensions some more. 

   $Id$
*/

#include <config.h>
#include <nc_tests.h>

#define FILE_NAME "tst_dims2.nc"

#define NDIMS1 1
#define NDIMS2 2      

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 dimensions some more.\n");
   printf("*** Checking non-coordinate variable with same name as dimension...");
   {
#define CRAZY "crazy"
#define NUTS "nuts"
#define NUM_CRAZY 3
#define NUM_NUTS 5
      int nuts_dimid, crazy_dimid, dimid_in;
      int varid, ncid;
      nc_type xtype_in, xtype = NC_CHAR;
      int ndims_in, nvars_in, natts_in, unlimdimid_in;
      char name_in[NC_MAX_NAME + 1];

      /* Create a file with 2 dims and one var. The var is named the
       * same as one of the dimensions, but uses the other dimension,
       * and thus is not a netCDF coordinate variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, CRAZY, NUM_CRAZY, &crazy_dimid)) ERR;
      if (nc_def_dim(ncid, NUTS, NUM_NUTS, &nuts_dimid)) ERR;
      if (nc_def_var(ncid, CRAZY, xtype, NDIMS1, &nuts_dimid, &varid)) ERR;

      /* Check out the file. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, &dimid_in, &natts_in)) ERR;
      if (strcmp(name_in, CRAZY) || xtype_in != xtype || ndims_in != NDIMS1 ||
          natts_in != 0 || dimid_in != nuts_dimid) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and check the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, &dimid_in, &natts_in)) ERR;
      if (strcmp(name_in, CRAZY) || xtype_in != xtype || ndims_in != NDIMS1 ||
          natts_in != 0 || dimid_in != nuts_dimid) ERR;

      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   printf("*** Checking 2D coordinate variable with user-provided code...");
   {
      /* This test is from user Dipl.-Ing. Christian Schlamkow,
       * University of Rostock, Coastal Engineering Group. Thanks
       * Christian! Next time I'm in Rostock I'll come by and buy you
       * a beer. ;-) */
#define TL 15
#define SHUFFLE 0
#define DEFLATE 1
#define DEFLATE_LEVEL 5
      int ncid;
      int time_dim;
      int tl_dim;
      int time_dimids[NDIMS2];
      int time_id;
      size_t time_index[NDIMS2];
      size_t time_count[NDIMS2];
      const char ttext[TL + 1]="20051224.150000";
      char ttext_in[TL + 1];
      int ndims_in, nvars_in, natts_in, unlimdimid_in, dimids_in[NDIMS2];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int i;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /*create dimensions*/
      if (nc_def_dim(ncid, "time", NC_UNLIMITED, &time_dim)) ERR;
      if (nc_def_dim(ncid, "tl", TL, &tl_dim)) ERR;

      /*create variables*/
      time_dimids[0]=time_dim;
      time_dimids[1]=tl_dim;
      if (nc_def_var(ncid, "time", NC_CHAR, NDIMS2, time_dimids, &time_id)) ERR;

      /*close dataset*/
      if (nc_close(ncid)) ERR;

      /*reopen dataset*/
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

      /*get dimensions and variable ids*/
      if (nc_inq_varid(ncid, "time", &time_id)) ERR;

      /*fill in data*/
      time_index[0] = 0;
      time_index[1] = 0;
      time_count[0] = 1;
      time_count[1] = TL; /* Note we are not writing NULL char. */
      if (nc_put_vara_text(ncid, time_id, time_index, time_count, ttext)) ERR;

      /*close dataset*/
      if (nc_close(ncid)) ERR;

      /* Reopen and check everything. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != NDIMS2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, "time") || xtype_in != NC_CHAR || ndims_in != NDIMS2 ||
          natts_in != 0) ERR;
      for (i = 0; i < NDIMS2; i++)
	 if (time_dimids[i] != dimids_in[i]) ERR;
      if (nc_get_vara_text(ncid, time_id, time_index, time_count, ttext_in)) ERR;
      ttext_in[TL] = 0; /* Add a NULL so strcmp will work. */
      if (strcmp(ttext, ttext_in)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking 2D coordinate variable with user-provided code some more...");
   {
      /* This test is from user Dipl.-Ing. Christian Schlamkow,
       * University of Rostock, Coastal Engineering Group. Thanks
       * Christian! Next time I'm in Rostock I'll come by and buy you
       * a beer. ;-) */
#define TL 15
#define SHUFFLE 0
#define DEFLATE 1
#define DEFLATE_LEVEL 5
#define NDIMS2 2
#define NUM_TIMES 16
      int ncid;
      int time_dim;
      int tl_dim;
      int time_dimids[NDIMS2];
      int time_id;
      size_t time_index[NDIMS2];
      size_t time_count[NDIMS2];
      const char ttext[TL + 1]="20051224.150000";
      char ttext_in[TL + 1];
      int ndims_in, nvars_in, natts_in, unlimdimid_in, dimids_in[NDIMS2];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int i;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /*create dimensions*/
      if (nc_def_dim(ncid, "time", NC_UNLIMITED, &time_dim)) ERR;
      if (nc_def_dim(ncid, "tl", TL, &tl_dim)) ERR;

      /*create variables*/
      time_dimids[0]=time_dim;
      time_dimids[1]=tl_dim;
      if (nc_def_var(ncid, "time", NC_CHAR, NDIMS2, time_dimids, &time_id)) ERR;

      /*close dataset*/
      if (nc_close(ncid)) ERR;

      /*reopen dataset*/
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

      /*get dimensions and variable ids*/
      if (nc_inq_varid(ncid, "time", &time_id)) ERR;

      time_index[1] = 0;
      time_count[0] = 1;
      time_count[1] = TL;
      for (time_index[0] = 0; time_index[0] < NUM_TIMES; time_index[0]++)
	 if(nc_put_vara_text(ncid, time_id, time_index, time_count, ttext)) ERR;

      /*close dataset*/
      if (nc_close(ncid)) ERR;

      /* Reopen and check everything. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != NDIMS2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, "time") || xtype_in != NC_CHAR || ndims_in != NDIMS2 ||
          natts_in != 0) ERR;
      for (i = 0; i < NDIMS2; i++)
	 if (time_dimids[i] != dimids_in[i]) ERR;
      time_index[0] = 0;
      if (nc_get_vara_text(ncid, time_id, time_index, time_count, ttext_in)) ERR;
      ttext_in[TL] = 0; /* Add a NULL so strcmp will work. */
      if (strcmp(ttext, ttext_in)) ERR;
      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking 2D coordinate variable...");
   {
#define TIME "time"
#define LEN "len"
#define NDIMS 2
      int dimids[NDIMS], dimids_in[NDIMS];
      int varid, ncid;
      nc_type xtype_in, xtype = NC_CHAR;
      int ndims_in, nvars_in, natts_in, unlimdimid_in;
      char name_in[NC_MAX_NAME + 1];
      int i;

      /* Create a file with a 2D time coordinate var, with one of the
       * dimensions being the length of the time string. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, TIME, NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_dim(ncid, LEN, 20, &dimids[1])) ERR;
      if (nc_def_var(ncid, TIME, xtype, 2, dimids, &varid)) ERR;

      /* Check out the file. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, TIME) || xtype_in != xtype || ndims_in != NDIMS ||
          natts_in != 0) ERR;
      for (i = 0; i < NDIMS; i++)
	 if (dimids[i] != dimids_in[i]) ERR;

      if (nc_close(ncid)) ERR;

      /* Recheck the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, TIME) || xtype_in != xtype || ndims_in != NDIMS ||
          natts_in != 0) ERR;
      for (i = 0; i < NDIMS; i++)
	 if (dimids[i] != dimids_in[i]) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking multiple unlimited dimensions...");
   {
#define MAX(x,y) ((x)>(y)?(x):(y))
#define NDIMS 2
#define MAX_VALUES 3

      int dimids[NDIMS], varid, ncid;
      double value[MAX_VALUES];
      size_t time_len, beam_len, start[NDIMS] = {0, 0}, count[NDIMS] = {1, MAX_VALUES};
      int i;

      /* Initialize some phony data. */
      for (i = 0; i < MAX_VALUES; i++)
	 value[i] = MAX_VALUES - i;

      /* Create a file with 2 unlimited dims, and one var that uses
       * both of them. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, "time", NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_dim(ncid, "beam", NC_UNLIMITED, &dimids[1])) ERR;
      if (nc_def_var(ncid, "depth", NC_DOUBLE, NDIMS, dimids, &varid)) ERR;

      /* Check the lengths of these dimensions. */
      if (nc_inq_dimlen(ncid, dimids[0], &time_len)) ERR;
      if (time_len) ERR;
      if (nc_inq_dimlen(ncid, dimids[1], &beam_len)) ERR;
      if (beam_len) ERR;

      /* Write some data, check dim lengths. */
      if (nc_put_vara_double(ncid, varid, start, count, value)) ERR;
      if (nc_inq_dimlen(ncid, dimids[0], &time_len)) ERR;
      if (time_len != 1) ERR;
      if (nc_inq_dimlen(ncid, dimids[1], &beam_len)) ERR;
      if (beam_len != MAX_VALUES) ERR;

      /* Add some more data, check dim lengths again. */
      start[0] = 1;
      if (nc_put_vara_double(ncid, varid, start, count, value)) ERR;
      if (nc_inq_dimlen(ncid, dimids[0], &time_len)) ERR;
      if (time_len != 2) ERR;
      if (nc_inq_dimlen(ncid, dimids[1], &beam_len)) ERR;
      if (beam_len != MAX_VALUES) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking multiple unlimited dimensions with more complex 2D test...");
   {
#define MAX(x,y) ((x)>(y)?(x):(y))
      int dimids[2];
      int varid, ncid, timeDimID, beamDimID;
      int i, j;
      int value[2000];
      size_t time_recs, beam_recs;	/* count of records in each dimension */
      size_t time_len, beam_len;	/* actual dimension lengths in each dimension */
      size_t start[] = {0, 0};
      size_t count[] = {1, 1};

      for (i = 0; i < 2000; i++)
	 value[i] = 2000 - i;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define 2 unlimited dimensions */
      if (nc_def_dim(ncid, "time", NC_UNLIMITED, &timeDimID)) ERR;
      if (nc_def_dim(ncid, "beam", NC_UNLIMITED, &beamDimID)) ERR;

      dimids[0] = timeDimID;
      dimids[1] = beamDimID;

      if (nc_def_var(ncid, "depth", NC_DOUBLE, 2, dimids, &varid)) ERR;

      if (nc_enddef(ncid)) ERR;

      time_recs = 0;
      beam_recs = 0;
      for (j = 0; j < 100; j++)
      {
	 if (j > 500)
	    count[1] = j;
	 else
	    count[1] = 1000-j;
       
	 if (nc_put_vara_int(ncid, varid, start, count, value)) ERR;
	 time_recs = MAX(time_recs, start[0] + count[0]);
	 beam_recs = MAX(beam_recs, start[1] + count[1]);
	 if (nc_inq_dimlen(ncid, timeDimID, &time_len)) ERR;
	 if (time_len != time_recs) ERR;
	 if (nc_inq_dimlen(ncid, beamDimID, &beam_len)) ERR;
	 if (beam_len != beam_recs) ERR;
	 start[0]++;
      }

      if (nc_close(ncid)) ERR;

      /* Check the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   printf("*** Checking 2D coordinate variable some more...");
   {
#define TIME "time"
#define LEN "len"
#define NDIMS 2
#define NUM_RECS 15
#define TL 15
      int dimids[NDIMS], dimids_in[NDIMS];
      int varid, ncid;
      nc_type xtype_in, xtype = NC_CHAR;
      int ndims_in, nvars_in, natts_in, unlimdimid_in;
      char name_in[NC_MAX_NAME + 1];
      size_t index[NDIMS], count[NDIMS], len_in;
      const char ttext[TL + 1]="20051224.150000";
      int i;

      /* Create a file with a 2D time coordinate var, with one of the
       * dimensions being the length of the time string. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, TIME, NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_dim(ncid, LEN, TL, &dimids[1])) ERR;
      if (nc_def_var(ncid, TIME, xtype, 2, dimids, &varid)) ERR;

      /* Check out the file. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, TIME) || xtype_in != xtype || ndims_in != NDIMS ||
          natts_in != 0) ERR;
      for (i = 0; i < NDIMS; i++)
	 if (dimids[i] != dimids_in[i]) ERR;

      /* Write some time values. */
      index[0] = 0;
      index[1] = 0;
      count[0] = 1;
      count[1] = TL; /* Note we are not writing NULL char. */
      if (nc_put_vara_text(ncid, varid, index, count, ttext)) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and check the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, TIME) || xtype_in != xtype || ndims_in != NDIMS ||
          natts_in != 0) ERR;
      for (i = 0; i < NDIMS; i++)
	 if (dimids[i] != dimids_in[i]) ERR;
      if (nc_inq_dim(ncid, dimids[0], name_in, &len_in)) ERR;
      if (strcmp(name_in, TIME) || len_in != 1) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen and write some more time values. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      
      /* Write some more time values. */
      index[1] = 0;
      count[0] = 1;
      count[1] = TL; /* Note we are not writing NULL char. */
      for (index[0] = 1; index[0] < NUM_RECS; index[0]++)
	 if (nc_put_vara_text(ncid, varid, index, count, ttext)) ERR;

      /* Check things again. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, TIME) || xtype_in != xtype || ndims_in != NDIMS ||
          natts_in != 0) ERR;
      for (i = 0; i < NDIMS; i++)
	 if (dimids[i] != dimids_in[i]) ERR;
      if (nc_inq_dim(ncid, dimids[0], name_in, &len_in)) ERR;
      if (strcmp(name_in, TIME) || len_in != NUM_RECS) ERR;
      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
