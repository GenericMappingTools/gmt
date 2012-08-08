/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/* Parser actions for constraint expressions */

/* Since oc does not use the constraint parser,
   they functions all just abort if called.
*/

#include "includes.h"
#include "cceconstraints.h"
#include "cceparselex.h"

#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif
#ifndef nullfree
#define nullfree(s) if((s)!=NULL) {free(s);} else {}
#endif

#ifdef PARSEDEBUG
extern int ccedebug;
#endif

static int ccelex(YYSTYPE* lvalp, CCEparsestate* state);
static void ccelexinit(char* input, CCElexstate** lexstatep);
static void ccelexcleanup(CCElexstate** lexstatep);

static Object collectlist(Object list0, Object decl);

static void
projections(CCEparsestate* state, Object list0)
{
    NClist* list = (NClist*)list0;
    if(list != NULL) {
        nclistfree(state->constraint->projections);
        state->constraint->projections = list;
    }
#ifdef DEBUG
fprintf(stderr,"	ce.projections: %s\n",
	ccetostring((CCEnode*)state->constraint));
#endif
}

static Object
projectionlist(CCEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}

static Object
projection(CCEparsestate* state, Object segments0)
{
    CCEprojection* p = (CCEprojection*)ccecreate(CES_PROJECT);
    NClist* segments = (NClist*)segments0;
    p->segments = segments;
#ifdef DEBUG
fprintf(stderr,"	ce.projection: %s\n",
	ccetostring((CCEnode*)p));
#endif
    return p;
}

static Object
segmentlist(CCEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}

static Object
segment(CCEparsestate* state, Object name, Object slices0)
{
    int i;
    CCEsegment* segment = (CCEsegment*)ccecreate(CES_SEGMENT);
    NClist* slices = (NClist*)slices0;
    segment->name = strdup((char*)name);
    if(slices != NULL && nclistlength(slices) > 0) {
        segment->slicesdefined = 1; /* but not declsizes */
	for(i=0;i<nclistlength(slices);i++) {
	    CCEslice* slice = (CCEslice*)nclistget(slices,i);
	    segment->slices[i] = *slice;
	    free(slice);
	}
	nclistfree(slices);
    } else
        segment->slicesdefined = 0;
#ifdef DEBUG
fprintf(stderr,"	ce.segment: %s\n",
	ccetostring((CCEnode*)segment));
#endif
    return segment;
}


static Object
rangelist(CCEparsestate* state, Object list0, Object decl)
{
    return collectlist(list0,decl);
}

static Object
range(CCEparsestate* state, Object sfirst, Object sstride, Object slast)
{
    CCEslice* slice = (CCEslice*)ccecreate(CES_SLICE);
    unsigned long first,stride,last;

    /* Note: that incoming arguments are strings; we must convert to size_t;
       but we do know they are legal integers or NULL */
    sscanf((char*)sfirst,"%lu",&first); /* always defined */
    if(slast != NULL)
        sscanf((char*)slast,"%lu",&last);
    else
	last = first;
    if(sstride != NULL)
        sscanf((char*)sstride,"%lu",&stride);
    else
	stride = 1; /* default */

    if(stride == 0)
    	cceerror(state,"Illegal index for range stride");
    if(last < first)
	cceerror(state,"Illegal index for range last index");
    slice->first  = first;
    slice->stride = stride;
    slice->stop   = last + 1;
    slice->length  = slice->stop - slice->first;
    slice->count  = slice->length / slice->stride;
#ifdef DEBUG
fprintf(stderr,"	ce.slice: %s\n",
	ccetostring((CCEnode*)slice));
#endif
    return slice;
}


static Object
collectlist(Object list0, Object decl)
{
    NClist* list = (NClist*)list0;
    if(list == NULL) list = nclistnew();
    nclistpush(list,(ncelem)decl);
    return list;
}

int
cceerror(CCEparsestate* state, char* msg)
{
    strcpy(state->errorbuf,msg);
    state->errorcode=1;
    return 0;
}

static void
cce_parse_cleanup(CCEparsestate* state)
{
    ccelexcleanup(&state->lexstate); /* will free */
}

static CCEparsestate*
ce_parse_init(char* input, CCEconstraint* constraint)
{
    CCEparsestate* state = NULL;
    if(input==NULL) {
        cceerror(state,"ce_parse_init: no input buffer");
    } else {
        state = (CCEparsestate*)calloc(1,sizeof(CCEparsestate));
        if(state==NULL) return (CCEparsestate*)NULL;
        state->errorbuf[0] = '\0';
        state->errorcode = 0;
        ccelexinit(input,&state->lexstate);
	state->constraint = constraint;
    }
    return state;
}

/* Wrapper for ceparse */
int
cdmceparse(char* input, CCEconstraint* constraint, char** errmsgp)
{
    CCEparsestate* state;
    int errcode = 0;

#ifdef PARSEDEBUG
ccedebug = 1;
#endif

    if(input != NULL) {
#ifdef DEBUG
fprintf(stderr,"cceeparse: input=%s\n",input);
#endif
        state = ce_parse_init(input,constraint);
        if(cceparse(state) == 0) {
#ifdef DEBUG
if(nclistlength(constraint->projections) > 0)
fprintf(stderr,"cceeparse: projections=%s\n",
        ccetostring((CCEnode*)constraint));
#endif
	} else {
	    if(errmsgp) *errmsgp = nulldup(state->errorbuf);
	}
	errcode = state->errorcode;
        cce_parse_cleanup(state);
    }
    return errcode;
}

#ifdef PARSEDEBUG
Object
debugobject(Object o)
{
    return o;
}
#endif

#include "ccelex.c"

#include "ccetab.c"
