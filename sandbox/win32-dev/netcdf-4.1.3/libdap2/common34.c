/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/common34.c,v 1.29 2010/05/25 13:53:02 ed Exp $
 *********************************************************************/

#include "ncdap3.h"

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif

extern CDFnode* v4node;

/* Define the set of protocols known to be constrainable */
static char* constrainableprotocols[] = {"http", "https",NULL};
static NCerror buildcdftree34r(NCDAPCOMMON*,OCobject,CDFnode*,CDFtree*,CDFnode**);
static void dupdimensions(OCobject, CDFnode*, NCDAPCOMMON*, CDFtree*);
static NCerror  attachsubset34r(CDFnode*, CDFnode*);
static void free1cdfnode34(CDFnode* node);
static CDFnode* clonedim(NCDAPCOMMON* nccomm, CDFnode* dim, CDFnode* var);
static int getcompletedimset3(CDFnode*, NClist*);

/* Define Procedures that are common to both
   libncdap3 and libncdap4
*/

/* Ensure every node has an initial base name defined and fullname */
/* Exceptions: anonymous dimensions. */
static NCerror
fix1node34(NCDAPCOMMON* nccomm, CDFnode* node)
{
    if(node->nctype == NC_Dimension && node->name == NULL) return NC_NOERR;
    ASSERT((node->name != NULL));
    nullfree(node->ncbasename);
    node->ncbasename = cdflegalname3(node->name);
    if(node->ncbasename == NULL) return NC_ENOMEM;
    nullfree(node->ncfullname);
    node->ncfullname = makecdfpathstring3(node,nccomm->cdf.separator);
    if(node->ncfullname == NULL) return NC_ENOMEM;
    if(node->nctype == NC_Primitive)
        node->externaltype = nctypeconvert(nccomm,node->etype);
    if(node->nctype == NC_Dimension)
        node->maxstringlength = nccomm->cdf.defaultstringlength;
    return NC_NOERR;
}

