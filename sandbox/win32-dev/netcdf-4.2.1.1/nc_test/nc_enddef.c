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
main() {			/* create nc_enddef.nc */

   int  ncid;			/* netCDF id */

   /* dimension ids */
   int dim_dim;

   /* dimension lengths */
   size_t dim_len = 1;

   /* variable ids */
   int var_id;

   /* rank (number of dimensions) for each variable */
#  define RANK_var 1

   /* variable shapes */
   int var_dims[RANK_var];

   /* enter define mode */
   int stat = nc_create("nc_enddef.nc", NC_CLOBBER, &ncid);
   check_err(stat,__LINE__,__FILE__);

   /* define dimensions */
   stat = nc_def_dim(ncid, "dim", dim_len, &dim_dim);
   check_err(stat,__LINE__,__FILE__);

   /* define variables */

   var_dims[0] = dim_dim;
   stat = nc_def_var(ncid, "var", NC_DOUBLE, RANK_var, var_dims, &var_id);
   check_err(stat,__LINE__,__FILE__);

   /* leave define mode */
   stat = nc_enddef (ncid);
   check_err(stat,__LINE__,__FILE__);

   {			/* store var */
    static double var[] = {1.};
    stat = nc_put_var_double(ncid, var_id, var);
    check_err(stat,__LINE__,__FILE__);
   }
   stat = nc_sync(ncid);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);
   return 0;
}
