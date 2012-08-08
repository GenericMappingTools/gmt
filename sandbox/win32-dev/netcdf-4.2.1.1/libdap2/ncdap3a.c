/********************************************************************* \
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "ncdap3.h"
#include "nc3dispatch.h"
#include "ncd3dispatch.h"
#include "dapalign.h"
#include "dapdump.h"
#include "oc.h"

#define getncid(drno) (((NC*)drno)->ext_ncid)

/*Forward*/
static NCerror getseqdimsize(NCDAPCOMMON*, CDFnode* seq, size_t* sizep);
static int fieldindex(CDFnode* parent, CDFnode* child);
static NCerror countsequence(NCDAPCOMMON*, CDFnode* node, size_t*);
static NCerror makeseqdim(NCDAPCOMMON*, CDFnode* node, size_t, CDFnode**);
static NCerror computeseqcountconstraints3(NCDAPCOMMON*,CDFnode*,NCbytes*);
static void computeseqcountconstraints3r(NCDAPCOMMON*, CDFnode*, CDFnode**);

void
freegetvara(Getvara* vara)
{
    if(vara == NULL) return;
    dcefree((DCEnode*)vara->varaprojection);
    nullfree(vara);
}

NCerror
freeNCDAPCOMMON(NCDAPCOMMON* dapcomm)
{
    /* abort the metadata file */
    (void)nc_abort(getncid(dapcomm));
    freenccache(dapcomm,dapcomm->cdf.cache);
    nclistfree(dapcomm->cdf.varnodes);
    nclistfree(dapcomm->cdf.seqnodes);
    nclistfree(dapcomm->cdf.gridnodes);
    nclistfree(dapcomm->cdf.usertypes);
    nclistfree(dapcomm->cdf.projectedvars);
    nullfree(dapcomm->cdf.recorddimname);

    /* free the trees */
    freecdfroot34(dapcomm->cdf.ddsroot);
    dapcomm->cdf.ddsroot = NULL;
    freecdfroot34(dapcomm->cdf.fullddsroot);
    dapcomm->cdf.fullddsroot = NULL;
    if(dapcomm->oc.ocdasroot != NULL)
        oc_root_free(dapcomm->oc.conn,dapcomm->oc.ocdasroot);
    dapcomm->oc.ocdasroot = NULL;
    oc_close(dapcomm->oc.conn); /* also reclaims remaining OC trees */
    nc_urifree(dapcomm->oc.url);
    nullfree(dapcomm->oc.urltext);
    nullfree(dapcomm->oc.rawurltext);

    dcefree((DCEnode*)dapcomm->oc.dapconstraint);
    dapcomm->oc.dapconstraint = NULL;

    free(dapcomm);

    return NC_NOERR;
}

