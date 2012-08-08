#include <nc_tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILENAME "tst_fillbug.nc"

static int count_udtypes(int ncid);

int
main(int argc, char **argv) 
{/* create file that caused seg fault in ncdump */

    int  ncid;  /* netCDF id */

    /* dimension ids */
    int Time_dim;
    int X_dim;
    int Y_dim;

    /* dimension lengths */
    size_t Time_len = NC_UNLIMITED;
    size_t X_len = 4;
    size_t Y_len = 3;

    /* variable ids */
    int Time_id;
    int P_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_Time 1
#   define RANK_P 3

    /* variable shapes */
    int Time_dims[RANK_Time];
    int P_dims[RANK_P];

   printf("\n*** Testing preparation of fillbug test.\n");
   printf("*** creating fillbug test file %s...", FILENAME);

    /* enter define mode */
    if (nc_create(FILENAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;

    /* define dimensions */
    if (nc_def_dim(ncid, "Time", Time_len, &Time_dim)) ERR;
    if (nc_def_dim(ncid, "X", X_len, &X_dim)) ERR;
    if (nc_def_dim(ncid, "Y", Y_len, &Y_dim)) ERR;

    /* define variables */

    Time_dims[0] = Time_dim;
    if (nc_def_var(ncid, "Time", NC_DOUBLE, RANK_Time, Time_dims, &Time_id)) ERR;

    P_dims[0] = Time_dim;
    P_dims[1] = Y_dim;
    P_dims[2] = X_dim;
    if (nc_def_var(ncid, "P", NC_FLOAT, RANK_P, P_dims, &P_id)) ERR;

    /* leave define mode */
    if (nc_enddef (ncid)) ERR;

    {/* assign variable data */
    static double Time_data[1]={3.14159};
    static size_t Time_startset[1] = {0};
    static size_t Time_countset[1] = {1};
    if (nc_put_vara(ncid, Time_id, Time_startset, Time_countset, Time_data)) ERR;
    }

    if (nc_close(ncid)) ERR;

    /* Try to duplicate segfault ncdump gets by making the same calls
     * to the netCDF-4 library, in the same order.  This doesn't
     * result in the same segfault, so either we have missed a call
     * made by ncdump, or an earlier ncdump bug masks the real problem
     * until a call is made into the netCDF-4 library ...  */
    if (nc_open(FILENAME, NC_NOWRITE, &ncid)) ERR;
    
    {   		
	/* We declare local arrays with small constant sizes to avoid
	 * all the mallocs and frees used in ncdump.  For the example
	 * above, the fixed-size arrays are ample. */
	int format, ndims, nvars, ngatts, xdimid, ndims_grp, dimids_grp[3],
	    unlimids[1], d_grp, nunlim, nvars_grp, varids_grp[3], v_grp,
	    varid, varndims, vardims[3], varnatts, vartype, dimids[3], is_recvar,
	    vdims[3], id, ntypes, numgrps, atttype, nc_status;
	size_t dimsize, len, attlen;
	char dimname[20], varname[20];
	if ( nc_inq_format(ncid, &format)) ERR;
	ntypes = count_udtypes(ncid);
	if ( nc_inq_typeids(ncid, &ntypes, NULL) ) ERR;
	if ( nc_inq_format(ncid, &format)) ERR;
	if ( nc_inq_grps(ncid, &numgrps, NULL) ) ERR;
	if ( nc_inq_typeids(ncid, &ntypes, NULL) ) ERR;
	if ( nc_inq(ncid, &ndims, &nvars, &ngatts, &xdimid) ) ERR;
	if ( nc_inq_ndims(ncid, &ndims_grp) ) ERR;
	if ( nc_inq_dimids(ncid, 0, dimids_grp, 0) ) ERR;
	if ( nc_inq_unlimdims(ncid, &nunlim, NULL) ) ERR;
	if ( nc_inq_unlimdims(ncid, &nunlim, unlimids) ) ERR;
	for (d_grp = 0; d_grp < ndims_grp; d_grp++) {
	    int dimid = dimids_grp[d_grp];
	    if ( nc_inq_dim(ncid, dimid, dimname, &dimsize) ) ERR;
	}
	if ( nc_inq_format(ncid, &format) ) ERR;
	if ( nc_inq_varids(ncid, &nvars_grp, varids_grp) ) ERR;
	for (v_grp = 0; v_grp < nvars_grp; v_grp++) {
	    varid = varids_grp[v_grp];
	    if ( nc_inq_varndims(ncid, varid, &varndims) ) ERR;
	    if ( nc_inq_var(ncid, varid, varname, &vartype, 0, vardims, 
			    &varnatts) ) ERR;
	    for (id = 0; id < varndims; id++) {
		if ( nc_inq_dimname(ncid, vardims[id], dimname) ) ERR;
	    }
	}
	for (v_grp = 0; v_grp < nvars_grp; v_grp++) {
	    varid = varids_grp[v_grp];
	    if( nc_inq_varndims(ncid, varid, &varndims) ) ERR;
	    if( nc_inq_var(ncid, varid, varname, &vartype, 0, vardims, 
			   &varnatts) ) ERR;
	    {
		is_recvar = 0;
		if ( nc_inq_varndims(ncid, varid, &ndims) ) ERR;
		if (ndims > 0) {
		    int nunlimdims;
		    int recdimids[3];
		    int dim, recdim;
		    if ( nc_inq_vardimid(ncid, varid, dimids) ) ERR;
		    if ( nc_inq_unlimdims(ncid, &nunlimdims, NULL) ) ERR;
		    if ( nc_inq_unlimdims(ncid, NULL, recdimids) ) ERR;
		    for (dim = 0; dim < ndims && is_recvar == 0; dim++) {
			for(recdim = 0; recdim < nunlimdims; recdim++) {
			    if(dimids[dim] == recdimids[recdim]) {
				is_recvar = 1;
				break;
			    }		
			}
		    }
		}
	    }
	    for (id = 0; id < varndims; id++) {
		if( nc_inq_dimlen(ncid, vardims[id], &len) ) ERR;
		vdims[id] = len;
	    }
	    nc_status = nc_inq_att(ncid,varid,_FillValue,&atttype,&attlen);
	    nc_status = nc_inq_att(ncid, varid, "units", &atttype, &attlen);
	    nc_status = nc_inq_att(ncid, varid, "C_format", &atttype, &attlen);
	    if (varid == 0) {
		/* read Time variable */
		static double Time_data;
		static size_t cor[RANK_Time] = {0};
		static size_t edg[RANK_Time] = {1};
		if (nc_get_vara(ncid, varid, cor, edg, &Time_data)) ERR;
	    } else {
		/* read data slices from P variable, should get fill values */
		static float P_data[4];
		static size_t cor[RANK_P] = {0, 0, 0};
		static size_t edg[RANK_P] = {1, 1, 4};

		/* first slice retrieved OK */
		if (nc_get_vara(ncid, varid, cor, edg, P_data)) ERR;
		
		/* In ncdump, reading second slice gets seg fault in
		 * nc4_open_var_grp(), but this attempt to do all the
		 * same netCDF calls as ncdump can't duplicate the
		 * error, which would seem to implicate ncdump rather
		 * than HDF5 or netCDF-4 library ... */
		cor[1] = 1;
		if (nc_get_vara(ncid, varid, cor, edg, P_data)) ERR;
	    }
	}
    }
    if (nc_close(ncid)) ERR;
      
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

/* return number of user-defined types in a group and all its subgroups */
static int
count_udtypes(int ncid) {
    int ntypes = 0;
    int numgrps;
    int *ncids;
    int i;
    int format;

    if( nc_inq_format(ncid, &format) ) ERR;

    if (format == NC_FORMAT_NETCDF4) {
	/* Get number of types in this group */
	if( nc_inq_typeids(ncid, &ntypes, NULL) ) ERR;
	if( nc_inq_grps(ncid, &numgrps, NULL) ) ERR;
	ncids = (int *) malloc(sizeof(int) * numgrps);
	if( nc_inq_grps(ncid, NULL, ncids) ) ERR;
	/* Add number of types in each subgroup, if any */
	for (i=0; i < numgrps; i++) {
	    ntypes += count_udtypes(ncids[i]);
	}
    }
    return ntypes;
}
