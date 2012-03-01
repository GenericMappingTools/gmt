/* This is part of the netCDF package. Copyright 2008 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with default fill values for variables of each type.

   $Id$
*/

#include <nc_tests.h>
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

#define FILE_NAME "tst_fills.nc" 

int
main(int argc, char **argv) 
{			/* create tst_classic_fills.nc */
   printf("\n*** Testing fill values.\n");
   printf("*** testing very simple scalar string var...");
   {
#define STRING_VAR_NAME "Petition_of_Right"
#define NDIMS_STRING 1

      int  ncid, varid;
      int varid_in;
      const char *data_out[NDIMS_STRING] = {
	 "Humbly show unto our Sovereign Lord the King, the Lords "
	 "Spiritual and Temporal, and Commons in Parliament assembles, "
	 "that whereas it is declared and enacted by a statute made in "
	 "the time of the reign of King Edward I, commonly called "
	 "Stratutum de Tellagio non Concedendo, that no tallage or "
	 "aid shall be laid or levied by the king or his heirs in this "
	 "realm, without the good will and assent of the archbishops, "
	 "bishops, earls, barons, knights, burgesses, and other the "
	 "freemen of the commonalty of this realm; and by authority "
	 "of parliament holden in the five-and-twentieth year of the "
	 "reign of King Edward III, it is declared and enacted, that "
	 "from thenceforth no person should be compelled to make any "
	 "loans to the king against his will, because such loans were "
	 "against reason and the franchise of the land; and by other "
	 "laws of this realm it is provided, that none should be charged "
	 "by any charge or imposition called a benevolence, nor by such "
	 "like charge; by which statutes before mentioned, and other the "
	 "good laws and statutes of this realm, your subjects have inherited "
	 "this freedom, that they should not be compelled to contribute "
	 "to any tax, tallage, aid, or other like charge not set by "
	 "common consent, in parliament."
      };
      char *data_in[NDIMS_STRING];
      size_t index1[NDIMS_STRING] = {0};

      /* Create file with a 1D string var. Set its fill value to the
       * empty string. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_var(ncid, STRING_VAR_NAME, NC_STRING, 0, NULL, &varid)) ERR;

      /* Check it out. */
      if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR;
      if (varid_in != varid) ERR;

      /* Write string. */
      if (nc_put_var1_string(ncid, varid_in, index1, data_out)) ERR;

      /* Get the string, check it, and free it. */
      if (nc_get_var_string(ncid, varid_in, data_in)) ERR;
      if (strcmp(data_in[0], data_out[0])) ERR;
      if (nc_free_string(1, data_in)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;

      /* Now re-open file, read data, and check values again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Get the string, check it, and free it. */
      strcpy(data_in, "");
      if (nc_get_var_string(ncid, varid_in, data_in)) ERR;
      if (strcmp(data_in[0], data_out[0])) ERR;
      if (nc_free_string(1, data_in)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing fill values of one var...");
   {
#define V1_NAME "v1"
#define MAX_VALS 10
      int  ncid, varid, rec_id, dims[2];
      static int rec[1] = {1};
      size_t start[2] = {0, 0};
      size_t count[2] = {1, MAX_VALS};
      char vals[MAX_VALS];
      int i;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions and two vars, a 1D coordinate var for
       * unlimited dimension, and a 2D var which uses the unlimited
       * dimension. */
      if (nc_def_dim(ncid, "rec", NC_UNLIMITED, &dims[0])) ERR;
      if (nc_def_dim(ncid, "len", MAX_VALS, &dims[1])) ERR;
      if (nc_def_var(ncid, "rec", NC_INT, 1, dims, &rec_id)) ERR;
      if (nc_def_var(ncid, V1_NAME, NC_CHAR, 2, dims, &varid)) ERR;

      /* Extend record dimension by 1. */
      if (nc_put_vara_int(ncid, rec_id, start, count, rec)) ERR;

      /* Read the other variable; it must have only fill values. */
      if (nc_get_vara_text(ncid, 1, start, count, vals)) ERR;
      for (i = 0; i < MAX_VALS; i++)
	 if(vals[i] != NC_FILL_CHAR) ERR;

      if (nc_close(ncid)) ERR;

      /* Now re-open file, read data, and check values again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Read the other variable; it must have only fill values. */
      if (nc_get_vara_text(ncid, 1, start, count, vals)) ERR;
      for (i = 0; i < MAX_VALS; i++)
	 if(vals[i] != NC_FILL_CHAR) ERR;

      if(nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing fill values of lots of vars...");
   {
      int  ncid;			/* netCDF id */

#define NVALS 10		/* values per fixed-size variable or record */
#define NFIXVARS 6		/* number of fixed-size vars, one of each type */
#define NRECVARS 6		/* number of record vars, one of each type */
#define RANK_REC 1
#define RANK_FIXVARS 1
#define RANK_RECVARS 2

      /* dimension ids */
      int rec_dim;
      int len_dim;

      /* dimension lengths */
      size_t rec_len = NC_UNLIMITED;
      size_t len_len = NVALS;

      /* variable ids */
      int rec_id;
      int fixvar_ids[NFIXVARS];
      int recvar_ids[NRECVARS];
      int rec_dims[RANK_REC];
      int fixvar_dims[RANK_FIXVARS];
      int recvar_dims[RANK_RECVARS];
      int fixvar, recvar, i;

      char *fnames[] = {"c", "b", "s", "i", "f", "d"};
      char *rnames[] = {"cr", "br", "sr", "ir", "fr", "dr"};
      nc_type types[] = {NC_CHAR, NC_BYTE, NC_SHORT, NC_INT, NC_FLOAT, NC_DOUBLE};

      /*if (nc_set_default_format(format + 1, NULL)) ERR;*/
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* define dimensions */
      if (nc_def_dim(ncid, "rec", rec_len, &rec_dim)) ERR;
      if (nc_def_dim(ncid, "len", len_len, &len_dim)) ERR;

      rec_dims[0] = rec_dim;
      if (nc_def_var(ncid, "rec", NC_INT, RANK_REC, rec_dims, &rec_id)) ERR;

      /* define fixed and record variables of all 6 primitive types */
      fixvar_dims[0] = len_dim;
      for (fixvar = 0; fixvar < NFIXVARS; fixvar++)
	 if (nc_def_var(ncid, fnames[fixvar], types[fixvar], RANK_FIXVARS,
			fixvar_dims, &fixvar_ids[fixvar])) ERR;

      recvar_dims[0] = rec_dim;
      recvar_dims[1] = len_dim;
      for (recvar = 0; recvar < NRECVARS; recvar++)
	 if (nc_def_var(ncid, rnames[recvar], types[recvar], RANK_RECVARS,
			recvar_dims, &recvar_ids[recvar])) ERR;

      /* leave define mode */
      if (nc_enddef(ncid)) ERR;

      {			/* store rec */
	 static size_t rec_start[RANK_REC];
	 static size_t rec_count[RANK_REC];
	 static int rec[] = {1};
	 rec_len = 1;			/* number of records of rec data */
	 rec_start[0] = 0;
	 rec_count[0] = rec_len;
	 if (nc_put_vara_int(ncid, rec_id, rec_start, rec_count, rec)) ERR;
      }
      if (nc_close(ncid)) ERR;

      /* Now re-open file, read data, and check values */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check that fixed-size variables are full of fill values */
      for (fixvar = 0; fixvar < NFIXVARS; fixvar++) {
	 int varid;
	 nc_type type;

	 if (nc_inq_varid(ncid, fnames[fixvar], &varid)) ERR;
	 if (nc_inq_vartype(ncid, varid, &type)) ERR;
	 switch(type) {
	    case NC_CHAR:
	    {
	       char vals[NVALS];
	       if (nc_get_var_text(ncid, varid, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_CHAR) ERR;
	    }
	    break;
	    case NC_BYTE:
	    {
	       signed char vals[NVALS];
	       if (nc_get_var_schar(ncid, varid, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_BYTE) ERR;
	    }
	    break;
	    case NC_SHORT:
	    {
	       short vals[NVALS];
	       if (nc_get_var_short(ncid, varid, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_SHORT) ERR;
	    }
	    break;
	    case NC_INT:
	    {
	       int vals[NVALS];
	       if (nc_get_var_int(ncid, varid, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_INT) ERR;
	    }
	    break;
	    case NC_FLOAT:
	    {
	       float vals[NVALS];
	       if (nc_get_var_float(ncid, varid, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_FLOAT) ERR;
	    }
	    break;
	    case NC_DOUBLE:
	    {
	       double vals[NVALS];
	       if (nc_get_var_double(ncid, varid, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if (vals[i] != NC_FILL_DOUBLE) ERR;
	    }
	    break;
	    default:
	       ERR;
	 }
      }

      /* Read record, check record variables have only fill values */
      for (recvar = 0; recvar < NRECVARS; recvar++) {
	 int varid;
	 nc_type type;
	 size_t start[] = {0, 0};
	 size_t count[] = {1, NVALS};

	 if (nc_inq_varid(ncid, rnames[recvar], &varid)) ERR;
	 if (nc_inq_vartype(ncid, varid, &type)) ERR;
	 switch(type) {
	    case NC_CHAR:
	    {
	       char vals[NVALS];
	       if (nc_get_vara_text(ncid, varid, start, count, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_CHAR) ERR;
	    }
	    break;
	    case NC_BYTE:
	    {
	       signed char vals[NVALS];
	       if (nc_get_vara_schar(ncid, varid, start, count, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_BYTE) ERR;
	    }
	    break;
	    case NC_SHORT:
	    {
	       short vals[NVALS];
	       if (nc_get_vara_short(ncid, varid, start, count, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_SHORT) ERR;
	    }
	    break;
	    case NC_INT:
	    {
	       int vals[NVALS];
	       if (nc_get_vara_int(ncid, varid, start, count, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_INT) ERR;
	    }
	    break;
	    case NC_FLOAT:
	    {
	       float vals[NVALS];
	       if (nc_get_vara_float(ncid, varid, start, count, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_FLOAT) ERR;
	    }
	    break;
	    case NC_DOUBLE:
	    {
	       double vals[NVALS];
	       if (nc_get_vara_double(ncid, varid, start, count, vals)) ERR;
	       for (i = 0; i < NVALS; i++)
		  if(vals[i] != NC_FILL_DOUBLE) ERR;
	    }
	    break;
	    default:
	       ERR;
	 }
      }

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
