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
#include "occlientparams.h"
#include "ocrc.h"
#include "occurlfunctions.h"
#include "ochttp.h"
#include "ocread.h"
#include "dapparselex.h"

/* Note: TMPPATH must end in '/' */
#ifdef __CYGWIN__
#define TMPPATH1 "/cygdrive/c/temp/"
#define TMPPATH2 "./"
#elif WIN32
#define TMPPATH1 "c:\\temp\\"
#define TMPPATH2 ".\\"
#else
#define TMPPATH1 "/tmp/"
#define TMPPATH2 "./"
#endif

/* Define default rc files and aliases*/
static char* rcfilenames[4] = {".daprc",".dodsrc",".ocrc",NULL};

static int ocextractddsinmemory(OCstate*,OCtree*,int);
static int ocextractddsinfile(OCstate*,OCtree*,int);
static char* constraintescape(const char* url);
static OCerror createtempfile(OCstate*,OCtree*);
static int createtempfile1(char*,char**);

static void ocsetcurlproperties(OCstate*);

extern OCnode* makeunlimiteddimension(void);

#ifdef WIN32
#include <fcntl.h>
#define _S_IREAD 256
#define _S_IWRITE 128
#else
#include <sys/stat.h>
#endif

/* Collect global state info in one place */
struct OCGLOBALSTATE ocglobalstate;

int
ocinternalinitialize(void)
{
    int stat = OC_NOERR;

    if(!ocglobalstate.initialized) {
        memset((void*)&ocglobalstate,0,sizeof(ocglobalstate));
	ocglobalstate.initialized = 1;
    }

    /* Compute some xdr related flags */
    xxdr_init();

    oc_loginit();

    oc_curl_protocols(&ocglobalstate); /* see what protocols are supported */

    /* compile the .dodsrc, if any */
    {
        char* path = NULL;
        char* homepath = NULL;
	char** alias;
	FILE* f = NULL;
        /* locate the configuration files: . first in '.',  then $HOME */
	for(alias=rcfilenames;*alias;alias++) {
            path = (char*)malloc(strlen("./")+strlen(*alias)+1);
	    if(path == NULL) return OC_ENOMEM;
            strcpy(path,"./");
            strcat(path,*alias);
  	    /* see if file is readable */
	    f = fopen(path,"r");
	    if(f != NULL) break;
    	    if(path != NULL) {free(path); path = NULL;} /* cleanup */
	}
	if(f == NULL) { /* try $HOME */
	    OCASSERT(path == NULL);
            homepath = getenv("HOME");
            if (homepath!= NULL) {
	        for(alias=rcfilenames;*alias;alias++) {
	            path = (char*)malloc(strlen(homepath)+1+strlen(*alias)+1);
	            if(path == NULL) return OC_ENOMEM;
	            strcpy(path,homepath);
	            strcat(path,"/");
	            strcat(path,*alias);
		    f = fopen(path,"r");
		    if(f != NULL) break;
 	            if(path != NULL) {free(path); path=NULL;}
		}
            }
	}
        if(f == NULL) {
            oc_log(LOGDBG,"Cannot find runtime configuration file");
	} else {
	    OCASSERT(path != NULL);
       	    fclose(f);
            if(ocdebug > 1)
		fprintf(stderr, "DODS RC file: %s\n", path);
            if(ocdodsrc_read(*alias,path) == 0)
	        oc_log(LOGERR, "Error parsing %s\n",path);
        }
        if(path != NULL) free(path);
    }
    return OCTHROW(stat);
}

