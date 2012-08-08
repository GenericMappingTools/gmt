#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netcdf.h"
#include "ncdispatch.h"

int
main(int argc, char** argv)
{
    const char* url = NULL;
    if(argc == 1) {
	fprintf(stderr,"no path specified");
	printf("%s","");
	exit(1);
    }
    url = NC_findtestserver(argv[1]);
    if(url == NULL)
	url = "";
    printf("%s",url);
    fflush(stdout);
    exit(0);
}


