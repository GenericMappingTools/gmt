/*********************************************************************
 * This is part of the Unidata netCDF package.
 * Copyright 2006, UCAR/Unidata
 * See COPYRIGHT file for copying and redistribution conditions.
 *
 * This program is part of the testing of file lengths done by the
 * test script tst_lentghs.sh.
 *
 * $Id$
 *********************************************************************/

#include <config.h>
#include <stdio.h>
#include <netcdf.h>

#define ERR do {fflush(stdout); fprintf(stderr, "Error, %s, line: %d\n", __FILE__, __LINE__); return(1);} while (0)

int
main(int ac, char *av[]) {
    int ncid, varid, data[] = {42};

    if (nc_open(av[1], NC_WRITE, &ncid)) ERR;
    if (nc_inq_varid(ncid, av[2], &varid)) ERR;
    if (nc_put_var_int(ncid, varid, data)) ERR;
    if (nc_close(ncid)) ERR;

    return 0;
}
