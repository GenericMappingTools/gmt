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

static NCerror buildncstructures(NCDAP3*);
static NCerror builddims(NCDAP3*);
static NCerror buildvars(NCDAP3*);
static NCerror buildglobalattrs3(NCDAP3*, int ncid, CDFnode* root);
static NCerror buildattribute3a(NCDAP3*, NCattribute* att, nc_type, int varid, int ncid);


extern CDFnode* v4node;
int nc3dinitialized = 0;


#define getncid(drno) (((NC*)drno)->ext_ncid)

/**************************************************/
/* Add an extra function whose sole purpose is to allow
   configure(.ac) to test for the presence of thiscode.
*/
int nc__opendap(void) {return 0;}

/**************************************************/
/* Do local initialization */

int
nc3dinitialize(void)
{
    int i;
    for(i=0;i<NC_MAX_VAR_DIMS;i++) {
        dapzerostart3[i] = 0;
	dapsinglecount3[i] = 1;
	dapsinglestride3[i] = 1;
    }
    compute_nccalignments();
    nc3dinitialized = 1;
    return NC_NOERR;
}

/**************************************************/
int
NCD3_new_nc(NC** ncpp)
{
    NCDAP3* ncp;
    /* Allocate memory for this info. */
    if (!(ncp = calloc(1, sizeof(struct NCDAP3)))) 
       return NC_ENOMEM;
    if(ncpp) *ncpp = (NC*)ncp;
    return NC_NOERR;
}

/**************************************************/

/* See ncd3dispatch.c for other version */
int
NCD3_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC** ncpp)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    NCDAP3* drno = NULL;
    char* modifiedpath;
    OCURI* tmpurl;
    char* ce = NULL;
    int ncid = -1;
    const char* value;
    int fd;
    char* tmpname = NULL;

    if(!nc3dinitialized) nc3dinitialize();


    if(!ocuriparse(path,&tmpurl)) PANIC("libncdap3: non-url path");
    ocurifree(tmpurl); /* no longer needed */

#ifdef OCCOMPILEBYDEFAULT
    /* set the compile flag by default */
    modifiedpath = (char*)emalloc(strlen(path)+strlen("[compile]")+1);
    strcpy(modifiedpath,"[compile]");
    strcat(modifiedpath,path);    
#else
    modifiedpath = nulldup(path);
