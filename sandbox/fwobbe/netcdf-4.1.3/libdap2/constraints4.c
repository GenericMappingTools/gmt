/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap4/constraints4.c,v 1.9 2010/04/13 03:36:31 dmh Exp $
 *********************************************************************/
#include "ncdap4.h"
#include "dapodom.h"

#ifdef DEBUG
#include "dapdump.h"
#endif

/* In order to construct the projection,
we need to make sure to match the relevant dimensions
against the relevant nodes in which the ultimate target
is contained.
*/
NCerror
buildvaraprojection4(Getvara* getvar,
		     const size_t* startp, const size_t* countp, const ptrdiff_t* stridep,
		     DCEprojection** projectionp)
{
    int i;
    NCerror ncstat = NC_NOERR;
    NClist* dimset;
    CDFnode* var = getvar->target;
    DCEprojection* projection = NULL;
    NClist* segments = NULL;
    DCEsegment* segment;

    segment = (DCEsegment*)dcecreate(CES_SEGMENT);
    segment->cdfnode = var;
    ASSERT((segment->cdfnode != NULL));
    segment->name = nulldup(segment->cdfnode->name);
    segment->slicesdefined = 0; /* temporary */
    segment->slicesdeclized = 0; /* temporary */
    segments = nclistnew();
    nclistpush(segments,(ncelem)segment);

    projection = (DCEprojection*)dcecreate(CES_PROJECT);
    projection->discrim = CES_VAR;
    projection->var = (DCEvar*)dcecreate(CES_VAR);
    projection->var->cdfleaf = var;
    projection->var->segments = segments;

    /* All slices are assigned to the first (and only segment) */
    dimset = var->array.dimensions;
    segment->rank = nclistlength(var->array.dimensions);
    for(i=0;i<segment->rank;i++) { 
        DCEslice* slice = &segment->slices[i];
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
        slice->first = startp[i];
	slice->stride = stridep[i];
	slice->count = countp[i];
        slice->length = slice->count * slice->stride;
	slice->stop = (slice->first + slice->length);
	ASSERT(dim->dim.declsize > 0);
    	slice->declsize = dim->dim.declsize;
    }
    segment->slicesdefined = 1;
    segment->slicesdeclized = 1;

    if(projectionp) *projectionp = projection;
    if(ncstat) dcefree((DCEnode*)projection);
    return ncstat;
}

/* Compute the set of prefetched data;
   note that even if caching is off, we will
   still prefetch the small variables.
*/
NCerror
prefetchdata4(NCDAPCOMMON* nccomm)
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

    /* Construct the projections for this set of vars */
    newconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
    /* Initially, the constraints are same as the merged constraints */
    newconstraint->projections = dceclonelist(constraint->projections);
    restrictprojection34(vars,newconstraint->projections);
    /* similar for selections */
    newconstraint->selections = dceclonelist(constraint->selections);
 
    ncstat = buildcachenode34(nccomm,newconstraint,vars,&cache,0);
    if(ncstat) goto done;

    /* Make cache node be the prefetch node */
    nccomm->cdf.cache->prefetch = cache;

if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
/* Log the set of prefetch variables */
NCbytes* buf = ncbytesnew();
ncbytescat(buf,"prefetch.vars: ");
for(i=0;i<nclistlength(vars);i++) {
CDFnode* var = (CDFnode*)nclistget(vars,i);
ncbytescat(buf," ");
ncbytescat(buf,makesimplepathstring3(var));
}
ncbytescat(buf,"\n");
oc_log(OCLOGNOTE,ncbytescontents(buf));
ncbytesfree(buf);
}

done:
    if(ncstat) {
	freenccachenode(nccomm,cache);
    }
    return THROW(ncstat);
}

#ifdef IGNORE
/* Based on the tactic, determine the set of variables to add */
static void
computevarset4(NCDAP4* drno, Getvara* getvar, NClist* varlist)
{
    int i;
    nclistclear(varlist);
    for(i=0;i<nclistlength(drno->dap.cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(drno->dap.cdf.varnodes,i);
#ifdef IGNORE
	int ok = 1;
	for(j=0;j<nclistlength(var->array.ncdimensions);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(var->array.ncdimensions,j);
	    if(dim->dim.declsize == NC_UNLIMITED) {ok = 0; break;}
	}
	if(!ok) continue;
#endif
        switch (getvar->tactic->tactic) {
        case tactic_all: /* add all visible variables */
	    nclistpush(varlist,(ncelem)var);
	    break;	    
        case tactic_partial: /* add only small variables + target */
	    if(var->estimatedsize < drno->dap.cdf.smallsizelimit
	       || getvar->target == var) {
		nclistpush(varlist,(ncelem)var);
	    }
	    break;	    
        case tactic_var: /* add only target var */
	    if(getvar->target == var) nclistpush(varlist,(ncelem)var);
	    break;	    
	default: break;
	}
    }
}
#endif

