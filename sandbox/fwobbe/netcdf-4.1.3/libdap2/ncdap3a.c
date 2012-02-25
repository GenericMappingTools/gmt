/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/ncdap3.c,v 1.94 2010/05/28 01:05:34 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "nc3dispatch.h"
#include "ncd3dispatch.h"
#include "dapalign.h"
#include "oc.h"
#include "ocdrno.h"
#include "dapdump.h"


#define getncid(drno) (((NC*)drno)->ext_ncid)

/*Forward*/
static NCerror getseqdimsize(NCDAPCOMMON*, CDFnode* seq, size_t* sizep);
static int fieldindex(CDFnode* parent, CDFnode* child);
static NCerror countsequence(NCDAPCOMMON*, CDFnode* node, size_t*);
static NCerror makeseqdim(NCDAPCOMMON*, CDFnode* node, size_t, CDFnode**);
static NCerror computeminconstraints3(NCDAPCOMMON*,CDFnode*,NCbytes*);

void
freegetvara(Getvara* vara)
{
    if(vara == NULL) return;
    dcefree((DCEnode*)vara->varaprojection);
    nullfree(vara);
}

NCerror
cleanNCDAPCOMMON(NCDAPCOMMON* nccomm)
{
    freenccache(nccomm,nccomm->cdf.cache);
    nclistfree(nccomm->cdf.varnodes);
    nclistfree(nccomm->cdf.seqnodes);
    nclistfree(nccomm->cdf.gridnodes);
    nclistfree(nccomm->cdf.usertypes);
    nullfree(nccomm->cdf.recorddim);

    /* free the trees */
    freecdfroot34(nccomm->cdf.ddsroot);
    nccomm->cdf.ddsroot = NULL;
    if(nccomm->oc.ocdasroot != NULL)
        oc_root_free(nccomm->oc.conn,nccomm->oc.ocdasroot);
    nccomm->oc.ocdasroot = NULL;
    oc_close(nccomm->oc.conn); /* also reclaims remaining OC trees */
    ocurifree(nccomm->oc.uri);
    nullfree(nccomm->oc.urltext);

    dcefree((DCEnode*)nccomm->oc.dapconstraint);
    nccomm->oc.dapconstraint = NULL;

    return NC_NOERR;
}


NCerror
cleanNCDAP3(NCDAP3* drno)
{
    return cleanNCDAPCOMMON(&drno->dap);
}


#ifdef IGNORE
/* Given a path, collect the set of dimensions along that path */
static void
collectdims3(NClist* path, NClist* dimset)
{
    int i,j;
    nclistclear(dimset);
    for(i=0;i<nclistlength(path);i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(node->nctype == NC_Sequence) {
	    CDFnode* sqdim = (CDFnode*)nclistget(node->array.dimensions,0);
	    if(DIMFLAG(sqdim,CDFDIMUNLIM))
		nclistclear(dimset); /* unlimited is always first */
        }
	for(j=0;j<nclistlength(node->array.dimensions);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
	    nclistpush(dimset,(ncelem)dim);
	}
	if(node->array.stringdim != NULL) 
	    nclistpush(dimset,(ncelem)node->array.stringdim);
    }
}
#endif

NCerror
addstringdims(NCDAP3* drno)
{
    /* for all variables of string type, we will need another dimension
       to represent the string; Accumulate the needed sizes and create
       the dimensions with a specific name: either as specified
       in DODS{...} attribute set or defaulting to the variable name.
       All such dimensions are global.
    */
    int i;
    NClist* varnodes = drno->dap.cdf.varnodes;
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	CDFnode* sdim = NULL;
	char dimname[4096];
	size_t dimsize;

	if(var->etype != NC_STRING && var->etype != NC_URL) continue;
	/* check is a string length was specified */
	if(var->dodsspecial.maxstrlen > 0)
	    dimsize = var->dodsspecial.maxstrlen;
	else
	    dimsize = var->maxstringlength;
	/* create a psuedo dimension for the charification of the string*/
	if(var->dodsspecial.dimname != NULL) {
	    strncpy(dimname,var->dodsspecial.dimname,sizeof(dimname));
	} else {
	    snprintf(dimname,sizeof(dimname),"maxStrlen%lu",
			(unsigned long)dimsize);
	}
	sdim = makecdfnode34(&drno->dap, dimname, OC_Dimension, OCNULL,
                             drno->dap.cdf.ddsroot);
	if(sdim == NULL) return THROW(NC_ENOMEM);
	nclistpush(drno->dap.cdf.ddsroot->tree->nodes,(ncelem)sdim);
	sdim->dim.dimflags |= CDFDIMSTRING;
	sdim->dim.declsize = dimsize;
	nullfree(sdim->ncbasename);
	nullfree(sdim->ncfullname);
	sdim->ncbasename = cdflegalname3(sdim->name);
	sdim->ncfullname = nulldup(sdim->ncbasename);
	/* tag the variable with its string dimension*/
	var->array.stringdim = sdim;
    }
    return NC_NOERR;
}

