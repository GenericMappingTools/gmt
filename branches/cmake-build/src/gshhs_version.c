/*
 * $Id$
 * Helper program to get GSHHS version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>

int main (int argc, char *argv[])
{
	int  status;               /* error status */
	int  ncid;                 /* netCDF ID */
	char version[16];          /* GSHHS version */

	/* open shoreline file */
	status = nc_open(argv[1], NC_NOWRITE, &ncid);
	if (status != NC_NOERR) {
		printf("cannot open file \"%s\".\n", argv[1]);
		exit (status);
	}

	/* get version string */
	nc_get_att_text (ncid, NC_GLOBAL, "version", version);
	if (status != NC_NOERR || strlen(version) == 0 ) {
		printf("cannot read version attribute from file \"%s\".\n", argv[1]);
		if (status == NC_NOERR)
			status = EXIT_FAILURE;
		exit (status);
	}

	/* return version string */
	printf("%s\n", version);
	return(0);
}
