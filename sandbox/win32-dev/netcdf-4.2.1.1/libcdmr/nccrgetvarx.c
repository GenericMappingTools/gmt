/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "includes.h"
#include "nccrgetvarx.h"
#include "nccrcvt.h"


#define DATAPREFIX "?req=data&var="


/* Forward */
static int makegetvar(NCCDMR*, CRnode*, void*, nc_type, NCCRgetvarx**);

static int crfetchdata(NCCDMR* cdmr, char* p, bytes_t* bufp, long* filetimep);

static int nccr_getcontent(NCCDMR* cdmr, NCCRgetvarx* varxinfo, bytes_t, size_t, void* data);

static void freegetvarx(NCCRgetvarx* varx);

static CRnode* locatevar(NCCDMR* cdmr, Data* data);

static char* urlconstraintstring(CCEprojection*);


/**************************************************/
int 
NCCR_getvarx(int ncid, int varid,
	      const size_t* startp,
	      const size_t* countp,
	      const ptrdiff_t* stridep,
	      void* data,
	      nc_type externaltype0)
{
    int ncstat = NC_NOERR;
    unsigned int i;
    NCCRgetvarx* varxinfo = NULL;
    CCEprojection* varxprojection = NULL;
    NC* drno;
    NCCDMR* cdmr;
    size_t localcount[NC_MAX_VAR_DIMS];
    NClist* vars = NULL;
    DataType datatype;
    nc_type internaltype;
    nc_type externaltype = externaltype0;
    CRshape shape;
    bytes_t buf = bytes_t_null;
    char* projectionstring = NULL;
    size_t offset;
    CRnode* streamvar; /* stream node mapping to var*/

    LOG((2, "nccr_get_varx: ncid 0x%x varid %d", ncid, varid));

    ncstat = NC_check_id(ncid, (NC**)&drno); 
    if(ncstat != NC_NOERR) goto done;
    cdmr = (NCCDMR*)drno->dispatchdata;

    /* Find the CRnode instance for this ncid */
    for(i=0;i<nclistlength(cdmr->variables);i++) {
	streamvar = (CRnode*)nclistget(cdmr->variables,i);
	if(streamvar->ncid == varid) break;
    }
    if(streamvar == NULL) {ncstat = NC_ENOTVAR; goto done;}

    ASSERT((streamvar->sort == _Variable || streamvar->sort == _Structure));

    /* Get the dimension info and typing */
    if(streamvar->sort == _Variable) {
        Variable* var = (Variable*)streamvar;
	crextractshape((CRnode*)var,&shape);
	datatype = var->dataType;
    } else if(streamvar->sort == _Structure) {
        Structure* var = (Structure*)streamvar;
	crextractshape((CRnode*)var,&shape);
	datatype = var->dataType;
    }

    /* Fill in missing arguments */
    if(startp == NULL)
	startp = nc_sizevector0;

    if(countp == NULL) {
        /* Accumulate the dimension sizes */
        for(i=0;i<shape.rank;i++) {
	    Dimension* dim = shape.dims[i];
	    localcount[i] = dimsize(dim);
	}
	countp = localcount;
    }

    if(stridep == NULL)
	stridep = nc_ptrdiffvector1;

    /* Validate the dimension sizes */
    for(i=0;i<shape.rank;i++) {
        Dimension* dim = shape.dims[i];
	if(startp[i] > dimsize(dim)
	   || startp[i]+countp[i] > dimsize(dim)) {
	    ncstat = NC_EINVALCOORDS;
	    goto done;	    
	}
    }	     

    /* Default to using the var type */
    internaltype = cvtstreamtonc(datatype);
    if(externaltype == NC_NAT) externaltype = internaltype;

    /* Validate any implied type conversion*/
    if(internaltype != externaltype && externaltype == NC_CHAR) {
	/* The only disallowed conversion is to/from char and non-byte
           numeric types*/
	switch (internaltype) {
	case NC_STRING:
	case NC_CHAR: case NC_BYTE: case NC_UBYTE:
	    break;
	default:
	    THROWCHK(NC_ECHAR);
	    goto done;
	}
    }

    ncstat = makegetvar(cdmr,streamvar,data,externaltype,&varxinfo);
    if(ncstat) {THROWCHK(NC_ENOMEM); goto done;}

    /* Load with constraints */
    vars = nclistnew();
    nclistpush(vars,(ncelem)varxinfo->target);

    /* Find the relevant projection from the predefined projections */
    for(i=0;i<nclistlength(cdmr->urlconstraint->projections);i++) {
	CCEprojection* p = (CCEprojection*)nclistget(cdmr->urlconstraint->projections,i);
	if(p->decl == streamvar) {varxinfo->projection = (CCEprojection*)cceclone((CCEnode*)p); break;}
    }
    ASSERT(varxinfo->projection != NULL); /* By construction */

    /* merge the getvarx start/stride/stop into the existing projection (as cloned) */
    ncstat = ccerestrictprojection(varxinfo->projection,shape.rank,startp,countp,stridep);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

#ifdef DEBUG
fprintf(stderr,"varx merge: %s\n",
	ccetostring((CCEnode*)varxinfo->projection));
#endif

    /* Convert the projections into a string for use
       with crfetchdata*/
    
    projectionstring = urlconstraintstring(varxinfo->projection);
#ifdef DEBUG
fprintf(stderr,"projectionstring: %s\n",projectionstring);
#endif

    /* Fetch */

    buf = bytes_t_null;
    ncstat = crfetchdata(cdmr,projectionstring,&buf,NULL);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    
    if(projectionstring) free(projectionstring); /* no longer needed */  

    /* Parse the data header */
    ncstat = nccr_decodedatamessage(&buf,&cdmr->datahdr,&offset);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Verify the variable in the data header */
    ASSERT((locatevar(cdmr,cdmr->datahdr) == streamvar));
 
    /* move the data into user's memory; watch out buf is structure copy */
    ncstat = nccr_getcontent(cdmr,varxinfo,buf,offset,data);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

done:
    if(buf.bytes != NULL) free(buf.bytes);
    ccefree((CCEnode*)varxprojection);
    freegetvarx(varxinfo);
    return THROW(ncstat);
}

