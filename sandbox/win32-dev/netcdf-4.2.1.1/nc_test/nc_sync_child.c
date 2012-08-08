#include <stdio.h>
#include <netcdf.h>
#include "nc_sync.h"

int
main()
{
    int	ncid;
    int	ncerr;
    int	everythingOK = 1;

    setbuf(stdout, NULL);	/* unbuffer stdout */

    /*
     * Open the netCDF file.
     */
    ncerr = nc_open("nc_sync.nc", 0, &ncid);
    if (ncerr != NC_NOERR)
    {
	fprintf(stderr, "nc_open() error: %s\n", nc_strerror(ncerr));
	everythingOK = 0;
    }
    else
    {
	double	var[DIM2][DIM1][DIM0];
	size_t	start[NDIM] = {0, 0, 0, 0};
	size_t	count[NDIM] = {1, DIM2, DIM1, DIM0};
	int	eofRead = 0;

	/*
	 * Loop over the unlimited dimension.
	 */
	while (everythingOK && !eofRead)
	{
	    int	i3;
	    int	ivar;
	    int	nitem;

	    /*
	     * Read the unlimited dimension index.
	     */
	    puts("CHILD: Getting notification");
	    fflush(stdout);
	    nitem = fread(&i3, sizeof(i3), 1, stdin);
	    if (nitem == 0)
	    {
		puts("CHILD: Read EOF");
		fflush(stdout);
		eofRead = 1;
	    }
	    else if (nitem != 1)
	    {
		perror("fread() error");
		everythingOK = 0;
	    }
	    else
	    {
		start[0] = i3;

		/*
		 * Synchronize the netCDF file.
		 */
		puts("CHILD: Calling nc_sync()");
		fflush(stdout);
		ncerr = nc_sync(ncid);
		if (ncerr != NC_NOERR)
		{
		    fprintf(
			stderr, "nc_sync() error: %s\n", nc_strerror(ncerr));
		    everythingOK = 0;
		}
		else
		{
		    printf("CHILD: Reading %d\n", i3);
		    fflush(stdout);

		    /*
		     * Loop over the variables.
		     */
		    for (ivar = 0; everythingOK && ivar < NVAR; ++ivar)
		    {
			ncerr =
			    nc_get_vara_double(
				ncid, ivar, start, count, (double*)var);
			if (ncerr != NC_NOERR)
			{
			    fprintf(
				stderr,
			    "nc_get_vara_double() error: %s: i3=%d, ivar=%d\n",
				nc_strerror(ncerr), i3, ivar);
			    everythingOK = 0;
			}
		    }
		}			/* netCDF file synchronized */
	    }				/* unlimited dimension index read */
	}				/* unlimited dimension loop */
    }					/* input netCDF file opened */

    return everythingOK ? 0 : 1;
}
