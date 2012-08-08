/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "ocinternal.h"
#include "ocdebug.h"
#include "ocdata.h"
#include "occontent.h"
#include "occlientparams.h"
#include "rc.h"
#include "curlfunctions.h"

#include "http.h"
#include "read.h"

/* Note: TMPPATH must end in '/' */
#ifdef __CYGWIN__
#define TMPPATH1 "c:/temp/"
#define TMPPATH2 "./"
#else
#define TMPPATH1 "/tmp/"
#define TMPPATH2 "./"
#endif

/* Define default rc files and aliases*/
static char* rcfilenames[3] = {".dodsrc",".httprc",NULL};

static int ocextractdds(OCstate*,OCtree*);
static char* constraintescape(const char* url);
#ifdef OC_DISK_STORAGE
static OCerror createtempfile(OCstate*,OCtree*);
static int createtempfile1(char*,char*);
#endif

static void ocsetcurlproperties(OCstate*);

extern OCnode* makeunlimiteddimension(void);

#ifdef WIN32
#include <fcntl.h>
#define _S_IREAD 256
#define _S_IWRITE 128
int mkstemp(char *tmpl)
{
   int ret=-1;

mktemp(tmpl); ret=open(tmpl,O_RDWR|O_BINARY|O_CREAT|O_EXCL|_O_SHORT_LIVED, _S_IREAD|_S_IWRITE);

   return ret;
}

#endif

/* Global flags*/
static int oc_big_endian; /* what is this machine? */
int oc_network_order; /* network order is big endian */
int oc_invert_xdr_double;
int oc_curl_file_supported;
int oc_curl_https_supported;

int
ocinternalinitialize(void)
{
    int stat = OC_NOERR;

    /* Compute if we are same as network order v-a-v xdr */
    
#ifdef XDRBYTEORDER
    {
        XDR xdrs;
        union {
            char tmp[sizeof(unsigned int)];
            unsigned int i;
        } u;
        int testint = 1;
        xrmem_create(&xdrs, (caddr_t)&u.tmp,sizeof(u.tmp), XDR_ENCODE);
        xdr_int(&xdrs, &testint);   
        oc_network_order = (udub.i == testint?1:0);
        oc_big_endian = oc_network_order;
    }
#else   
    {
        int testint = 0x00000001;
        char *byte = (char *)&testint;
        oc_big_endian = (byte[0] == 0 ? 1 : 0);
	oc_network_order = oc_big_endian;
    }

#endif /*XDRBYTEORDER*/
    {
        /* It turns out that various machines
           store double in different formats.
           This affects the conversion code ocdata.c
           when reconstructing; probably could deduce this
           from oc_big_endian, but not sure.
        */
        XDR xdrs;
	union {
	    double dv;
	    unsigned int i;
	    unsigned int iv[2];
	    char tmp[sizeof(double)];
	} udub;
        double testdub = 18000;
	/* verify double vs int size */
        if(sizeof(double) != (2 * sizeof(unsigned int)))
            ocpanic("|double| != 2*|int|");
        if(sizeof(udub) != sizeof(double))
            ocpanic("|double| != |udub|");
	memset((void*)&udub,0,sizeof(udub));
        xdrmem_create(&xdrs, (caddr_t)&udub.tmp,sizeof(udub.tmp), XDR_ENCODE);
        xdr_double(&xdrs, &testdub);
	udub.iv[0] = ocntoh(udub.iv[0]);
	udub.iv[1] = ocntoh(udub.iv[1]);
	if (udub.dv == testdub)
		oc_invert_xdr_double = 0;
	else { /* swap and try again */
	    unsigned int temp = udub.iv[0];
	    udub.iv[0] = udub.iv[1];
	    udub.iv[1] = temp;
	    if (udub.dv != testdub)
		ocpanic("cannot unpack xdr_double");
	    oc_invert_xdr_double = 1;
	}
    }
    oc_loginit();

    /* Determine if this version of curl supports
       "file://..." &/or "https://..." urls.
    */
    {
        const char* const* proto; /*weird*/
        curl_version_info_data* curldata;
        curldata = curl_version_info(CURLVERSION_NOW);
        oc_curl_file_supported = 0;
        oc_curl_https_supported = 0;
        for(proto=curldata->protocols;*proto;proto++) {
            if(strcmp("file",*proto)==0) {oc_curl_file_supported=1;break;}
            if(strcmp("https",*proto)==0) {oc_curl_https_supported=1;break;}
        }
        if(ocdebug > 0) {
            oc_log(LOGNOTE,"Curl file:// support = %d",oc_curl_file_supported);
            oc_log(LOGNOTE,"Curl https:// support = %d",oc_curl_file_supported);
        }
    }

    /* compile the .dodsrc, if any */
    {
        char* path = NULL;
        char* homepath = NULL;
	char** alias;
	FILE* f = NULL;
        /* locate the configuration files: . first, then $HOME */
	for(alias=rcfilenames;*alias;alias++) {
            path = (char*)malloc(strlen("./")+strlen(*alias)+1);
	    if(path == NULL) return OC_ENOMEM;
            strcpy(path,"./");
            strcat(path,*alias);
  	    /* see if file is readable */
	    f = fopen(path,"r");
	    if(f != NULL) break;
	    /* try $HOME */
            homepath = getenv("HOME");
            if (homepath!= NULL) {
		if(path != NULL) free(path);
	        path = (char*)malloc(strlen(homepath)+1+strlen(*alias)+1);
	        if(path == NULL) return OC_ENOMEM;
	        strcpy(path,homepath);
	        strcat(path,"/");
	        strcat(path,*alias);
		f = fopen(path,"r");
		if(f != NULL) break;
            }
	}
        if(f == NULL) {
            oc_log(LOGWARN,"Cannot find runtime configuration file");
	} else {
       	    fclose(f);
            if(ocdebug > 1)
		fprintf(stderr, "DODS RC file: %s\n", path);
            if(ocdodsrc_read(*alias,path) == 0)
	        oc_log(LOGERR, "Error parsing %s\n",path);
        }
        if(path != NULL) {free(path) ; path = NULL;}
    }
    return OCTHROW(stat);
}