NCerror
fixnodes34(NCDAPCOMMON* nccomm, NClist* cdfnodes)
{
    int i;
    for(i=0;i<nclistlength(cdfnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(cdfnodes,i);
	NCerror err = fix1node34(nccomm,node);
	if(err) return err;
    }
    return NC_NOERR;
}

#ifdef IGNORE
NCerror
computecdfinfo34(NCDAPCOMMON* nccomm, NClist* cdfnodes)
{
    int i;
    /* Ensure every node has an initial base name defined and fullname */
    /* Exceptions: anonymous dimensions. */
    for(i=0;i<nclistlength(cdfnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(cdfnodes,i);
        if(node->nctype == NC_Dimension && node->name == NULL) continue;
	ASSERT((node->name != NULL));
        nullfree(node->ncbasename);
        node->ncbasename = cdflegalname3(node->name);
        nullfree(node->ncfullname);
	node->ncfullname = makecdfpathstring3(node,nccomm->cdf.separator);
if(node==v4node && node->ncfullname[0] != 'Q')dappanic("");
    }
    for(i=0;i<nclistlength(cdfnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(cdfnodes,i);
        if(node->nctype == NC_Primitive)
            node->externaltype = nctypeconvert(node->etype);
    }
    for(i=0;i<nclistlength(cdfnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(cdfnodes,i);
	if(node->nctype != NC_Dimension) continue;
	node->maxstringlength = nccomm->cdf.defaultstringlength;
    }
    return NC_NOERR;
}
#endif

NCerror
fixgrid34(NCDAPCOMMON* nccomm, CDFnode* grid)
{
    unsigned int i,glen;
    CDFnode* array;

    glen = nclistlength(grid->subnodes);
    array = (CDFnode*)nclistget(grid->subnodes,0);	        
    if(nccomm->controls.flags & (NCF_NC3)) {
        /* Rename grid Array: variable, but leave its oc base name alone */
        nullfree(array->ncbasename);
        array->ncbasename = nulldup(grid->ncbasename);
        if(!array->ncbasename) return NC_ENOMEM;
    }
    /* validate and modify the grid structure */
    if((glen-1) != nclistlength(array->array.dimensions)) goto invalid;
    for(i=1;i<glen;i++) {
	CDFnode* arraydim = (CDFnode*)nclistget(array->array.dimensions,i-1);
	CDFnode* map = (CDFnode*)nclistget(grid->subnodes,i);
	CDFnode* mapdim;
	/* map must have 1 dimension */
	if(nclistlength(map->array.dimensions) != 1) goto invalid;
	/* and the map name must match the ith array dimension */
	if(!DIMFLAG(arraydim,CDFDIMANON)
	   && strcmp(arraydim->name,map->name)!= 0)
	    goto invalid;
	/* and the map name must match its dim name (if any) */
	mapdim = (CDFnode*)nclistget(map->array.dimensions,0);
	if(!DIMFLAG(mapdim,CDFDIMANON) && strcmp(mapdim->name,map->name)!= 0)
	    goto invalid;
	/* Add appropriate names for the anonymous dimensions */
	/* Do the map name first, so the array dim may inherit */
	if(DIMFLAG(mapdim,CDFDIMANON)) {
	    nullfree(mapdim->name);
	    nullfree(mapdim->ncbasename);
	    mapdim->name = nulldup(map->name);
	    if(!mapdim->name) return NC_ENOMEM;
	    mapdim->ncbasename = cdflegalname3(mapdim->name);
	    if(!mapdim->ncbasename) return NC_ENOMEM;
	    DIMFLAGCLR(mapdim,CDFDIMANON);
	}
	if(DIMFLAG(arraydim,CDFDIMANON)) {
	    nullfree(arraydim->name); /* just in case */
	    nullfree(arraydim->ncbasename);
	    arraydim->name = nulldup(map->name);
	    if(!arraydim->name) return NC_ENOMEM;
	    arraydim->ncbasename = cdflegalname3(arraydim->name);
	    if(!arraydim->ncbasename) return NC_ENOMEM;
	    DIMFLAGCLR(arraydim,CDFDIMANON);
	}
        if(FLAGSET(nccomm->controls,(NCF_NCDAP|NCF_NC3))) {
	    char tmp[3*NC_MAX_NAME];
            /* Add the grid name to the basename of the map */
	    snprintf(tmp,sizeof(tmp),"%s%s%s",map->container->ncbasename,
					  nccomm->cdf.separator,
					  map->ncbasename);
	    nullfree(map->ncbasename);
            map->ncbasename = nulldup(tmp);
	    if(!map->ncbasename) return NC_ENOMEM;
	}
    }
    return NC_NOERR;
invalid:
    return NC_EINVAL; /* mal-formed grid */
}

/* Given an dimension, compute its effective 0-based
   index in the complete set of dimension of its
   containing variable. The result should mimic
   the libnc-dap indices.
*/
static int
computedimindex3(CDFnode* var, CDFnode* dim)
{
    int i,index;
    NClist* vardims = var->array.dimensions;
    for(index=-1,i=0;i<nclistlength(vardims);i++) {
        if(dim == (CDFnode*)nclistget(vardims,i)) {index=i; break;}
    }
    ASSERT((index >=0));
    return index;
}

static CDFnode*
clonedim(NCDAPCOMMON* nccomm, CDFnode* dim, CDFnode* var)
{
    CDFnode* clone;
    clone = makecdfnode34(nccomm,dim->name,OC_Dimension,
			  OCNULL,dim->container);
    /* Record its existence */
    nclistpush(dim->container->root->tree->nodes,(ncelem)clone);
    clone->dim = dim->dim; /* copy most everything */
    clone->dim.dimflags |= CDFDIMCLONE;
    clone->dim.array = var;
    return clone;
}

/* Give each dimensioned object a unique set of inherited dimensions */
NCerror
clonecdfdims34(NCDAPCOMMON* nccomm)
{
    int i,j;
    NClist* vars = nccomm->cdf.varnodes;

    for(i=0;i<nclistlength(vars);i++) {
	CDFnode* node = (CDFnode*)nclistget(vars,i);
	if(node->array.dimensions != NULL) {
            NClist* clonedims = nclistnew();
            NClist* dims = nclistnew();
	    int ninherit = getcompletedimset3(node,dims);
	    int rank = nclistlength(dims);
	    int hasstringdim = (node->array.stringdim != NULL?1:0);
	    rank -= hasstringdim;
            for(j=0;j<rank;j++) {
	        CDFnode* dim = (CDFnode*)nclistget(dims,j);
	        CDFnode* clone = dim;
	        if(j<ninherit) clone = clonedim(nccomm,dim,node);
	        nclistpush(clonedims,(ncelem)clone);
	    }
	    nclistfree(dims);
	    if(node->array.stringdim != NULL) {
	        nclistpush(clonedims,(ncelem)clonedim(nccomm,node->array.stringdim,node));
	    }
	    node->array.dimensions = clonedims;
	}
    }     
    return NC_NOERR;
}

static int
getcompletedimset3(CDFnode* var, NClist* dimset)
{
    int i,j;
    NClist* path = nclistnew();
    int inherited = 0; /* not including stringdim */
    CDFnode* node;

    nclistclear(dimset);
    /* Get the inherited dimensions first*/
    collectnodepath3(var,path,WITHOUTDATASET);
    for(i=0;i<nclistlength(path)-1;i++) {
	node = (CDFnode*)nclistget(path,i);
	if(node->nctype == NC_Sequence) {
	    CDFnode* sqdim = (CDFnode*)nclistget(node->array.dimensions,0);
	    if(DIMFLAG(sqdim,CDFDIMUNLIM)) {
		nclistclear(dimset); /* unlimited is always first */
		inherited = 0;
	    }	
        }
	for(j=0;j<nclistlength(node->array.dimensions);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
	    nclistpush(dimset,(ncelem)dim);
	}
    }
    inherited = nclistlength(dimset); /* mark the # of inherited dimensions */
    /* Now add the base dimensions */
    node = (CDFnode*)nclistpop(path);    
    for(j=0;j<nclistlength(node->array.dimensions);j++) {
	CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
	nclistpush(dimset,(ncelem)dim);
    }
    if(node->array.stringdim != NULL) 
	    nclistpush(dimset,(ncelem)node->array.stringdim);
    nclistfree(path);
    return inherited;
}

/* Provide short and/or unified names for dimensions. */
NCerror
computecdfdimnames34(NCDAPCOMMON* nccomm)
{
    int i,j;
    char tmp[NC_MAX_NAME*2];
    NClist* conflicts = nclistnew();
    NClist* vars = nccomm->cdf.varnodes;
    NClist* alldims = nclistnew();

    /* Start by assigning ncbasenames and ncfullnames to dimensions */
    /* Do on a per-var basis */
    for(i=0;i<nclistlength(vars);i++) {
        NClist* dims;
	CDFnode* var = (CDFnode*)nclistget(vars,i);
	if(nclistlength(var->array.dimensions) == 0) continue;
	dims = var->array.dimensions;
        for(j=0;j<nclistlength(dims);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(dims,j);
	    nclistpush(alldims,(ncelem)dim); /* collect all the dimensions */
	    if(DIMFLAG(dim,CDFDIMANON)) {
	        int index = computedimindex3(var,dim);
                snprintf(tmp,sizeof(tmp),"%s_%d",
                            var->ncbasename,index);
                nullfree(dim->ncbasename);
                dim->ncbasename = cdflegalname3(tmp);
                snprintf(tmp,sizeof(tmp),"%s_%d",
                            var->ncfullname,index);
                nullfree(dim->ncfullname);
                dim->ncfullname = cdflegalname3(tmp);
    	    } else { /* !anonymous */
	        nullfree(dim->ncbasename);
	        dim->ncbasename = cdflegalname3(dim->name);
    	        nullfree(dim->ncfullname);
	        dim->ncfullname = nulldup(dim->ncbasename);
	    }
	}
    }
    nclistunique(alldims); /* remove duplicates */

    /* Handle the easy case where two dims have same name and sizes.
       Make the second and later ones point to the leader dimension.
       Exception: if this is the record dim, then make that one the leader.
    */
    for(i=0;i<nclistlength(alldims);i++) {
        int match = 0;
	CDFnode* dupdim = NULL;
	CDFnode* basedim = (CDFnode*)nclistget(alldims,i);
	if(basedim == nccomm->cdf.unlimited && DIMFLAG(basedim,CDFDIMRECORD))
	    continue;
	if(basedim->dim.basedim != NULL) continue; /* already processed*/
	for(j=i+1;j<nclistlength(alldims);j++) {
	    dupdim = (CDFnode*)nclistget(alldims,j);
	    if(dupdim->dim.basedim != NULL) continue; /* already processed */
	    match = (strcmp(dupdim->ncfullname,basedim->ncfullname) == 0
	             && dupdim->dim.declsize == basedim->dim.declsize);
            if(match) {
	        dupdim->dim.basedim = basedim; /* same name and size*/
	    }
	}
    }

    /* Process record dim */
    if(nccomm->cdf.unlimited != NULL && DIMFLAG(nccomm->cdf.unlimited,CDFDIMRECORD)) {
	CDFnode* recdim = nccomm->cdf.unlimited;
	for(i=0;i<nclistlength(alldims);i++) {
	    int match;
	    CDFnode* dupdim = (CDFnode*)nclistget(alldims,i);
	    if(dupdim->dim.basedim != NULL) continue; /* already processed */
	    match = (strcmp(dupdim->ncfullname,recdim->ncfullname) == 0
	             && dupdim->dim.declsize == recdim->dim.declsize);
            if(match) {
	        dupdim->dim.basedim = recdim;
	    }
	}
    }

    /* Remaining case: same name and different sizes*/
    /* => rename second dim by appending a counter */

    for(i=0;i<nclistlength(alldims);i++) {
	CDFnode* basedim = (CDFnode*)nclistget(alldims,i);
	if(basedim->dim.basedim != NULL) continue; /* ignore*/
	/* Collect all conflicting dimensions */
	nclistclear(conflicts);
        for(j=i+1;j<nclistlength(alldims);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(alldims,j);
	    if(dim->dim.basedim != NULL) continue; /* ignore*/	    
	    if(strcmp(dim->ncfullname,basedim->ncfullname)!=0) continue;
	    if(dim->dim.declsize == basedim->dim.declsize) continue;
	    nclistpush(conflicts,(ncelem)dim);
	}
	/* Now, rename all the conflicting dimensions */
	for(j=0;j<nclistlength(conflicts);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(conflicts,j);
	    snprintf(tmp,sizeof(tmp),"%s%d",dim->ncfullname,j+1);
	    nullfree(dim->ncfullname);
	    dim->ncfullname = nulldup(tmp);
	}
    }

    /* Finally, verify unique names for dimensions*/
    for(i=0;i<nclistlength(alldims);i++) {
	CDFnode* dim1 = (CDFnode*)nclistget(alldims,i);
	if(dim1->dim.basedim != NULL) continue;
	for(j=0;j<i;j++) {
	    CDFnode* dim2 = (CDFnode*)nclistget(alldims,j);
	    if(dim2->dim.basedim != NULL) continue;
	    if(strcmp(dim1->ncfullname,dim2->ncfullname)==0) {
		PANIC1("duplicate dim names: %s",dim1->ncfullname);
	    }
	}
    }
    /* clean up*/
    nclistfree(conflicts);
    nclistfree(alldims);
    return NC_NOERR;
}

NCerror
makegetvar34(NCDAPCOMMON* nccomm, CDFnode* var, void* data, nc_type dsttype, Getvara** getvarp)
{
    Getvara* getvar;
    NCerror ncstat = NC_NOERR;

    getvar = (Getvara*)calloc(1,sizeof(Getvara));
    MEMCHECK(getvar,NC_ENOMEM);
    if(getvarp) *getvarp = getvar;

    getvar->target = var;
    getvar->memory = data;
    getvar->dsttype = dsttype;
    getvar->target = var;
    if(ncstat) nullfree(getvar);
    return ncstat;
}

int
constrainable34(OCURI* durl)
{
   char** protocol = constrainableprotocols;
   for(;*protocol;protocol++) {
	if(strcmp(durl->protocol,*protocol)==0)
	    return 1;
   }
   return 0;
}

CDFnode*
makecdfnode34(NCDAPCOMMON* nccomm, char* name, OCtype octype,
             /*optional*/ OCobject ocnode, CDFnode* container)
{
    CDFnode* node;
    assert(nccomm != NULL);
    node = (CDFnode*)calloc(1,sizeof(CDFnode));
    if(node == NULL) return (CDFnode*)NULL;

    node->name = NULL;
    if(name) {
        size_t len = strlen(name);
        if(len >= NC_MAX_NAME) len = NC_MAX_NAME-1;
        node->name = (char*)malloc(len+1);
	if(node->name == NULL) return NULL;
	memcpy(node->name,name,len);
	node->name[len] = '\0';
    }
    node->nctype = octypetonc(octype);
    node->dds = ocnode;
    node->subnodes = nclistnew();
    /* Initially, these two are the same; dimension
       inheritance will split
    */
    node->array.dimensions0 = nclistnew();
    node->array.dimensions = node->array.dimensions0;
    node->container = container;
    if(ocnode != OCNULL) {
	oc_inq_primtype(nccomm->oc.conn,ocnode,&octype);
        node->etype = octypetonc(octype);
    }
    return node;
}

/* Given an OCnode tree, mimic it as a CDFnode tree;
   Add DAS attributes if DAS is available
*/
NCerror
buildcdftree34(NCDAPCOMMON* nccomm, OCobject ocroot, OCdxd occlass, CDFnode** cdfrootp)
{
    CDFnode* root = NULL;
    CDFtree* tree = (CDFtree*)calloc(1,sizeof(CDFtree));
    NCerror err = NC_NOERR;
    tree->ocroot = ocroot;
    tree->nodes = nclistnew();
    tree->occlass = occlass;
    tree->owner = nccomm;

    err = buildcdftree34r(nccomm,ocroot,NULL,tree,&root);
    if(!err) {
	if(occlass != OCDAS)
	    fixnodes34(nccomm,tree->nodes);
	if(cdfrootp) *cdfrootp = root;
    }
    return err;
}        

static NCerror
buildcdftree34r(NCDAPCOMMON* nccomm, OCobject ocnode, CDFnode* container,
                CDFtree* tree, CDFnode** cdfnodep)
{
    unsigned int i,ocrank,ocnsubnodes;
    OCtype octype;
    char* ocname = NULL;
    NCerror ncerr = NC_NOERR;
    CDFnode* cdfnode;

    oc_inq_class(nccomm->oc.conn,ocnode,&octype);
    oc_inq_name(nccomm->oc.conn,ocnode,&ocname);
    oc_inq_rank(nccomm->oc.conn,ocnode,&ocrank);
    oc_inq_nsubnodes(nccomm->oc.conn,ocnode,&ocnsubnodes);

    switch (octype) {
    case OC_Dataset:
    case OC_Grid:
    case OC_Structure:
    case OC_Sequence:
    case OC_Primitive:
	cdfnode = makecdfnode34(nccomm,ocname,octype,ocnode,container);
	nclistpush(tree->nodes,(ncelem)cdfnode);
	if(tree->root == NULL) {
	    tree->root = cdfnode;
	    cdfnode->tree = tree;
	}		
	break;

    case OC_Dimension:
    default: PANIC1("buildcdftree: unexpect OC node type: %d",(int)octype);

    }    
    /* cross link */
    cdfnode->root = tree->root;

    if(ocrank > 0) dupdimensions(ocnode,cdfnode,nccomm,tree);
    for(i=0;i<ocnsubnodes;i++) {
	OCobject ocsubnode;
	CDFnode* subnode;
	oc_inq_ith(nccomm->oc.conn,ocnode,i,&ocsubnode);
	ncerr = buildcdftree34r(nccomm,ocsubnode,cdfnode,tree,&subnode);
	if(ncerr) return ncerr;
	nclistpush(cdfnode->subnodes,(ncelem)subnode);
    }
    nullfree(ocname);
    if(cdfnodep) *cdfnodep = cdfnode;
    return ncerr;
}

static void
dupdimensions(OCobject ocnode, CDFnode* cdfnode, NCDAPCOMMON* nccomm, CDFtree* tree)
{
    unsigned int i,ocrank;
 
    oc_inq_rank(nccomm->oc.conn,ocnode,&ocrank);
    assert(ocrank > 0);
    for(i=0;i<ocrank;i++) {
	CDFnode* cdfdim;
	OCobject ocdim;
	char* ocname;
	size_t declsize;

	oc_inq_ithdim(nccomm->oc.conn,ocnode,i,&ocdim);
	oc_inq_dim(nccomm->oc.conn,ocdim,&declsize,&ocname);

	cdfdim = makecdfnode34(nccomm,ocname,OC_Dimension,
                              ocdim,cdfnode->container);
	if(ocname == NULL) DIMFLAGSET(cdfdim,CDFDIMANON);
	nullfree(ocname);
	nclistpush(tree->nodes,(ncelem)cdfdim);
	/* Initially, constrained and unconstrained are same */
	cdfdim->dim.declsize = declsize;
	cdfdim->dim.declsize0 = declsize;
	cdfdim->dim.array = cdfnode;
	nclistpush(cdfnode->array.dimensions,(ncelem)cdfdim);
    }    
}

/* Note: this routine only applies some common
   client parameters, other routines may apply
   specific ones.
*/

NCerror
applyclientparams34(NCDAPCOMMON* nccomm)
{
    int i,len;
    int dfaltstrlen = DEFAULTSTRINGLENGTH;
    int dfaltseqlim = DEFAULTSEQLIMIT;
    const char* value;
    char tmpname[NC_MAX_NAME+32];
    char* pathstr;
    OCconnection conn = nccomm->oc.conn;
    unsigned long limit;

    nccomm->cdf.cache->cachelimit = DFALTCACHELIMIT;
    value = oc_clientparam_get(conn,"cachelimit");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.cache->cachelimit = limit;

    nccomm->cdf.fetchlimit = DFALTFETCHLIMIT;
    value = oc_clientparam_get(conn,"fetchlimit");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.fetchlimit = limit;

    nccomm->cdf.smallsizelimit = DFALTSMALLLIMIT;
    value = oc_clientparam_get(conn,"smallsizelimit");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.smallsizelimit = limit;

    nccomm->cdf.cache->cachecount = DFALTCACHECOUNT;
#ifdef HAVE_GETRLIMIT
    { struct rlimit rl;
      if(getrlimit(RLIMIT_NOFILE, &rl) >= 0) {
	nccomm->cdf.cache->cachecount = (size_t)(rl.rlim_cur / 2);
      }
    }
#endif
    value = oc_clientparam_get(conn,"cachecount");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.cache->cachecount = limit;
    /* Ignore limit if not caching */
    if(!FLAGSET(nccomm->controls,NCF_CACHE))
        nccomm->cdf.cache->cachecount = 0;

    if(oc_clientparam_get(conn,"nolimit") != NULL)
	dfaltseqlim = 0;
    value = oc_clientparam_get(conn,"limit");
    if(value != NULL && strlen(value) != 0) {
        if(sscanf(value,"%d",&len) && len > 0) dfaltseqlim = len;
    }
    nccomm->cdf.defaultsequencelimit = dfaltseqlim;

    /* allow embedded _ */
    value = oc_clientparam_get(conn,"stringlength");
    if(value != NULL && strlen(value) != 0) {
        if(sscanf(value,"%d",&len) && len > 0) dfaltstrlen = len;
    }
    nccomm->cdf.defaultstringlength = dfaltstrlen;

    /* String dimension limits apply to variables */
    for(i=0;i<nclistlength(nccomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.varnodes,i);
	/* Define the client param stringlength for this variable*/
	var->maxstringlength = dfaltstrlen; /* unless otherwise stated*/
	strcpy(tmpname,"stringlength_");
	pathstr = makeocpathstring3(conn,var->dds,".");
	strcat(tmpname,pathstr);
	nullfree(pathstr);
	value = oc_clientparam_get(conn,tmpname);	
        if(value != NULL && strlen(value) != 0) {
            if(sscanf(value,"%d",&len) && len > 0) var->maxstringlength = len;
	}
    }
    /* Sequence limits apply to sequences */
    for(i=0;i<nclistlength(nccomm->cdf.ddsroot->tree->nodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.ddsroot->tree->nodes,i);
	if(var->nctype != NC_Sequence) continue;
	var->sequencelimit = dfaltseqlim;
	strcpy(tmpname,"nolimit_");
	pathstr = makeocpathstring3(conn,var->dds,".");
	strcat(tmpname,pathstr);
	if(oc_clientparam_get(conn,tmpname) != NULL)
	    var->sequencelimit = 0;
	strcpy(tmpname,"limit_");
	strcat(tmpname,pathstr);
	value = oc_clientparam_get(conn,tmpname);
        if(value != NULL && strlen(value) != 0) {
            if(sscanf(value,"%d",&len) && len > 0)
		var->sequencelimit = len;
	}
	nullfree(pathstr);
    }
    return NC_NOERR;
}

void
freecdfroot34(CDFnode* root)
{
    int i;
    CDFtree* tree;
    NCDAPCOMMON* nccomm;
    if(root == NULL) return;
    tree = root->tree;
    ASSERT((tree != NULL));
    /* Explicitly FREE the ocroot */
    nccomm = tree->owner;
    oc_root_free(nccomm->oc.conn,tree->ocroot);
    tree->ocroot = OCNULL;
    for(i=0;i<nclistlength(tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(tree->nodes,i);
	free1cdfnode34(node);
    }
    nclistfree(tree->nodes);
    nullfree(tree);
}

/* Free up a single node, but not any
   nodes it points to.
*/  
static void
free1cdfnode34(CDFnode* node)
{
    unsigned int j,k;
    if(node == NULL) return;
    nullfree(node->name);
    nullfree(node->ncbasename);
    nullfree(node->ncfullname);
    if(node->attributes != NULL) {
	for(j=0;j<nclistlength(node->attributes);j++) {
	    NCattribute* att = (NCattribute*)nclistget(node->attributes,j);
	    nullfree(att->name);
	    for(k=0;k<nclistlength(att->values);k++)
		nullfree((char*)nclistget(att->values,k));
	    nclistfree(att->values);
	    nullfree(att);
	}
    }
    nullfree(node->dodsspecial.dimname);
    nclistfree(node->subnodes);
    nclistfree(node->attributes);
    /* Check to see if we need to free both dimensions and dimensions0 */
    if(node->array.dimensions != node->array.dimensions0)
        nclistfree(node->array.dimensions0);
    nclistfree(node->array.dimensions);

    /* Clean up the ncdap4 fields also */
    nullfree(node->typename);
    nullfree(node->vlenname);
    nullfree(node);
}

/* Return true if node and node1 appear to refer to the same thing;
   takes grid->structure changes into account.
   Two versions exist:
   1. version to use on DDS after pseudodimensioning has occurred
      (nodematch)
   2. version to use on all other cases (simplenodematch)
   [this is more complicated that I desire; need to fix soon]
*/
int
nodematch34(CDFnode* node1, CDFnode* node2)
{
    if(node1 == NULL) return (node2==NULL);
    if(node2 == NULL) return 0;
    if(node1->nctype != node2->nctype) {
	/* Check for Grid->Structure match */
	if((node1->nctype == NC_Structure && node2->nctype == NC_Grid)
	   || (node2->nctype == NC_Structure && node1->nctype == NC_Grid)){
	   if(node1->name == NULL || node2->name == NULL
	      || strcmp(node1->name,node2->name) !=0) return 0;	    	
	} else return 0;
    }
    /* Add hack to address the screwed up Columbia server */
    if(node1->nctype == NC_Dataset) return 1;
    if(node1->nctype == NC_Primitive
       && node1->etype != node2->etype) return 0;
    if(node1->name != NULL && node2->name != NULL
       && strcmp(node1->name,node2->name)!=0) return 0;
    if(nclistlength(node1->array.dimensions)
       != nclistlength(node2->array.dimensions)) {/*look closer*/
	ASSERT((node1->array.dimensions0 != NULL));
	ASSERT((node2->array.dimensions0 != NULL));
        if(node1->nctype != NC_Sequence) {
	    /* Locate original dimensions */
	    unsigned int rank1 = nclistlength(node1->array.dimensions0);
	    unsigned int rank2 = nclistlength(node2->array.dimensions0);
   	    if(rank1 != rank2) return 0;
	}
    }
    return 1;
}

int
simplenodematch34(CDFnode* node1, CDFnode* node2)
{
    if(node1 == NULL) return (node2==NULL);
    if(node2 == NULL) return 0;
    if(node1->nctype != node2->nctype) {
	/* Check for Grid->Structure match */
	if((node1->nctype == NC_Structure && node2->nctype == NC_Grid)
	   || (node2->nctype == NC_Structure && node1->nctype == NC_Grid)){
	   if(node1->name == NULL || node2->name == NULL
	      || strcmp(node1->name,node2->name) !=0) return 0;	    	
	} else return 0;
    }
    /* Add hack to address the screwed up Columbia server */
    if(node1->nctype == NC_Dataset) return 1;
    if(node1->nctype == NC_Primitive
       && node1->etype != node2->etype) return 0;
    if(node1->name != NULL && node2->name != NULL
       && strcmp(node1->name,node2->name)!=0) return 0;
    if(nclistlength(node1->array.dimensions0)
       != nclistlength(node2->array.dimensions0)) return 0;
    return 1;
}

/*
Given DDS node, locate the node
in a DATADDS that matches the DDS node.
Return NULL if no node found
*/

#ifdef IGNORE
static CDFnode*
findxnode34r(NClist* path, int depth, CDFnode* xnode)
{
    unsigned int i;
    CDFnode* pathnode;
    unsigned int len = nclistlength(path);
    int lastnode = (depth == (len - 1));

    if(depth >= len) return NULL;

    pathnode = (CDFnode*)nclistget(path,depth);

    /* If this path element matches the current xnode, then recurse */
    if(nodematch34(pathnode,xnode)) {
        if(lastnode) return xnode;
        for(i=0;i<nclistlength(xnode->subnodes);i++) {
	    CDFnode* xsubnode = (CDFnode*)nclistget(xnode->subnodes,i);
	    CDFnode* matchnode;
	    matchnode = findxnode34r(path,depth+1,xsubnode);	    
	    if(matchnode != NULL) return matchnode;
	}
    } else
    /* Ok, we have a node mismatch; normally return NULL,
       but must handle the special case of an elided Grid.
    */
    if(pathnode->nctype == NC_Grid && xnode->nctype == NC_Primitive) {
	/* Try to match the xnode to one of the subparts of the grid */
	CDFnode* matchnode;
	matchnode = findxnode34r(path,depth+1,xnode);	    
	if(matchnode != NULL) return matchnode;
    }
    /* Could not find node, return NULL */
    return NULL;
}


CDFnode*
findxnode34(CDFnode* target, CDFnode* xroot)
{
    CDFnode* xtarget = NULL;
    NClist* path = nclistnew();
    collectnodepath3(target,path,WITHDATASET);
    xtarget = findxnode34r(path,0,xroot);
    nclistfree(path);
    return xtarget;
}
#endif

void
unattach34(CDFnode* root)
{
    unsigned int i;
    CDFtree* xtree = root->tree;
    for(i=0;i<nclistlength(xtree->nodes);i++) {
	CDFnode* xnode = (CDFnode*)nclistget(xtree->nodes,i);
	/* break bi-directional link */
        xnode->attachment = NULL;
    }
}

static void
setattach(CDFnode* target, CDFnode* srcnode)
{
    target->attachment = srcnode;
    srcnode->attachment = target;
    /* Transfer important information */
    target->externaltype = srcnode->externaltype;
    target->maxstringlength = srcnode->maxstringlength;
    target->sequencelimit = srcnode->sequencelimit;
    target->ncid = srcnode->ncid;
    /* also transfer libncdap4 info */
    target->typeid = srcnode->typeid;
    target->typesize = srcnode->typesize;
}

static NCerror
attachdims34(CDFnode* xnode, CDFnode* ddsnode)
{
    unsigned int i;
    for(i=0;i<nclistlength(xnode->array.dimensions);i++) {
	CDFnode* xdim = (CDFnode*)nclistget(xnode->array.dimensions,i);
	CDFnode* ddim = (CDFnode*)nclistget(ddsnode->array.dimensions,i);
	setattach(xdim,ddim);
    }
    return NC_NOERR;
}

#ifdef IGNORE
/* Attach all dstnodes to all srcnodes; all dstnodes must match */
static NCerror
attachall34r(CDFnode* dstnode, CDFnode* srcnode)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;

    ASSERT((nodematch34(dstnode,srcnode)));
    setattach(dstnode,srcnode);    

    if(dstnode->array.rank > 0) {
	attachdims34(dstnode,srcnode);
    }

    /* Try to match dstnode subnodes against srcnode subnodes */
    if(nclistlength(dstnode->subnodes) != nclistlength(srcnode->subnodes))
	{THROWCHK(ncstat=NC_EINVAL); goto done;}

    for(i=0;i<nclistlength(dstnode->subnodes);i++) {
        CDFnode* dstsubnode = (CDFnode*)nclistget(dstnode->subnodes,i);
        CDFnode* srcsubnode = (CDFnode*)nclistget(srcnode->subnodes,i);
        if(!nodematch34(dstsubnode,srcsubnode))
	    {THROWCHK(ncstat=NC_EINVAL); goto done;}
        ncstat = attachall34r(dstsubnode,srcsubnode);
	if(ncstat) goto done;
    }
done:
    return THROW(ncstat);
}

/* 
Match nodes in one tree to nodes in another.
Usually used to attach the DATADDS to the DDS,
but not always.
*/
NCerror
attachall34(CDFnode* dstroot, CDFnode* srcroot)
{
    NCerror ncstat = NC_NOERR;

    if(dstroot->attachment) unattach34(dstroot);
    if(srcroot != NULL && srcroot->attachment) unattach34(srcroot);
    if(!nodematch34(dstroot,srcroot)) {THROWCHK(ncstat=NC_EINVAL); goto done;}
    ncstat = attachall34r(dstroot,srcroot);
done:
    return ncstat;
}
#endif

/* 
Match a DATADDS node to a DDS node.
It is assumed that both trees have been regridded if necessary.
*/

static NCerror
attach34r(CDFnode* xnode, NClist* path, int depth)
{
    unsigned int i,plen,lastnode,gridable;
    NCerror ncstat = NC_NOERR;
    CDFnode* pathnode;
    CDFnode* pathnext;

    plen = nclistlength(path);
    if(depth >= plen) {THROWCHK(ncstat=NC_EINVAL); goto done;}

    lastnode = (depth == (plen-1));
    pathnode = (CDFnode*)nclistget(path,depth);
    ASSERT((simplenodematch34(xnode,pathnode)));
    setattach(xnode,pathnode);    

    if(lastnode) goto done; /* We have the match and are done */

    if(nclistlength(xnode->array.dimensions) > 0) {
	attachdims34(xnode,pathnode);
    }

    ASSERT((!lastnode));
    pathnext = (CDFnode*)nclistget(path,depth+1);

    gridable = (pathnext->nctype == NC_Grid && depth+2 < plen);

    /* Try to find an xnode subnode that matches pathnext */
    for(i=0;i<nclistlength(xnode->subnodes);i++) {
        CDFnode* xsubnode = (CDFnode*)nclistget(xnode->subnodes,i);
        if(simplenodematch34(xsubnode,pathnext)) {
	    ncstat = attach34r(xsubnode,path,depth+1);
	    if(ncstat) goto done;
        } else if(gridable && xsubnode->nctype == NC_Primitive) {
            /* grids may or may not appear in the datadds;
	       try to match the xnode subnodes against the parts of the grid
	    */
   	    CDFnode* pathnext2 = (CDFnode*)nclistget(path,depth+2);
	    if(simplenodematch34(xsubnode,pathnext2)) {
	        ncstat = attach34r(xsubnode,path,depth+2);
                if(ncstat) goto done;
	    }
	}
    }
done:
    return THROW(ncstat);
}

NCerror
attach34(CDFnode* xroot, CDFnode* ddstarget)
{
    NCerror ncstat = NC_NOERR;
    NClist* path = nclistnew();
    CDFnode* ddsroot = ddstarget->root;

    if(xroot->attachment) unattach34(xroot);
    if(ddsroot != NULL && ddsroot->attachment) unattach34(ddsroot);
    if(!simplenodematch34(xroot,ddsroot))
	{THROWCHK(ncstat=NC_EINVAL); goto done;}
    collectnodepath3(ddstarget,path,WITHDATASET);
    ncstat = attach34r(xroot,path,0);
done:
    nclistfree(path);
    return ncstat;
}

/* 
Match nodes in src tree to nodes in dst tree;
src tree is typically a structural subset of dst tree.
WARNING: Dimensions are not attached 
*/

NCerror
attachsubset34(CDFnode* dstroot, CDFnode* srcroot)
{
    NCerror ncstat = NC_NOERR;

    if(srcroot == NULL) {THROWCHK(ncstat=NC_NOERR); goto done;}
    if(!nodematch34(dstroot,srcroot)) {THROWCHK(ncstat=NC_EINVAL); goto done;}
    ncstat = attachsubset34r(dstroot,srcroot);
done:
    return ncstat;
}

static NCerror
attachsubset34r(CDFnode* dstnode, CDFnode* srcnode)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;
    int fieldindex;

    ASSERT((nodematch34(dstnode,srcnode)));
    setattach(dstnode,srcnode);

    /* Try to match dstnode subnodes against srcnode subnodes */

    fieldindex = 0;
    for(fieldindex=0,i=0;i<nclistlength(srcnode->subnodes) && fieldindex<nclistlength(dstnode->subnodes);i++) {
        CDFnode* srcsubnode = (CDFnode*)nclistget(srcnode->subnodes,i);
        CDFnode* dstsubnode = (CDFnode*)nclistget(dstnode->subnodes,fieldindex);
        if(nodematch34(dstsubnode,srcsubnode)) {
            ncstat = attachsubset34r(dstsubnode,srcsubnode);
   	    if(ncstat) goto done;
	    fieldindex++;
	}
    }
done:
    return THROW(ncstat);
}

