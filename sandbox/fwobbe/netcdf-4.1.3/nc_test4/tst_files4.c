/* This is part of the netCDF package.
   Copyright 2010 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 file from user-reported error. This code based on an
   ncgen output. 

   $Id$
*/

#include <config.h>
#include <stdio.h>
#include <nc_tests.h>
#include "netcdf.h"
#include <string.h>

#define FILE_NAME "tst_files4.nc"
#define SNAPSHOT_LEN NC_UNLIMITED
#define AXIS_LEN 3
#define PARTICLE_LEN 256
#define SLICE_LEN 2
#define NDIMS1 1
#define NDIMS4 4
#define CLASSICAL "classical"

int
main() {/* create data.nc */

    int  ncid;  
    int classical_grp;

    /* dimension ids */
    int snapshot_dim;
    int axis_dim;
    int particle_dim;
    int slice_dim;


    size_t snapshot_len = SNAPSHOT_LEN;
    size_t axis_len = AXIS_LEN;
    size_t particle_len = PARTICLE_LEN;
    size_t slice_len = SLICE_LEN;

    int snapshot_id;
    int position_id;
    int axis_id;

    printf("\n*** Testing NetCDF-4 with user-supplied sample file.\n");
    printf("*** testing creation of sample file...");
    {
       double data[SLICE_LEN * PARTICLE_LEN * AXIS_LEN];
       size_t start[4] = {0, 0, 0, 0};
       size_t count[4] = {1, SLICE_LEN, PARTICLE_LEN, AXIS_LEN};
       int dimids[NDIMS4], grpid;
       int ndims_in, nvars_in, natts_in, ngrps_in;
       int unlimdimid;
       char name_in[NC_MAX_NAME + 1];
       int i;

       /* Sample data. */
       for (i = 0; i < SLICE_LEN * PARTICLE_LEN * AXIS_LEN; i++)
	  data[i] = 42.42;
       
       if (nc_create(FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
       if (nc_def_grp(ncid, CLASSICAL, &classical_grp)) ERR;

       /* define dimensions */
       if (nc_def_dim(classical_grp, "snapshot", snapshot_len, &snapshot_dim)) ERR;
       if (nc_def_dim(classical_grp, "axis", axis_len, &axis_dim)) ERR;
       if (nc_def_dim(classical_grp, "particle", particle_len, &particle_dim)) ERR;
       if (nc_def_dim(classical_grp, "slice", slice_len, &slice_dim)) ERR;

       dimids[0] = snapshot_dim;
       dimids[1] = slice_dim;
       dimids[2] = particle_dim;
       dimids[3] = axis_dim;
       if (nc_def_var(classical_grp, "position", NC_DOUBLE, NDIMS4, 
		      dimids, &position_id)) ERR;

       /* First write some position data. */
       if (nc_put_vara_double(classical_grp, position_id, 
			      start, count, data)) ERR;

       /* Now define some coordinate variables. */
       if (nc_def_var(classical_grp, "snapshot", NC_INT, NDIMS1, 
		      &snapshot_dim, &snapshot_id)) ERR;
       if (nc_def_var(classical_grp, "axis", NC_CHAR, NDIMS1, 
		      &axis_dim, &axis_id)) ERR;

       /* Check some stuff. */
       if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid)) ERR;
       if (ndims_in != 0 || nvars_in != 0 || natts_in != 0 || unlimdimid != -1) ERR;
       if (nc_inq_grps(ncid, &ngrps_in, &grpid)) ERR;
       if (ngrps_in != 1) ERR;
       if (nc_inq_grpname(grpid, name_in)) ERR;
       if (strcmp(name_in, CLASSICAL)) ERR;
       if (nc_inq(classical_grp, &ndims_in, &nvars_in, &natts_in, &unlimdimid)) ERR;
       if (ndims_in != 4 || nvars_in != 3 || natts_in != 0 || unlimdimid != 0) ERR;
       

       if (nc_close(ncid)) ERR;

       if (nc_open(FILE_NAME, 0, &ncid)) ERR;
       if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
#ifdef EXTRA_TESTS
    printf("*** testing opening of many files...");
    {
       int i, ncid;

       for (i = 0; i < 32768; i++) 
       {
	  if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR_RET;
	  if (nc_close(ncid)) ERR_RET;
       }
       if (nc_create(FILE_NAME, 0, &ncid)) ERR_RET;
       if (nc_def_var(ncid, "blah", NC_CHAR, 0, NULL, NULL)) ERR;
       if (nc_close(ncid)) ERR_RET;
       /*printf("last ncid: %d\n", ncid);*/
    }
    SUMMARIZE_ERR;
#endif
    FINAL_RESULTS;
}