NCerror
addstringdims(NCDAPCOMMON* dapcomm)
{
    /* for all variables of string type, we will need another dimension
       to represent the string; Accumulate the needed sizes and create
       the dimensions with a specific name: either as specified
       in DODS{...} attribute set or defaulting to the variable name.
       All such dimensions are global.
    */
    int i;
    NClist* varnodes = dapcomm->cdf.varnodes;
    CDFnode* globalsdim = NULL;
    char dimname[4096];
    size_t dimsize;

    /* Start by creating the global string dimension */
    snprintf(dimname,sizeof(dimname),"maxStrlen%lu",
	    (unsigned long)dapcomm->cdf.defaultstringlength);
    globalsdim = makecdfnode34(dapcomm, dimname, OC_Dimension, NULL,
                                 dapcomm->cdf.ddsroot);
    nclistpush(dapcomm->cdf.ddsroot->tree->nodes,(ncelem)globalsdim);
    DIMFLAGSET(globalsdim,CDFDIMSTRING);
    globalsdim->dim.declsize = dapcomm->cdf.defaultstringlength;
    globalsdim->dim.declsize0 = globalsdim->dim.declsize;
    globalsdim->dim.array = dapcomm->cdf.ddsroot;
    globalsdim->ncbasename = cdflegalname3(dimname);
    globalsdim->ncfullname = nulldup(globalsdim->ncbasename);
    dapcomm->cdf.globalstringdim = globalsdim;

    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	CDFnode* sdim = NULL;

	/* Does this node need a string dim? */
	if(var->etype != NC_STRING && var->etype != NC_URL) continue;

	dimsize = 0;
	if(var->dodsspecial.maxstrlen > 0)
	    dimsize = var->dodsspecial.maxstrlen;
	else
	    dimsize = var->maxstringlength;

	/* check is a variable-specific string length was specified */
	if(dimsize == 0) 
	    sdim = dapcomm->cdf.globalstringdim; /* use default */
	else {
	    /* create a psuedo dimension for the charification of the string*/
	    if(var->dodsspecial.dimname != NULL)
	        strncpy(dimname,var->dodsspecial.dimname,sizeof(dimname));
	    else
	        snprintf(dimname,sizeof(dimname),"maxStrlen%lu",
			 (unsigned long)dimsize);
	    sdim = makecdfnode34(dapcomm, dimname, OC_Dimension, NULL,
                                 dapcomm->cdf.ddsroot);
	    if(sdim == NULL) return THROW(NC_ENOMEM);
	    nclistpush(dapcomm->cdf.ddsroot->tree->nodes,(ncelem)sdim);
	    DIMFLAGSET(sdim,CDFDIMSTRING);
	    sdim->dim.declsize = dimsize;
	    sdim->dim.declsize0 = dimsize;
	    sdim->dim.array = var;
	    sdim->ncbasename = cdflegalname3(sdim->ocname);
	    sdim->ncfullname = nulldup(sdim->ncbasename);
	}
	/* tag the variable with its string dimension*/
	var->array.stringdim = sdim;
    }
    return NC_NOERR;
}

NCerror
defrecorddim3(NCDAPCOMMON* dapcomm)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;
    NClist* basedims;

    if(dapcomm->cdf.recorddimname == NULL) return NC_NOERR; /* ignore */
    /* Locate the base dimension matching the record dim */
    basedims = dapcomm->cdf.dimnodes;
    for(i=0;i<nclistlength(basedims);i++) {
        CDFnode* dim = (CDFnode*)nclistget(basedims,i);
	if(strcmp(dim->ocname,dapcomm->cdf.recorddimname) != 0) continue;
        DIMFLAGSET(dim,CDFDIMRECORD);
	dapcomm->cdf.recorddim = dim;
	break;
    }

    return ncstat;
}

NCerror
defseqdims(NCDAPCOMMON* dapcomm)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;
    int seqdims = 1; /* default is to compute seq dims counts */

    /* Does the user want to compute actual sequence sizes? */
    if(paramvalue34(dapcomm,"noseqdims")) seqdims = 0;

    /*
	Compute and define pseudo dimensions for sequences
	meeting the following qualifications:
	1. all parents (transitively) of the sequence must
           be either a dataset or a scalar structure.
	2. it must be possible to find a usable sequence constraint.
	All other sequences will be ignored.
    */

    for(i=0;i<nclistlength(dapcomm->cdf.seqnodes);i++) {
        CDFnode* seq = (CDFnode*)nclistget(dapcomm->cdf.seqnodes,i);
	size_t seqsize;
	CDFnode* sqdim = NULL;
	CDFnode* container;
	/* Does this sequence match the requirements for use ? */
	seq->usesequence = 1; /* assume */
	for(container=seq->container;container != NULL;container=container->container) {
	    if(container->nctype == NC_Dataset) break;
	    if(container->nctype != NC_Structure
	       || nclistlength(container->array.dimset0) > 0)
		{seq->usesequence = 0; break;}/* no good */
	}	
	/* Does the user want us to compute the actual sequence dim size? */
	if(seq->usesequence && seqdims) {
	    ncstat = getseqdimsize(dapcomm,seq,&seqsize);
	    if(ncstat != NC_NOERR) {
                /* Cannot read sequence; mark as unusable */
	        seq->usesequence = 0;
	    }
	} else { /* !seqdims default to size = 1 */
	    seqsize = 1; 
	}
	if(seq->usesequence) {
	    /* Note: we are making the dimension in the dds root tree */
            ncstat = makeseqdim(dapcomm,seq,seqsize,&sqdim);
            if(ncstat) goto fail;
            seq->array.seqdim = sqdim;
	} else
            seq->array.seqdim = NULL;
    }