NCerror
defrecorddim3(NCDAP3* drno)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;
    NClist* alldims;

    ASSERT((drno->dap.cdf.recorddim != NULL));

    /* Locate the dimension matching the record dim */
    alldims = getalldims3(drno->dap.cdf.varnodes,1);
    for(i=0;i<nclistlength(alldims);i++) {
        CDFnode* dim = (CDFnode*)nclistget(alldims,i);
	if(dim->nctype != NC_Dimension) continue;    
	if(dim->dim.basedim != NULL) continue; /* not the controlling dim */
	if(strcmp(dim->name,drno->dap.cdf.recorddim) != 0) continue;
	if(DIMFLAG(dim,CDFDIMCLONE)) PANIC("cloned record dim");
	if(drno->dap.cdf.unlimited != NULL) PANIC("Multiple unlimited");
        DIMFLAGSET(dim,CDFDIMUNLIM|CDFDIMRECORD);
	drno->dap.cdf.unlimited = dim;
    }
    nclistfree(alldims);
    /* Now, locate all the string dims and see if they are the record dim,
       then replace */
    if(drno->dap.cdf.unlimited != NULL) {
	CDFnode* unlim = drno->dap.cdf.unlimited;
        for(i=0;i<nclistlength(drno->dap.cdf.varnodes);i++) {
            CDFnode* var = (CDFnode*)nclistget(drno->dap.cdf.varnodes,i);
	    CDFnode* sdim = var->array.stringdim;
            if(sdim == NULL) continue;
	    if(strcmp(sdim->ncfullname,unlim->ncfullname)==0
	       && sdim->dim.declsize == unlim->dim.declsize) {
	        var->array.stringdim = unlim;
	        nclistpop(var->array.dimensions);
	        nclistpush(var->array.dimensions,(ncelem)drno->dap.cdf.unlimited);
	    }
	}
    }

    return ncstat;
}

NCerror
defseqdims(NCDAP3* drno)
{
    unsigned int i;
    CDFnode* unlimited = NULL;
    NCerror ncstat = NC_NOERR;
    int seqdims = 1; /* default is to compute seq dims counts */

    /* Does the user want to see which dims are sequence dims? */
    if(paramcheck34(&drno->dap,"show","seqdims")) seqdims = 0;
  
    /* Build the unlimited node if needed */
    if(!FLAGSET(drno->dap.controls,NCF_NOUNLIM)) {
        unlimited = makecdfnode34(&drno->dap,"unlimited",OC_Dimension,OCNULL,drno->dap.cdf.ddsroot);
        nclistpush(drno->dap.cdf.ddsroot->tree->nodes,(ncelem)unlimited);
        nullfree(unlimited->ncbasename);
        nullfree(unlimited->ncfullname);
        unlimited->ncbasename = cdflegalname3(unlimited->name);
        unlimited->ncfullname = nulldup(unlimited->ncbasename);
        DIMFLAGSET(unlimited,CDFDIMUNLIM);
        drno->dap.cdf.unlimited = unlimited;
    }

    /* Compute and define pseudo dimensions for all sequences */

    for(i=0;i<nclistlength(drno->dap.cdf.seqnodes);i++) {
        CDFnode* seq = (CDFnode*)nclistget(drno->dap.cdf.seqnodes,i);
	CDFnode* sqdim;
	size_t seqsize;

        seq->array.dimensions = nclistnew();

	if(!seq->usesequence) {
	    /* Mark sequence with unlimited dimension */
	    seq->array.seqdim = unlimited;
	    nclistpush(seq->array.dimensions,(ncelem)unlimited);
	    continue;
	}

	/* Does the user want us to compute the sequence dim size? */
	sqdim = NULL;
	if(seqdims) {
	    ncstat = getseqdimsize(&drno->dap,seq,&seqsize);
	    if(ncstat != NC_NOERR) {
                /* Cannot get DATADDDS; convert to unlimited */
		sqdim = unlimited;
	    }
	} else { /* !seqdims default to size = 1 */
	    seqsize = 1; 
	}
	if(sqdim == NULL) {
	    /* Note: we are making the dimension in the dds root tree */
            ncstat = makeseqdim(&drno->dap,seq,seqsize,&sqdim);
            if(ncstat) goto fail;
	}
        seq->array.seqdim = sqdim;
	nclistpush(seq->array.dimensions,(ncelem)sqdim);
    }

fail:
    return ncstat;
}