#endif

    /* Use libsrc code to establish initial NC(alias NCDRNO) structure */
    tmpname = nulldup(PSEUDOFILE);
    fd = mkstemp(tmpname);
    if(fd < 0) {THROWCHK(errno); goto done;}
    /* Now, use the file to create the netcdf file */
    if(sizeof(size_t) == sizeof(unsigned int))
        ncstat = NC3_create(tmpname,NC_CLOBBER,0,0,NULL,0,NULL,
                            dispatch,(NC**)&drno);
    else
        ncstat = NC3_create(tmpname,NC_CLOBBER|NC_64BIT_OFFSET,0,0,NULL,0,NULL,
                            dispatch,(NC**)&drno);
    /* free the original fd */
    close(fd);
    /* unlink the temp file so it will automatically be reclaimed */
    unlink(tmpname);
    nullfree(tmpname);
    /* Avoid fill */
    NC3_set_fill(ncid,NC_NOFILL,NULL);

    /* Setup tentative DRNO state*/
    drno->dap.controller = (NC*)drno;
    drno->dap.oc.urltext = modifiedpath;
    ocuriparse(drno->dap.oc.urltext,&drno->dap.oc.uri);
    if(!constrainable34(drno->dap.oc.uri))
	SETFLAG(drno->dap.controls,NCF_UNCONSTRAINABLE);
    drno->dap.cdf.separator = ".";
    drno->dap.cdf.smallsizelimit = DFALTSMALLLIMIT;
    drno->dap.cdf.cache = createnccache();
    drno->nc.dispatch = dispatch;

    /* process control client parameters */
    applyclientparamcontrols3(&drno->dap);

    drno->dap.oc.dapconstraint = (DCEconstraint*)dcecreate(CES_CONSTRAINT);
    drno->dap.oc.dapconstraint->projections = nclistnew();
    drno->dap.oc.dapconstraint->selections = nclistnew();

    /* Check to see if we are unconstrainable */
    if(FLAGSET(drno->dap.controls,NCF_UNCONSTRAINABLE)) {
	if(drno->dap.oc.uri->constraint != NULL
	   && strlen(drno->dap.oc.uri->constraint) > 0) {
	    nclog(NCLOGWARN,"Attempt to constrain an unconstrainable data source: %s",
		   drno->dap.oc.uri->constraint);
	}
	/* ignore all constraints */
    } else {
        /* Parse constraints to make sure that they are syntactically correct */
        ncstat = parsedapconstraints(&drno->dap,drno->dap.oc.uri->constraint,drno->dap.oc.dapconstraint);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
#ifdef DEBUG
fprintf(stderr,"parsed constraint: %s\n",
	dumpconstraint(drno->dap.oc.dapconstraint));
#endif

    /* Pass to OC */
    ocstat = oc_open(drno->dap.oc.urltext,&drno->dap.oc.conn);
    if(ocstat != OC_NOERR) {THROWCHK(ocstat); goto done;}

    if(paramcheck34(&drno->dap,"show","fetch"))
	SETFLAG(drno->dap.controls,NCF_SHOWFETCH);

    /* Turn on logging; only do this after oc_open*/
    value = oc_clientparam_get(drno->dap.oc.conn,"log");
    if(value != NULL) {
	ncloginit();
        ncsetlogging(1);
        nclogopen(value);
	oc_loginit();
        oc_setlogging(1);
        oc_logopen(value);
    }

    /* fetch and build the (almost) unconstrained DDS for use as
       template */
    ncstat = fetchtemplatemetadata3(&drno->dap);
    if(ncstat != NC_NOERR) goto done;

    /* Process the constraints to map the CDF tree */
    ncstat = mapconstraints3(&drno->dap);
    if(ncstat != NC_NOERR) goto done;

    /* fetch and build the constrained DDS */
    ncstat = fetchconstrainedmetadata3(&drno->dap);
    if(ncstat != NC_NOERR) goto done;

    /* The following actions are WRT to the
	constrained tree */

    /* Accumulate useful nodes sets  */
    ncstat = computecdfnodesets3(&drno->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Fix grids */
    ncstat = fixgrids3(&drno->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Locate and mark usable sequences */
    ncstat = sequencecheck3(&drno->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Conditionally suppress variables not in usable
       sequences */
    if(FLAGSET(drno->dap.controls,NCF_NOUNLIM)) {
        ncstat = suppressunusablevars3(&drno->dap);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* apply client parameters (after computcdfinfo and computecdfvars)*/
    ncstat = applyclientparams34(&drno->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Add (as needed) string dimensions*/
    ncstat = addstringdims(drno);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    if(nclistlength(drno->dap.cdf.seqnodes) > 0) {
	/* Build the sequence related dimensions */
        ncstat = defseqdims(drno);
        if(ncstat) {THROWCHK(ncstat); goto done;}
    }

    /* Build a cloned set of dimensions for every variable */
    ncstat = clonecdfdims34(&drno->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Re-compute the dimension names*/
    ncstat = computecdfdimnames34(&drno->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Deal with zero size dimensions */
    ncstat = fixzerodims3(&drno->dap);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    if(nclistlength(drno->dap.cdf.seqnodes) == 0
       && drno->dap.cdf.recorddim != NULL) {
	/* Attempt to use the DODS_EXTRA info to turn
           one of the dimensions into unlimited. Can only do it
           in a sequence free DDS.
        */
        ncstat = defrecorddim3(drno);
        if(ncstat) {THROWCHK(ncstat); goto done;}
   }

    /* Re-compute the var names*/
    ncstat = computecdfvarnames3(&drno->dap,drno->dap.cdf.ddsroot,drno->dap.cdf.varnodes);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    /* Estimate the variable sizes */
    estimatevarsizes3(&drno->dap);

    /* Build the meta data */
    ncstat = buildncstructures(drno);

    if(ncstat != NC_NOERR) {
        del_from_NCList((NC*)drno); /* undefine here */
	{THROWCHK(ncstat); goto done;}
    }

    /* Do any necessary data prefetch */
    ncstat = prefetchdata3(&drno->dap);
    if(ncstat != NC_NOERR) {
        del_from_NCList((NC*)drno); /* undefine here */
	{THROWCHK(ncstat); goto done;}
    }

    /* Mark as no longer indef */
    fClr(drno->nc.flags, NC_INDEF);
    /* Mark as no longer writable */
    fClr(drno->nc.nciop->ioflags, NC_WRITE);

    if(ncpp) *ncpp = (NC*)drno;

    return THROW(NC_NOERR);

done:
    if(drno != NULL) {
	int ncid = drno->nc.ext_ncid;
	cleanNCDAP3(drno);
	NC3_abort(ncid);
    }
    if(ce) nullfree(ce);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

int
NCD3_close(int ncid)
{
    int ncstatus = NC_NOERR;
    NCDAP3* drno;

    ncstatus = NC_check_id(ncid, (NC**)&drno); 
    if(ncstatus != NC_NOERR) return THROW(ncstatus);

    cleanNCDAP3(drno);
    NC3_abort(ncid);    

    return THROW(ncstatus);
}

/**************************************************/
static NCerror
buildncstructures(NCDAP3* drno)

{
    NCerror ncstat = NC_NOERR;
    CDFnode* dds = drno->dap.cdf.ddsroot;
    ncstat = buildglobalattrs3(drno,getncid(drno),dds);
    if(ncstat != NC_NOERR) goto done;
    ncstat = builddims(drno);
    if(ncstat != NC_NOERR) goto done;
    ncstat = buildvars(drno);
    if(ncstat != NC_NOERR) goto done;
done:
    return THROW(ncstat);
}

static NCerror
builddims(NCDAP3* drno)
{
    int i;
    NCerror ncstat = NC_NOERR;
    int dimid;
    int ncid = getncid(drno);
    int defunlimited = 0;
    NClist* dimset = NULL;

    /* collect all dimensions from variables */
    dimset = getalldims3(drno->dap.cdf.varnodes,1);
    /* exclude unlimited */
    for(i=nclistlength(dimset)-1;i>=0;i--) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
        if(DIMFLAG(dim,CDFDIMUNLIM)) {
	    defunlimited = 1;
	    nclistremove(dimset,i);
        }
    }
    /* Sort by fullname just for the fun of it */
    for(;;) {
	int last = nclistlength(dimset) - 1;
	int swap = 0;
        for(i=0;i<last;i++) {
	    CDFnode* dim1 = (CDFnode*)nclistget(dimset,i);
	    CDFnode* dim2 = (CDFnode*)nclistget(dimset,i+1);
   	    if(strcmp(dim1->ncfullname,dim2->ncfullname) > 0) {
		nclistset(dimset,i,(ncelem)dim2);
		nclistset(dimset,i+1,(ncelem)dim1);
		swap = 1;
		break;
	    }
	}
	if(!swap) break;
    }
    /* Define unlimited only if needed */ 
    if(defunlimited && drno->dap.cdf.unlimited != NULL) {
	CDFnode* unlimited = drno->dap.cdf.unlimited;
	size_t unlimsize;
        ncstat = nc_def_dim(ncid,
			unlimited->name,
			NC_UNLIMITED,
			&unlimited->ncid);
        if(ncstat != NC_NOERR) goto done;
        if(DIMFLAG(unlimited,CDFDIMRECORD)) {
	    /* This dimension was defined as unlimited by DODS_EXTRA */
	    unlimsize = unlimited->dim.declsize;
	} else { /* Sequence UNLIMITED */
	    unlimsize = 0;
	}
        /* Set the effective size of UNLIMITED;
           note that this cannot be done thru the normal API.*/
        NC_set_numrecs((NC*)drno,unlimsize);
    }

    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
        if(dim->dim.basedim != NULL) continue; /* handle below */
        ncstat = nc_def_dim(ncid,dim->ncfullname,dim->dim.declsize,&dimid);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
        dim->ncid = dimid;
    }

    /* Make all duplicate dims have same dimid as basedim*/
    /* (see computecdfdimnames)*/
    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
        if(dim->dim.basedim != NULL) {
	    dim->ncid = dim->dim.basedim->ncid;
	}
    }
done:
    nclistfree(dimset);
    return THROW(ncstat);
}

/* Simultaneously build any associated attributes*/
/* and any necessary pseudo-dimensions for string types*/
static NCerror
buildvars(NCDAP3* drno)
{
    int i,j,dimindex;
    NCerror ncstat = NC_NOERR;
    int varid;
    int ncid = getncid(drno);
    NClist* varnodes = drno->dap.cdf.varnodes;

    ASSERT((varnodes != NULL));
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
        int dimids[NC_MAX_VAR_DIMS];
	unsigned int ncrank;
        NClist* vardims = NULL;

	if(!var->visible) continue;
	if(var->array.basevar != NULL) continue;

#ifdef DEBUG
fprintf(stderr,"buildvars.candidate=|%s|\n",var->ncfullname);
#endif

	vardims = var->array.dimensions;
	ncrank = nclistlength(vardims);
	if(ncrank > 0) {
	    dimindex = 0;
            for(j=0;j<ncrank;j++) {
                CDFnode* dim = (CDFnode*)nclistget(vardims,j);
                dimids[dimindex++] = dim->ncid;
 	    }
        }   
        ncstat = nc_def_var(ncid,var->ncfullname,
                        var->externaltype,
                        ncrank,
                        (ncrank==0?NULL:dimids),
                        &varid);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
        var->ncid = varid;
	if(var->attributes != NULL) {
	    for(j=0;j<nclistlength(var->attributes);j++) {
		NCattribute* att = (NCattribute*)nclistget(var->attributes,j);
		ncstat = buildattribute3a(drno,att,var->etype,varid,ncid);
        	if(ncstat != NC_NOERR) goto done;
	    }
	}
	/* Tag the variable with its DAP path */
	if(paramcheck34(&drno->dap,"show","projection"))
	    showprojection3(drno,var);
    }    
done:
    return THROW(ncstat);
}


static NCerror
buildglobalattrs3(NCDAP3* drno, int ncid, CDFnode* root)
{
    int i;
    NCerror ncstat = NC_NOERR;
    const char* txt;
    char *nltxt, *p;
    NCbytes* buf = NULL;
    NClist* cdfnodes;

    if(root->attributes != NULL) {
        for(i=0;i<nclistlength(root->attributes);i++) {
   	    NCattribute* att = (NCattribute*)nclistget(root->attributes,i);
	    ncstat = buildattribute3a(drno,att,NC_NAT,NC_GLOBAL,ncid);
            if(ncstat != NC_NOERR) goto done;
	}
    }

    /* Add global attribute identifying the sequence dimensions */
    if(paramcheck34(&drno->dap,"show","seqdims")) {
        buf = ncbytesnew();
        cdfnodes = drno->dap.cdf.ddsroot->tree->nodes;
        for(i=0;i<nclistlength(cdfnodes);i++) {
	    CDFnode* dim = (CDFnode*)nclistget(cdfnodes,i);
	    if(dim->nctype != NC_Dimension) continue;
	    if(DIMFLAG(dim,CDFDIMSEQ)) {
	        char* cname = cdflegalname3(dim->name);
	        if(ncbyteslength(buf) > 0) ncbytescat(buf,", ");
	        ncbytescat(buf,cname);
	        nullfree(cname);
	    }
	}
        if(ncbyteslength(buf) > 0) {
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_sequence_dimensions",
	           ncbyteslength(buf),ncbytescontents(buf));
	}
    }

    /* Define some additional system global attributes
       depending on show= clientparams*/
    /* Ignore failures*/

    if(paramcheck34(&drno->dap,"show","translate")) {
        /* Add a global attribute to show the translation */
        ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_translate",
	           strlen("netcdf-3"),"netcdf-3");
    }
    if(paramcheck34(&drno->dap,"show","url")) {
	if(drno->dap.oc.urltext != NULL)
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_url",
				       strlen(drno->dap.oc.urltext),drno->dap.oc.urltext);
    }
    if(paramcheck34(&drno->dap,"show","dds")) {
	txt = NULL;
	if(drno->dap.cdf.ddsroot != NULL)
  	    txt = oc_inq_text(drno->dap.oc.conn,drno->dap.cdf.ddsroot->dds);
	if(txt != NULL) {
	    /* replace newlines with spaces*/
	    nltxt = nulldup(txt);
	    for(p=nltxt;*p;p++) {if(*p == '\n' || *p == '\r' || *p == '\t') {*p = ' ';}};
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_dds",strlen(nltxt),nltxt);
	    nullfree(nltxt);
	}
    }
    if(paramcheck34(&drno->dap,"show","das")) {
	txt = NULL;
	if(drno->dap.oc.ocdasroot != OCNULL)
	    txt = oc_inq_text(drno->dap.oc.conn,drno->dap.oc.ocdasroot);
	if(txt != NULL) {
	    nltxt = nulldup(txt);
	    for(p=nltxt;*p;p++) {if(*p == '\n' || *p == '\r' || *p == '\t') {*p = ' ';}};
            ncstat = nc_put_att_text(ncid,NC_GLOBAL,"_das",strlen(nltxt),nltxt);
	    nullfree(nltxt);
	}
    }

done:
    ncbytesfree(buf);
    return THROW(ncstat);
}

static NCerror
buildattribute3a(NCDAP3* drno, NCattribute* att, nc_type vartype, int varid, int ncid)
{
    int i;
    NCerror ncstat = NC_NOERR;
    char* cname = cdflegalname3(att->name);
    unsigned int nvalues = nclistlength(att->values);

    /* If the type of the attribute is string, then we need*/
    /* to convert to a single character string by concatenation.
	modified: 10/23/09 to insert newlines.
	modified: 10/28/09 to interpret escapes
    */
    if(att->etype == NC_STRING || att->etype == NC_URL) {
	char* newstring;
	size_t newlen = 0;
	for(i=0;i<nvalues;i++) {
	    char* s = (char*)nclistget(att->values,i);
	    newlen += (1+strlen(s));
	}
	newstring = (char*)malloc(newlen);
        MEMCHECK(newstring,NC_ENOMEM);
	newstring[0] = '\0';
	for(i=0;i<nvalues;i++) {
	    char* s = (char*)nclistget(att->values,i);
	    if(i > 0) strcat(newstring,"\n");
	    strcat(newstring,s);
	}
        dapexpandescapes(newstring);
	if(newstring[0]=='\0')
	    ncstat = nc_put_att_text(ncid,varid,cname,1,newstring);
	else
	    ncstat = nc_put_att_text(ncid,varid,cname,strlen(newstring),newstring);
	free(newstring);
    } else {
	nc_type atype;
	unsigned int typesize;
	void* mem;
	/* It turns out that some servers upgrade the type
           of _FillValue in order to correctly preserve the
           original value. However, since the type of the
           underlying variable is not changes, we get a type
           mismatch. So, make sure the type of the fillvalue
           is the same as that of the controlling variable.
	*/
        if(varid != NC_GLOBAL && strcmp(att->name,"_FillValue")==0)
	    atype = nctypeconvert(&drno->dap,vartype);
	else
	    atype = nctypeconvert(&drno->dap,att->etype);
	typesize = nctypesizeof(atype);
	mem = malloc(typesize * nvalues);
        ncstat = dapcvtattrval3(atype,mem,att->values);
        ncstat = nc_put_att(ncid,varid,cname,atype,nvalues,mem);
	nullfree(mem);
    }
    free(cname);
    return THROW(ncstat);
}
