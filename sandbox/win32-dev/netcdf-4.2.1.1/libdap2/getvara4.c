#include "ncdap4.h"
#include "dapodom.h"
#include "dapalign.h"
#ifdef DEBUG
#include "dapdump.h"
#endif
#include "ncd3dispatch.h"

static NCerror getcontent4(NCDAPCOMMON*, Getvara*, CDFnode* rootnode, void* data);
static NCerror getcontent4r(NCDAPCOMMON*, Getvara*, CDFnode* tnode, OCdata, NCbytes*);
static NCerror getcontent4prim(NCDAPCOMMON* dapcomm, Getvara*, CDFnode* tnode, DCEsegment*,
		               OCdata currentcontent, NCbytes* memory);
static int findfield(CDFnode* node, CDFnode* subnode);
static int contiguousdims(Dapodometer* odom);
static NCerror ncdap4convert(nc_type srctype, nc_type dsttype, char* memory, char* value, size_t);
static void makewholesegment4(DCEsegment* seg, NClist* dimset);




/*
See the comment preceding nc3d_getvarx
to understand how constraints are handled.
*/
int 
NCD4_get_vara(int ncid, int varid,
	      const size_t* startp,
	      const size_t* countp,
	      const ptrdiff_t* stridep,
	      void* data,
	      nc_type externaltype0)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    NC* drno;
    NCDAPCOMMON* dapcomm;
    CDFnode* cdfvar; /* cdf node mapping to var*/
    NClist* varnodes;
    Getvara* varainfo = NULL;
    CDFnode* xtarget = NULL;
    CDFnode* target = NULL;
    DCEprojection* varaprojection = NULL;
    NCcachenode* cachenode = NULL;
    nc_type externaltype = externaltype0;
    size_t localcount[NC_MAX_VAR_DIMS];
    NClist* ncdimsall;
    size_t ncrank;
    NClist* vars = NULL;
    DCEconstraint* fetchconstraint = NULL;
    DCEprojection* fetchprojection = NULL;
    DCEprojection* walkprojection = NULL;
    int state;
#define FETCHWHOLE 1 /* fetch whole data set */
#define FETCHPART  2 /* fetch constrained variable */
#define CACHED     4 /* whole variable is already in the cache */

    LOG((2, "nc_get_vara: ncid 0x%x varid %d", ncid, varid));

    ncstat = NC_check_id(ncid, (NC**)&drno); 
    if(ncstat != NC_NOERR) goto fail;
    dapcomm = (NCDAPCOMMON*)drno->dispatchdata;

    /* Find cdfnode corresponding to the var.*/
    varnodes = dapcomm->cdf.varnodes;
    cdfvar = NULL;
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(varnodes,i);
	if(node->ncid == varid) {
	    cdfvar = node;
	    break;
	}
    }
    ASSERT((cdfvar != NULL));

    /* Get the dimension info */
    ncdimsall = cdfvar->array.dimset0;
    ncrank = nclistlength(ncdimsall);

#ifdef DEBUG
 {
int i;
fprintf(stderr,"getvarx: %s",cdfvar->ncfullname);
for(i=0;i<ncrank;i++)
  fprintf(stderr,"[%ld:%ld:%ld]",
	(long)startp[i],
	(long)countp[i],
	(long)stridep[i]
	);
fprintf(stderr,"\n");
 }
#endif

    /* Fill in missing arguments */
    if(startp == NULL)
	startp = nc_sizevector0;

    if(countp == NULL) {
        /* Accumulate the dimension sizes */
        for(i=0;i<ncrank;i++) {
	    CDFnode* dim = (CDFnode*)nclistget(ncdimsall,i);
	    localcount[i] = dim->dim.declsize;
	}
	countp = localcount;
    }

    if(stridep == NULL)
	stridep = nc_ptrdiffvector1;

    /* Validate the dimension sizes */
    for(i=0;i<ncrank;i++) {
        CDFnode* dim = (CDFnode*)nclistget(ncdimsall,i);
	if(startp[i] > dim->dim.declsize
	   || startp[i]+countp[i] > dim->dim.declsize) {
	    ncstat = NC_EINVALCOORDS;
	    goto fail;	    
	}
    }	     