static NCerror
getseqdimsize(NCDAPCOMMON* nccomm, CDFnode* seq, size_t* sizep)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCconnection conn = nccomm->oc.conn;
    OCdata rootcontent = OCNULL;
    OCobject ocroot;
    CDFnode* dxdroot;
    CDFnode* xseq;
    NCbytes* minconstraints = ncbytesnew();
    size_t seqsize;

    /* Read the minimal amount of data in order to get the count */
    /* If the url is unconstrainable, then get the whole thing */
    computeminconstraints3(nccomm,seq,minconstraints);
#ifdef DEBUG
fprintf(stderr,"minconstraints: %s\n",ncbytescontents(minconstraints));
#endif
    /* Obtain the record counts for the sequence */
    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
        ocstat = dap_oc_fetch(nccomm,conn,NULL,OCDATADDS,&ocroot);
    else
        ocstat = dap_oc_fetch(nccomm,conn,ncbytescontents(minconstraints),OCDATADDS,&ocroot);
    if(ocstat) goto fail;
    ncstat = buildcdftree34(nccomm,ocroot,OCDATA,&dxdroot);
    if(ncstat) goto fail;	
    /* attach DATADDS to DDS */
    ncstat = attach34(dxdroot,seq);
    if(ncstat) goto fail;	
    /* WARNING: we are now switching to datadds tree */
    xseq = seq->attachment;
    ncstat = countsequence(nccomm,xseq,&seqsize);
    if(ncstat) goto fail;
#ifdef DEBUG
fprintf(stderr,"sequencesize: %s = %lu\n",seq->name,(unsigned long)seqsize);
#endif
    /* throw away the fetch'd trees */
    unattach34(nccomm->cdf.ddsroot);
    freecdfroot34(dxdroot);
    if(ncstat != NC_NOERR) {
        /* Cannot get DATADDDS; convert to unlimited */
	char* code;
	char* msg;
	long httperr;
	oc_svcerrordata(nccomm->oc.conn,&code,&msg,&httperr);
	if(code != NULL) {
	    nclog(NCLOGERR,"oc_fetch_datadds failed: %s %s %l",
			code,msg,httperr);
	}
	ocstat = OC_NOERR;
    }		
    if(sizep) *sizep = seqsize;

fail:
    ncbytesfree(minconstraints);
    oc_data_free(conn,rootcontent);
    if(ocstat) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

static NCerror
makeseqdim(NCDAPCOMMON* nccomm, CDFnode* seq, size_t count, CDFnode** sqdimp)
{
    CDFnode* sqdim;
    CDFnode* root = seq->root;
    CDFtree* tree = root->tree;

    /* build the dimension with given size */
    sqdim = makecdfnode34(nccomm,seq->name,OC_Dimension,OCNULL,root);
    if(sqdim == NULL) return THROW(NC_ENOMEM);
    nclistpush(tree->nodes,(ncelem)sqdim);
    nullfree(sqdim->ncbasename);
    nullfree(sqdim->ncfullname);
    sqdim->ncbasename = cdflegalname3(seq->name);
    sqdim->ncfullname = nulldup(sqdim->ncbasename);
    sqdim->dim.declsize = count;
    DIMFLAGSET(sqdim,CDFDIMSEQ);
    sqdim->dim.array = seq;
    if(sqdimp) *sqdimp = sqdim;
    return NC_NOERR;
}

