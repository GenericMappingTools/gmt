/* This is part of the netCDF package.  Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program tests fixes for bugs reported with accessing
   fixed-length scalar string variables and variable-length scalar
   string attributes from HDF5 files through the netCDF-4 API.

   Here's a HDF5 sample programs:
   http://hdf.ncsa.uiuc.edu/training/other-ex5/sample-programs/strings.c
*/

#include <config.h>
#include <nc_tests.h>
#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_strbug.h5"
#define VS_ATT_NAME "vsatt"
#define FS_ATT_NAME "fsatt"
#define VS_VAR_NAME "vsvar"
#define FS_VAR_NAME "fsvar"
#define V1D_VAR_NAME "v1dvar"
#define FSTR_LEN 20
#define DIM1      4
#define RANK      1

int
main()
{
    char *vs    = "variable-length string";
    char fsdata[]   = "fixed-length string";
    char *v1ddata[DIM1] = {"strings","of","variable","length"};
    int i;
    char ch;

    printf("\n*** Creating file for checking fix to bugs in accessing strings from HDF5 non-netcdf-4 file.\n");
    {
	hid_t fileid, scalar_spaceid, vstypeid, fstypeid, vsattid, fsattid, vsdsetid, fsdsetid;
	hid_t class;
	size_t type_size = FSTR_LEN;
	hid_t v1dattid, v1ddsetid;
	hid_t v1dspaceid;
	hsize_t dims[1] = {DIM1};

	if ((scalar_spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
	if ((v1dspaceid = H5Screate_simple(RANK, dims, NULL)) < 0) ERR;
	
	/* Create variable-length and fixed-length string types. */
	if ((vstypeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
	if (H5Tset_size(vstypeid, H5T_VARIABLE) < 0) ERR;
	
	if ((fstypeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
	if (H5Tset_size(fstypeid, type_size) < 0) ERR;

	/* Create new file, using default properties. */
	if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
	
	/* Create scalar datasets of variable- and fixed-length strings. */
	if ((vsdsetid = H5Dcreate (fileid, VS_VAR_NAME, vstypeid, scalar_spaceid, 
				   H5P_DEFAULT)) < 0) ERR;
	if (H5Dwrite (vsdsetid, vstypeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &vs)) ERR;
	if ((fsdsetid = H5Dcreate (fileid, FS_VAR_NAME, fstypeid, scalar_spaceid, 
				   H5P_DEFAULT)) < 0) ERR;
	if (H5Dwrite (fsdsetid, fstypeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &fsdata)) ERR;

	/* Create 1D dataset of variable-length strings. */
	if ((v1ddsetid = H5Dcreate (fileid, V1D_VAR_NAME, vstypeid, v1dspaceid, 
				   H5P_DEFAULT)) < 0) ERR;
	if (H5Dwrite (v1ddsetid, vstypeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &v1ddata)) ERR;
	
	/* Write scalar global attributes of these types. */
	if ((vsattid = H5Acreate(fileid, VS_ATT_NAME, vstypeid, scalar_spaceid, 
				 H5P_DEFAULT)) < 0) ERR;
	if (H5Awrite(vsattid, vstypeid, &vs) < 0) ERR;
	if ((fsattid = H5Acreate(fileid, FS_ATT_NAME, fstypeid, scalar_spaceid, 
				 H5P_DEFAULT)) < 0) ERR;
	if (H5Awrite(fsattid, fstypeid, &fsdata) < 0) ERR;
	
	/* Close up. */
	if (H5Aclose(vsattid) < 0) ERR;
	if (H5Aclose(fsattid) < 0) ERR;
	if (H5Sclose(scalar_spaceid) < 0) ERR;
	if (H5Sclose(v1dspaceid) < 0) ERR;
	if (H5Tclose(vstypeid) < 0) ERR;
	if (H5Tclose(fstypeid) < 0) ERR;
	if (H5Dclose(vsdsetid) < 0) ERR;
	if (H5Dclose(fsdsetid) < 0) ERR;
	if (H5Dclose(v1ddsetid) < 0) ERR;
	if (H5Fclose(fileid) < 0) ERR;
    }

    printf("*** Checking reading variable-length HDF5 string var through netCDF-4 API...");
    {
	int ncid, varid, ndims;
	nc_type type;
	char *data_in;
	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	if (nc_inq_varid(ncid, VS_VAR_NAME, &varid)) ERR;
	if (nc_inq_vartype(ncid, varid, &type)) ERR;
	if (type != NC_STRING) ERR;
	if (nc_inq_varndims(ncid, varid, &ndims )) ERR;
	if (ndims != 0) ERR;
	if (nc_get_var_string(ncid, varid, &data_in)) ERR;
	if (strcmp(vs, data_in)) ERR;
	if (nc_free_string(1, &data_in)) ERR;
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** Checking reading fixed-length HDF5 string var through netCDF-4 API...");
    {
    	int ncid, varid, ndims;
    	nc_type type;
    	char *data_in;
    	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    	if (nc_inq_varid(ncid, FS_VAR_NAME, &varid)) ERR;
    	if (nc_inq_vartype(ncid, varid, &type)) ERR;
    	if (type != NC_STRING) ERR;
    	if (nc_inq_varndims(ncid, varid, &ndims )) ERR;
    	if (ndims != 0) ERR;
    	if (nc_get_var_string(ncid, varid, &data_in)) ERR;
    	if (strcmp(fsdata, data_in)) ERR;
	if (nc_free_string(1, &data_in)) ERR;
    	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** Checking reading variable-length HDF5 string att through netCDF-4 API...");
    {
	int ncid, varid, ndims;
	nc_type type;
	size_t len;
	char *data_in;
	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	if (nc_inq_att(ncid, NC_GLOBAL, VS_ATT_NAME, &type, &len)) ERR;
	if (type != NC_STRING) ERR;
	if (len != 1) ERR;
        if (nc_get_att_string(ncid, NC_GLOBAL, VS_ATT_NAME, &data_in)) ERR;
	if (strcmp(vs, data_in)) ERR;
	if (nc_free_string(1, &data_in)) ERR;
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** Checking reading fixed-length HDF5 string att through netCDF-4 API...");
    {
	int ncid, varid, ndims;
	nc_type type;
	size_t len;
	char *data_in;
	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	if (nc_inq_att(ncid, NC_GLOBAL, FS_ATT_NAME, &type, &len)) ERR;
	if (type != NC_CHAR) ERR;
	if (len != FSTR_LEN) ERR;
	if (!(data_in = malloc(len))) ERR;
        /* if (nc_get_att_string(ncid, NC_GLOBAL, FS_ATT_NAME, &data_in)) ERR; */
        if (nc_get_att_text(ncid, NC_GLOBAL, FS_ATT_NAME, data_in)) ERR;
	if (strcmp(fsdata, data_in)) ERR;
	free(data_in);
	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;

    printf("*** Checking reading variable-length HDF5 strings var through netCDF-4 API...");
    {
    	int ncid, varid, ndims;
    	nc_type type;
	int *dimids;
	size_t nstrings;
    	char **data_in;
    	if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
    	if (nc_inq_varid(ncid, V1D_VAR_NAME, &varid)) ERR;
    	if (nc_inq_vartype(ncid, varid, &type)) ERR;
    	if (type != NC_STRING) ERR;
    	if (nc_inq_varndims(ncid, varid, &ndims )) ERR;
    	if (ndims != RANK) ERR;
	if (!(dimids = malloc(ndims * sizeof(int)))) ERR;
    	if (nc_inq_vardimid(ncid, varid, dimids)) ERR;
	if (nc_inq_dimlen(ncid, dimids[0], &nstrings)) ERR;
	if (!(data_in = (char **)malloc(nstrings * sizeof(char *)))) ERR;
    	if (nc_get_var_string(ncid, varid, data_in)) ERR;
	for (i = 0; i < nstrings; i++) {
	    if(strcmp(v1ddata[i], data_in[i])) ERR;
	}
    	if (nc_free_string(nstrings, data_in)) ERR;
	free(data_in);
	free(dimids);
    	if (nc_close(ncid)) ERR;
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
