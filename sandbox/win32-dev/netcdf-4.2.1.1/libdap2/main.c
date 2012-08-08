/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/ncdap3.c,v 1.94 2010/05/28 01:05:34 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "dapdump.h"

#define LIMIT 1000

int verbose;
char* fileurl;
int debug;
extern int ocdebug;

/* Forward */
static void usage(void);
static int fail(void);
static void check_err(int stat);
static void init(void);
static void dumpflags(void);
static void dumpchars(size_t nelems, char* memory);

int
main(int argc, char **argv)
{
    int c, stat;
    int ncid;
    NChdr* hdr;

    init();

    opterr=1;
    while ((c = getopt(argc, argv, "dvau:D:l:p:c:t:R:")) != EOF) {
      switch(c) {
      case 'v': verbose=1; break;
      case 'd': debug=1; break;
      case 'D': debug=atoi(optarg); break;
      default: break;
      }
    }

    if(debug > 0) {ocdebug = debug;}

    argc -= optind;
    argv += optind;

    if(argc > 0 && fileurl == NULL)
	fileurl = nulldup(argv[0]);

    if(fileurl == NULL) fileurl = getenv("FILEURL");

    if(fileurl == NULL) {
	fprintf(stderr,"no file url specified\n");
	usage();
    }

    if(verbose) dumpflags();

    if(verbose) {fprintf(stdout,"initializing\n"); fflush(stdout);}

    stat = nc_open(fileurl,NC_NOWRITE,&ncid);
    check_err(stat);

    /* dump meta data */
    stat = dumpmetadata(ncid,&hdr);
    check_err(stat);

    /* Get data */
if(1)
    {
	int i,j,limited;
	size_t start[NC_MAX_VAR_DIMS];
	size_t count[NC_MAX_VAR_DIMS];
	/* Walk each variable */
	for(i=0;i<hdr->nvars;i++) {
	    Var* var = &hdr->vars[i];
	    size_t nelems = 1;
	    limited = 0;
	    for(j=0;j<var->ndims;j++) {
		start[j] = 0;
		assert(var->dimids[j] == hdr->dims[var->dimids[j]].dimid);
		count[j] = hdr->dims[var->dimids[j]].size;
		/* put a limit on each count */
	        if(count[j] > LIMIT) {count[j] = LIMIT; limited = 1;}
		nelems *= count[j];
	    }
	    if(nelems > 0) {
		size_t typesize = nctypesizeof(var->nctype);
		size_t memsize = typesize*nelems;
	        void* memory = malloc(memsize);
	        fprintf(stdout,"%s: size=%lu (%lu * %lu) ; ",
			var->name, (unsigned long)memsize, (unsigned long)typesize, (unsigned long)nelems);
		fflush(stdout);
	        stat = nc_get_vara(ncid,var->varid,start,count,memory);
	        check_err(stat);
		/* dump memory */
		switch (var->nctype){
		case NC_CHAR:
	            fprintf(stdout,"\""); fflush(stdout);
		    dumpchars(nelems,memory);
	            fprintf(stdout,"\"");
		    break;		    
		default:
		    for(j=0;j<nelems;j++) {
		        if(j > 0) fprintf(stdout," ");
		        dumpdata1(var->nctype,j,memory);
		    }
		}
		if(limited) fprintf(stdout,"...");
	        fprintf(stdout,"\n");
		free(memory);
	    } else
		fprintf(stdout,"%s: no data\n",var->name);
	}
        fprintf(stdout,"\n");	
    }

    nc_close(ncid);
    return 0;
}

static void
dumpchars(size_t nelems, char* memory)
{
    int i,c;
    char* q;
    char* p = (char*)malloc(nelems+1);
    memcpy(p,memory,nelems);
    p[nelems] = 0;
    for(q=p,i=0;i<nelems;i++,q++) {if(*q == 0) {*q=' ';}}

    for(q=p,i=0;(c=*q++);) {
	if(c == ' ') {
	    char* s = q;
	    ptrdiff_t len = 0;
	    while((c=*s) == ' ') s++;
	    len = (s - q);
	    if(len >= 6) {
		fprintf(stdout,"...");
	    } else {
		while(len-- >= 0) fprintf(stdout," ");
	    }
	    q = s;		
	} else if(c > ' ' && c < 127) {
	    fprintf(stdout,"%c",c);
	} else switch(c) {
	    case '\n': fprintf(stdout,"\\n"); break;
	    case '\r': fprintf(stdout,"\\r"); break;
	    case '\t': fprintf(stdout,"\\t"); break;
	    case '\f': fprintf(stdout,"\\f"); break;	
	    default: fprintf(stdout,"0x%2x",c);
	}
    }
}



static void
init(void)
{
    debug = 0;
    verbose=0;
    fileurl = NULL;
}

static void
dumpflags(void)
{
    fprintf(stderr,"url=%s\n",fileurl);
    if(verbose) fprintf(stderr,"verbose=%d\n",verbose);
    if(debug) fprintf(stderr,"debug=%d\n",debug);
}

static void
usage(void)
{
    fprintf(stderr,"usage: test [-t] [-d] [-oc] [-nd] [-r] [-u] <fileurl>\n");
    fail();
}

static void
check_err(int stat)
{
    if(stat == NC_NOERR) return;
    fprintf(stderr,"error status returned: (%d) %s\n",stat,
	nc_strerror(stat)
	);
    fail();
}

static int
fail(void)
{
    fflush(stdout); fflush(stderr);
    exit(1);
    return 0;
}

