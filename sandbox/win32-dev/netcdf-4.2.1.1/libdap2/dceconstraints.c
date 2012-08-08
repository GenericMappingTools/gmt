/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.c,v 1.40 2010/05/27 21:34:07 dmh Exp $
 *********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "nclist.h"
#include "ncbytes.h"
#include "nclog.h"

#include "netcdf.h"
#include "dceconstraints.h"
#include "dapdebug.h"
#include "dceparselex.h"

#define DEBUG

#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif
#ifndef nullfree
#define nullfree(s) {if((s)!=NULL) {free(s);} else {}}
#endif

static char* opstrings[] = OPSTRINGS ;

static int mergeprojection(DCEprojection* dst, DCEprojection* src);
static void ceallnodesr(DCEnode* node, NClist* allnodes, CEsort which);

/* Parse incoming url constraints, if any,
   to check for syntactic correctness
*/ 
int
dapparseconstraints(char* constraints, DCEconstraint* dapconstraint)
{
    int ncstat = NC_NOERR;
    char* errmsg;

    assert(dapconstraint != NULL);
    nclistclear(dapconstraint->projections);
    nclistclear(dapconstraint->selections);

    ncstat = dapceparse(constraints,dapconstraint,&errmsg);
    if(ncstat) {
	nclog(NCLOGWARN,"DAP constraint parse failure: %s",errmsg);
	if(errmsg) free(errmsg);
        nclistclear(dapconstraint->projections);
        nclistclear(dapconstraint->selections);
    }

#ifdef DEBUG
fprintf(stderr,"constraint: %s",dcetostring((DCEnode*)dapconstraint));
#endif
    return ncstat;
}

/* Worksheet

mg.st = md.st * ms.st
mg.f  = md.f+(ms.f*md.st)
mg.l  = ((ms.l-1) / ms.st) * mg.st + 1
mg.p  = mg.f + mg.l
mg.c  = mg.l / mg.st

0000000000111111111122222222223
0123456789012345678901234567890
          xxxxxx
                xxxxxx
 0  1  2  3  4  5  6  7  8        md=(st=3 f=1 l=25 p=26)
          0     1     2           ms=(st=2 f=3 l=5  p=8 )
		                  ----------------------------
                                  mg=(st=6 f=10 p=23 l=13)
c = 4 / 2 = 2
l = 2 * 6 + 1 = 13

0000000000111111
0123456789012345
 0 1 2 3 4                md=(st=2 f=1 l=9 p=10)
     0 1 2                ms=(st=1 f=2 l=3 p=5)
                          ----------------------------
                          mg=(st=2 f=5 p=10 l=5 )
c = 2/1 = 2
l = 2 * 2 + 1 = 13

0000000000111111111
0123456789012345678
 0 1 2 3 4 5 6 7 8        md=(st=2 f=1 l=17 p=18)
       0   1   2          ms=(st=2 f=3 l=5 p=8)
		          ----------------------------
                          mg=(st=4 f=7 p=16 l=9 )
c = 4/2 = 2
l = 2 * 4 + 1 = 9

0000000000111111111
0123456789012345678
 0 1 2 3 4                md=(st=2 f=1 l=9 p=10)
 0 1 2 3 4                ms=(st=1 f=0 l=5 p=5)
		          ----------------------------
                          mg=(st=2 f=1 p=10 l=9 )
c = 4/1 = 4
l = 4 * 2 + 1 = 9

00000
01234
01                        md=(st=1 f=0 l=2 p=2)
0                         ms=(st=1 f=0 l=1 p=1)
		          ----------------------------
                          mg=(st=1 f=0 p=1 l=1 )
c = 0/1 = 0
l = 0 * 1 + 1 = 1

000000000011
012345678901
012                       md=(st=1 f=0 l=3 p=3)
012                       ms=(st=1 f=0 l=3 p=2)
		          ----------------------------
                          mg=(st=1 f=0 p=3 l=3 )
c = 2/1 = 2
l = 2 * 1 + 1 = 3

*/

/* Merge slice src into slice dst; dst != src */