static NCerror
countsequence(NCDAPCOMMON* nccomm, CDFnode* xseq, size_t* sizep)
{
    unsigned int i;
    NClist* path = nclistnew();
    OCdata parent = OCNULL;
    OCdata child = OCNULL;
    OCdata tmp;
    CDFnode* prev = NULL;
    int index;
    OCerror ocstat = OC_NOERR;
    NCerror ncstat = NC_NOERR;
    OCconnection conn = nccomm->oc.conn;
    size_t recordcount;
    CDFnode* xroot;

    ASSERT((xseq->nctype == NC_Sequence));

    parent = oc_data_new(conn);
    child = oc_data_new(conn);

    collectnodepath3(xseq,path,WITHDATASET);

    prev = (CDFnode*)nclistget(path,0);
    ASSERT((prev->nctype == NC_Dataset));

    xroot = xseq->root;
    ocstat = oc_data_root(conn,xroot->tree->ocroot,parent);
    if(ocstat) goto fail;

    for(i=1;i<nclistlength(path);i++) {
	xseq = (CDFnode*)nclistget(path,i);
	index = fieldindex(prev,xseq);
	ocstat = oc_data_ith(conn,parent,index,child);
	if(ocstat) goto fail;
	prev = xseq;
	/* swap the content markers */
	tmp = parent;
	parent = child;
	child = tmp;
    }
    oc_data_count(conn,parent,&recordcount);
    if(sizep) *sizep = recordcount;

fail:
    nclistfree(path);
    if(ocstat) ncstat = ocerrtoncerr(ocstat);
    oc_data_free(conn,parent);
    oc_data_free(conn,child);
    return ncstat;
}