#ifdef DEBUG
 { NClist* dims = cdfvar->array.dimset0;
fprintf(stderr,"getvarx: %s/%d",cdfvar->ncfullname,(int)nclistlength(dims));
if(nclistlength(dims) > 0) {int i;
for(i=0;i<nclistlength(dims);i++) 
fprintf(stderr,"[%lu:%lu:%lu]",(unsigned long)startp[i],(unsigned long)countp[i],(unsigned long)stridep[i]);
fprintf(stderr," -> ");
for(i=0;i<nclistlength(dims);i++) 
if(stridep[i]==1)
fprintf(stderr,"[%lu:%lu]",(unsigned long)startp[i],(unsigned long)((startp[i]+countp[i])-1));
else
fprintf(stderr,"[%lu:%lu:%lu]",
(unsigned long)startp[i],
(unsigned long)stridep[i],
(unsigned long)(((startp[i]+countp[i])*stridep[i])-1));
}
fprintf(stderr,"\n");
 }
#endif

    /* Default to using the var type */
    if(externaltype == NC_NAT) externaltype = cdfvar->externaltype;

    /* Validate any implied type conversion*/
    if(cdfvar->nctype == NC_Primitive
       && cdfvar->etype != externaltype && externaltype == NC_CHAR) {
	/* The only disallowed conversion is to/from char and non-byte
           numeric types*/
	switch (cdfvar->etype) {
	case NC_STRING: case NC_URL:
	case NC_CHAR: case NC_BYTE: case NC_UBYTE:
	    break;
	default:
	    THROWCHK(NC_ECHAR);
	    goto fail;
	}
    }

    ncstat = makegetvar34(dapcomm,cdfvar,data,externaltype,&varainfo);
    if(ncstat) {THROWCHK(NC_ENOMEM); goto fail;}


    state = 0;
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE)) {
	state = FETCHWHOLE;
	cachenode = dapcomm->cdf.cache->prefetch;	
	ASSERT((cachenode != NULL));
#ifdef DEBUG
fprintf(stderr,"Unconstrained: reusing prefetch\n");
#endif
#ifdef IGNORE
    } else if(iscached(dapcomm,cdfvar,&cachenode)) {
#else
    } else if(iscached(dapcomm,cdfvar,&cachenode)) {
#endif
        /* If it is cached, then it is a whole variable but may still
           need to apply constraints */
#ifdef DEBUG
fprintf(stderr,"Reusing cache\n");
#endif
	ASSERT(cachenode->wholevariable); /* by construction */
	state = CACHED;
    } else {/* load using constraints */
	state = FETCHPART;
    }

    ASSERT(state != 0);    


    ncstat = buildvaraprojection3(varainfo,
				  startp,countp,nc_ptrdiffvector1,
			          &varaprojection);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* In all cases, we need to construct the single projection
       as the merge of the url projections and the vara projection.
    */

    /* Convert the start/stop/stride info into a projection */
    ncstat = buildvaraprojection3(varainfo,
		                  startp,countp,stridep,
                                  &varaprojection);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    fetchprojection = NULL;
    walkprojection = NULL;
    if(state == FETCHPART) {
	/* Create a merge of the url projections and the vara projection */
	ncstat = daprestrictprojection(dapcomm->oc.dapconstraint->projections,
					varaprojection,&fetchprojection);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
	/* Clone the projection for use with the walk */
	walkprojection = (DCEprojection*)dceclone((DCEnode*)fetchprojection);
	/* We might still end up with a whole variable */
	if(dceiswholeprojection(fetchprojection))
	    state = FETCHWHOLE;
        /* Build the complete constraint to use in the fetch */
        fetchconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
        /* merged constraint just uses the url constraint selection */
        fetchconstraint->selections = dceclonelist(dapcomm->oc.dapconstraint->selections);
        fetchconstraint->projections = nclistnew();
	nclistpush(fetchconstraint->projections,(ncelem)fetchprojection);
#ifdef DEBUG
fprintf(stderr,"getvarx: fetchconstraint: %s\n",dumpconstraint(fetchconstraint));
#endif
        /* buildcachenode3 will create a new cachenode and
           will also fetch the corresponding datadds */
        vars = nclistnew();
	nclistpush(vars,(ncelem)varainfo->target);
        ncstat = buildcachenode34(dapcomm,fetchconstraint,vars,&cachenode,0);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
    } else if(state == FETCHWHOLE) {
        /* buildcachenode3 will create a new cachenode and
           will also fetch the corresponding datadds */
        vars = nclistnew();
	nclistpush(vars,(ncelem)varainfo->target);
        /* Build the complete constraint to use in the fetch */
        fetchconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
        /* merged constraint just uses the url constraint selection */
        fetchconstraint->selections = dceclonelist(dapcomm->oc.dapconstraint->selections);
	/* Use no projections */
        fetchconstraint->projections = nclistnew();
        ncstat = buildcachenode34(dapcomm,fetchconstraint,vars,&cachenode,0);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
    }

    ASSERT(cachenode != NULL);

