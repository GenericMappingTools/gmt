/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#include "ncdap3.h"
#include "dapdump.h"

static int iscacheableconstraint(DCEconstraint* con);

/* Return 1 if we can reuse cached data to address
   the current get_vara request; return 0 otherwise.
   Target is in the constrained tree space.
   Currently, if the target matches a cache that is not
   a whole variable, then match is false.
*/
int
iscached(NCDAPCOMMON* nccomm, CDFnode* target, NCcachenode** cachenodep)
{
    int i,j,found,index;
    NCcache* cache;
    NCcachenode* cachenode;

    found = 0;
    if(target == NULL) goto done;

    /* Match the target variable against the prefetch, if any */
    /* Note that prefetches are always whole variable */
    cache = nccomm->cdf.cache;
    cachenode = cache->prefetch;
    if(cachenode!= NULL) {
        for(found=0,i=0;i<nclistlength(cachenode->vars);i++) {
            CDFnode* var = (CDFnode*)nclistget(cachenode->vars,i);
	    if(var == target) {
                if(cachenodep) *cachenodep = cachenode;
		found=1;
		goto done;
	    }
	}
    }

    /*search other cache nodes starting at latest first */
    index = 0;
    for(i=nclistlength(cache->nodes)-1;i>=0;i--) {
        cachenode = (NCcachenode*)nclistget(cache->nodes,i);
	/* We currently do not try to match constraints;
           If the cachenode is constrained by more than
           simple wholevariable projections, then skip it.
        */
	if(!cachenode->wholevariable) continue;
        for(found=0,j=0;j<nclistlength(cachenode->vars);j++) {
            CDFnode* var = (CDFnode*)nclistget(cachenode->vars,j);
            if(var == target) {found=1;index=i;break;}
	}
	if(found) break;
    }

    if(found) {
        ASSERT((cachenode != NULL));
        if(nclistlength(cache->nodes) > 1) {
	    /* Manage the cache nodes as LRU */
	    nclistremove(cache->nodes,index);
	    nclistpush(cache->nodes,(ncelem)cachenode);
	}
        if(cachenodep) *cachenodep = cachenode;
    }

done:
#ifdef DEBUG
fprintf(stderr,"iscached: search: %s\n",makecdfpathstring3(target,"."));
if(found)
   fprintf(stderr,"iscached: found: %s\n",dumpcachenode(cachenode));
else
   fprintf(stderr,"iscached: notfound\n");
#endif
    return found;
}

/* Compute the set of prefetched data.
   Notes:
   1. Even if caching is off, we will
       still prefetch the small variables,
       unless prefetch is specifically disabled.
   2. All prefetches are whole variable fetches.
   3. If the data set is unconstrainable, we
      will prefetch the whole thing
*/
NCerror
prefetchdata3(NCDAPCOMMON* nccomm)
{
    int i,j;
    NCerror ncstat = NC_NOERR;
    NClist* allvars = nccomm->cdf.varnodes;
    DCEconstraint* urlconstraint = nccomm->oc.dapconstraint;
    NClist* vars = nclistnew();
    NCcachenode* cache = NULL;
    DCEconstraint* newconstraint = NULL;
    int isnc4 = FLAGSET(nccomm->controls,NCF_NC4);

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
        /* If we cannot constrain and caching is enabled,
           then pull in everything */
        if(FLAGSET(nccomm->controls,NCF_CACHE)) {
	    for(i=0;i<nclistlength(allvars);i++) {
	        nclistpush(vars,nclistget(allvars,i));
	    }
	} else { /* do no prefetching */
    	    nccomm->cdf.cache->prefetch = NULL;
	    goto done;
	}
    } else { /* can use projections to get subset of variables */
	/* pull in those variables of sufficiently small size */
        for(i=0;i<nclistlength(allvars);i++) {
            CDFnode* var = (CDFnode*)nclistget(allvars,i);
            size_t nelems = 1;

	    /* Do not attempt to prefetch any variables in the
               nc_open url's projection list
	    */
	    if(nclistcontains(nccomm->cdf.projectedvars,(ncelem)var))
		continue;

            if(!isnc4) {
	        /* If netcdf 3 and var is a sequence or under a sequence, then never prefetch */
	        if(var->nctype == NC_Sequence || dapinsequence(var)) continue;
	    }

            /* Compute the # of elements in the variable */
            for(j=0;j<nclistlength(var->array.dimset0);j++) {
                CDFnode* dim = (CDFnode*)nclistget(var->array.dimset0,j);
                nelems *= dim->dim.declsize;
	    }
if(SHOWFETCH) {
nclog(NCLOGDBG,"prefetch: %s=%lu",var->ncfullname,(unsigned long)nelems);
}
	    if(nelems <= nccomm->cdf.smallsizelimit) {
	        nclistpush(vars,(ncelem)var);
if(SHOWFETCH) {
nclog(NCLOGDBG,"prefetch: %s",var->ncfullname);
}
	    }
	}
    }

    /* If there are no vars, then do nothing */
    if(nclistlength(vars) == 0) {
	nccomm->cdf.cache->prefetch = NULL;
	goto done;
    }

    /* Create a single constraint consisting of the projections for the variables;
       each projection is whole variable. The selections are passed on as is.
    */

    newconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
    newconstraint->projections = nclistnew();
    newconstraint->selections = dceclonelist(urlconstraint->selections);

    for(i=0;i<nclistlength(vars);i++) {
	CDFnode* var = (CDFnode*)nclistget(vars,i);
	DCEprojection* varprojection;
	/* convert var to a projection */
	ncstat = dapvar2projection(var,&varprojection);
	if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	nclistpush(newconstraint->projections,(ncelem)varprojection);
    }