static int
fieldindex(CDFnode* parent, CDFnode* child)
{
    unsigned int i;
    for(i=0;i<nclistlength(parent->subnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(parent->subnodes,i);
	if(node == child) return i;
    }
    return -1;
}

NCerror
showprojection3(NCDAP3* drno, CDFnode* var)
{
    int i,rank;
    NCerror ncstat = NC_NOERR;
    NCbytes* projection = ncbytesnew();
    NClist* path = nclistnew();
    /* Collect the set of DDS node name forming the xpath */
    collectnodepath3(var,path,WITHOUTDATASET);
    for(i=0;i<nclistlength(path);i++) {
        CDFnode* node = (CDFnode*)nclistget(path,i);
	if(i > 0) ncbytescat(projection,".");
	ncbytescat(projection,node->name);
    }
    /* Now, add the dimension info */
    rank = nclistlength(var->array.dimensions);
    for(i=0;i<rank;i++) {
	CDFnode* dim = (CDFnode*)nclistget(var->array.dimensions,i);
	char tmp[32];
	ncbytescat(projection,"[");
	snprintf(tmp,sizeof(tmp),"%lu",(unsigned long)dim->dim.declsize);
	ncbytescat(projection,tmp);
	ncbytescat(projection,"]");
    }    
    /* Define the attribute */
    ncstat = nc_put_att_text(getncid(drno),var->ncid,
                               "_projection",
		               ncbyteslength(projection),
			       ncbytescontents(projection));
    return ncstat;
}

#ifdef IGNORE
NCerror
detachdatadds3(NCDAP3* drno)
{
    int i;
    for(i=0;i<nclistlength(nccomm->cdf.dds->tree.nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(nccomm->cdf.dds->tree.nodes,i);
	node->active = 0;
	node->dim.datasize = node->dim.declsize;
   }
   return NC_NOERR;
}

NCerror
attachdatadds3(NCDAPCOMMON* nccomm)
{
    int i;
    NClist* cdfnodes = nccomm->cdf.dds->tree.nodes;
    for(i=0;i<nclistlength(cdfnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(cdfnodes,i);
	OCobject dds = node->dds;
	if(dds == OCNULL) continue;
	node->active = oc_datadds_active(nccomm->oc.conn,dds);
	if(node->nctype == NC_Dimension) {
	    oc_datadds_dimsize(nccomm->oc.conn,node->dds,&node->dim.datasize);
	}
    }
    return NC_NOERR;
}
#endif

/*
This is more complex than one might think. We want to find
a path to a variable inside the given node so that we can
ask for a single instance of that variable to minimize the
amount of data we retrieve. However, we want to avoid passing
through any nested sequence. This is possible because of the way
that sequencecheck() works.
*/
static NCerror
computeminconstraints3(NCDAPCOMMON* nccomm, CDFnode* seq, NCbytes* minconstraints)
{
    NClist* path = nclistnew();
    CDFnode* var;
    CDFnode* candidate;
    unsigned int i,j,ndims;
    char* prefix;

    /* Locate a variable that is inside this sequence */
    /* Preferably one that is a numeric type*/
    for(candidate=NULL,var=NULL,i=0;i<nclistlength(nccomm->cdf.varnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(nccomm->cdf.varnodes,i);
	if(node->array.sequence == seq) {
	    if(node->nctype == NC_Primitive) {
		switch(node->etype) {
		case NC_BYTE: case NC_SHORT: case NC_INT:
		case NC_FLOAT: case NC_DOUBLE:
		case NC_UBYTE: case NC_USHORT: case NC_UINT:
		case NC_INT64: case NC_UINT64:
		    if(var == NULL) {
			var = node; /* good choice */
		    }
		    break;
		case NC_CHAR: case NC_STRING:
		default:
		    candidate = node; /* usable */
		    break;
		}
	    }
	}
    }
    if(var == NULL && candidate != NULL) var = candidate;
    else if(var == NULL) return THROW(NC_EINVAL);

    /* collect seq path prefix */
    prefix = makecdfpathstring3(seq->container,".");
    ncbytescat(minconstraints,prefix);
    if(strlen(prefix) > 0) ncbytescat(minconstraints,".");

    /* Compute a short path from the var back to and including
       the sequence
    */
    collectnodepath3(var,path,WITHOUTDATASET);
    while(nclistlength(path) > 0) {
	CDFnode* node = (CDFnode*)nclistget(path,0);
	if(node == seq) break;
	nclistremove(path,0);
    }
    ASSERT((nclistlength(path) > 0));

    /* construct the projection path using minimal index values */
    for(i=0;i<nclistlength(path);i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(i > 0) ncbytescat(minconstraints,".");
	ncbytescat(minconstraints,node->name);
	if(node == seq) {
	    /* Use the limit */
	    if(node->sequencelimit > 0) {
		char tmp[64];
		snprintf(tmp,sizeof(tmp),"[0:%lu]",
		         (unsigned long)(node->sequencelimit - 1));
		ncbytescat(minconstraints,tmp);
	    }
	} else if(nclistlength(node->array.dimensions) > 0) {
	    ndims = nclistlength(node->array.dimensions);
	    for(j=0;j<ndims;j++) {
		CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
		if(dim->dim.dimflags & CDFDIMSTRING) {
		    ASSERT((j == (ndims - 1)));
		    break;
		}
		ncbytescat(minconstraints,"[0]");
	    }
	}
    }
    nclistfree(path);
    /* Finally, add in any selection from the original URL */
    if(nccomm->oc.uri->selection != NULL)
        ncbytescat(minconstraints,nccomm->oc.uri->selection);
    nullfree(prefix);
    return NC_NOERR;
}

static unsigned long
cdftotalsize3(NClist* dimensions)
{
    unsigned int i;
    unsigned long total = 1;
    if(dimensions != NULL) {
	for(i=0;i<nclistlength(dimensions);i++) {
	    CDFnode* dim = (CDFnode*)nclistget(dimensions,i);
	    total *= dim->dim.declsize;
	}
    }
    return total;
}

/* Estimate variables sizes and then resort the variable list
   by that size
*/
void
estimatevarsizes3(NCDAPCOMMON* nccomm)
{
    int ivar;
    unsigned int rank;
    size_t totalsize = 0;

    for(ivar=0;ivar<nclistlength(nccomm->cdf.varnodes);ivar++) {
        CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.varnodes,ivar);
	NClist* ncdims = var->array.dimensions;
	rank = nclistlength(ncdims);
	if(rank == 0) { /* use instance size of the type */
	    var->estimatedsize = nctypesizeof(var->etype);
#ifdef DEBUG
fprintf(stderr,"scalar %s.estimatedsize = %lu\n",
	makecdfpathstring3(var,"."),var->estimatedsize);
#endif
	} else {
	    unsigned long size = cdftotalsize3(ncdims);
	    size *= nctypesizeof(var->etype);
#ifdef DEBUG
fprintf(stderr,"array %s(%u).estimatedsize = %lu\n",
	makecdfpathstring3(var,"."),rank,size);
#endif
	    var->estimatedsize = size;
	}
	totalsize += var->estimatedsize;
    }
#ifdef DEBUG
fprintf(stderr,"total estimatedsize = %lu\n",totalsize);
#endif
    nccomm->cdf.totalestimatedsize = totalsize;
}

NCerror
fetchtemplatemetadata3(NCDAPCOMMON* nccomm)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCobject ocroot = OCNULL;
    CDFnode* ddsroot = NULL;
    char* ce = NULL;

    /* Temporary hack: we need to get the selection string
       from the url
    */
    /* Get (almost) unconstrained DDS; In order to handle functions
       correctly, those selections must always be included
    */
    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
	ce = NULL;
    else
        ce = nulldup(nccomm->oc.uri->selection);

    /* Get selection constrained DDS */
    ocstat = dap_oc_fetch(nccomm,nccomm->oc.conn,ce,OCDDS,&ocroot);
    if(ocstat != OC_NOERR) {
	/* Special Hack. If the protocol is file, then see if
           we can get the dds from the .dods file
        */
	if(strcmp(nccomm->oc.uri->protocol,"file") != 0) {
	    THROWCHK(ocstat); goto done;
	}
	/* Fetch the data dds */
        ocstat = dap_oc_fetch(nccomm,nccomm->oc.conn,ce,OCDATADDS,&ocroot);
        if(ocstat != OC_NOERR) {
	    THROWCHK(ocstat); goto done;
	}
	/* Note what we did */
	nclog(NCLOGWARN,"Cannot locate .dds file, using .dods file");
    }

    /* Get selection constrained DAS */
    if(nccomm->oc.ocdasroot != OCNULL)
	oc_root_free(nccomm->oc.conn,nccomm->oc.ocdasroot);
    nccomm->oc.ocdasroot = OCNULL;
    ocstat = dap_oc_fetch(nccomm,nccomm->oc.conn,ce,OCDAS,&nccomm->oc.ocdasroot);
    if(ocstat != OC_NOERR) {
	/* Ignore but complain */
	nclog(NCLOGWARN,"Could not read DAS; ignored");
        nccomm->oc.ocdasroot = OCNULL;	
	ocstat = OC_NOERR;
    }

    /* Construct our parallel dds tree */
    ncstat = buildcdftree34(nccomm,ocroot,OCDDS,&ddsroot);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
#ifdef IGNORE
    if(nccomm->cdf.ddsroot != NULL)
	freecdfroot34(&drno->dap->cdf.ddsroot);
#endif
    nccomm->cdf.ddsroot = ddsroot;

    /* Combine */
    ncstat = dapmerge3(nccomm,ddsroot,nccomm->oc.ocdasroot);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

done:
    nullfree(ce);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

NCerror
fetchconstrainedmetadata3(NCDAPCOMMON* nccomm)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCobject ocroot;
    CDFnode* ddsroot; /* constrained */
    char* ce = NULL;

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
	ce = NULL;
    else
        ce = buildconstraintstring3(nccomm->oc.dapconstraint);

    if(ce == NULL || strlen(ce) == 0) {
	/* no need to get the dds again; just imprint on self */
        ncstat = imprintself3(nccomm->cdf.ddsroot);
        if(ncstat) goto fail;
    } else {
        ocstat = dap_oc_fetch(nccomm,nccomm->oc.conn,ce,OCDDS,&ocroot);
        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}

        /* Construct our parallel dds tree; including attributes*/
        ncstat = buildcdftree34(nccomm,ocroot,OCDDS,&ddsroot);
        if(ncstat) goto fail;

        if(!FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE)) {
            /* fix DAP server problem by adding back any missing grid nodes */
            ncstat = regrid3(ddsroot,nccomm->cdf.ddsroot,nccomm->oc.dapconstraint->projections);    
            if(ncstat) goto fail;
	}

#ifdef DEBUG
fprintf(stderr,"constrained:\n%s",dumptree(ddsroot));
#endif

        /* Imprint the constrained DDS data over the unconstrained DDS */
        ncstat = imprint3(nccomm->cdf.ddsroot,ddsroot);
        if(ncstat) goto fail;

        /* Throw away the constrained DDS */
        freecdfroot34(ddsroot);
    }

fail:
    nullfree(ce);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

/* Suppress variables not in usable sequences */
NCerror
suppressunusablevars3(NCDAPCOMMON* nccomm)
{
    int i,j;
    int found = 1;
    NClist* path = nclistnew();
    while(found) {
	found = 0;
	for(i=0;i<nclistlength(nccomm->cdf.varnodes);i++) {
	    CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.varnodes,i);
	    /* See if this var is under an unusable sequence */
	    nclistclear(path);
	    collectnodepath3(var,path,WITHOUTDATASET);
	    for(j=0;j<nclistlength(path);j++) {
		CDFnode* node = (CDFnode*)nclistget(path,j);
		if(node->nctype == NC_Sequence
		   && !node->usesequence) {
#ifdef DEBUG
fprintf(stderr,"suppressing sequence var: %s\n",node->ncfullname);
#endif
		    nclistremove(nccomm->cdf.varnodes,i);
		    found = 1;
		    break;
		}
	    }
	    if(found) break;
	}
    }
    nclistfree(path);
    return NC_NOERR;
}