/**************************************************/
OCerror
ocopen(OCstate** statep, const char* url)
{
    int stat = OC_NOERR;
    OCstate * state = NULL;
    OCURI* tmpurl = NULL;
    CURL* curl = NULL; /* curl handle*/

    if(!ocuriparse(url,&tmpurl)) {OCTHROWCHK(stat=OC_EBADURL); goto fail;}
    
    stat = occurlopen(&curl);
    if(stat != OC_NOERR) {OCTHROWCHK(stat); goto fail;}

    state = (OCstate*)ocmalloc(sizeof(OCstate)); /* ocmalloc zeros memory*/
    if(state == NULL) {OCTHROWCHK(stat=OC_ENOMEM); goto fail;}

    /* Setup DAP state*/
    state->header.magic = OCMAGIC;
    state->header.occlass = OC_State;
    state->curl = curl;
    state->trees = oclistnew();
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
ocfetch(OCstate* state, const char* constraint, OCdxd kind, OCflags flags,
        OCnode** rootp)
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
	if((flags & OCONDISK) != 0) {/* store in file */
	    /* Create the datadds file immediately
               so that DRNO can reference it*/
            /* Make the tmp file*/
            stat = createtempfile(state,tree);
            if(stat) {OCTHROWCHK(stat); goto fail;}
            stat = readDATADDS(state,tree,flags);
	    if(stat == OC_NOERR) {
                /* Separate the DDS from data and return the dds;
                   will modify packet */
                stat = ocextractddsinfile(state,tree,flags);
	    }
	} else { /*inmemory*/
            stat = readDATADDS(state,tree,flags);
	    if(stat == OC_NOERR) {
                /* Separate the DDS from data and return the dds;
               will modify packet */
            stat = ocextractddsinmemory(state,tree,flags);
	}
	}
	break;
    default:
	break;
    }/*switch*/
    if(stat != OC_NOERR) {
	/* Obtain any http code */
	state->error.httpcode = ocfetchhttpcode(state->curl);
	if(state->error.httpcode >= 400) {
	    oc_log(LOGWARN,"oc_open: Could not read url; http error = %l",state->error.httpcode);
	} else {
	    oc_log(LOGWARN,"oc_open: Could not read url");
	}
	goto fail;
    }

    tree->nodes = NULL;
    stat = DAPparse(state,tree,tree->text);
    /* Check and report on an error return from the server */
    if(stat == OC_EDAPSVC  && state->error.code != NULL) {
	oc_log(LOGERR,"oc_open: server error retrieving url: code=%s message=\"%s\"",
		  state->error.code,	
		  (state->error.message?state->error.message:""));
    }
    if(stat) {OCTHROWCHK(stat); goto fail;}
    root = tree->root;
    /* make sure */
    tree->root = root;
    root->tree = tree;

    /* Verify the parse */
    switch (kind) {
    case OCDAS:
        if(root->octype != OC_Attributeset)
	    {OCTHROWCHK(stat=OC_EDAS); goto fail;}
	break;
    case OCDDS:
        if(root->octype != OC_Dataset)
	    {OCTHROWCHK(stat=OC_EDDS); goto fail;}
	break;
    case OCDATADDS:
        if(root->octype != OC_Dataset)
	    {OCTHROWCHK(stat=OC_EDATADDS); goto fail;}
	/* Modify the tree kind */
	tree->dxdclass = OCDATADDS;
	break;
    default: return OC_EINVAL;
    }

    if(kind != OCDAS) {
        /* Process ocnodes to mark those that are cacheable */
        ocmarkcacheable(state,root);
        /* Process ocnodes to handle various semantic issues*/
        occomputesemantics(tree->nodes);
    }

    /* Process ocnodes to compute name info*/
    occomputefullnames(tree->root);

     if(kind == OCDATADDS) {
	if((flags & OCONDISK) != 0) {
            tree->data.xdrs = xxdr_filecreate(tree->data.file,tree->data.bod);
	} else {
	    /* Switch to zero based memory */
            tree->data.xdrs
		= xxdr_memcreate(tree->data.memory,tree->data.datasize,tree->data.bod);
	}
        MEMCHECK(tree->data.xdrs,OC_ENOMEM);
	/* Compile the data into a more accessible format */
	stat = occompile(state,tree->root);
	if(stat != OC_NOERR)
	    goto fail;
    }

    /* Put root into the state->trees list */
    oclistpush(state->trees,(ocelem)root);

    if(rootp) *rootp = root;
    return stat;

fail:
    if(root != NULL)
	ocroot_free(root);
    else if(tree != NULL)
	octree_free(tree);
    return OCTHROW(stat);
}