#ifdef DEBUG
fprintf(stderr,"cache.datadds=%s\n",dumptree(cachenode->datadds));
#endif

    /* attach DATADDS to DDS */
    unattach34(dapcomm->cdf.ddsroot);
    ncstat = attachsubset34(cachenode->datadds,dapcomm->cdf.ddsroot);
    if(ncstat) goto fail;	

    /* Now, walk to the relevant instance */
    switch (state) {
    case FETCHWHOLE: case CACHED:
	/* Create a merge of the url projections and the vara projection */
	ncstat = daprestrictprojection(dapcomm->oc.dapconstraint->projections,
					varaprojection,&walkprojection);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
	break;
    case FETCHPART:
	/* Derive the proper walk projection from the
	   fetchprojection (actually the clone created above) */
	ncstat = dapshiftprojection(walkprojection);/*arg will be modified */
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
	break;
    default: PANIC("illegal getvarx state");
    }

#ifdef DEBUG
fprintf(stderr,"getvarx: walkprojection: %s\n",dumpprojection(walkprojection));
#endif

    /* Fix up varainfo to use the cache */
    varainfo->cache = cachenode;
    varainfo->varaprojection = walkprojection;
    walkprojection = NULL;

    /* Get the var correlate from the datadds */
    target = varainfo->target;
    /* xtarget is in the datadds space */
    xtarget = target->attachment;
    if(xtarget == NULL) 
	{THROWCHK(ncstat=NC_ENODATA); goto fail;}

    /* Switch to datadds tree space*/
    varainfo->target = xtarget;
    ncstat = getcontent4(dapcomm,varainfo,cachenode->datadds,data);
    if(ncstat != OC_NOERR) {THROWCHK(ncstat); goto fail;}
    goto ok;

fail:
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
ok:
    nclistfree(vars);
    dcefree((DCEnode*)fetchconstraint);
    dcefree((DCEnode*)varaprojection);
    freegetvara(varainfo);
    return THROW(ncstat);
}

static NCerror
getcontent4(NCDAPCOMMON* dapcomm, Getvara* xgetvar, CDFnode* xroot, void* data)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    NCbytes* memory;
    size_t alloc;
    int fieldindex;
    CDFnode* tnode = xgetvar->target;
    OCconnection conn = dapcomm->oc.conn;
    OCdata rootcontent = OCNULL;
    OCdata fieldcontent = OCNULL;
    OCdata dimcontent = OCNULL;
    OCdata seqcontent = OCNULL;
    OCdata gridcontent = OCNULL;
    Dapodometer* odom = NULL;
    OCobject ocroot = OCNULL;
    int caching = FLAGSET(dapcomm->controls,NCF_CACHE);
    int unconstrainable = FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE);
    nc_type externaltype = xgetvar->dsttype;

    /* is var from a toplevel grid? */
    int virtual = tnode->virtual;
    /* is var a toplevel Sequence? */
    int seqvar = daptopseq(tnode);

    if(seqvar) {
        alloc = sizeof(nc_vlen_t);
    } if(tnode->nctype == NC_Primitive) {
	/* compute the # elements */
	size_t nelems = dimproduct3(tnode->array.dimset0);
	/* Pull the field size from the externaltype. */
        alloc = (nctypesizeof(externaltype)*nelems);
    } else {
	ASSERT((tnode->attachment != NULL));
	/* Pull the field size from the node type size. */
        alloc = (size_t)(tnode->attachment->typesize.field.size);
    }