/*
For variables which have a zero size dimension,
either use unlimited, or make them invisible.
*/
NCerror
fixzerodims3(NCDAPCOMMON* nccomm)
{
    int i,j;
    for(i=0;i<nclistlength(nccomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.varnodes,i);
        NClist* ncdims = var->array.dimensions;
	if(nclistlength(ncdims) == 0) continue;
        for(j=0;j<nclistlength(ncdims);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(ncdims,j);
	    if(DIMFLAG(dim,CDFDIMUNLIM)) continue;
	    if(dim->dim.declsize == 0) {
		if(j == 0) {/* can make it unlimited */
		    nclistset(ncdims,j,(ncelem)nccomm->cdf.unlimited);
		} else { /* make node invisible */
		    var->visible = 0;
		    var->zerodim = 1;
		}
	    }
	}
    }
    return NC_NOERR;
}

void
applyclientparamcontrols3(NCDAPCOMMON* nccomm)
{
    const char* value;
    OCURI* uri = nccomm->oc.uri;

    /* enable/disable caching */
    value = ocurilookup(uri,"cache");    
    if(value == NULL)
	SETFLAG(nccomm->controls,DFALTCACHEFLAG);
    else if(strlen(value) == 0)
	SETFLAG(nccomm->controls,NCF_CACHE);
    else if(strcmp(value,"1")==0 || value[0] == 'y')
	SETFLAG(nccomm->controls,NCF_CACHE);

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
	SETFLAG(nccomm->controls,NCF_CACHE);

    nclog(NCLOGNOTE,"Caching=%d",FLAGSET(nccomm->controls,NCF_CACHE));

    SETFLAG(nccomm->controls,(NCF_NC3|NCF_NCDAP));

}

/* Accumulate a set of all the known dimensions */
NClist*
getalldims3(NClist* vars, int visibleonly)
{
    int i,j;
    NClist* dimset = nclistnew();

    /* get bag of all dimensions */
    for(i=0;i<nclistlength(vars);i++) {
	CDFnode* var = (CDFnode*)nclistget(vars,i);
	if(!visibleonly || var->visible) {
            NClist* vardims = var->array.dimensions;
   	    for(j=0;j<nclistlength(vardims);j++) {
	        CDFnode* dim = (CDFnode*)nclistget(vardims,j);
	        nclistpush(dimset,(ncelem)dim);
	    }
	}
    }
    /* make unique */
    nclistunique(dimset);
    return dimset;
}