/**************************************************/
OCerror
ocopen(OCstate** statep, const char* url)
{
    int stat = OC_NOERR;
    OCstate * state = NULL;
    OCURI* tmpurl;
    CURL* curl = NULL; /* curl handle*/

    if(!ocuriparse(url,&tmpurl)) {OCTHROWCHK(stat=OC_EBADURL); goto fail;}
    
    stat = occurlopen(&curl);
    if(stat != OC_NOERR) {OCTHROWCHK(stat); goto fail;}

    state = (OCstate*)ocmalloc(sizeof(OCstate)); /* ocmalloc zeros memory*/
    if(state == NULL) {OCTHROWCHK(stat=OC_ENOMEM); goto fail;}

    /* Setup DAP state*/
    state->magic = OCMAGIC;
    state->curl = curl;
    state->trees = oclistnew();
    state->contentlist = NULL;
    state->uri = tmpurl;
    if(!ocuridecodeparams(state->uri)) {
	oc_log(LOGWARN,"Could not parse client parameters");
    }
    state->packet = ocbytesnew();
    ocbytessetalloc(state->packet,DFALTPACKETSIZE); /*initial reasonable size*/

    /* set curl properties for this link */
    ocsetcurlproperties(state);

    if(statep) *statep = state;
    return OCTHROW(stat);   

fail:
    ocurifree(tmpurl);
    if(state != NULL) ocfree(state);
    if(curl != NULL) occurlclose(curl);
    return OCTHROW(stat);
}