#ifdef READCHECK
fprintf(stderr,"getcontent4: |%s| = %lu\n",tnode->name,alloc);
#endif
    memory = ncbytesnew();
    ncbytessetcontents(memory,data,alloc);

    /* Get the root content*/
    ocroot = xroot->tree->ocroot;
    rootcontent = oc_data_new(conn);
    ocstat = oc_data_root(conn,ocroot,rootcontent);
    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}

    /* Move to the field for this var;
       handle grid vars separately */
    if(virtual) {
        int gridindex;
	/* Get index of the grid node wrt to the root node */
        gridindex = findfield(xroot,tnode->container);
        if(gridindex < 0) {ncstat = NC_ENOTVAR; goto fail;}
        gridcontent = oc_data_new(conn);
        ocstat = oc_data_ith(conn,rootcontent,gridindex,gridcontent);
        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
        /* Now move to the relevant grid field */
        fieldindex = findfield(tnode->container,tnode);
        if(fieldindex < 0) {ncstat = NC_ENOTVAR; goto fail;}
        fieldcontent = oc_data_new(conn);
        ocstat = oc_data_ith(conn,gridcontent,fieldindex,fieldcontent);
        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
    } else if(seqvar) { /* Move to the seq data */
        int seqindex = findfield(xroot,tnode);
        if(seqindex < 0) {ncstat = NC_ENOTVAR; goto fail;}
	seqcontent = oc_data_new(conn);
        ocstat = oc_data_ith(conn,rootcontent,seqindex,seqcontent);
        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
    } else {
	fieldindex = findfield(xroot,tnode);
        if(fieldindex < 0) {ncstat = NC_ENOTVAR; goto fail;}
	if(tnode->virtual) {
            int structindex;
	    /* Get index of the struct node wrt to the root node */
            structindex = findfield(xroot,tnode->container);
            if(structindex < 0) {ncstat = NC_ENOTVAR; goto fail;}
	    /* Get index of the structure field wrt to the virtual struct */
            fieldindex = findfield(tnode->container,tnode);
            if(fieldindex < 0) {ncstat = NC_ENOTVAR; goto fail;}
	    /* Actual field is structindex + fieldindex */
	    fieldindex += structindex;
            fieldcontent = oc_data_new(conn);
            ocstat = oc_data_ith(conn,fieldcontent,fieldindex,fieldcontent);
            if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	} else {
            fieldcontent = oc_data_new(conn);
            ocstat = oc_data_ith(conn,rootcontent,fieldindex,fieldcontent);
            if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	}
    }

    if(seqvar) {
        ASSERT((seqcontent != OCNULL));
        ncstat=getcontent4r(dapcomm,xgetvar,tnode,seqcontent,memory);
    } else if(tnode->nctype == NC_Primitive) {
	/* Stride the dimensions and get the instances */
	DCEsegment* segment = NULL;
        ASSERT((nclistlength(xgetvar->varaprojection->var->segments)==1));
	segment = (DCEsegment*)nclistget(xgetvar->varaprojection->var->segments,0);
        ASSERT((fieldcontent != OCNULL));
	ncstat = getcontent4prim(dapcomm,xgetvar,tnode,segment,fieldcontent,memory);
    } else if(nclistlength(tnode->array.dimset0) > 0) {
	/* Stride the dimensions and get the instances */
	DCEsegment* segment = NULL;
	ASSERT((nclistlength(xgetvar->varaprojection->var->segments)==1));
	segment = (DCEsegment*)nclistget(xgetvar->varaprojection->var->segments,0);
	if(caching || unconstrainable) {
            odom = newdapodometer(segment->slices,0,segment->rank);
	} else { /*Since vara was projected out, build a simple odometer*/
            odom = newsimpledapodometer(segment,segment->rank);
	}
        while(dapodometermore(odom)) {
            /* Compute which instance to move to*/
	    unsigned int dimoffset = dapodometercount(odom);
            dimcontent = oc_data_new(conn);
	    ocstat = oc_data_ith(conn,fieldcontent,dimoffset,dimcontent);
	    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	    ncstat = getcontent4r(dapcomm,xgetvar,tnode,dimcontent,memory);
	    dapodometerincr(odom);
	}
        freedapodometer(odom);
    } else {
        ncstat=getcontent4r(dapcomm,xgetvar,tnode,fieldcontent,memory);
    }