int
dceslicemerge(DCEslice* dst, DCEslice* src)
{
    int err = NC_NOERR;
    DCEslice tmp;

    tmp.stride   = (dst->stride * src->stride);
    tmp.first    = (dst->first+((src->first)*(dst->stride)));
    tmp.length   = (((src->length - 1) / src->stride) * tmp.stride) + 1;
    tmp.stop     = tmp.first + tmp.length;
    tmp.count    = tmp.length / tmp.stride;
    tmp.declsize = dst->declsize;
    if(tmp.length % tmp.stride != 0) tmp.count++;
    if(tmp.first >= dst->stop || tmp.stop > dst->stop)
	err = NC_EINVALCOORDS;
    else
	*dst = tmp;
    return err;
}


/*
Given two projection lists, merge
src into dst taking
overlapping projections into acct.
*/
int
dapmergeprojections(NClist* dst, NClist* src)
{
    int i;
    NClist* cat = nclistnew();
    int ncstat = NC_NOERR;

#ifdef DEBUG
fprintf(stderr,"dapmergeprojection: dst = %s\n",dcetostring((DCEnode*)dst));
fprintf(stderr,"dapmergeprojection: src = %s\n",dcetostring((DCEnode*)src));
#endif

    /* get dst concat clone(src) */
    nclistsetalloc(cat,nclistlength(dst)+nclistlength(src));
    for(i=0;i<nclistlength(dst);i++) {
	DCEprojection* p = (DCEprojection*)nclistget(dst,i);
	nclistpush(cat,(ncelem)p);
    }    
    for(i=0;i<nclistlength(src);i++) {
	DCEprojection* p = (DCEprojection*)nclistget(src,i);
	nclistpush(cat,(ncelem)dceclone((DCEnode*)p));
    }    

    nclistclear(dst);

    /* Repeatedly pull elements from the concat,
       merge with all duplicates, and stick into
       the dst
    */
    while(nclistlength(cat) > 0) {
	DCEprojection* target = (DCEprojection*)nclistremove(cat,0);
	if(target == NULL) continue;
        if(target->discrim != CES_VAR) continue;
        for(i=0;i<nclistlength(cat);i++) {
	    DCEprojection* p2 = (DCEprojection*)nclistget(cat,i);
	    if(p2 == NULL) continue;
	    if(p2->discrim != CES_VAR) continue;
	    if(dcesamepath(target->var->segments,
			   p2->var->segments)!=0) continue;
	    /* This entry matches our current target; merge  */
	    ncstat = mergeprojection(target,p2);
	    /* null out this merged entry and release it */
	    nclistset(cat,i,(ncelem)NULL);	    
	    dcefree((DCEnode*)p2);	    
	}		    
	/* Capture the clone */
	nclistpush(dst,(ncelem)target);
    }	    
    nclistfree(cat);
    return ncstat;
}

static int
mergeprojection(DCEprojection* dst, DCEprojection* src)
{
    int ncstat = NC_NOERR;
    int i,j;

    /* merge segment by segment;
       |dst->segments| == |src->segments|
       by construction
    */
    assert((dst->discrim == CES_VAR && src->discrim == CES_VAR));
    assert((nclistlength(dst->var->segments) == nclistlength(src->var->segments)));    
    for(i=0;i<nclistlength(dst->var->segments);i++) {
	DCEsegment* dstseg = (DCEsegment*)nclistget(dst->var->segments,i);
	DCEsegment* srcseg = (DCEsegment*)nclistget(src->var->segments,i);
	for(j=0;j<dstseg->rank;j++) {
	    dceslicemerge(dstseg->slices+j,
			  srcseg->slices+j);
	}
    }
    return ncstat;
}

/* Convert an DCEprojection instance into a string
   that can be used with the url
*/

char*
buildprojectionstring(NClist* projections)
{
    char* pstring;
    NCbytes* buf = ncbytesnew();
    dcelisttobuffer(projections,buf,",");
    pstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return pstring;
}

char*
buildselectionstring(NClist* selections)
{
    NCbytes* buf = ncbytesnew();
    char* sstring;
    dcelisttobuffer(selections,buf,",");
    sstring = ncbytesdup(buf);
    ncbytesfree(buf);
    return sstring;
}