OCerror
ocfetch(OCstate* state, const char* constraint, OCdxd kind, OCnode** rootp)
{
    OCtree* tree = NULL;
    OCnode* root = NULL;
    OCerror stat = OC_NOERR;
    
    tree = (OCtree*)ocmalloc(sizeof(OCtree));
    MEMCHECK(tree,OC_ENOMEM);
    memset((void*)tree,0,sizeof(OCtree));
    tree->dxdclass = kind;
    tree->state = state;
    tree->constraint = constraintescape(constraint);
    if(tree->constraint == NULL)
	tree->constraint = nulldup(constraint);

    /* Set curl properties: pwd, flags, proxies, ssl */
    if((stat=ocset_user_password(state))!= OC_NOERR) goto fail;
    if((stat=ocset_curl_flags(state)) != OC_NOERR) goto fail;
    if((stat=ocset_proxy(state)) != OC_NOERR) goto fail;
    if((stat=ocset_ssl(state)) != OC_NOERR) goto fail;

    ocbytesclear(state->packet);

    switch (kind) {
    case OCDAS:
        stat = readDAS(state,tree);
	if(stat == OC_NOERR) {
            tree->text = ocbytesdup(state->packet);
	    if(tree->text == NULL) stat = OC_EDAS;
	}
	break;
    case OCDDS:
        stat = readDDS(state,tree);
	if(stat == OC_NOERR) {
            tree->text = ocbytesdup(state->packet);
	    if(tree->text == NULL) stat = OC_EDDS;
	}
	break;
    case OCDATADDS:
#ifdef OC_DISK_STORAGE
       /* Create the datadds file immediately
           so that DRNO can reference it*/
        /* Make the tmp file*/
        stat = createtempfile(state,tree);
        if(stat) {OCTHROWCHK(stat); goto unwind;}
        stat = readDATADDS(state,tree);
	if(stat == OC_NOERR) {
            /* Separate the DDS from data and return the dds;
	       will modify packet */
            stat = ocextractdds(state,tree);
	}
#else
        stat = readDATADDS(state,tree);
	if(stat == OC_NOERR) {
            /* Separate the DDS from data*/
            stat = ocextractdds(state,tree);
	    tree->data.xdrdata = ocbytesdup(state->packet);
	}
#endif
	break;
    }
    if(stat != OC_NOERR) {
	/* Obtain any http code */
	state->error.httpcode = ocfetchhttpcode(state->curl);
	if(state->error.httpcode >= 400) {
	    oc_log(LOGWARN,"oc_open: Could not read url; http error = %l",state->error.httpcode);
	} else {
	    oc_log(LOGWARN,"oc_open: Could not read url");
	}
	return OCTHROW(stat);
    }

    tree->nodes = NULL;
    stat = DAPparse(state,tree,tree->text);
    /* Check and report on an error return from the server */
    if(stat == OC_EDAPSVC  && state->error.code != NULL) {
	oc_log(LOGERR,"oc_open: server error retrieving url: code=%s message=\"%s\"",
		  state->error.code,	
		  (state->error.message?state->error.message:""));
    }
    if(stat) {OCTHROWCHK(stat); goto unwind;}
    root = tree->root;
    /* make sure */
    tree->root = root;
    root->tree = tree;

    /* Verify the parse */
    switch (kind) {
    case OCDAS:
        if(root->octype != OC_Attributeset)
	    {OCTHROWCHK(stat=OC_EDAS); goto unwind;}
	break;
    case OCDDS:
        if(root->octype != OC_Dataset)
	    {OCTHROWCHK(stat=OC_EDDS); goto unwind;}
	break;
    case OCDATADDS:
        if(root->octype != OC_Dataset)
	    {OCTHROWCHK(stat=OC_EDATADDS); goto unwind;}
	/* Modify the tree kind */
	tree->dxdclass = OCDATADDS;
	break;
    default: return OC_EINVAL;
    }

    if(kind != OCDAS) {
        /* Process ocnodes to fix various semantic issues*/
        computeocsemantics(tree->nodes);
    }

    /* Process ocnodes to compute name info*/
    computeocfullnames(tree->root);

    if(kind != OCDAS) {
        /* Process ocnodes to compute sizes when uniform in size*/
        ocsetsize(tree->root);
    }

    if(kind == OCDATADDS) {
        tree->data.xdrs = (XDR*)ocmalloc(sizeof(XDR));
        MEMCHECK(tree->data.xdrs,OC_ENOMEM);
#ifdef OC_DISK_STORAGE
        ocxdrstdio_create(tree->data.xdrs,tree->data.file,XDR_DECODE);
#else
	xdrmem_create(tree->data.xdrs,tree->data.xdrdata,tree->data.datasize,XDR_DECODE);
#endif
        if(!xdr_setpos(tree->data.xdrs,tree->data.bod)) return xdrerror();
    }

#ifdef OC_DISK_STORAGE
    if(ocdebug == 0 && tree->data.filename != NULL) {
	unlink(tree->data.filename);
    }
#endif

    /* Put root into the state->trees list */
    oclistpush(state->trees,(ocelem)root);

    if(rootp) *rootp = root;
    return stat;

unwind:
    ocfreetree(tree);
fail:
    return OCTHROW(stat);
}