/*ok:*/
fail:
    oc_data_free(conn,gridcontent);
    oc_data_free(conn,seqcontent);
    oc_data_free(conn,dimcontent);
    oc_data_free(conn,fieldcontent);
    oc_data_free(conn,rootcontent);
    ncbytesfree(memory);/* leaves contents untouched */
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

/* Recursive walker part of getcontent */
static NCerror
getcontent4r(NCDAPCOMMON* dapcomm,
	     Getvara* xgetvar,
	     CDFnode* tnode, /* type definition */
             OCdata currentcontent,
	     NCbytes* memory)
{
    unsigned int i;
    OCerror ocstat = OC_NOERR;
    NCerror ncstat = NC_NOERR;
    OCmode mode;
    OCconnection conn = dapcomm->oc.conn;
    OCdata reccontent = OCNULL;
    OCdata fieldcontent = OCNULL;
    OCdata dimcontent = OCNULL;
    Dapodometer* odom = NULL;
    NCbytes* vlenmemory = NULL;
    nc_vlen_t vlenref = {0,NULL};
    int caching = FLAGSET(dapcomm->controls,NCF_CACHE);
    int unconstrainable = FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE);

    oc_data_mode(conn,currentcontent,&mode);
#ifdef READCHECK
{
    int rank = nclistlength(tnode->array.dimset0);
fprintf(stderr,"getcontent4r: rank=%lu mode=%d nctype=%s\n",
	(unsigned long)rank,mode,nctypetostring(tnode->nctype));
}
#endif

    if(tnode->nctype == NC_Primitive) {
	DCEsegment seg; /* temporary */
	seg.name = tnode->ocname;
	seg.cdfnode = tnode;
	makewholesegment4(&seg,tnode->array.dimset0);
	ncstat = getcontent4prim(dapcomm,xgetvar,tnode,&seg,currentcontent,memory);
	goto done;
    }

    if(tnode->virtual) {
fprintf(stderr,"VIRTUAL ENCOUNTER\n");
abort();
    }

    switch (mode) {
    case OCARRAYMODE: {
        unsigned int rank = nclistlength(tnode->array.dimset0);
	DCEsegment seg; /* temporary */
	ASSERT((tnode->nctype == NC_Structure));
	seg.name = tnode->ocname;
	seg.cdfnode = tnode;
	makewholesegment4(&seg,tnode->array.dimset0);

        /* The goal here is to walk the indices (if any)
           and extract each instance */
        ASSERT((rank > 0));
	if(caching || unconstrainable) {
            odom = newdapodometer(seg.slices,0,rank);	    
	} else { /*Since vara was projected out, build a simple odometer*/
            odom = newsimpledapodometer(&seg,rank);
	}
	dimcontent = oc_data_new(conn);
        while(dapodometermore(odom)) {
            /* Compute which instance to move to*/
	    unsigned int dimoffset = dapodometercount(odom);
	    ocstat = oc_data_ith(conn,currentcontent,dimoffset,dimcontent);
	    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	    ncstat = getcontent4r(dapcomm,xgetvar,tnode,dimcontent,memory);
	}
        } break;

    case OCFIELDMODE: /* Walk each field in turn */
	/* Align the buffer to struct boundary*/
	alignbuffer3(memory,tnode->typesize.instance.alignment);	
        fieldcontent = oc_data_new(conn);
        for(i=0;i<nclistlength(tnode->subnodes);i++) {
            CDFnode* subnode = (CDFnode*)nclistget(tnode->subnodes,i);
	    ocstat = oc_data_ith(conn,currentcontent,i,fieldcontent);
	    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	    ncstat = getcontent4r(dapcomm,xgetvar,subnode,fieldcontent,memory);
	}
	break;

    case OCSEQUENCEMODE:
	/* Collect the set of records as a separate memory structure */
	vlenmemory = ncbytesnew();	
	ncbytessetalloc(vlenmemory,4096);
#ifdef READCHECK
fprintf(stderr,"getcontent4r: record: vlenmemory=%lx\n",
	(unsigned long)ncbytescontents(vlenmemory));
#endif
        reccontent = oc_data_new(conn);
	for(i=0;;i++) {
	    ocstat = oc_data_ith(conn,currentcontent,i,reccontent);
	    if(ocstat == OC_EINVALCOORDS) {ocstat=OC_NOERR; break;} /* no more records */
	    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	    ncstat = getcontent4r(dapcomm,xgetvar,tnode,reccontent,vlenmemory);
        }
	vlenref.len = i;
	vlenref.p = ncbytesextract(vlenmemory);

#ifdef READCHECK
fprintf(stderr,"getcontent4r: record: vlen.p=%lx\n",
	(unsigned long)vlenref.p);
#endif
	/* Now put an nc_vlen_t into our buffer */
	alignbuffer3(memory,tnode->typesize.field.alignment);	
	ncbytesappendn(memory,(char*)&vlenref,sizeof(vlenref));
#ifdef READCHECK
fprintf(stderr,"getcontent4r: record: memory=%lx |memory|=%lu\n",
	(unsigned long)ncbytescontents(memory),(unsigned long)ncbyteslength(memory));
#endif
	break;

    default: ncstat = NC_EINVAL; {THROWCHK(ncstat); goto fail;}
    }