char*
buildconstraintstring(DCEconstraint* constraints)
{
    NCbytes* buf = ncbytesnew();
    char* result = NULL;
    dcetobuffer((DCEnode*)constraints,buf);
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

DCEnode*
dceclone(DCEnode* node)
{
    DCEnode* result = NULL;

    result = (DCEnode*)dcecreate(node->sort);
    if(result == NULL) goto done;

    switch (node->sort) {

    case CES_SLICE: {
	DCEslice* clone = (DCEslice*)result;
	DCEslice* orig = (DCEslice*)node;
	*clone = *orig;
    } break;

    case CES_SEGMENT: {
	DCEsegment* clone = (DCEsegment*)result;
	DCEsegment* orig = (DCEsegment*)node;
	*clone = *orig;	
        clone->name = nulldup(orig->name);
	if(orig->rank > 0)
	    memcpy(clone->slices,orig->slices,orig->rank*sizeof(DCEslice));
    } break;

    case CES_VAR: {
	DCEvar* clone = (DCEvar*)result;
	DCEvar* orig = (DCEvar*)node;
	*clone = *orig;
	clone->segments = dceclonelist(clone->segments);
    } break;

    case CES_FCN: {
	DCEfcn* clone = (DCEfcn*)result;
	DCEfcn* orig = (DCEfcn*)node;
	*clone = *orig;
        clone->name = nulldup(orig->name);
	clone->args = dceclonelist(orig->args);
    } break;

    case CES_CONST: {
	DCEconstant* clone = (DCEconstant*)result;
	DCEconstant* orig = (DCEconstant*)node;
	*clone = *orig;
        if(clone->discrim ==  CES_STR)
	    clone->text = nulldup(clone->text);
    } break;

    case CES_VALUE: {
	DCEvalue* clone = (DCEvalue*)result;
	DCEvalue* orig = (DCEvalue*)node;
	*clone = *orig;
        switch (clone->discrim) {
        case CES_CONST:
	    clone->constant = (DCEconstant*)dceclone((DCEnode*)orig->constant); break;
        case CES_VAR:
	    clone->var = (DCEvar*)dceclone((DCEnode*)orig->var); break;
        case CES_FCN:
	    clone->fcn = (DCEfcn*)dceclone((DCEnode*)orig->fcn); break;
        default: assert(0);
        }
    } break;

    case CES_PROJECT: {
	DCEprojection* clone = (DCEprojection*)result;
	DCEprojection* orig = (DCEprojection*)node;
	*clone = *orig;	
	switch (orig->discrim) {
	case CES_VAR:
            clone->var = (DCEvar*)dceclone((DCEnode*)orig->var); break;
	case CES_FCN:
            clone->fcn = (DCEfcn*)dceclone((DCEnode*)orig->fcn); break;
	default: assert(0);
	}
    } break;

    case CES_SELECT: {
	DCEselection* clone = (DCEselection*)result;
	DCEselection* orig = (DCEselection*)node;
	*clone = *orig;	
	clone->lhs = (DCEvalue*)dceclone((DCEnode*)orig->lhs);
	clone->rhs = dceclonelist(orig->rhs);
    } break;

    case CES_CONSTRAINT: {
	DCEconstraint* clone = (DCEconstraint*)result;
	DCEconstraint* orig = (DCEconstraint*)node;
	*clone = *orig;	
	clone->projections = dceclonelist(orig->projections);	
	clone->selections = dceclonelist(orig->selections);
    } break;

    default:
	assert(0);
    }

done:
    return result;
}

NClist*
dceclonelist(NClist* list)
{
    int i;
    NClist* clone;
    if(list == NULL) return NULL;
    clone = nclistnew();
    for(i=0;i<nclistlength(list);i++) {
	DCEnode* node = (DCEnode*)nclistget(list,i);
	DCEnode* newnode = dceclone((DCEnode*)node);
	nclistpush(clone,(ncelem)newnode);
    }
    return clone;
}

void
dcefree(DCEnode* node)
{
    if(node == NULL) return;

    switch (node->sort) {

    case CES_VAR: {
	DCEvar* target = (DCEvar*)node;
	dcefreelist(target->segments);	
    } break;

    case CES_FCN: {
	DCEfcn* target = (DCEfcn*)node;
	dcefreelist(target->args);	
        nullfree(target->name);
    } break;

    case CES_CONST: {
	DCEconstant* target = (DCEconstant*)node;
	if(target->discrim == CES_STR)
	    nullfree(target->text);
    } break;

    case CES_VALUE: {
	DCEvalue* target = (DCEvalue*)node;
	switch(target->discrim) {
        case CES_CONST: dcefree((DCEnode*)target->constant); break;    
        case CES_VAR: dcefree((DCEnode*)target->var); break;    
        case CES_FCN: dcefree((DCEnode*)target->fcn); break;    
        default: assert(0);
        }
    } break;

    case CES_PROJECT: {
	DCEprojection* target = (DCEprojection*)node;
	switch (target->discrim) {
	case CES_VAR: dcefree((DCEnode*)target->var); break;
	case CES_FCN: dcefree((DCEnode*)target->fcn); break;
	default: assert(0);
	}
    } break;

    case CES_SELECT: {
	DCEselection* target = (DCEselection*)node;
	dcefreelist(target->rhs);
	dcefree((DCEnode*)target->lhs);
    } break;

    case CES_CONSTRAINT: {
	DCEconstraint* target = (DCEconstraint*)node;
	dcefreelist(target->projections);
	dcefreelist(target->selections);
    } break;

    case CES_SEGMENT: {
	DCEsegment* target = (DCEsegment*)node;
	target->rank = 0;
        nullfree(target->name);
    } break;

    case CES_SLICE: {
    } break;

    default:
	assert(0);
    }

    /* final action */
    free(node);
}

void
dcefreelist(NClist* list)
{
    int i;
    if(list == NULL) return;
    for(i=0;i<nclistlength(list);i++) {
	DCEnode* node = (DCEnode*)nclistget(list,i);
	dcefree((DCEnode*)node);
    }
    nclistfree(list);
}

char*
dcetostring(DCEnode* node)
{
    char* s;
    NCbytes* buf = ncbytesnew();
    dcetobuffer(node,buf);
    s = ncbytesextract(buf);
    ncbytesfree(buf);
    return s;
}

void
dcetobuffer(DCEnode* node, NCbytes* buf)
{
    int i;
    char tmp[1024];

    if(buf == NULL) return;
    if(node == NULL) {ncbytesappend(buf,'?'); return;}

    switch (node->sort) {

    case CES_SLICE: {
	    DCEslice* slice = (DCEslice*)node;
	    size_t last = (slice->first+slice->length)-1;
	    if(last > slice->declsize && slice->declsize > 0)
	        last = slice->declsize - 1;
            if(slice->count == 1) {
                snprintf(tmp,sizeof(tmp),"[%lu]",
	            (unsigned long)slice->first);
            } else if(slice->stride == 1) {
                snprintf(tmp,sizeof(tmp),"[%lu:%lu]",
	            (unsigned long)slice->first,
	            (unsigned long)last);
            } else {
	        snprintf(tmp,sizeof(tmp),"[%lu:%lu:%lu]",
		    (unsigned long)slice->first,
		    (unsigned long)slice->stride,
		    (unsigned long)last);
	    }
            ncbytescat(buf,tmp);
    } break;

    case CES_SEGMENT: {
	DCEsegment* segment = (DCEsegment*)node;
        int rank = segment->rank;
	char* name = (segment->name?segment->name:"<unknown>");
	ncbytescat(buf,nulldup(name));
        if(!dceiswholesegment(segment)) {
	    for(i=0;i<rank;i++) {
	        DCEslice* slice = segment->slices+i;
                dcetobuffer((DCEnode*)slice,buf);
	    }
	}
    } break;

    case CES_VAR: {
	DCEvar* var = (DCEvar*)node;
	dcelisttobuffer(var->segments,buf,".");
    } break;

    case CES_FCN: {
	DCEfcn* fcn = (DCEfcn*)node;
        ncbytescat(buf,fcn->name);
        ncbytescat(buf,"(");
	dcelisttobuffer(fcn->args,buf,",");
        ncbytescat(buf,")");
    } break;

    case CES_CONST: {
	DCEconstant* value = (DCEconstant*)node;
        switch (value->discrim) {
        case CES_STR:
	    ncbytescat(buf,value->text);
	    break;		
        case CES_INT:
            snprintf(tmp,sizeof(tmp),"%lld",value->intvalue);
            ncbytescat(buf,tmp);
    	break;
        case CES_FLOAT:
            snprintf(tmp,sizeof(tmp),"%g",value->floatvalue);
            ncbytescat(buf,tmp);
	    break;
        default: assert(0);
	}
    } break;

    case CES_VALUE: {
	DCEvalue* value = (DCEvalue*)node;
        switch (value->discrim) {
        case CES_CONST:
    	    dcetobuffer((DCEnode*)value->constant,buf);
      	    break;		
        case CES_VAR:
    	    dcetobuffer((DCEnode*)value->var,buf);
    	    break;
        case CES_FCN:
    	    dcetobuffer((DCEnode*)value->fcn,buf);
    	    break;
        default: assert(0);
        }
    } break;

    case CES_PROJECT: {
	DCEprojection* target = (DCEprojection*)node;
	switch (target->discrim) {
	case CES_VAR:
	    dcetobuffer((DCEnode*)target->var,buf);
	    break;
	case CES_FCN: dcetobuffer((DCEnode*)target->fcn,buf); break;
	default: assert(0);
	}
    } break;

    case CES_SELECT: {
	DCEselection* sel = (DCEselection*)node;
	dcetobuffer((DCEnode*)sel->lhs,buf);
        if(sel->operator == CES_NIL) break;
        ncbytescat(buf,opstrings[(int)sel->operator]);
        if(nclistlength(sel->rhs) > 1)
            ncbytescat(buf,"{");
	dcelisttobuffer(sel->rhs,buf,",");
        if(nclistlength(sel->rhs) > 1)
	    ncbytescat(buf,"}");
    } break;

    case CES_CONSTRAINT: {
	DCEconstraint* con = (DCEconstraint*)node;
        if(con->projections != NULL && nclistlength(con->projections) > 0) {
            dcelisttobuffer(con->projections,buf,",");
	}
        if(con->selections != NULL && nclistlength(con->selections) > 0) {
	    ncbytescat(buf,"&"); /* because & is really a prefix */
            dcelisttobuffer(con->selections,buf,"&");
	}
    } break;

    case CES_NIL: {
	ncbytescat(buf,"<nil>");
    } break;

    default:
	assert(0);
    }
}

char*
dcelisttostring(NClist* list, char* sep)
{
    char* s;
    NCbytes* buf = ncbytesnew();
    dcelisttobuffer(list,buf,sep);
    s = ncbytesextract(buf);
    ncbytesfree(buf);
    return s;
}

void
dcelisttobuffer(NClist* list, NCbytes* buf, char* sep)
{
    int i;
    if(list == NULL || buf == NULL) return;
    if(sep == NULL) sep = ",";
    for(i=0;i<nclistlength(list);i++) {
	DCEnode* node = (DCEnode*)nclistget(list,i);
	if(i>0) ncbytescat(buf,sep);
	dcetobuffer((DCEnode*)node,buf);
    }
}

/* Collect all nodes within a specified constraint tree */
/* Caller frees result */
NClist*
dceallnodes(DCEnode* node, CEsort which)
{
    NClist* allnodes = nclistnew();
    ceallnodesr(node,allnodes,which);
    return allnodes;
}

static void
ceallnodesr(DCEnode* node, NClist* allnodes, CEsort which)
{
    int i;
    if(node == NULL) return;
    if(nclistcontains(allnodes,(ncelem)node)) return;
    if(which == CES_NIL || node->sort == which)
        nclistpush(allnodes,(ncelem)node);
    switch(node->sort) {
    case CES_FCN: {
	DCEfcn* fcn = (DCEfcn*)node;
	for(i=0;i<nclistlength(fcn->args);i++) {
	    ceallnodesr((DCEnode*)nclistget(fcn->args,i),allnodes,which);
	}
    } break;
    case CES_VAR: {
	DCEvar* var = (DCEvar*)node;
	for(i=0;i<nclistlength(var->segments);i++) {
	    ceallnodesr((DCEnode*)nclistget(var->segments,i),allnodes,which);
	}
    } break;
    case CES_VALUE: {
	DCEvalue* value = (DCEvalue*)node;
	if(value->discrim == CES_VAR)
	    ceallnodesr((DCEnode*)value->var,allnodes,which);
	else if(value->discrim == CES_FCN)
	    ceallnodesr((DCEnode*)value->fcn,allnodes,which);
	else
	    ceallnodesr((DCEnode*)value->constant,allnodes,which);
    } break;
    case CES_SELECT: {
	DCEselection* selection = (DCEselection*)node;
        ceallnodesr((DCEnode*)selection->lhs,allnodes,which);
	for(i=0;i<nclistlength(selection->rhs);i++)
            ceallnodesr((DCEnode*)nclistget(selection->rhs,i),allnodes,which);
    } break;
    case CES_PROJECT: {
	DCEprojection* projection = (DCEprojection*)node;
	if(projection->discrim == CES_VAR)
	    ceallnodesr((DCEnode*)projection->var,allnodes,which);
	else
	    ceallnodesr((DCEnode*)projection->fcn,allnodes,which);
    } break;
    case CES_CONSTRAINT: {
	DCEconstraint* constraint = (DCEconstraint*)node;
	for(i=0;i<nclistlength(constraint->projections);i++)
	    ceallnodesr((DCEnode*)nclistget(constraint->projections,i),allnodes,which);
	for(i=0;i<nclistlength(constraint->selections);i++)
	    ceallnodesr((DCEnode*)nclistget(constraint->selections,i),allnodes,which);
    } break;

    /* All others have no subnodes */
    default:
	break;
    }
}

DCEnode*
dcecreate(CEsort sort)
{
    DCEnode* node = NULL;

    switch (sort) {

    case CES_SLICE: {
	DCEslice* target = (DCEslice*)calloc(1,sizeof(DCEslice));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
    } break;

    case CES_SEGMENT: {
	int i;
	DCEsegment* target = (DCEsegment*)calloc(1,sizeof(DCEsegment));
	if(target == NULL) return NULL;
	/* Initialize the sort of the slices */
	for(i=0;i<NC_MAX_VAR_DIMS;i++)
	    target->slices[i].node.sort = CES_SLICE;		
	node = (DCEnode*)target;
    } break;

    case CES_CONST: {
	DCEconstant* target = (DCEconstant*)calloc(1,sizeof(DCEconstant));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
	target->discrim = CES_NIL;
    } break;

    case CES_VALUE: {
	DCEvalue* target = (DCEvalue*)calloc(1,sizeof(DCEvalue));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
	target->discrim = CES_NIL;
    } break;

    case CES_VAR: {
	DCEvar* target = (DCEvar*)calloc(1,sizeof(DCEvar));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
    } break;

    case CES_FCN: {
	DCEfcn* target = (DCEfcn*)calloc(1,sizeof(DCEfcn));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
    } break;

    case CES_PROJECT: {
	DCEprojection* target = (DCEprojection*)calloc(1,sizeof(DCEprojection));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
    } break;

    case CES_SELECT: {
	DCEselection* target = (DCEselection*)calloc(1,sizeof(DCEselection));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
	target->operator = CEO_NIL;
    } break;

    case CES_CONSTRAINT: {
	DCEconstraint* target = (DCEconstraint*)calloc(1,sizeof(DCEconstraint));
	if(target == NULL) return NULL;
	node = (DCEnode*)target;
    } break;

    default:
	assert(0);
    }

    /* final action */
    node->sort = sort;
    return node;
}

void
dcemakewholeslice(DCEslice* slice, size_t declsize)
{
    slice->first = 0;
    slice->stride = 1;
    slice->length = declsize;
    slice->stop = declsize;
    slice->declsize = declsize;
    slice->count = declsize;
}

int
dceiswholeslice(DCEslice* slice)
{
    if(slice->first != 0 || slice->stride != 1) return 0;
    return 1;
}

int
dceiswholesegment(DCEsegment* seg)
{
    int i,whole;
    
    if(!seg->slicesdefined) return 0; /* actually, we don't know */
    whole = 1; /* assume so */
    for(i=0;i<seg->rank;i++) {
	if(!dceiswholeslice(&seg->slices[i])) {whole = 0; break;}	
    }
    return whole;
}

int
dceiswholevar(DCEvar* var)
{
    int i,whole;
    whole = 1; /* assume so */
    for(i=0;i<nclistlength(var->segments);i++) {
        DCEsegment* segment = (DCEsegment*)nclistget(var->segments,i);
	if(!dceiswholesegment(segment)) {whole = 0; break;}	
    }
    return whole;
}

int
dcesamepath(NClist* list1, NClist* list2)
{
    int i;
    int len = nclistlength(list1);
    if(len != nclistlength(list2)) return 0;
    for(i=0;i<len;i++) {
	DCEsegment* s1 = (DCEsegment*)nclistget(list1,i);
	DCEsegment* s2 = (DCEsegment*)nclistget(list2,i);
	if(strcmp(s1->name,s2->name) != 0) return 0;
    }
    return 1;
}