fail:
    return ncstat;
}

static NCerror
getseqdimsize(NCDAPCOMMON* dapcomm, CDFnode* seq, size_t* sizep)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OClink conn = dapcomm->oc.conn;
    OCdatanode rootcontent = NULL;
    OCddsnode ocroot;
    CDFnode* dxdroot;
    CDFnode* xseq;
    NCbytes* seqcountconstraints = ncbytesnew();
    size_t seqsize;

    /* Read the minimal amount of data in order to get the count */
    /* If the url is unconstrainable, then get the whole thing */
    computeseqcountconstraints3(dapcomm,seq,seqcountconstraints);
#ifdef DEBUG
fprintf(stderr,"seqcountconstraints: %s\n",ncbytescontents(seqcountconstraints));
#endif

    /* Fetch the minimal data */
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
        ocstat = dap_fetch(dapcomm,conn,NULL,OCDATADDS,&ocroot);
    else
        ocstat = dap_fetch(dapcomm,conn,ncbytescontents(seqcountconstraints),OCDATADDS,&ocroot);
    if(ocstat) goto fail;

    ncstat = buildcdftree34(dapcomm,ocroot,OCDATA,&dxdroot);
    if(ncstat) goto fail;	
    /* attach DATADDS to DDS */
    ncstat = attach34(dxdroot,seq);
    if(ncstat) goto fail;	

    /* WARNING: we are now switching to datadds tree */
    xseq = seq->attachment;
    ncstat = countsequence(dapcomm,xseq,&seqsize);
    if(ncstat) goto fail;

#ifdef DEBUG
fprintf(stderr,"sequencesize: %s = %lu\n",seq->ocname,(unsigned long)seqsize);
#endif

    /* throw away the fetch'd trees */
    unattach34(dapcomm->cdf.ddsroot);
    freecdfroot34(dxdroot);
    if(ncstat != NC_NOERR) {
        /* Cannot get DATADDDS*/
	char* code;
	char* msg;
	long httperr;
	oc_svcerrordata(dapcomm->oc.conn,&code,&msg,&httperr);
	if(code != NULL) {
	    nclog(NCLOGERR,"oc_fetch_datadds failed: %s %s %l",
			code,msg,httperr);
	}
	ocstat = OC_NOERR;
    }		
    if(sizep) *sizep = seqsize;

fail:
    ncbytesfree(seqcountconstraints);
    oc_data_free(conn,rootcontent);
    if(ocstat) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

static NCerror
makeseqdim(NCDAPCOMMON* dapcomm, CDFnode* seq, size_t count, CDFnode** sqdimp)
{
    CDFnode* sqdim;
    CDFnode* root = seq->root;
    CDFtree* tree = root->tree;

    /* build the dimension with given size; keep the dimension anonymous */
    sqdim = makecdfnode34(dapcomm,seq->ocname,OC_Dimension,NULL,root);
    if(sqdim == NULL) return THROW(NC_ENOMEM);
    nclistpush(tree->nodes,(ncelem)sqdim);
    /* Assign a name to the sequence node */
    sqdim->ncbasename = cdflegalname3(seq->ocname);
    sqdim->ncfullname = nulldup(sqdim->ncbasename);
    DIMFLAGSET(sqdim,CDFDIMSEQ);
    sqdim->dim.declsize = count;
    sqdim->dim.declsize0 = count;
    sqdim->dim.array = seq;
    if(sqdimp) *sqdimp = sqdim;
    return NC_NOERR;
}