static OCerror
createtempfile(OCstate* state, OCtree* tree)
{
    int fd = 0;
    char* name = NULL;
    fd = createtempfile1(TMPPATH1,&name);
    if(fd < 0)
        fd = createtempfile1(TMPPATH2,&name);
    if(fd < 0) {
        oc_log(LOGERR,"oc_open: attempt to open tmp file failed: %s",name);
        return errno;
    }
#ifdef OCDEBUG
    oc_log(LOGNOTE,"oc_open: using tmp file: %s",name);
#endif
    tree->data.filename = name; /* remember our tmp file name */
    tree->data.file = fdopen(fd,"w+");
    if(tree->data.file == NULL) return OC_EOPEN;
    /* unlink the temp file so it will automatically be reclaimed */
    if(ocdebug == 0) unlink(tree->data.filename);
    return OC_NOERR;
}

int
createtempfile1(char* tmppath, char** tmpnamep)
{
    int fd = 0;
    char* tmpname = NULL;
    tmpname = (char*)malloc(strlen(tmppath)+strlen("dataddsXXXXXX")+1);
    if(tmpname == NULL) return -1;
    strcpy(tmpname,tmppath);
#ifdef HAVE_MKSTEMP
    strcat(tmpname,"dataddsXXXXXX");
    /* Note Potential problem: old versions of this function
       leave the file in mode 0666 instead of 0600 */
    fd = mkstemp(tmpname);
#else /* !HAVE_MKSTEMP */
    /* Need to simulate by using some kind of pseudo-random number */
    strcat(tmpname,"datadds");
    {
	int rno = rand();
	char spid[7];
	if(rno < 0) rno = -rno;
        sprintf(spid,"%06d",rno);
        strcat(tmpname,spid);
#  ifdef WIN32
        fd=open(tmpname,O_RDWR|O_BINARY|O_CREAT|O_EXCL|FILE_ATTRIBUTE_TEMPORARY, _S_IREAD|_S_IWRITE);
#  else
        fd=open(tmpname,O_RDWR|O_CREAT|O_EXCL, S_IRWXU);
#  endif
    }
#endif /* !HAVE_MKSTEMP */
    if(tmpname == NULL) return -1;
    if(tmpnamep) *tmpnamep = tmpname;
    return fd;
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
	ocroot_free(root);
    }
    oclistfree(state->trees);
    ocurifree(state->uri);
    ocbytesfree(state->packet);
    ocfree(state->error.code);
    ocfree(state->error.message);
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
ocextractddsinmemory(OCstate* state, OCtree* tree, OCflags flags)
{
    OCerror stat = OC_NOERR;
    size_t ddslen, bod, bodfound;
    /* Read until we find the separator (or EOF)*/
    bodfound = ocfindbod(state->packet,&bod,&ddslen);
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
    /* Extract the inmemory contents */
    tree->data.memory = ocbytesextract(state->packet);
#ifdef OCIGNORE
    /* guarantee the data part is on an 8 byte boundary */
    if(tree->data.bod % 8 != 0) {
        unsigned long count = tree->data.datasize - tree->data.bod;
        memcpy(tree->xdrmemory,tree->xdrmemory+tree->data.bod,count);
        tree->data.datasize = count;
	tree->data.bod = 0;
	tree->data.ddslen = 0;
    }
#endif
    if(tree->text == NULL) stat = OC_EDATADDS;
    return OCTHROW(stat);
}

static OCerror
ocextractddsinfile(OCstate* state, OCtree* tree, OCflags flags)
{
    OCerror stat = OC_NOERR;
    size_t ddslen, bod, bodfound;

    /* Read until we find the separator (or EOF)*/
    ocbytesclear(state->packet);
    rewind(tree->data.file);
    bodfound = 0;
    do {
        char chunk[1024];
	size_t count;
	/* read chunks of the file until we find the separator*/
        count = fread(chunk,1,sizeof(chunk),tree->data.file);
	if(count <= 0) break; /* EOF;*/
        ocbytesappendn(state->packet,chunk,count);
	bodfound = ocfindbod(state->packet,&bod,&ddslen);
    } while(!bodfound);
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
    /* reset the position of the tmp file*/
    fseek(tree->data.file,tree->data.bod,SEEK_SET);
    if(tree->text == NULL) stat = OC_EDATADDS;
    return OCTHROW(stat);
}

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
    base = ocuribuild(state->uri,NULL,NULL,OCURIENCODE);
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
