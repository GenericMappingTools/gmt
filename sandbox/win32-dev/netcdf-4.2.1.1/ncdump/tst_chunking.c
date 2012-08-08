/* This is part of the netCDF package. Copyright 2010 University
   Corporation for Atmospheric Research/Unidata.  See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a chunkable test file for nccopy to test chunking.

   $Id$
*/

#include <netcdf.h>
#include <nc_tests.h>

#define FILE_NAME "tst_chunking.nc"
#define VAR_RANK 7
#define IVAR_NAME "ivar"
#define FVAR_NAME "fvar"
#define NVALS 45360		/* 7 * 4 * 2 * 3 * 5 * 6 * 9 */

int
main(int argc, char **argv) {
    int ncid;
    int ivarid, fvarid;
    char *dim_names[VAR_RANK] = {"dim0", "dim1", "dim2", "dim3", "dim4", "dim5", "dim6"};
    size_t dim_lens[VAR_RANK] = {7, 4, 2, 3, 5, 6, 9};
    int ivar_dims[VAR_RANK];
    int fvar_dims[VAR_RANK];
    int ivar_data[NVALS];
    float fvar_data[NVALS];
    int r, i;

    printf("*** creating chunkable test file %s...\n", FILE_NAME);

    if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
    for(r = 0; r < VAR_RANK; r++) {
	if (nc_def_dim(ncid, dim_names[r], dim_lens[r], &ivar_dims[r])) ERR;
	fvar_dims[VAR_RANK - 1 - r] = ivar_dims[r];
   }
    if (nc_def_var(ncid, IVAR_NAME, NC_INT, VAR_RANK, ivar_dims, &ivarid)) ERR;
    if (nc_def_var(ncid, FVAR_NAME, NC_FLOAT, VAR_RANK, fvar_dims, &fvarid)) ERR;
    if (nc_enddef (ncid)) ERR;
    for(i=0; i < NVALS; i++) {
	ivar_data[i] = i;
	fvar_data[i] = NVALS - i;
    }
    if (nc_put_var(ncid, ivarid, ivar_data)) ERR;
    if (nc_put_var(ncid, fvarid, fvar_data)) ERR;
    if (nc_close(ncid)) ERR;
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