static NCerror
countsequence(NCDAPCOMMON* dapcomm, CDFnode* xseq, size_t* sizep)
{
    unsigned int i;
    NClist* path = nclistnew();
    int index;
    OCerror ocstat = OC_NOERR;
    NCerror ncstat = NC_NOERR;
    OClink conn = dapcomm->oc.conn;
    size_t recordcount;
    CDFnode* xroot;
    OCdatanode data = NULL;

    ASSERT((xseq->nctype == NC_Sequence));

    /* collect the path to the sequence node */
    collectnodepath3(xseq,path,WITHDATASET);

    /* Get tree root */
    ASSERT(xseq->root == (CDFnode*)nclistget(path,0));
    xroot = xseq->root;
    ocstat = oc_data_getroot(conn,xroot->tree->ocroot,&data);
    if(ocstat) goto done;

    /* Basically we use the path to walk the data instances to reach
       the sequence instance
    */
    for(i=0;i<nclistlength(path);i++) {
        CDFnode* current = (CDFnode*)nclistget(path,i);
	OCdatanode nextdata = NULL;
	CDFnode* next = NULL;
	
	/* invariant: current = ith node in path; data = corresponding
           datanode
        */

	/* get next node in next and next instance in nextdata */
	if(current->nctype == NC_Structure
	   || current->nctype == NC_Dataset) {
	    if(nclistlength(current->array.dimset0) > 0) {
		/* Cannot handle this case */
		ncstat = THROW(NC_EDDS);
		goto done;
	    }
	    /* get next node in path; structure/dataset => exists */
	    next = (CDFnode*)nclistget(path,i+1);
	    index = fieldindex(current,next);
            /* Move to appropriate field */
	    ocstat = oc_data_ithfield(conn,data,index,&nextdata);
	    if(ocstat) goto done;
	    oc_data_free(conn,data);
	    data = nextdata; /* set up for next loop iteration */
	} else if(current->nctype ==  NC_Sequence) {
	    /* Check for nested Sequences */
	    if(current != xseq) {
		/* Cannot handle this case */
		ncstat = THROW(NC_EDDS);
		goto done;
	    }
	    /* Get the record count */
	    ocstat = oc_data_recordcount(conn,data,&recordcount);
    	    if(sizep) *sizep = recordcount;
	    oc_data_free(conn,data); /* reclaim */
	    break; /* leave the loop */
	} else {
	    PANIC("unexpected mode");
	    return NC_EINVAL;
        }
    }

done:
    nclistfree(path);
    if(ocstat) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
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
showprojection3(NCDAPCOMMON* dapcomm, CDFnode* var)
{
    int i,rank;
    NCerror ncstat = NC_NOERR;
    NCbytes* projection = ncbytesnew();
    NClist* path = nclistnew();
    NC* drno = dapcomm->controller;

    /* Collect the set of DDS node name forming the xpath */
    collectnodepath3(var,path,WITHOUTDATASET);
    for(i=0;i<nclistlength(path);i++) {
        CDFnode* node = (CDFnode*)nclistget(path,i);
	if(i > 0) ncbytescat(projection,".");
	ncbytescat(projection,node->ocname);
    }
    /* Now, add the dimension info */
    rank = nclistlength(var->array.dimset0);
    for(i=0;i<rank;i++) {
	CDFnode* dim = (CDFnode*)nclistget(var->array.dimset0,i);
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

/*
This is more complex than one might think. We want to find
a path to a variable inside the given node so that we can
ask for a single instance of that variable to minimize the
amount of data we retrieve. However, we want to avoid passing
through any nested sequence. This is possible because of the way
that sequencecheck() works.
TODO: some servers will not accept an unconstrained fetch, so
make sure we always have a constraint.
*/

static NCerror
computeseqcountconstraints3(NCDAPCOMMON* dapcomm, CDFnode* seq, NCbytes* seqcountconstraints)
{
    int i,j;
    NClist* path = NULL;
    CDFnode* var = NULL;

    ASSERT(seq->nctype == NC_Sequence);
    computeseqcountconstraints3r(dapcomm,seq,&var);

    ASSERT((var != NULL));

    /* Compute var path */
    path = nclistnew();
    collectnodepath3(var,path,WITHOUTDATASET);

    /* construct the projection path using minimal index values */
    for(i=0;i<nclistlength(path);i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(i > 0) ncbytescat(seqcountconstraints,".");
	ncbytescat(seqcountconstraints,node->ocname);
	if(node == seq) {
	    /* Use the limit */
	    if(node->sequencelimit > 0) {
		char tmp[64];
		snprintf(tmp,sizeof(tmp),"[0:%lu]",
		         (unsigned long)(node->sequencelimit - 1));
		ncbytescat(seqcountconstraints,tmp);
	    }
	} else if(nclistlength(node->array.dimset0) > 0) {
	    int ndims = nclistlength(node->array.dimset0);
	    for(j=0;j<ndims;j++) {
		CDFnode* dim = (CDFnode*)nclistget(node->array.dimset0,j);
		if(DIMFLAG(dim,CDFDIMSTRING)) {
		    ASSERT((j == (ndims - 1)));
		    break;
		}
		ncbytescat(seqcountconstraints,"[0]");
	    }
	}
    }
    /* Finally, add in any selection from the original URL */
    if(dapcomm->oc.url->selection != NULL)
        ncbytescat(seqcountconstraints,dapcomm->oc.url->selection);
    nclistfree(path);
    return NC_NOERR;    
}


/* Given an existing candidate, see if we prefer newchoice */
static CDFnode*
prefer(CDFnode* candidate, CDFnode* newchoice)
{
    nc_type newtyp;
    nc_type cantyp;
    int newisstring;
    int canisstring;
    int newisscalar;
    int canisscalar;

    /* always choose !null over null */
    if(newchoice == NULL)
	return candidate;
    if(candidate == NULL)
	return newchoice;

    newtyp = newchoice->etype;
    cantyp = candidate->etype;
    newisstring = (newtyp == NC_STRING || newtyp == NC_URL);
    canisstring = (cantyp == NC_STRING || cantyp == NC_URL);
    newisscalar = (nclistlength(newchoice->array.dimset0) == 0);
    canisscalar = (nclistlength(candidate->array.dimset0) == 0);

    ASSERT(candidate->nctype == NC_Atomic && newchoice->nctype == NC_Atomic);
    
    /* choose non-string over string */
    if(canisstring && !newisstring)
	return newchoice;
    if(!canisstring && newisstring)
	return candidate;

    /* choose scalar over array */
    if(canisscalar && !newisscalar)
	return candidate;
    if(!canisscalar && newisscalar)
	return candidate;

    /* otherwise choose existing candidate */
    return candidate;
}

/* computeseqcountconstraints3 recursive helper function */
static void
computeseqcountconstraints3r(NCDAPCOMMON* dapcomm, CDFnode* node, CDFnode** candidatep)
{
    CDFnode* candidate;
    CDFnode* compound;
    unsigned int i;

    candidate = NULL;
    compound = NULL;

    for(i=0;i<nclistlength(node->subnodes);i++) {
        CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,i);
        if(subnode->nctype == NC_Structure || subnode->nctype == NC_Grid)
	    compound = subnode; /* save for later recursion */
	else if(subnode->nctype == NC_Atomic)
	    candidate = prefer(candidate,subnode);
    }
    if(candidate == NULL && compound == NULL) {
	PANIC("cannot find candidate for seqcountconstraints for a sequence");
    } else if(candidate != NULL && candidatep != NULL) {
	*candidatep = candidate;
    } else { /* compound != NULL by construction */
	/* recurse on a nested grids or strucures */
        computeseqcountconstraints3r(dapcomm,compound,candidatep);
    }
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
estimatevarsizes3(NCDAPCOMMON* dapcomm)
{
    int ivar;
    unsigned int rank;
    size_t totalsize = 0;

    for(ivar=0;ivar<nclistlength(dapcomm->cdf.varnodes);ivar++) {
        CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,ivar);
	NClist* ncdims = var->array.dimset0;
	rank = nclistlength(ncdims);
	if(rank == 0) { /* use instance size of the type */
	    var->estimatedsize = nctypesizeof(var->etype);
#ifdef DEBUG1
fprintf(stderr,"scalar %s.estimatedsize = %lu\n",
	makecdfpathstring3(var,"."),var->estimatedsize);
#endif
	} else {
	    unsigned long size = cdftotalsize3(ncdims);
	    size *= nctypesizeof(var->etype);
#ifdef DEBUG1
fprintf(stderr,"array %s(%u).estimatedsize = %lu\n",
	makecdfpathstring3(var,"."),rank,size);
#endif
	    var->estimatedsize = size;
	}
	totalsize += var->estimatedsize;
    }
#ifdef DEBUG1
fprintf(stderr,"total estimatedsize = %lu\n",totalsize);
#endif
    dapcomm->cdf.totalestimatedsize = totalsize;
}

NCerror
fetchtemplatemetadata3(NCDAPCOMMON* dapcomm)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCddsnode ocroot = NULL;
    CDFnode* ddsroot = NULL;
    char* ce = NULL;

    /* Temporary hack: we need to get the selection string
       from the url
    */
    /* Get (almost) unconstrained DDS; In order to handle functions
       correctly, those selections must always be included
    */
    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
	ce = NULL;
    else
        ce = nulldup(dapcomm->oc.url->selection);

    /* Get selection constrained DDS */
    ocstat = dap_fetch(dapcomm,dapcomm->oc.conn,ce,OCDDS,&ocroot);
    if(ocstat != OC_NOERR) {
	/* Special Hack. If the protocol is file, then see if
           we can get the dds from the .dods file
        */
	if(strcmp(dapcomm->oc.url->protocol,"file") != 0) {
	    THROWCHK(ocstat); goto done;
	}
	/* Fetch the data dds */
        ocstat = dap_fetch(dapcomm,dapcomm->oc.conn,ce,OCDATADDS,&ocroot);
        if(ocstat != OC_NOERR) {
	    THROWCHK(ocstat); goto done;
	}
	/* Note what we did */
	nclog(NCLOGWARN,"Cannot locate .dds file, using .dods file");
    }

    /* Get selection constrained DAS */
    ocstat = dap_fetch(dapcomm,dapcomm->oc.conn,ce,OCDAS,&dapcomm->oc.ocdasroot);
    if(ocstat != OC_NOERR) {
	/* Ignore but complain */
	nclog(NCLOGWARN,"Could not read DAS; ignored");
        dapcomm->oc.ocdasroot = NULL;	
	ocstat = OC_NOERR;
    }

    /* Construct our parallel dds tree */
    ncstat = buildcdftree34(dapcomm,ocroot,OCDDS,&ddsroot);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    dapcomm->cdf.fullddsroot = ddsroot;

done:
    nullfree(ce);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

NCerror
fetchconstrainedmetadata3(NCDAPCOMMON* dapcomm)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OCddsnode ocroot;
    CDFnode* ddsroot; /* constrained */
    char* ce = NULL;

    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
	ce = NULL;
    else
        ce = buildconstraintstring3(dapcomm->oc.dapconstraint);
    {
        ocstat = dap_fetch(dapcomm,dapcomm->oc.conn,ce,OCDDS,&ocroot);
        if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto fail;}

        /* Construct our parallel dds tree; including attributes*/
        ncstat = buildcdftree34(dapcomm,ocroot,OCDDS,&ddsroot);
        if(ncstat) goto fail;
	ocroot = NULL; /* avoid duplicate reclaim */

	dapcomm->cdf.ddsroot = ddsroot;
	ddsroot = NULL; /* to avoid double reclamation */

        if(!FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE)) {
            /* fix DAP server problem by adding back any missing grid structure nodes */
            ncstat = regrid3(dapcomm->cdf.ddsroot,dapcomm->cdf.fullddsroot,dapcomm->oc.dapconstraint->projections);    
            if(ncstat) goto fail;
	}

#ifdef DEBUG
fprintf(stderr,"constrained:\n%s",dumptree(dapcomm->cdf.ddsroot));
#endif

        /* Combine DDS and DAS */
	if(dapcomm->oc.ocdasroot != NULL) {
            ncstat = dapmerge3(dapcomm,dapcomm->cdf.ddsroot->ocnode,
                               dapcomm->oc.ocdasroot);
            if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
	}

        /* map the constrained DDS to the unconstrained DDS */
        ncstat = mapnodes3(dapcomm->cdf.ddsroot,dapcomm->cdf.fullddsroot);
        if(ncstat) goto fail;

    }