done:
fail:
    ncbytesfree(vlenmemory);
    oc_data_free(conn,dimcontent);
    oc_data_free(conn,fieldcontent);
    oc_data_free(conn,reccontent);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

static NCerror
getcontent4prim(NCDAPCOMMON* dapcomm,
	        Getvara* xgetvar,
                CDFnode* tnode,
                DCEsegment* segment,
	        OCdata currentcontent,
                NCbytes* memory)
{
    OCerror ocstat = OC_NOERR;
    NCerror ncstat = NC_NOERR;
    unsigned int rank;
    OCconnection conn = dapcomm->oc.conn;
    Dapodometer* odom = NULL;
    unsigned int memoffset;
    DCEslice* slices = segment->slices;
    int caching = FLAGSET(dapcomm->controls,NCF_CACHE);
    int unconstrainable = FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE);
    size_t internaltypesize, externaltypesize;
    nc_type internaltype = segment->cdfnode->etype;
    nc_type externaltype = xgetvar->dsttype;
    int bit8type; /* is this an 8 bit type? */

    if(externaltype == NC_NAT) externaltype = internaltype;

    internaltypesize = nctypesizeof(internaltype);
    externaltypesize = nctypesizeof(externaltype);

    switch (internaltype) {
    case NC_CHAR: case NC_BYTE: case NC_UBYTE: bit8type = 1; break;
    default: bit8type = 0; break;
    }

    rank = nclistlength(tnode->array.dimset0);
    ASSERT((rank == segment->rank));
#ifdef READCHECK
fprintf(stderr,"getcontent4prim: tnode=%s/%d segment=%s internaltype=%s externaltype=%s memory=%lx internaltypesize=%lu externaltypesize=%lu\n",
	tnode->name,rank,dumpsegment(segment),
	nctypetostring(internaltype),nctypetostring(externaltype),
        (unsigned long)ncbytescontents(memory),
	(unsigned long)internaltypesize,
	(unsigned long)externaltypesize);
#endif
    alignbuffer3(memory,ncctypealignment(externaltype)); /* Align to the proper boundary */
    memoffset = ncbyteslength(memory);
#ifdef READCHECK
fprintf(stderr,"getcontent4prim: alignment=%d memory.after=%lx\n",
	ncctypealignment(externaltype),(unsigned long)ncbytescontents(memory));
