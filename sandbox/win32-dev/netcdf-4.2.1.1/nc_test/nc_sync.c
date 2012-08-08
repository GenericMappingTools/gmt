#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <netcdf.h>
#include "nc_sync.h"

int
nc_sync_sub(int ncid)
{
    int		everythingOK = 1;
    FILE*	pipe;		/* IPC pipe to child process */

    setbuf(stdout, NULL);	/* unbuffer stdout */

    /*
     * Execute the child process.
     */
    pipe = popen("nc_sync_child", "w");

    if (pipe == NULL)
    {
	perror("popen() error");
	everythingOK = 0;
    }
    else
    {
	double	var[DIM2][DIM1][DIM0];

	setbuf(pipe, NULL);	/* unbuffer IPC pipe */

	/*
	 * Initialize the variable array.
	 */
	{
	    int		i = 0;
	    int		n = DIM0*DIM1*DIM2;
	    double	*dp = (double*)var;
	    for (i = 0; i < n; i++)
		*dp++ = i;
	}

	/*
	 * Write the variable array.
	 */
	{
	    int		i3;
	    int		ncerr;
	    size_t	start[NDIM] = {0, 0, 0, 0};
	    size_t	count[NDIM] = {1, DIM2, DIM1, DIM0};

	    /*
	     * Loop over the unlimited dimension.
	     */
	    for (i3 = 0; everythingOK && i3 < DIM3; ++i3)
	    {
		int	ivar;

		start[0] = i3;
		printf("PARENT: Writing %d\n", i3);
		fflush(stdout);

		/*
		 * Loop over the variables.
		 */
		for (ivar = 0; everythingOK && ivar < NVAR; ++ivar)
		{
		    ncerr =
			nc_put_vara_double(
			    ncid, ivar, start, count, (double*)var);
		    if (ncerr != NC_NOERR)
		    {
			fprintf(
			    stderr, "nc_put_vara_double() error: %s\n",
			    nc_strerror(ncerr));
			everythingOK = 0;
		    }
		}
		if (everythingOK)
		{
		    /*
		     * Synchronize the netCDF file.
		     */
		    puts("PARENT: Calling nc_sync()");
		    fflush(stdout);
		    ncerr = nc_sync(ncid);
		    if (ncerr != NC_NOERR)
		    {
			fprintf(
			    stderr, "nc_sync() error: %s\n",
			    nc_strerror(ncerr));
			everythingOK = 0;
		    }
		    else
		    {
			/*
			 * Notify the child process.
			 */
			puts("PARENT: Notifying child");
			fflush(stdout);
			if (fwrite(&i3, sizeof(i3), 1, pipe) != 1)
			{
			    perror("fwrite() error");
			    everythingOK = 0;
			}
		    }
		}		/* variables written */
	    }			/* unlimited dimension loop */
	}			/* write block */

	fclose(pipe);

	{
	    int   status;
	    pid_t pid;

	    pid = wait(&status);
	}
    }				/* successfull popen() */

    return everythingOK ? 0 : 1;
}

void
check_err(const int stat, const int line, const char *file)
{
    if (stat != NC_NOERR) {
       (void) fprintf(stderr, "line %d of %s: %s\n", line, file, 
	   nc_strerror(stat));
        exit(1);
    }
}