void
occlose(OCstate* state)
{
    unsigned int i;
    if(state == NULL) return;

    /* Warning: ocfreeroot will attempt to remove the root from state->trees */
    /* Ok in this case because we are popping the root out of state->trees */
    for(i=0;i<oclistlength(state->trees);i++) {
	OCnode* root = (OCnode*)oclistpop(state->trees);
	ocfreeroot(root);
    }
    oclistfree(state->trees);
    ocurifree(state->uri);
    ocbytesfree(state->packet);
    ocfree(state->error.code);
    ocfree(state->error.message);
    if(state->contentlist != NULL) {
	struct OCcontent* next;
	struct OCcontent* curr = state->contentlist;
	while(curr != NULL) {
	    next = curr->next;
	    ocfree(curr);
	    curr = next;
	}
    }
    ocfree(state->curlflags.useragent);
    ocfree(state->curlflags.cookiejar);
    ocfree(state->curlflags.cookiefile);
    ocfree(state->ssl.certificate);
    ocfree(state->ssl.key);
    ocfree(state->ssl.keypasswd);
    ocfree(state->ssl.cainfo);
    ocfree(state->ssl.capath); 
    ocfree(state->proxy.host);
    ocfree(state->creds.username);
    ocfree(state->creds.password);
    if(state->curl != NULL) occurlclose(state->curl);
    ocfree(state);
}

static OCerror
ocextractdds(OCstate* state, OCtree* tree)
{
    OCerror stat = OC_NOERR;
    size_t ddslen, bod, bodfound;
#ifdef OC_DISK_STORAGE
    /* Read until we find the separator (or EOF)*/
    ocbytesclear(state->packet);
    rewind(tree->data.file);
    do {
	char chunk[1024];
	size_t count;
	/* read chunks of the file until we find the separator*/
        count = fread(chunk,1,sizeof(chunk),tree->data.file);
	if(count <= 0) break; /* EOF;*/
        ocbytesappendn(state->packet,chunk,count);
	bodfound = findbod(state->packet,&bod,&ddslen);
    } while(!bodfound);
#else /*!OC_DISK_STORAGE*/
    /* Read until we find the separator (or EOF)*/
    bodfound = findbod(state->packet,&bod,&ddslen);
#endif
    if(!bodfound) {/* No BOD; pretend */
	bod = tree->data.bod;
	ddslen = tree->data.datasize;
    }
    tree->data.bod = bod;
    tree->data.ddslen = ddslen;
    /* copy out the dds */
    if(ddslen > 0) {
        tree->text = (char*)ocmalloc(ddslen+1);
        memcpy((void*)tree->text,(void*)ocbytescontents(state->packet),ddslen);
        tree->text[ddslen] = '\0';
    } else
	tree->text = NULL;
#ifdef OC_DISK_STORAGE
    /* reset the position of the tmp file*/
    fseek(tree->data.file,tree->data.bod,SEEK_SET);
#else
    /* If the data part is not on an 8 byte boundary, make it so */
    if(tree->data.bod % 8 != 0) {
        unsigned long count = tree->data.datasize - tree->data.bod;
	char* dst = ocbytescontents(state->packet);
	char* src = dst + tree->data.bod;
	int i;
	/* memcpy((void*)dst,(void*)src,count); overlap*/
	for(i=0;i<count;i++) dst[i] = src[i]; /* avoid memcpy overlap */
	tree->data.datasize = count;
	tree->data.bod = 0;
	tree->data.ddslen = 0;
    }
#endif
    if(tree->text == NULL) stat = OC_EDATADDS;
    return OCTHROW(stat);
}

