/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test libcf variable stuff.

   $Id$
*/

#include <config.h>
#include <libcf.h>
#include <netcdf.h>
#include <nc_tests.h>

#define FILE_NAME "tst_vars.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing libcf variable stuff.\n");

#define INSTITUTION "Unidata Programmaing Drone Sector 7G"
#define SOURCE "We buy our data at DataMart."
#define COMMENT "Happy trails to you!"
#define REFERENCES "Why bother?"
#define MAX_LEN 100
#define NDIMS 1
#define VAR_NAME "Programmer_Productivity"
#define DIM_NAME "Lines_of_code_per_cube_per_second"

   printf("*** testing variable notes...");
   {
      int ncid, varid, dimids[NDIMS];
      size_t ilen, slen, clen, rlen;
      char inst_in[MAX_LEN], source_in[MAX_LEN];
      char comment_in[MAX_LEN], ref_in[MAX_LEN];

      /* CReate a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, NC_UNLIMITED, dimids)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIMS, dimids, &varid)) ERR;
      if (nccf_def_notes(ncid, varid, INSTITUTION, SOURCE, 
			 COMMENT, REFERENCES)) ERR;

      /* Check the file. */
      if (nccf_inq_notes(ncid, varid, &ilen, inst_in, &slen, source_in,
			 &clen, comment_in, &rlen, ref_in)) ERR;
      if (ilen != strlen(INSTITUTION) + 1 || slen != strlen(SOURCE) + 1 ||
	  clen != strlen(COMMENT) + 1 || rlen != strlen(REFERENCES) + 1) ERR;
      if (strncmp(inst_in, INSTITUTION, ilen) || 
	  strncmp(source_in, SOURCE, slen) || 
	  strncmp(comment_in, COMMENT, clen) || 
	  strncmp(ref_in, REFERENCES, rlen)) ERR;

      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, 0, &ncid)) ERR;

      /* Check the file. */
      if (nccf_inq_notes(ncid, varid, &ilen, inst_in, &slen, source_in,
			 &clen, comment_in, &rlen, ref_in)) ERR;
      if (ilen != strlen(INSTITUTION) + 1 || slen != strlen(SOURCE) + 1 ||
	  clen != strlen(COMMENT) + 1 || rlen != strlen(REFERENCES) + 1) ERR;
      if (strncmp(inst_in, INSTITUTION, ilen) || 
	  strncmp(source_in, SOURCE, slen) || 
	  strncmp(comment_in, COMMENT, clen) || 
	  strncmp(ref_in, REFERENCES, rlen)) ERR;

      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;

#define VALID_MIN -10.0
#define VALID_MAX 10.0
#define FILL_VALUE 99.9
#define VAR2_NAME "manager_salary"

   printf("*** testing missing values...");
   {
      int ncid, varid, varid2, dimids[NDIMS];
      int valid_min = VALID_MIN, valid_max = VALID_MAX;
      int fill_value = FILL_VALUE;
      int fill_value_in, valid_min_in, valid_max_in;

      /* CReate a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, NC_UNLIMITED, dimids)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIMS, dimids, &varid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_INT, NDIMS, dimids, &varid2)) ERR;
      if (nccf_def_var_missing(ncid, varid, &fill_value, &valid_min,
			       &valid_max)) ERR;

      /* Check the file. */
      if (nccf_inq_var_missing(ncid, varid, &fill_value_in, 
			       &valid_min_in, &valid_max_in)) ERR;
      if (fill_value_in != fill_value || valid_min_in != valid_min ||
	  valid_max_in != valid_max) ERR;
      if (nccf_inq_var_missing(ncid, varid2, &fill_value_in, 
			       &valid_min_in, &valid_max_in)) ERR;
      if (fill_value_in != NC_FILL_INT || valid_min_in != CF_INT_MIN ||
	  valid_max_in != CF_INT_MAX) ERR;

      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, 0, &ncid)) ERR;

      /* Check the file. */
      if (nccf_inq_var_missing(ncid, varid, &fill_value_in, 
			       &valid_min_in, &valid_max_in)) ERR;
      if (fill_value_in != fill_value || valid_min_in != valid_min ||
	  valid_max_in != valid_max) ERR;

      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