if(SHOWFETCH) {
char* s = dumpprojections(newconstraint->projections);
LOG1(NCLOGNOTE,"prefetch.final: %s",s);
nullfree(s);
}
    ncstat = buildcachenode34(nccomm,newconstraint,vars,&cache,!isnc4);
    newconstraint = NULL; /* buildcachenode34 takes control of newconstraint */
    if(ncstat) goto done;
    cache->wholevariable = 1; /* All prefetches are whole variable */
    /* Make cache node be the prefetch node */
    nccomm->cdf.cache->prefetch = cache;
if(SHOWFETCH) {
LOG0(NCLOGNOTE,"prefetch.complete");
}

if(SHOWFETCH) {
char* s = NULL;
/* Log the set of prefetch variables */
NCbytes* buf = ncbytesnew();
ncbytescat(buf,"prefetch.vars: ");
for(i=0;i<nclistlength(vars);i++) {
CDFnode* var = (CDFnode*)nclistget(vars,i);
ncbytescat(buf," ");
s = makecdfpathstring3(var,".");
ncbytescat(buf,s);
nullfree(s);
}
ncbytescat(buf,"\n");
nclog(NCLOGNOTE,"%s",ncbytescontents(buf));
ncbytesfree(buf);
}

done:
    nclistfree(vars);
    dcefree((DCEnode*)newconstraint);    
    if(ncstat) freenccachenode(nccomm,cache);
    return THROW(ncstat);
}

