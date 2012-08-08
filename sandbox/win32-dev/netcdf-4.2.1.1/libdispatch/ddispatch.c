#include "ncdispatch.h"
#include "nc_uri.h"

extern int NCSUBSTRATE_intialize(void);

/* Define vectors of zeros and ones for use with various nc_get_varX function*/
size_t nc_sizevector0[NC_MAX_DIMS];
size_t nc_sizevector1[NC_MAX_DIMS];
ptrdiff_t nc_ptrdiffvector1[NC_MAX_DIMS];

/* Define the known protocols and their manipulations */
static struct NCPROTOCOLLIST {
    char* protocol;
    char* substitute;
    int   modelflags;
} ncprotolist[] = {
    {"http",NULL,0},
    {"https",NULL,0},
    {"file",NULL,NC_DISPATCH_NCD},
    {"dods","http",NC_DISPATCH_NCD},
    {"dodss","https",NC_DISPATCH_NCD},
    {"cdmr","http",NC_DISPATCH_NCR|NC_DISPATCH_NC4},
    {"cdmrs","https",NC_DISPATCH_NCR|NC_DISPATCH_NC4},
    {"cdmremote","http",NC_DISPATCH_NCR|NC_DISPATCH_NC4},
    {"cdmremotes","https",NC_DISPATCH_NCR|NC_DISPATCH_NC4},
    {NULL,NULL,0} /* Terminate search */
};

/* Define the server to ping in order;
   make the order attempt to optimize
   against future changes.
*/
static const char* servers[] = {
"http://motherlode.ucar.edu:8081", /* try this first */
"http://remotetest.unidata.ucar.edu",
"http://remotetest.ucar.edu",
"http://motherlode.ucar.edu:8080",
"http://motherlode.ucar.edu",
"http://remotetests.unidata.ucar.edu",
"http://remotetests.ucar.edu",
NULL
};

/*
static nc_type longtype = (sizeof(long) == sizeof(int)?NC_INT:NC_INT64);
static nc_type ulongtype = (sizeof(unsigned long) == sizeof(unsigned int)?NC_UINT:NC_UINT64);
*/

/* Allow dispatch to do initialization */
int
NCDISPATCH_initialize(void)
{
    extern int NCSUBSTRATE_initialize(void);
    int i;
    NCSUBSTRATE_initialize();
    for(i=0;i<NC_MAX_VAR_DIMS;i++) {
	nc_sizevector0[i] = 0;
        nc_sizevector1[i] = 1;
        nc_ptrdiffvector1[i] = 1;
    }
    return NC_NOERR;
}

/* search list of servers and return first that succeeds when
   concatenated with the specified path part
*/
const char*
NC_findtestserver(const char* path)
{
#ifdef USE_DAP
    /* NCDAP_ping is defined in libdap2/ncdap3.c */
    const char** svc;
    if(path == NULL) path = "";
    for(svc=servers;*svc != NULL;svc++) {
        int stat;
        char url[4096];
	snprintf(url,sizeof(url),"%s%s%s",
			*svc,
			(path[0] == '/' ? "" : "/"),
			path);
	stat = NCDAP_ping(url);
	if(stat == NC_NOERR)
	    return *svc;
    }
#endif
    return NULL;
}


/* return 1 if path looks like a url; 0 otherwise */
int
NC_testurl(const char* path)
{
    int isurl = 0;
    NC_URI* tmpurl = NULL;
    char* p;

    if(path == NULL) return 0;

    /* find leading non-blank */
    for(p=(char*)path;*p;p++) {if(*p != ' ') break;}

    /* Do some initial checking to see if this looks like a file path */
    if(*p == '/') return 0; /* probably an absolute file path */

    /* Ok, try to parse as a url */
    if(nc_uriparse(path,&tmpurl)) {
	/* Do some extra testing to make sure this really is a url */
        /* Look for a knownprotocol */
        struct NCPROTOCOLLIST* protolist;
        for(protolist=ncprotolist;protolist->protocol;protolist++) {
	    if(strcmp(tmpurl->protocol,protolist->protocol) == 0) {
	        isurl=1;
		break;
	    }		
	}
	nc_urifree(tmpurl);
	return isurl;
    }
    return 0;
}

/*
Return the OR of some of the NC_DISPATCH flags
Assumes that the path is known to be a url
*/

int
NC_urlmodel(const char* path)
{
    int model = 0;
    NC_URI* tmpurl = NULL;
    struct NCPROTOCOLLIST* protolist;

    if(!nc_uriparse(path,&tmpurl)) goto done;

    /* Look at any prefixed parameters */
    if(nc_urilookup(tmpurl,"netcdf4",NULL)
       || nc_urilookup(tmpurl,"netcdf-4",NULL)) {
	model = (NC_DISPATCH_NC4|NC_DISPATCH_NCD);
    } else if(nc_urilookup(tmpurl,"netcdf3",NULL)
              || nc_urilookup(tmpurl,"netcdf-3",NULL)) {
	model = (NC_DISPATCH_NC3|NC_DISPATCH_NCD);
    } else if(nc_urilookup(tmpurl,"cdmremote",NULL)
	      || nc_urilookup(tmpurl,"cdmr",NULL)) {
	model = (NC_DISPATCH_NCR|NC_DISPATCH_NC4);
    }

    if(model == 0) {
        /* Now look at the protocol */
        for(protolist=ncprotolist;protolist->protocol;protolist++) {
	    if(strcmp(tmpurl->protocol,protolist->protocol) == 0) {
    	        model |= protolist->modelflags;
    	        if(protolist->substitute) {
    	            if(tmpurl->protocol) free(tmpurl->protocol);
    		    tmpurl->protocol = strdup(protolist->substitute);
    	        }
    	        break;	    
	    }
	}
    }	

    /* Force NC_DISPATCH_NC3 if necessary */
    if((model & NC_DISPATCH_NC4) == 0)
	model |= (NC_DISPATCH_NC3 | NC_DISPATCH_NCD);

done:
    nc_urifree(tmpurl);
    return model;
}

/* Override dispatch table management */
static NC_Dispatch* NC_dispatch_override = NULL;

/* Override dispatch table management */
NC_Dispatch*
NC_get_dispatch_override(void) {
    return NC_dispatch_override;
}

void NC_set_dispatch_override(NC_Dispatch* d)
{
    NC_dispatch_override = d;
}

/* Overlay by treating the tables as arrays of void*.
   Overlay rules are:
        overlay    base    merge
        -------    ----    -----
          null     null     null
          null      y        y
           x       null      x
           x        y        x
*/

int
NC_dispatch_overlay(const NC_Dispatch* overlay, const NC_Dispatch* base, NC_Dispatch* merge)
{
    void** voverlay = (void**)overlay;
    void** vmerge;
    int i, count = sizeof(NC_Dispatch) / sizeof(void*);
    /* dispatch table must be exact multiple of sizeof(void*) */
    assert(count * sizeof(void*) == sizeof(NC_Dispatch));
    *merge = *base;
    vmerge = (void**)merge;
    for(i=0;i<count;i++) {
        if(voverlay[i] == NULL) continue;
        vmerge[i] = voverlay[i];
    }
    /* Finally, the merge model should always be the overlay model */
    merge->model = overlay->model;
    return NC_NOERR;
}