#endif
    if(rank == 0) { /* singleton data item*/
        char value[32]; /* big enough to hold any single primitive value*/
	void* memdata;
	ASSERT((internaltypesize <= sizeof(value)));
        if(!ncbytessetlength(memory,memoffset+externaltypesize))
	    return NC_ENOMEM;
#ifdef READCHECK
fprintf(stderr,"getcontent4prim: scalar: memory=%lx |memory|=%lu\n",
	(unsigned long)ncbytescontents(memory),(unsigned long)ncbyteslength(memory));
#endif
	memdata = ncbytescontents(memory)+memoffset;
#ifdef READCHECK
fprintf(stderr,"getcontent4prim: scalar: memdata=%lx\n",(unsigned long)memdata);
#endif
        ocstat = oc_data_get(conn,currentcontent,value,sizeof(value),0,1);
        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	ncstat = ncdap4convert(internaltype,externaltype,memdata,value,1);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
    } else { /* rank > 0 */
	size_t requested, arraysize;
	char* memdata;
	int contiguous;

	if(caching || unconstrainable) {	
            odom = newdapodometer(slices,0,rank);
        } else {
            odom = newsimpledapodometer(segment,rank);
	}

	/* Compute the actual # of values requested by the caller */
	requested = dapodometerpoints(odom);

	if(bit8type && externaltype == NC_STRING) { /* special case */
	    arraysize = nctypesizeof(NC_STRING); /* allocate a single string */
	} else
	    arraysize = externaltypesize*requested;

        if(!ncbytessetlength(memory,memoffset+arraysize)) return NC_ENOMEM;
	memdata = ncbytescontents(memory)+memoffset;

#ifdef TEST
{
size_t dimcount;
oc_data_count(conn,currentcontent,&dimcount);
printf("read: %s: memoffset=%lu externaltypesize=%lu dimcount=%lu requested=%lu arraysize=%lu\n",
	tnode->name,(unsigned long)memoffset,(unsigned long)externaltypesize,
        (unsigned long)dimcount,(unsigned long)requested,arraysize);
fflush(stdout);
}
#endif
	
	/* Optimize off the use of the odometer by checking the slicing
           to locate the largest possible suffix of dimensions that
           represent a contiguous chunk; However do not do this if
           external type conversion is needed.
        */

	contiguous = contiguousdims(odom);	
	if(contiguous < odom->rank && internaltype == externaltype) {
	    /* Compute the chunk size */
	    size_t chunkcount;

	    ASSERT((odom->rank != 1 || contiguous == 0));
	    chunkcount = dapodometerspace(odom,contiguous);

	    if(contiguous == 0) {
		/* The whole vara slice is usable:
		   read the whole chunk at one shot.
		*/		   
	        /* copy the data locally before conversion */
		ocstat = oc_data_get(conn,currentcontent,memdata,
			               arraysize,0,chunkcount);
                if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	    } else { /* odom->rank > 1 && contiguous > 0 */
		size_t memchunk = (externaltypesize * chunkcount);
		/* Use the odometer to find each contiguous chunk. */
		odom->rank = contiguous; /* move in chunksize chunks */
	        while(dapodometermore(odom)) {
		    size_t nchunks = dapodometercount(odom);
		    size_t chunkoffset = nchunks * chunkcount;
	            ocstat = oc_data_get(conn,currentcontent,memdata,
                                           arraysize,chunkoffset,chunkcount);
	            if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
		    memdata += memchunk;
		    arraysize -= memchunk;
		    dapodometerincr(odom);
		}
	    }
	} else if(bit8type && externaltype == NC_STRING) {
	    /* special case conversion */
	    char* s = (char*)malloc(requested+1); /* bytes will go into a single string */
	    char** sp = (char**)memdata;
	    ocstat = oc_data_get(conn,currentcontent,s,requested,0,requested);
	    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	    *sp = s;
	    memdata += externaltypesize;
	} else { /*Oh well,use the odometer to walk to the appropriate fields*/
	    char value[32]; /* to hold any value*/
	    while(dapodometermore(odom)) {
		size_t dimoffset = dapodometercount(odom);
	        ocstat = oc_data_get(conn,currentcontent,value,sizeof(value),dimoffset,1);
	        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
	        ocstat = ncdap4convert(internaltype,externaltype,memdata,value,1);
	        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}
		memdata += externaltypesize;		
		dapodometerincr(odom);
            }
	}
    }
/*ok:*/
fail:
    freedapodometer(odom);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

static int
findfield(CDFnode* node, CDFnode* subnode)
{
    size_t i;
    for(i=0;i<nclistlength(node->subnodes);i++) {
	CDFnode* test = (CDFnode*)nclistget(node->subnodes,i);
	if(test == subnode) return i;
    }
    return -1;
}

/*
Find the largest prefix of n dimensions such that:
n .. odom.rank are complete: they have start of 0,
stride of 1 and count == dimensions size.
*/
static int 
contiguousdims(Dapodometer* odom)
{
    unsigned int i;
    int mark = 0;
    if(odom->rank <= 1) return 0;
    for(i=0;i<odom->rank;i++) {
	ASSERT((odom->slices[i].declsize != 0));
	if(odom->slices[i].stride != 1
	   || odom->slices[i].first > 0
           || odom->slices[i].length != odom->slices[i].declsize)
	    mark=i;
    }
    return mark+1;
}