NCerror
buildcachenode34(NCDAPCOMMON* nccomm,
	        DCEconstraint* constraint,
		NClist* varlist,
		NCcachenode** cachep,
		int isprefetch)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OClink conn = nccomm->oc.conn;
    OCddsnode ocroot = NULL;
    CDFnode* dxdroot = NULL;
    NCcachenode* cachenode = NULL;
    char* ce = NULL;

    ce = buildconstraintstring3(constraint);

    ocstat = dap_fetch(nccomm,conn,ce,OCDATADDS,&ocroot);
    nullfree(ce);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

    ncstat = buildcdftree34(nccomm,ocroot,OCDATA,&dxdroot);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* regrid */
    if(!FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
        ncstat = regrid3(dxdroot,nccomm->cdf.ddsroot,constraint->projections);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* create the cache node */
    cachenode = createnccachenode();
    cachenode->prefetch = isprefetch;
    cachenode->vars = nclistclone(varlist);
    cachenode->datadds = dxdroot;
    /* Give the constraint over to the cachenode */
    cachenode->constraint = constraint;
    constraint = NULL;
    cachenode->wholevariable = iscacheableconstraint(cachenode->constraint);

    /* save the root content*/
    cachenode->ocroot = ocroot;
    ocstat = oc_data_getroot(conn,ocroot,&cachenode->content);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

    /* capture the packet size */
    ocstat = oc_raw_xdrsize(conn,ocroot,&cachenode->xdrsize);
    if(ocstat) {THROWCHK(ocerrtoncerr(ocstat)); goto done;}

#ifdef DEBUG
fprintf(stderr,"buildcachenode: new cache node: %s\n",
	dumpcachenode(cachenode));
#endif
    /* Insert into the cache. If not caching, then
       remove any previous cache node
    */
    if(!isprefetch) {
	NCcache* cache = nccomm->cdf.cache;
	if(cache->nodes == NULL) cache->nodes = nclistnew();
	/* remove cache nodes to get below the max cache size */
	while(cache->cachesize + cachenode->xdrsize > cache->cachelimit
	      && nclistlength(cache->nodes) > 0) {
	    NCcachenode* node = (NCcachenode*)nclistremove(cache->nodes,0);
#ifdef DEBUG
fprintf(stderr,"buildcachenode: purge cache node: %s\n",
	dumpcachenode(cachenode));
#endif
	    cache->cachesize -= node->xdrsize;
	    freenccachenode(nccomm,node);
	}
	/* Remove cache nodes to get below the max cache count */
	/* If not caching, then cachecount should be 0 */
	while(nclistlength(cache->nodes) > cache->cachecount) {
	    NCcachenode* node = (NCcachenode*)nclistremove(cache->nodes,0);
#ifdef DEBUG
fprintf(stderr,"buildcachenode: count purge cache node: %s\n",
	dumpcachenode(node));
#endif
	    cache->cachesize -= node->xdrsize;
	    freenccachenode(nccomm,node);
        }
        nclistpush(nccomm->cdf.cache->nodes,(ncelem)cachenode);
        cache->cachesize += cachenode->xdrsize;
    }

#ifdef DEBUG
fprintf(stderr,"buildcachenode: %s\n",dumpcachenode(cachenode));
#endif

done:
    if(constraint != NULL) dcefree((DCEnode*)constraint);
    if(cachep) *cachep = cachenode;
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    if(ncstat) {
	freecdfroot34(dxdroot);
	freenccachenode(nccomm,cachenode);
    }
    return THROW(ncstat);
}

NCcachenode*
createnccachenode(void)
{
    NCcachenode* mem = (NCcachenode*)calloc(1,sizeof(NCcachenode));
    return mem;
}

void
freenccachenode(NCDAPCOMMON* nccomm, NCcachenode* node)
{
    if(node == NULL) return;
#ifdef DEBUG
fprintf(stderr,"freecachenode: %s\n",
	dumpcachenode(node));
#endif
    oc_data_free(nccomm->oc.conn,node->content);
    dcefree((DCEnode*)node->constraint);
    freecdfroot34(node->datadds);
    nclistfree(node->vars);
    nullfree(node);
}

void
freenccache(NCDAPCOMMON* nccomm, NCcache* cache)
{
    int i;
    if(cache == NULL) return;
    freenccachenode(nccomm,cache->prefetch);
    for(i=0;i<nclistlength(cache->nodes);i++) {
	freenccachenode(nccomm,(NCcachenode*)nclistget(cache->nodes,i));
    }
    nclistfree(cache->nodes);
    nullfree(cache);
}

NCcache*
createnccache(void)
{
    NCcache* c = (NCcache*)calloc(1,sizeof(NCcache));
    c->cachelimit = DFALTCACHELIMIT;
    c->cachesize = 0;
    c->nodes = nclistnew();
    c->cachecount = DFALTCACHECOUNT;
    return c;
}

static int
iscacheableprojection(DCEprojection* proj)
{
    int i,cacheable;
    if(proj->discrim != CES_VAR) return 0;
    cacheable = 1; /* assume so */
    for(i=0;i<nclistlength(proj->var->segments);i++) {
        DCEsegment* segment = (DCEsegment*)nclistget(proj->var->segments,i);
	if(!iswholesegment(segment)) {cacheable = 0; break;}	
    }
    return cacheable;
}

static int
iscacheableconstraint(DCEconstraint* con)
{
    int i;
    if(con == NULL) return 1;
    if(con->selections != NULL && nclistlength(con->selections) > 0)
	return 0; /* cant deal with selections */
    for(i=0;i<nclistlength(con->projections);i++) {
        if(!iscacheableprojection((DCEprojection*)nclistget(con->projections,i)))
	    return 0;
    }
    return 1;
}