#ifdef OC_DISK_STORAGE
static OCerror
createtempfile(OCstate* state, OCtree* tree)
{
    int fd,slen;
    char* name;
    slen = strlen(TMPPATH1);
    if(slen < strlen(TMPPATH2)) slen = strlen(TMPPATH2);
    slen += strlen("datadds") + strlen("XXXXXX");
    name = (char*)ocmalloc(slen+1);
    MEMCHECK(name,OC_ENOMEM);
    fd = createtempfile1(name, TMPPATH1);
    if(fd < 0)
        fd = createtempfile1(name, TMPPATH2);
    if(fd < 0) {
        oc_log(LOGERR,"oc_open: attempt to open tmp file %s failed",name);
        return errno;
    }
    oc_log(LOGNOTE,"oc_open: using tmp file: %s",name);
    tree->data.filename = name; /* remember our tmp file name */
    tree->data.file = fdopen(fd,"w+");
    if(tree->data.file == NULL) return OC_EOPEN;
    /* unlink the temp file so it will automatically be reclaimed */
    if(ocdebug == 0) unlink(tree->data.filename);
    return OC_NOERR;
}

int
createtempfile1(char* name, char* tmppath)
{
    char* p;
    int c,fd;
    strcpy(name,tmppath);
    strcat(name,"datadds");
    strcat(name,"XXXXXX");
    p = name + strlen("datadds");
    /* \', and '/' to '_' and '.' to '-'*/
    for(;(c=*p);p++) {
        if(c == '\\' || c == '/') {*p = '_';}
        else if(c == '.') {*p = '-';}
    }
    /* Note Potential problem: old versions of this function
       leave the file in mode 0666 instead of 0600 */
    fd = mkstemp(name);
    return fd;
}
#endif /*OC_DISK_STORAGE*/

/* Allow these (non-alpha-numerics) to pass thru */
static char okchars[] = "&/:;,.=?@'\"<>{}!|\\^[]`~";
static char hexdigits[] = "0123456789abcdef";


/* Modify constraint to use %XX escapes */
static char*
constraintescape(const char* url)
{
    size_t len;
    char* p;
    int c;
    char* eurl;

    if(url == NULL) return NULL;
    len = strlen(url);
    eurl = ocmalloc(1+3*len); /* worst case: c -> %xx */
    MEMCHECK(eurl,NULL);
    p = eurl;
    *p = '\0';
    while((c=*url++)) {
	if(c >= '0' && c <= '9') {*p++ = c;}
	else if(c >= 'a' && c <= 'z') {*p++ = c;}
	else if(c >= 'A' && c <= 'Z') {*p++ = c;}
	else if(strchr(okchars,c) != NULL) {*p++ = c;}
	else {
	    *p++ = '%';
	    *p++ = hexdigits[(c & 0xf0)>>4];
	    *p++ = hexdigits[(c & 0xf)];
	}
    }
    *p = '\0';
    return eurl;
}

OCerror
ocupdatelastmodifieddata(OCstate* state)
{
    OCerror status = OC_NOERR;
    long lastmodified;
    char* base = NULL;
    base = ocuribuild(state->uri,NULL,NULL,0);
    status = ocfetchlastmodified(state->curl, base, &lastmodified);
    free(base);
    if(status == OC_NOERR) {
	state->datalastmodified = lastmodified;
    }
    return status;
}

/*
    Set curl properties for link based on rc files
*/
static void
ocsetcurlproperties(OCstate* state)
{
    CURLcode cstat = CURLE_OK;

    /* process the triple store wrt to this state */
    if(ocdodsrc_process(state) != OC_NOERR) {
	oc_log(LOGERR,"Malformed .opendaprc configuration file");
	goto fail;
    }
    if(state->creds.username == NULL && state->creds.password == NULL) {
        if(state->uri->user != NULL && state->uri->password != NULL) {
	    /* this overrides .dodsrc */
            if(state->creds.password) free(state->creds.password);
            state->creds.password = nulldup(state->uri->password);
            if(state->creds.username) free(state->creds.username);
            state->creds.username = nulldup(state->uri->user);
	}
    }
    return;

fail:
    if(cstat != CURLE_OK)
	oc_log(LOGERR, "curl error: %s", curl_easy_strerror(cstat));
    return;
}