static char*
urlconstraintstring(CCEprojection* projection)
{
    char* projectionstring = ccetostring((CCEnode*)projection);
    return projectionstring;
}

static void
freegetvarx(NCCRgetvarx* varx)
{
    if(varx == NULL) return;
    ccefree((CCEnode*)varx->projection);
    free(varx);
}

static int
makegetvar(NCCDMR* cdmr, CRnode* var, void* data, nc_type externaltype, NCCRgetvarx** varxp)
{
    NCCRgetvarx* varx;
    int ncstat = NC_NOERR;

    varx = (NCCRgetvarx*)calloc(1,sizeof(NCCRgetvarx));
    MEMCHECK(varx,NC_ENOMEM);
    if(varxp) *varxp = varx;

    varx->target = var;
    varx->externaltype = externaltype;
    varx->internaltype = cvtstreamtonc(nccr_gettype(var));
    varx->projection = NULL;
    return ncstat;
}

#ifdef IGNORE
/* In order to construct the projection,
we need to make sure to match the relevant dimensions
against the relevant nodes in which the ultimate target
is contained.
*/
static int
buildvarxprojection(NCCRgetvarx* varxinfo,
		     const size_t* startp,
                     const size_t* countp,
		     const ptrdiff_t* stridep,
		     CCEprojection** projectionp)
{
    int i;
    int ncstat = NC_NOERR;
    CRnode* var = varxinfo->target;
    CCEprojection* projection = NULL;
    NClist* segments = NULL;
    CCEsegment* segment;
    CRshape shape;

    segment = (CCEsegment*)ccecreate(CES_SEGMENT);
    segment->decl = var;
    ASSERT((segment->decl != NULL));
    segment->name = nulldup(nccr_getname(segment->decl));
    segment->slicesdefined = 0; /* temporary */
    segment->slicesdeclized = 0; /* temporary */
    segments = nclistnew();
    nclistpush(segments,(ncelem)segment);

    projection = (CCEprojection*)ccecreate(CES_PROJECT);
    projection->decl = var;
    projection->segments = segments;

    crextractshape((CRnode*)var,&shape);
    segment->rank = shape.rank;
    for(i=0;i<segment->rank;i++) { 
        CCEslice* slice = &segment->slices[i];
	Dimension* dim = shape.dims[i];
        slice->first = startp[i];
	slice->stride = stridep[i];
	slice->count = countp[i];
        slice->length = slice->count * slice->stride;
	slice->stop = (slice->first + slice->length);
	ASSERT(dimsize(dim) > 0);
    	slice->declsize = dimsize(dim);
    }
    segment->slicesdefined = 1;
    segment->slicesdeclized = 1;

    if(projectionp) *projectionp = projection;
    if(ncstat) ccefree((CCEnode*)projection);
    return ncstat;
}
#endif

