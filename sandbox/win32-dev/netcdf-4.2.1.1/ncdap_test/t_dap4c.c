#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>

#define VAR "i32"

#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

#undef DEBUG

int
main()
{
    int ncid, varid;
    int retval;
    int i32[100];
    size_t start[1];
    size_t count[1];
    int ok = 1;    

    char* topsrcdir;
    char url[4096];

    /* Assume that TESTS_ENVIRONMENT was set */
    topsrcdir = getenv("TOPSRCDIR");
    if(topsrcdir == NULL) {
        fprintf(stderr,"*** FAIL: $abs_top_srcdir not defined: location= %s:%d\n",__FILE__,__LINE__);
        exit(1);
    }    
    strcpy(url,"file://");
    strcat(url,topsrcdir);
    strcat(url,"/ncdap_test/testdata3/test.02");

    if ((retval = nc_open(url, 0, &ncid)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid, VAR, &varid)))
       ERR(retval);

    start[0] = 0;
    count[0] = 26;
    if ((retval = nc_get_vara_int(ncid, varid, start, count, i32)))
    if(retval != NC_EINVALCOORDS) {
	printf("nc_get_vara_int did not return NC_EINVALCOORDS");
	ok = 0;
    }

    nc_close(ncid);

    printf(ok?"*** PASS\n":"*** FAIL\n");
    return 0;
}