int
main() {			/* create nc_sync.nc */

   int  ncid;			/* netCDF id */

   /* dimension ids */
   int dim0_dim;
   int dim1_dim;
   int dim2_dim;
   int dim3_dim;

   /* dimension lengths */
   size_t dim0_len = 53;
   size_t dim1_len = 67;
   size_t dim2_len = 30;
   size_t dim3_len = NC_UNLIMITED;

   /* variable ids */
   int var0_id;
   int var1_id;
   int var2_id;
   int var3_id;
   int var4_id;
   int var5_id;
   int var6_id;
   int var7_id;
   int var8_id;
   int var9_id;
   int var10_id;
   int var11_id;
   int var12_id;
   int var13_id;
   int var14_id;
   int var15_id;
   int var16_id;
   int var17_id;
   int var18_id;
   int var19_id;
   int var20_id;
   int var21_id;
   int var22_id;
   int var23_id;
   int var24_id;
   int var25_id;
   int var26_id;
   int var27_id;
   int var28_id;
   int var29_id;
   int var30_id;
   int var31_id;
   int var32_id;
   int var33_id;
   int var34_id;
   int var35_id;
   int var36_id;

   /* rank (number of dimensions) for each variable */
#  define RANK_var0 4
#  define RANK_var1 4
#  define RANK_var2 4
#  define RANK_var3 4
#  define RANK_var4 4
#  define RANK_var5 4
#  define RANK_var6 4
#  define RANK_var7 4
#  define RANK_var8 4
#  define RANK_var9 4
#  define RANK_var10 4
#  define RANK_var11 4
#  define RANK_var12 4
#  define RANK_var13 4
#  define RANK_var14 4
#  define RANK_var15 4
#  define RANK_var16 4
#  define RANK_var17 4
#  define RANK_var18 4
#  define RANK_var19 4
#  define RANK_var20 4
#  define RANK_var21 4
#  define RANK_var22 4
#  define RANK_var23 4
#  define RANK_var24 4
#  define RANK_var25 4
#  define RANK_var26 4
#  define RANK_var27 4
#  define RANK_var28 4
#  define RANK_var29 4
#  define RANK_var30 4
#  define RANK_var31 4
#  define RANK_var32 4
#  define RANK_var33 4
#  define RANK_var34 4
#  define RANK_var35 4
#  define RANK_var36 4

   /* variable shapes */
   int var0_dims[RANK_var0];
   int var1_dims[RANK_var1];
   int var2_dims[RANK_var2];
   int var3_dims[RANK_var3];
   int var4_dims[RANK_var4];
   int var5_dims[RANK_var5];
   int var6_dims[RANK_var6];
   int var7_dims[RANK_var7];
   int var8_dims[RANK_var8];
   int var9_dims[RANK_var9];
   int var10_dims[RANK_var10];
   int var11_dims[RANK_var11];
   int var12_dims[RANK_var12];
   int var13_dims[RANK_var13];
   int var14_dims[RANK_var14];
   int var15_dims[RANK_var15];
   int var16_dims[RANK_var16];
   int var17_dims[RANK_var17];
   int var18_dims[RANK_var18];
   int var19_dims[RANK_var19];
   int var20_dims[RANK_var20];
   int var21_dims[RANK_var21];
   int var22_dims[RANK_var22];
   int var23_dims[RANK_var23];
   int var24_dims[RANK_var24];
   int var25_dims[RANK_var25];
   int var26_dims[RANK_var26];
   int var27_dims[RANK_var27];
   int var28_dims[RANK_var28];
   int var29_dims[RANK_var29];
   int var30_dims[RANK_var30];
   int var31_dims[RANK_var31];
   int var32_dims[RANK_var32];
   int var33_dims[RANK_var33];
   int var34_dims[RANK_var34];
   int var35_dims[RANK_var35];
   int var36_dims[RANK_var36];

   /* enter define mode */
   int stat = nc_create("nc_sync.nc", NC_CLOBBER, &ncid);
   check_err(stat,__LINE__,__FILE__);

   /* define dimensions */
   stat = nc_def_dim(ncid, "dim0", dim0_len, &dim0_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "dim1", dim1_len, &dim1_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "dim2", dim2_len, &dim2_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "dim3", dim3_len, &dim3_dim);
   check_err(stat,__LINE__,__FILE__);

   /* define variables */

   var0_dims[0] = dim3_dim;
   var0_dims[1] = dim2_dim;
   var0_dims[2] = dim1_dim;
   var0_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var0", NC_DOUBLE, RANK_var0, var0_dims, &var0_id);
   check_err(stat,__LINE__,__FILE__);

   var1_dims[0] = dim3_dim;
   var1_dims[1] = dim2_dim;
   var1_dims[2] = dim1_dim;
   var1_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var1", NC_DOUBLE, RANK_var1, var1_dims, &var1_id);
   check_err(stat,__LINE__,__FILE__);

   var2_dims[0] = dim3_dim;
   var2_dims[1] = dim2_dim;
   var2_dims[2] = dim1_dim;
   var2_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var2", NC_DOUBLE, RANK_var2, var2_dims, &var2_id);
   check_err(stat,__LINE__,__FILE__);

   var3_dims[0] = dim3_dim;
   var3_dims[1] = dim2_dim;
   var3_dims[2] = dim1_dim;
   var3_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var3", NC_DOUBLE, RANK_var3, var3_dims, &var3_id);
   check_err(stat,__LINE__,__FILE__);

   var4_dims[0] = dim3_dim;
   var4_dims[1] = dim2_dim;
   var4_dims[2] = dim1_dim;
   var4_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var4", NC_DOUBLE, RANK_var4, var4_dims, &var4_id);
   check_err(stat,__LINE__,__FILE__);

   var5_dims[0] = dim3_dim;
   var5_dims[1] = dim2_dim;
   var5_dims[2] = dim1_dim;
   var5_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var5", NC_DOUBLE, RANK_var5, var5_dims, &var5_id);
   check_err(stat,__LINE__,__FILE__);

   var6_dims[0] = dim3_dim;
   var6_dims[1] = dim2_dim;
   var6_dims[2] = dim1_dim;
   var6_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var6", NC_DOUBLE, RANK_var6, var6_dims, &var6_id);
   check_err(stat,__LINE__,__FILE__);

   var7_dims[0] = dim3_dim;
   var7_dims[1] = dim2_dim;
   var7_dims[2] = dim1_dim;
   var7_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var7", NC_DOUBLE, RANK_var7, var7_dims, &var7_id);
   check_err(stat,__LINE__,__FILE__);

   var8_dims[0] = dim3_dim;
   var8_dims[1] = dim2_dim;
   var8_dims[2] = dim1_dim;
   var8_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var8", NC_DOUBLE, RANK_var8, var8_dims, &var8_id);
   check_err(stat,__LINE__,__FILE__);

   var9_dims[0] = dim3_dim;
   var9_dims[1] = dim2_dim;
   var9_dims[2] = dim1_dim;
   var9_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var9", NC_DOUBLE, RANK_var9, var9_dims, &var9_id);
   check_err(stat,__LINE__,__FILE__);

   var10_dims[0] = dim3_dim;
   var10_dims[1] = dim2_dim;
   var10_dims[2] = dim1_dim;
   var10_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var10", NC_DOUBLE, RANK_var10, var10_dims, &var10_id);
   check_err(stat,__LINE__,__FILE__);

   var11_dims[0] = dim3_dim;
   var11_dims[1] = dim2_dim;
   var11_dims[2] = dim1_dim;
   var11_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var11", NC_DOUBLE, RANK_var11, var11_dims, &var11_id);
   check_err(stat,__LINE__,__FILE__);

   var12_dims[0] = dim3_dim;
   var12_dims[1] = dim2_dim;
   var12_dims[2] = dim1_dim;
   var12_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var12", NC_DOUBLE, RANK_var12, var12_dims, &var12_id);
   check_err(stat,__LINE__,__FILE__);

   var13_dims[0] = dim3_dim;
   var13_dims[1] = dim2_dim;
   var13_dims[2] = dim1_dim;
   var13_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var13", NC_DOUBLE, RANK_var13, var13_dims, &var13_id);
   check_err(stat,__LINE__,__FILE__);

   var14_dims[0] = dim3_dim;
   var14_dims[1] = dim2_dim;
   var14_dims[2] = dim1_dim;
   var14_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var14", NC_DOUBLE, RANK_var14, var14_dims, &var14_id);
   check_err(stat,__LINE__,__FILE__);

   var15_dims[0] = dim3_dim;
   var15_dims[1] = dim2_dim;
   var15_dims[2] = dim1_dim;
   var15_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var15", NC_DOUBLE, RANK_var15, var15_dims, &var15_id);
   check_err(stat,__LINE__,__FILE__);

   var16_dims[0] = dim3_dim;
   var16_dims[1] = dim2_dim;
   var16_dims[2] = dim1_dim;
   var16_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var16", NC_DOUBLE, RANK_var16, var16_dims, &var16_id);
   check_err(stat,__LINE__,__FILE__);

   var17_dims[0] = dim3_dim;
   var17_dims[1] = dim2_dim;
   var17_dims[2] = dim1_dim;
   var17_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var17", NC_DOUBLE, RANK_var17, var17_dims, &var17_id);
   check_err(stat,__LINE__,__FILE__);

   var18_dims[0] = dim3_dim;
   var18_dims[1] = dim2_dim;
   var18_dims[2] = dim1_dim;
   var18_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var18", NC_DOUBLE, RANK_var18, var18_dims, &var18_id);
   check_err(stat,__LINE__,__FILE__);

   var19_dims[0] = dim3_dim;
   var19_dims[1] = dim2_dim;
   var19_dims[2] = dim1_dim;
   var19_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var19", NC_DOUBLE, RANK_var19, var19_dims, &var19_id);
   check_err(stat,__LINE__,__FILE__);

   var20_dims[0] = dim3_dim;
   var20_dims[1] = dim2_dim;
   var20_dims[2] = dim1_dim;
   var20_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var20", NC_DOUBLE, RANK_var20, var20_dims, &var20_id);
   check_err(stat,__LINE__,__FILE__);

   var21_dims[0] = dim3_dim;
   var21_dims[1] = dim2_dim;
   var21_dims[2] = dim1_dim;
   var21_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var21", NC_DOUBLE, RANK_var21, var21_dims, &var21_id);
   check_err(stat,__LINE__,__FILE__);

   var22_dims[0] = dim3_dim;
   var22_dims[1] = dim2_dim;
   var22_dims[2] = dim1_dim;
   var22_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var22", NC_DOUBLE, RANK_var22, var22_dims, &var22_id);
   check_err(stat,__LINE__,__FILE__);

   var23_dims[0] = dim3_dim;
   var23_dims[1] = dim2_dim;
   var23_dims[2] = dim1_dim;
   var23_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var23", NC_DOUBLE, RANK_var23, var23_dims, &var23_id);
   check_err(stat,__LINE__,__FILE__);

   var24_dims[0] = dim3_dim;
   var24_dims[1] = dim2_dim;
   var24_dims[2] = dim1_dim;
   var24_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var24", NC_DOUBLE, RANK_var24, var24_dims, &var24_id);
   check_err(stat,__LINE__,__FILE__);

   var25_dims[0] = dim3_dim;
   var25_dims[1] = dim2_dim;
   var25_dims[2] = dim1_dim;
   var25_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var25", NC_DOUBLE, RANK_var25, var25_dims, &var25_id);
   check_err(stat,__LINE__,__FILE__);

   var26_dims[0] = dim3_dim;
   var26_dims[1] = dim2_dim;
   var26_dims[2] = dim1_dim;
   var26_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var26", NC_DOUBLE, RANK_var26, var26_dims, &var26_id);
   check_err(stat,__LINE__,__FILE__);

   var27_dims[0] = dim3_dim;
   var27_dims[1] = dim2_dim;
   var27_dims[2] = dim1_dim;
   var27_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var27", NC_DOUBLE, RANK_var27, var27_dims, &var27_id);
   check_err(stat,__LINE__,__FILE__);

   var28_dims[0] = dim3_dim;
   var28_dims[1] = dim2_dim;
   var28_dims[2] = dim1_dim;
   var28_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var28", NC_DOUBLE, RANK_var28, var28_dims, &var28_id);
   check_err(stat,__LINE__,__FILE__);

   var29_dims[0] = dim3_dim;
   var29_dims[1] = dim2_dim;
   var29_dims[2] = dim1_dim;
   var29_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var29", NC_DOUBLE, RANK_var29, var29_dims, &var29_id);
   check_err(stat,__LINE__,__FILE__);

   var30_dims[0] = dim3_dim;
   var30_dims[1] = dim2_dim;
   var30_dims[2] = dim1_dim;
   var30_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var30", NC_DOUBLE, RANK_var30, var30_dims, &var30_id);
   check_err(stat,__LINE__,__FILE__);

   var31_dims[0] = dim3_dim;
   var31_dims[1] = dim2_dim;
   var31_dims[2] = dim1_dim;
   var31_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var31", NC_DOUBLE, RANK_var31, var31_dims, &var31_id);
   check_err(stat,__LINE__,__FILE__);

   var32_dims[0] = dim3_dim;
   var32_dims[1] = dim2_dim;
   var32_dims[2] = dim1_dim;
   var32_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var32", NC_DOUBLE, RANK_var32, var32_dims, &var32_id);
   check_err(stat,__LINE__,__FILE__);

   var33_dims[0] = dim3_dim;
   var33_dims[1] = dim2_dim;
   var33_dims[2] = dim1_dim;
   var33_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var33", NC_DOUBLE, RANK_var33, var33_dims, &var33_id);
   check_err(stat,__LINE__,__FILE__);

   var34_dims[0] = dim3_dim;
   var34_dims[1] = dim2_dim;
   var34_dims[2] = dim1_dim;
   var34_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var34", NC_DOUBLE, RANK_var34, var34_dims, &var34_id);
   check_err(stat,__LINE__,__FILE__);

   var35_dims[0] = dim3_dim;
   var35_dims[1] = dim2_dim;
   var35_dims[2] = dim1_dim;
   var35_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var35", NC_DOUBLE, RANK_var35, var35_dims, &var35_id);
   check_err(stat,__LINE__,__FILE__);

   var36_dims[0] = dim3_dim;
   var36_dims[1] = dim2_dim;
   var36_dims[2] = dim1_dim;
   var36_dims[3] = dim0_dim;
   stat = nc_def_var(ncid, "var36", NC_DOUBLE, RANK_var36, var36_dims, &var36_id);
   check_err(stat,__LINE__,__FILE__);

   /* leave define mode */
   stat = nc_enddef (ncid);
   check_err(stat,__LINE__,__FILE__);

   nc_sync_sub(ncid);

   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);

   return 0;
}