static NCerror
ncdap4convert(nc_type srctype, nc_type dsttype, char* memory, char* value, size_t count)
{
    NCerror ncstat = NC_NOERR;
    size_t i;

#define CASE(nc1,nc2) (nc1*256+nc2)
    switch (CASE(srctype,dsttype)) {

/* the NC_STRING/URL case is only used by ncdap4 */
case CASE(NC_STRING,NC_STRING):
case CASE(NC_STRING,NC_URL):
case CASE(NC_URL,NC_STRING):
case CASE(NC_URL,NC_URL):
    for(i=0;i<count;i++) {
        *((char**)memory) = (char*) *(char**)value;    
        memory += nctypesizeof(dsttype);
    }
    break;

/* conversion of string/url to char is illegal
   because caller will not know size
*/
case CASE(NC_STRING,NC_CHAR):
case CASE(NC_URL,NC_CHAR):
    ncstat = NC_EINVAL;
    break;

/* convert char/byte/ubyte to string by creating a single string;
   not clear this is correct
*/
case CASE(NC_BYTE,NC_STRING):
case CASE(NC_UBYTE,NC_STRING):
case CASE(NC_CHAR,NC_STRING): {
    char* s = (char*)malloc(count+1);
    char** smem = (char**)memory;
    memcpy((void*)s,value,count);
    s[count] = '\0';
    *smem = s;
    memory += nctypesizeof(dsttype);
    } break;

    default:
	ncstat = dapconvert3(srctype,dsttype,memory,value,count);
	break;
    }

/*ok:*/
    return THROW(ncstat);
}

#ifdef IGNORE
/* In order to construct the projection,
we need to make sure to match the relevant dimensions
against the relevant nodes in which the ultimate target
is contained.
*/
static NCerror
buildvaraprojection4(NCDAPCOMMON* dapcomm, Getvara* getvar, NCbytes* buf)
{
    int i, dimdex;
    CDFnode* node;
    CDFnode* target = getvar->target;
    NClist* path = NULL;

#ifdef DEBUG
fprintf(stderr,"vara: %s\n",getvaraprint(getvar));
#endif
    if(!getvar->projected) return NC_NOERR;

    path = nclistnew();
    collectnodepath3(target,path,WITHOUTDATASET);
    if(nclistlength(path) == 0) return NC_NOERR;
    
    ncbytesclear(buf);
    for(dimdex=0,i=0;i<nclistlength(path);i++) {
	node = (CDFnode*)nclistget(path,i);
	if(i>0) ncbytescat(buf,"."); /* we are past the first projection*/
	ncbytescat(buf,node->name);
#ifdef IGNORE
	if(node->nctype == NC_Sequence) {
	    ASSERT((node->usesequence == 1));
	    dimdex++;
	} else if(nclistlength(node->array.dimset0) > 0) {
	    makevarprojection3(getvar->slices,dimdex,nclistlength(node->array.dimset0),buf);
	    dimdex += nclistlength(node->array.dimset0);
	}
#endif
    }
#ifdef DEBUG
fprintf(stderr,"projection=|%s|",ncbytescontents(buf)); fflush(stderr);
#endif
    nclistfree(path);
    return NC_NOERR;
}

static void
makevarprojection3(NCslice* slices, int dimdex, int ndims, NCbytes* buf)
{
    int i;
    char tmp[1024];
    NCslice* slice = slices+dimdex;
    for(i=0;i<ndims;i++,slice++) {
	snprintf(tmp,sizeof(tmp),"[%lu:%lu:%lu]",
			(unsigned long)slice->first,
			(unsigned long)slice->stride,
			(unsigned long)(slice->stop - 1));
	ncbytescat(buf,tmp);
    }
}
#endif

static void
makewholesegment4(DCEsegment* seg, NClist* dimset)
{
    int i;
    unsigned int rank;

    rank = nclistlength(dimset);

    seg->rank = rank;
    for(i=0;i<rank;i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
	dcemakewholeslice(&seg->slices[i],dim->dim.declsize);
    }
    seg->slicesdefined  = 1;
    seg->slicesdeclized = 1;
}