static int
crfetchdata(NCCDMR* cdmr, char* projection, bytes_t* bufp, long* filetimep)
{
    int ncstat = NC_NOERR;
    char* curlurl;
    long filetime;
    bytes_t buf;
    char* fullprojection;
    size_t len;

    /* Construct the complete projection suffix */
    len = strlen(projection);
    len += strlen(DATAPREFIX);
    len++; /* null terminator */
    fullprojection = (char*) malloc(len);
    if(fullprojection == NULL) {ncstat=NC_ENOMEM; goto done;}
    strcpy(fullprojection,DATAPREFIX);
    strcat(fullprojection,projection);

    /* fetch data */
    buf = bytes_t_null;
    curlurl = nc_uribuild(cdmr->uri,NULL,fullprojection,0);
    free(fullprojection);
    if(curlurl == NULL) {ncstat=NC_ENOMEM; goto done;}
    ncstat = nccr_fetchurl(cdmr,cdmr->curl.curl,curlurl,&buf,&filetime);
    free(curlurl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    if(filetimep) *filetimep = filetime;
    if(bufp) *bufp = buf;
    
done:
    return ncstat;
}

/*
Find the node in the Header* tree matching the var in Data tree.
Note that this needs to eventually change because storing the
variables's full path name introduces the possibility of ambiguity
if the name uses non-standard characters. The solution is
to replace the varName in struct Data with a vector of names
(similar to CRpath).
*/

static CRnode*
locatevar(NCCDMR* cdmr, Data* data)
{
    char segment[1024];
    char* p;
    CRnode* match = NULL;
    int i;
    CRpath* path = NULL;

    /* Split the name at the dots and store the pieces */
    p = data->varName;
    for(;;) {
	ptrdiff_t len;
	char* dot = strchr(p,'.');
	if(dot == NULL) break;	
	len = (dot-p);
	if(len >= sizeof(segment)) len = sizeof(segment-1);	
	strncpy(segment,dot,len);
	segment[len] = '\0';
	path = crpathappend(path,segment);
	p = dot+1;
    }
    /* do the last segment */
    if(*p) {
	path = crpathappend(path,p);
    }	
    
    /* Now locate the matching variable */
    for(i=0;i<nclistlength(cdmr->variables);i++) {
	CRnode* test = (CRnode*)nclistget(cdmr->variables,i);
	if(crpathmatch(test->pathname,path) == 1) {
	    if(match != NULL) {
		nclog(NCLOGERR,"Ambiguous name in Data object");
	    } else match = test;
	}
    }
    if(match == NULL) {
	nclog(NCLOGERR,"Cannot locate variable in Data object");
    }    
    return match;
}


/*
Assumptions:
1. there is only 1 projection variable in response

Not-Assumed:
*/


static int
nccr_getcontent(NCCDMR* cdmr, NCCRgetvarx* varxinfo, bytes_t data, size_t offset, void* memory)
{
    int ncstat = NC_NOERR;
    int i;
    size_t count;
    int localbig = ((cdmr->controls & BIGENDIAN)?1:0);
    int databig = ((cdmr->datahdr->bigend.defined
			   && cdmr->datahdr->bigend.value)?1:0);
    int swap = (localbig == databig?0:1);
    int internaltypesize = nctypelen(varxinfo->internaltype);

    /* modify data */
    data.nbytes -= offset;
    data.bytes += offset;

    /* First, check to see if any type conversion will be necessary */
    if(varxinfo->internaltype == varxinfo->externaltype) {
	/* We can short circuit the transfer and not do any type conversion */
	/* Separate out the integer-like types from the string types */

	switch(varxinfo->internaltype) {

	default: { /* numeric types */
	    /* Compute the amount to move */
	    size_t vlen = 0;
	    ncstat = nccr_decodedatacount(&data,&vlen,&count);
	    if(ncstat) {goto done;}
	    count = count/internaltypesize;
	    /* swap and convert */
	    ncstat = nccrconvert(varxinfo->internaltype,varxinfo->externaltype,
				 data.bytes+vlen,memory,
				 count,swap);
	    if(ncstat) {goto done;}
	} break;

	case ENUM1:
	case ENUM2:
	case ENUM4:
	    break;

	/* Variable length types */
	case OPAQUE:
	case STRING: {
	    /* Get count of the number of objects */
	    size_t nobjects;
	    size_t len;
	    char** pp = (char**)memory;
	    size_t localoffset = offset;
	    ncstat = nccr_decodedatacount(&data,&localoffset,&nobjects);
	    if(ncstat) {goto done;}
	    for(i=0;i<nobjects;i++) {
		char* p;
		/* Get the length (|length|) */		
	        ncstat = nccr_decodedatacount(&data,&localoffset,&len);
 		if(ncstat) {goto done;}
	        p = (char*)malloc(len+1);
		memcpy(p,data.bytes+localoffset,len);				
		p[len] = '\0';
		*pp++ = p;
	    }
	} break;

	/* We should never see these */
	case SEQUENCE:
	case STRUCTURE:
	    PANIC("unexpected internal type");
	}
    } else {/* Guess we have to do the conversions */
	size_t vlen;
	ncstat = nccr_decodedatacount(&data,&vlen,&count);
	if(ncstat) {goto done;}

	switch(varxinfo->internaltype) {

	default: {
	    ncstat = nccrconvert(varxinfo->internaltype,varxinfo->externaltype,
			     data.bytes+vlen,memory,
			     count,swap);
	    if(ncstat) {goto done;}
	} break;

	/* Variable length types */
	case OPAQUE:
	case STRING: {
	    /* Get count of the number of objects */
	    size_t nobjects;
	    size_t len, vlen;
	    char** pp = (char**)memory;
	    ncstat = nccr_decodedatacount(&data,&vlen,&nobjects);
	    if(ncstat) {goto done;}
	    for(i=0;i<nobjects;i++) {
		char* p;
		/* Get the length (|length|) */		
	        ncstat = nccr_decodedatacount(&data,&vlen,&len);
 		if(ncstat) {goto done;}
	        p = (char*)malloc(len+1);
		memcpy(p,data.bytes+vlen,len);				
		p[len] = '\0';
		*pp++ = p;
	    }
	} break;

	/* We should never see these */
	case SEQUENCE:
	case STRUCTURE:
	    PANIC("unexpected internal type");
	}
    }
    
done:
    return ncstat;    
}
