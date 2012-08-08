#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
	   (void) fprintf(stderr, "line %d of %s: %s\n", line, file, nc_strerror(stat));
        exit(1);
    }
}

int
main() {			/* create rec2Gp.nc */

   int  stat;			/* return status */
   int  ncid;			/* netCDF id */
   int rec, i, j, k;
   signed char x[] = {42, 21};

   /* dimension ids */
   int rec_dim;
   int i_dim;
   int j_dim;
   int k_dim;
   int n_dim;

#define NUMRECS 1
#define I_LEN 4104
#define J_LEN 1023
#define K_LEN 1023
#define N_LEN 2

   /* dimension lengths */
   size_t rec_len = NC_UNLIMITED;
   size_t i_len = I_LEN;
   size_t j_len = J_LEN;
   size_t k_len = K_LEN;
   size_t n_len = N_LEN;

   /* variable ids */
   int var1_id;
   int x_id;

   /* rank (number of dimensions) for each variable */
#  define RANK_var1 4
#  define RANK_x 2

   /* variable shapes */
   int var1_dims[RANK_var1];
   int x_dims[RANK_x];

   /* enter define mode */
   stat = nc_create("rec2Gp.nc", NC_CLOBBER|NC_64BIT_OFFSET, &ncid);
   check_err(stat,__LINE__,__FILE__);

   /* define dimensions */
   stat = nc_def_dim(ncid, "rec", rec_len, &rec_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "i", i_len, &i_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "j", j_len, &j_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "k", k_len, &k_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "n", n_len, &n_dim);
   check_err(stat,__LINE__,__FILE__);

   /* define variables */

   var1_dims[0] = rec_dim;
   var1_dims[1] = i_dim;
   var1_dims[2] = j_dim;
   var1_dims[3] = k_dim;
   stat = nc_def_var(ncid, "var1", NC_BYTE, RANK_var1, var1_dims, &var1_id);
   check_err(stat,__LINE__,__FILE__);

   x_dims[0] = rec_dim;
   x_dims[1] = n_dim;
   stat = nc_def_var(ncid, "x", NC_BYTE, RANK_x, x_dims, &x_id);
   check_err(stat,__LINE__,__FILE__);
   /* don't initialize variables with fill values */
   stat = nc_set_fill(ncid, NC_NOFILL, 0);
   check_err(stat,__LINE__,__FILE__);

   /* leave define mode */
   stat = nc_enddef (ncid);
   check_err(stat,__LINE__,__FILE__);

   {			/* store var1 */
       int n = 0;
       static signed char var1[J_LEN][K_LEN];
       static size_t var1_start[RANK_var1] = {0, 0, 0, 0};
       static size_t var1_count[RANK_var1] = {1, 1, J_LEN, K_LEN};
       static size_t x_start[RANK_x] = {0, 0};
       static size_t x_count[RANK_x] = {1, N_LEN};
       for(rec=0; rec<NUMRECS; rec++) {
	   var1_start[0] = rec;
	   x_start[0] = rec;
	   for(i=0; i<I_LEN; i++) {
	       for(j=0; j<J_LEN; j++) {
		   for (k=0; k<K_LEN; k++) {
		       var1[j][k] = n++;
		   }
	       }
	       var1_start[1] = i;
	       stat = nc_put_vara_schar(ncid, var1_id, var1_start, var1_count, &var1[0][0]);
	       check_err(stat,__LINE__,__FILE__);
	   }
       }
       stat = nc_put_vara_schar(ncid, x_id, x_start, x_count, x);
       check_err(stat,__LINE__,__FILE__);
   }

   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_open("rec2Gp.nc", NC_NOWRITE, &ncid);
   check_err(stat,__LINE__,__FILE__);

   {			/* read var1 */
       int n = 0;
       static signed char var1[J_LEN][K_LEN];
       static size_t var1_start[RANK_var1] = {0, 0, 0, 0};
       static size_t var1_count[RANK_var1] = {1, 1, J_LEN, K_LEN};
       static size_t x_start[RANK_x] = {0, 0};
       static size_t x_count[RANK_x] = {1, N_LEN};
       for(rec=0; rec<NUMRECS; rec++) {
	   var1_start[0] = rec;
	   x_start[0] = rec;
	   for(i=0; i<I_LEN; i++) {
	       var1_start[1] = i;
	       stat = nc_get_vara_schar(ncid, var1_id, var1_start, var1_count, &var1[0][0]);
	       check_err(stat,__LINE__,__FILE__);
	       for(j=0; j<J_LEN; j++) {
		   for (k=0; k<K_LEN; k++) {
		       if (var1[j][k] != (signed char) n) {
			   printf("Error on read, var1[%d, %d, %d, %d] = %d wrong, should be %d !\n", rec, i, j, k, var1[j][k], (signed char) n); 
			   return 1;
		       }
		       n++;
		   }
	       }
	   }
	   nc_get_vara_schar(ncid, x_id, x_start, x_count, x);
	   if(x[0] != 42 || x[1] != 21) {
	       printf("Error on read, x[] = %d, %d\n", x[0], x[1]);
	       return 1;
	   }
       }
   }
   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);

   return 0;
}
