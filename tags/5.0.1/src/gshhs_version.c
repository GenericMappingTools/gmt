/*
 * $Id$
 * Helper program to get GSHHS version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>

#define BUF_SIZE 64
#define VERSION_ATT_NAME "version"

int main (int argc, char *argv[])
{
	int    status;               /* error status */
	int    ncid;                 /* netCDF ID */
	size_t v_len;                /* version length */
	char   version[BUF_SIZE];    /* GSHHS version */

	if (argc < 2) {
		fprintf(stderr, "usage: gshhs_version file\n");
		exit (EXIT_FAILURE);
	}

	/* open shoreline file */
	status = nc_open(argv[1], NC_NOWRITE, &ncid);
	if (status != NC_NOERR) {
		fprintf(stderr, "cannot open file \"%s\".\n", argv[1]);
		exit (status);
	}

	/* get length of version string */
	status = nc_inq_attlen (ncid, NC_GLOBAL, VERSION_ATT_NAME, &v_len);
	if (status != NC_NOERR) {
		fprintf(stderr, "cannot inquire version attribute length from file \"%s\".\n", argv[1]);
		exit (status);
	}
	if (v_len == 0 || v_len > BUF_SIZE) {
		fprintf(stderr, "invalid version attribute length: %d\n", v_len);
		exit (EXIT_FAILURE);
	}

	/* get version string */
	nc_get_att_text (ncid, NC_GLOBAL, VERSION_ATT_NAME, version);
	if (status != NC_NOERR || strlen(version) == 0 ) {
		fprintf(stderr, "cannot read version attribute from file \"%s\".\n", argv[1]);
		if (status == NC_NOERR)
			status = EXIT_FAILURE;
		exit (status);
	}

	/* null-terminate version string */
	version[v_len] = '\0';

	/* return version string */
	fprintf(stdout, "%s\n", version);

	return(0);
}
