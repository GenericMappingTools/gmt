/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/
#include "ncdap3.h"
#include "dapodom.h"
#include "dapdump.h"

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
           If the cachenode is constrained, then skip it
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
fprintf(stderr,"iscached: search: %s\n",makesimplepathstring3(target));
if(found)
   fprintf(stderr,"iscached: found: %s\n",dumpcachenode(cachenode));
else
   fprintf(stderr,"iscached: notfound\n");
#endif
    return found;
}

/* Compute the set of prefetched data;
   note that even if caching is off, we will
   still prefetch the small variables.
*/
NCerror
prefetchdata3(NCDAPCOMMON* nccomm)
{
    int i,j;
    NCerror ncstat = NC_NOERR;
    NClist* allvars = nccomm->cdf.varnodes;
    DCEconstraint* constraint = nccomm->oc.dapconstraint;
    NClist* vars = nclistnew();
    NCcachenode* cache = NULL;
    DCEconstraint* newconstraint = NULL;

    /* Check if we can do constraints */
    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) { /*cannot constrain*/
        /* If we cannot constrain, then pull in everything */
	for(i=0;i<nclistlength(allvars);i++) {
	    nclistpush(vars,nclistget(allvars,i));
	}
    } else { /* can do constraints */
	/* pull in those variables of sufficiently small size */
        for(i=0;i<nclistlength(allvars);i++) {
            CDFnode* var = (CDFnode*)nclistget(allvars,i);
            size_t nelems = 1;
    
	    /* If var is a sequence or under a sequence, then never prefetch */
	    if(var->nctype == NC_Sequence || dapinsequence(var)) continue;

            /* Compute the # of elements in the variable */
            for(j=0;j<nclistlength(var->array.dimensions);j++) {
                CDFnode* dim = (CDFnode*)nclistget(var->array.dimensions,j);
                nelems *= dim->dim.declsize;
	    }
	    if(nelems <= nccomm->cdf.smallsizelimit)
	        nclistpush(vars,(ncelem)var);
	}
    }

    /* If there are no vars, then do nothing */
    if(nclistlength(vars) == 0) {
	nccomm->cdf.cache->prefetch = NULL;
	goto done;
    }

    newconstraint = (DCEconstraint*)dceclone((DCEnode*)constraint);
    /* Construct the projections for this set of vars */
    /* Initially, the constraints are same as the merged constraints */
    restrictprojection34(vars,newconstraint->projections);
    /* similar for selections */
    /* Currently do nothing */

    ncstat = buildcachenode34(nccomm,newconstraint,vars,&cache,1);
    if(ncstat) goto done;
    newconstraint = NULL; /* buildcachenode34 takes control of newconstraint */
    cache->wholevariable = 1; /* All prefetches are whole variable */

if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
nclog(NCLOGNOTE,"prefetch.");
}

#ifdef DEBUG
/* Log the set of prefetch variables */
NCbytes* buf = ncbytesnew();
ncbytescat(buf,"prefetch.vars: ");
for(i=0;i<nclistlength(vars);i++) {
CDFnode* var = (CDFnode*)nclistget(vars,i);
ncbytescat(buf," ");
ncbytescat(buf,makesimplepathstring3(var));
}
ncbytescat(buf,"\n");
nclog(NCLOGNOTE,"%s",ncbytescontents(buf));
ncbytesfree(buf);
#endif

    /* Make cache node be the prefetch node */
    nccomm->cdf.cache->prefetch = cache;

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
    OCconnection conn = nccomm->oc.conn;
    OCobject ocroot = OCNULL;
    CDFnode* dxdroot = NULL;
    NCcachenode* cachenode = NULL;
    char* ce = NULL;

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
        ce = NULL;
    else
        ce = buildconstraintstring3(constraint);

    ocstat = dap_oc_fetch(nccomm,conn,ce,OCDATADDS,&ocroot);
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
    cachenode->constraint = constraint;
    cachenode->wholevariable = iswholeconstraint(cachenode->constraint);

    /* save the root content*/
    cachenode->ocroot = ocroot;
    cachenode->content = oc_data_new(conn);
    ocstat = oc_data_root(conn,ocroot,cachenode->content);
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
	while(cache->cachesize + cachenode->xdrsize > cache->cachelimit) {
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
    oc_data_free(nccomm->oc.conn,node->content);
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