fail:
    nullfree(ce);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return ncstat;
}

/* Suppress variables not in usable sequences*/
NCerror
suppressunusablevars3(NCDAPCOMMON* dapcomm)
{
    int i,j;
    int found = 1;
    NClist* path = nclistnew();

    while(found) {
	found = 0;
	/* Walk backwards to aid removal semantics */
	for(i=nclistlength(dapcomm->cdf.varnodes)-1;i>=0;i--) {
	    CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
	    /* See if this var is under an unusable sequence */
	    nclistclear(path);
	    collectnodepath3(var,path,WITHOUTDATASET);
	    for(j=0;j<nclistlength(path);j++) {
		CDFnode* node = (CDFnode*)nclistget(path,j);
		if(node->nctype == NC_Sequence
		   && !node->usesequence) {
#ifdef DEBUG
fprintf(stderr,"suppressing var in unusable sequence: %s.%s\n",node->ncfullname,var->ncbasename);
#endif
		    found = 1;
		    break;
		}
	    }
	    if(found) break;
	}
        if(found) nclistremove(dapcomm->cdf.varnodes,i);
    }
    nclistfree(path);
    return NC_NOERR;
}


/*
For variables which have a zero size dimension,
make them invisible.
*/
NCerror
fixzerodims3(NCDAPCOMMON* dapcomm)
{
    int i,j;
    for(i=0;i<nclistlength(dapcomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(dapcomm->cdf.varnodes,i);
        NClist* ncdims = var->array.dimsetplus;
	if(nclistlength(ncdims) == 0) continue;
        for(j=0;j<nclistlength(ncdims);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(ncdims,j);
	    if(dim->dim.declsize == 0) {
	 	/* make node invisible */
		var->visible = 0;
		var->zerodim = 1;
	    }
	}
    }
    return NC_NOERR;
}

void
applyclientparamcontrols3(NCDAPCOMMON* dapcomm)
{
    /* clear the flags */
    CLRFLAG(dapcomm->controls,NCF_CACHE);
    CLRFLAG(dapcomm->controls,NCF_PREFETCH);
    CLRFLAG(dapcomm->controls,NCF_SHOWFETCH);
    CLRFLAG(dapcomm->controls,NCF_NC3);
    CLRFLAG(dapcomm->controls,NCF_NCDAP);

    /* Turn on any default on flags */
    SETFLAG(dapcomm->controls,DFALT_ON_FLAGS);    
    SETFLAG(dapcomm->controls,(NCF_NC3|NCF_NCDAP));

    /* enable/disable caching */
    if(paramcheck34(dapcomm,"cache",NULL))
	SETFLAG(dapcomm->controls,NCF_CACHE);
    else if(paramcheck34(dapcomm,"nocache",NULL))
	CLRFLAG(dapcomm->controls,NCF_CACHE);

    /* enable/disable cache prefetch */
    if(paramcheck34(dapcomm,"prefetch",NULL))
	SETFLAG(dapcomm->controls,NCF_PREFETCH);
    else if(paramcheck34(dapcomm,"noprefetch",NULL))
	CLRFLAG(dapcomm->controls,NCF_PREFETCH);


    if(FLAGSET(dapcomm->controls,NCF_UNCONSTRAINABLE))
	SETFLAG(dapcomm->controls,NCF_CACHE);

    if(paramcheck34(dapcomm,"show","fetch"))
	SETFLAG(dapcomm->controls,NCF_SHOWFETCH);

    nclog(NCLOGNOTE,"Caching=%d",FLAGSET(dapcomm->controls,NCF_CACHE));

}
