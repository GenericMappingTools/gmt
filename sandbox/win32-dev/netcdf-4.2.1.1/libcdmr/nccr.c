/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "includes.h"
#include "nccrdispatch.h"
#include "nc4internal.h"

#ifdef HAVE_GETRLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#endif

static void freeNCCDMR(NCCDMR* cdmr);
static int nccr_process_projections(NCCDMR* cdmr);

static int nccr_collect_allvariables(NClist* streamnodes, NClist* varset);
static int nccr_mark_segment_decls(CCEprojection* p, CRnode* leaf);
static int nccr_collect_projection_variables(NCCDMR* cdmr);
static int nccr_mark_visible(NCCDMR* cdmr);

/**************************************************/
#ifdef NOTUSED
int
NCCR_new_nc(NC** ncpp)
{
    NCCR* ncp;

    /* Allocate memory for this info. */
    if (!(ncp = calloc(1, sizeof(struct NCCR)))) 
       return NC_ENOMEM;
    if(ncpp) *ncpp = (NC*)ncp;
    return NC_NOERR;
}
#endif

/**************************************************/
/* See ncd4dispatch.c for other version */
int
NCCR_open(const char * path, int mode,
               int basepe, size_t *chunksizehintp,
 	       int useparallel, void* mpidata,
               NC_Dispatch* dispatch, NC** ncpp)
{
    int ncstat = NC_NOERR;
    NC_URI* tmpurl;
    NC* drno = NULL;
    NCCDMR* cdmr = NULL;
    char* tmpname = NULL;
    const char* lookups = NULL;
    bytes_t buf;
    long filetime;
    ast_err aststat = AST_NOERR;
    char* curlurl = NULL;

    LOG((1, "nc_open_file: path %s mode %d", path, mode));

    if(!nc_uriparse(path,&tmpurl)) PANIC("libcdmr: non-url path");
    nc_urifree(tmpurl); /* no longer needed */

    /* Check for legal mode flags */
    if((mode & NC_WRITE) != 0) ncstat = NC_EINVAL;
    else if(mode & (NC_WRITE|NC_CLOBBER)) ncstat = NC_EPERM;
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    mode = (mode & ~(NC_MPIIO | NC_MPIPOSIX));
    /* Despite the above check, we want the file to be initially writable */
    mode |= (NC_WRITE|NC_CLOBBER);

    /* Setup our NC and NCCDMR state*/
    drno = (NC*)calloc(1,sizeof(NC));
    if(drno == NULL) {ncstat = NC_ENOMEM; goto fail;}
    /* compute an ncid */
    ncstat = add_to_NCList(drno);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    cdmr = (NCCDMR*)calloc(1,sizeof(NCCDMR));
    if(cdmr == NULL) {ncstat = NC_ENOMEM; goto fail;}

    drno->dispatch = dispatch;
    drno->dispatchdata = cdmr;

    cdmr->controller = drno;
    cdmr->urltext = nulldup(path);
    nc_uriparse(cdmr->urltext,&cdmr->uri);

#ifdef WORDS_BIGENDIAN
    cdmr->controls |= BIGENDIAN;
#endif

    /* Use libsrc4 code for storing metadata */
    tmpname = nulldup(PSEUDOFILE);
    /* Now, use the file to create the netcdf file */
    ncstat = nc_create(tmpname,NC_CLOBBER|NC_NETCDF4,&drno->substrate);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* free the filename so it will automatically go away*/
    unlink(tmpname);
    nullfree(tmpname);

    /* Avoid fill */
    nc_set_fill(drno->substrate,NC_NOFILL,NULL);

    /* Create the curl connection (does not make the server connection)*/
    ncstat = nccr_curlopen(&cdmr->curl.curl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* Turn on logging; only do this after open*/
    if(nc_urilookup(cdmr->uri,"log",&lookups)) {
	ncloginit();
        ncsetlogging(1);
        nclogopen(lookups);
    }

    if(nc_urilookup(cdmr->uri,"show",&lookups)) {
	int i;
	for(i=0;i<strlen(lookups);i++) {
	    if(lookups[i] ==  ',') continue;
	    if(strcmp("fetch",lookups+i)==0) {
	        cdmr->controls |= SHOWFETCH;
		break;
	    } else if(strcmp("datavars",lookups+i)==0) {
	        cdmr->controls |= DATAVARS;
		break;
	    }
	}
    }

    /* fetch unconstrained meta data */
    buf = bytes_t_null;
    curlurl = nc_uribuild(cdmr->uri,NULL,"?req=header",0);
    if(curlurl == NULL) {ncstat=NC_ENOMEM; goto fail;}
    ncstat = nccr_fetchurl(cdmr,cdmr->curl.curl,curlurl,&buf,&filetime);
    free(curlurl);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* Parse the meta data */
    ncstat = nccr_decodeheadermessage(&buf,&cdmr->ncstreamhdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    if(buf.bytes != NULL) free(buf.bytes);

    /* Compute various things about the Header tree */

    /* Collect all nodes and fill in the CRnode part*/
    cdmr->streamnodes = nclistnew();
    ncstat = nccr_walk_Header(cdmr->ncstreamhdr,cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* Compute the stream pathnames */
    ncstat = nccr_compute_pathnames(cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* Map dimension references to matching declaration */
    ncstat = nccr_map_dimensions(cdmr->streamnodes);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* Replace dimension references with matching declaration */
    nccr_deref_dimensions(cdmr->streamnodes);

    /* Collect all potential variables */
    cdmr->allvariables = nclistnew();
    ncstat = nccr_collect_allvariables(cdmr->streamnodes,cdmr->allvariables);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* Deal with any constraint in the URL */
    cdmr->urlconstraint = (CCEconstraint*)ccecreate(CES_CONSTRAINT);
    if(cdmr->uri->constraint != NULL
       && strlen(cdmr->uri->constraint) > 0) {
        /* Parse url constraint to test syntactically correctness */
        ncstat = cdmparseconstraint(cdmr->uri->constraint,cdmr->urlconstraint);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}
#ifdef DEBUG
fprintf(stderr,"url constraint: %s\n",
	ccetostring((CCEnode*)cdmr->uri->constraint));
#endif
    }

    ncstat = nccr_process_projections(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    ncstat = nccr_collect_projection_variables(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    ncstat = nccr_mark_visible(cdmr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    /* build the netcdf-4 pseudo metadata */
    ncstat = nccr_buildnc(cdmr,cdmr->ncstreamhdr);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto fail;}

    {
        /* Mark as no longer writable and no longer indef;
           requires breaking abstraction  */
	NC* nc;
        NC_FILE_INFO_T* nfit = NULL;
        NC_HDF5_FILE_INFO_T* h5 = NULL;
        NC_GRP_INFO_T *grp = NULL;
        ncstat = NC_check_id(drno->substrate, &nc);
        /* Find our metadata for this file. */
        ncstat = nc4_find_nc_grp_h5(drno->substrate, &nfit, &grp, &h5);
        if(ncstat) {THROWCHK(ncstat); goto fail;}
        /* Mark as no longer indef (do NOT use nc_enddef until diskless is working)*/
        h5->flags &= ~(NC_INDEF);
        /* Mark as no longer writeable */
        h5->no_write = 1;
    }

    if(ncpp) *ncpp = (NC*)drno;
    return ncstat;

fail:
    if(drno != NULL) NCCR_close(drno->ext_ncid);
    if(aststat != AST_NOERR) {ncstat = nccr_cvtasterr(aststat);}
    return THROW(ncstat);
}

int
NCCR_close(int ncid)
{
    NC* drno;
    NCCDMR* cdmr;
    int ncstat = NC_NOERR;

    LOG((1, "nc_close: ncid 0x%x", ncid));

    ncstat = NC_check_id(ncid, (NC**)&drno); 
    if(ncstat != NC_NOERR) return THROW(ncstat);

    cdmr = (NCCDMR*)drno->dispatchdata;

    nc_abort(drno->substrate);

    /* remove ourselves from NClist */
    del_from_NCList(drno);
    /* clean NC* */
    freeNCCDMR(cdmr);
    if(drno->path != NULL) free(drno->path);
    free(drno);
    return THROW(ncstat);
}

/**************************************************/
/* Auxilliary routines                            */
/**************************************************/

static void
freeNCCDMR(NCCDMR* cdmr)
{
    if(cdmr == NULL) return;
    if(cdmr->urltext) free(cdmr->urltext);
    nc_urifree(cdmr->uri);
    if(cdmr->curl.curl) nccr_curlclose(cdmr->curl.curl);
    if(cdmr->curl.host) free(cdmr->curl.host);
    if(cdmr->curl.useragent) free(cdmr->curl.useragent);
    if(cdmr->curl.cookiefile) free(cdmr->curl.cookiefile);
    if(cdmr->curl.certificate) free(cdmr->curl.certificate);
    if(cdmr->curl.key) free(cdmr->curl.key);
    if(cdmr->curl.keypasswd) free(cdmr->curl.keypasswd);
    if(cdmr->curl.cainfo) free(cdmr->curl.cainfo);
    if(cdmr->curl.capath) free(cdmr->curl.capath);
    if(cdmr->curl.username) free(cdmr->curl.username);
    if(cdmr->curl.password) free(cdmr->curl.password);
    free(cdmr);
}

static int
nccr_collect_allvariables(NClist* streamnodes, NClist* varset)
{
    int ncstat = NC_NOERR;
    int i;
	/* A node is a variable if
	 * 1. its sort is _Variable or _Structure
	 * 2. its parent is a group
	*/
    /* Walk the set of all groups */
    for(i=0;i<nclistlength(streamnodes);i++) {
        CRnode* node = (CRnode*)nclistget(streamnodes,i);	
	if(node->sort == _Group) {
	    int j;
            Group* grp = (Group*)node;
	    /* Walk the variables */
	    for(j=0;j<grp->vars.count;j++) {
		nclistpush(varset,(ncelem)grp->vars.values[j]);		
	    }
	    /* Walk the structures */
	    for(j=0;j<grp->structs.count;j++) {
		nclistpush(varset,(ncelem)grp->structs.values[j]);
	    }
	}
    }

    /* Mark as initially invisible */
    for(i=0;i<nclistlength(varset);i++) {
        CRnode* node = (CRnode*)nclistget(varset,i);	
	node->flags.visible = 0;
    }

    return ncstat;
}

/*
1. Compute the complete set of projections
2. map projection variables to corresponding stream variable nodes
*/

static int
nccr_process_projections(NCCDMR* cdmr)
{
    int i,j;
    int ncstat = NC_NOERR;
    CCEconstraint* constraint = cdmr->urlconstraint;
    NClist* projections;
    ASSERT((constraint != NULL));
    /* Setup the constraint */
    if(constraint->projections == NULL)
	constraint->projections = nclistnew();

    /* If there are no constraints, then we will
       make every known variable be in the projection set */
    if(nclistlength(constraint->projections) == 0) {
        projections = constraint->projections;
	ASSERT((projections != NULL));
        for(i=0;i<nclistlength(cdmr->allvariables);i++) {/*Walk the set of known variables */
	    CCEprojection* newp;
	    CCEsegment* newseg;
	    CRnode* var = (CRnode*)nclistget(cdmr->allvariables,i);

	    ASSERT((var->sort == _Variable || var->sort == _Structure));

	    /* Construct a corresponding projection for this variable */
	    newp = (CCEprojection*)ccecreate(CES_PROJECT);
	    newp->decl = var;
	    newp->segments = nclistnew();
	    newseg = (CCEsegment*)ccecreate(CES_SEGMENT);
	    newseg->name = nulldup(nccr_getname(var));
	    ccemakewholesegment(newseg,var);/*treat as simple projection*/
            newseg->decl = var;
	    /* Do we need to compute crpaths? */
            nclistpush(newp->segments,(ncelem)newseg);
	    nclistpush(projections,(ncelem)newp);
	}
    } else {/* nclistlength(projections) > 0 */
        /* map projection variables to corresponding known stream variable */
        projections = constraint->projections;
        if(nclistlength(projections) > 0) {
	    int found = 0;
            for(i=0;i<nclistlength(cdmr->allvariables) && !found;i++) {
	        CRnode* cdmnode = (CRnode*)nclistget(cdmr->allvariables,i);
                for(j=0;j<nclistlength(projections);j++) {	
	            CCEprojection* p = (CCEprojection*)nclistget(projections,j);
	            if(pathmatch(p->segments,cdmnode->pathname)) {
			/* Mark the projection decl and the segment decls */
			p->decl = cdmnode;
			ncstat = nccr_mark_segment_decls(p,cdmnode);
		        found = 1;
		        break;
		    }
		}
	    }
            if(!found) {ncstat = NC_ENOTVAR; goto done;}
	}
    }
#ifdef DEBUG
{ int ix;
/* Report projection variables */
fprintf(stderr,"projections:");
for(ix=0;ix<nclistlength(projections);ix++) {
fprintf(stderr," %s",nccr_getname(((CCEprojection*)nclistget(projections,ix))->decl));
}
fprintf(stderr,"\n");
}
#endif


done:
    return ncstat;
}

static int
nccr_collect_projection_variables(NCCDMR* cdmr)
{
    int i;
    /* Walk the set of projections to collect nodes as the variable set*/
    ASSERT((cdmr->urlconstraint != NULL && cdmr->urlconstraint->projections != NULL));
    if(cdmr->variables == NULL)
	cdmr->variables = nclistnew();
    nclistclear(cdmr->variables);
    for(i=0;i<nclistlength(cdmr->urlconstraint->projections);i++) {
	CCEprojection* p = (CCEprojection*)nclistget(cdmr->urlconstraint->projections,i);
	ASSERT(p->decl != NULL);
	nclistpush(cdmr->variables,(ncelem)p->decl);
	/* Mark as visible */
	p->decl->flags.visible = 1;
    }
#ifdef DEBUG
{ int ix;
fprintf(stderr,"visible:");
for(ix=0;ix<nclistlength(cdmr->urlconstraint->projections);ix++) {
fprintf(stderr," %s",nccr_getname(((CCEprojection*)nclistget(cdmr->urlconstraint->projections,ix))->decl));
}
fprintf(stderr,"\n");
}
#endif
    return NC_NOERR;
}

static int
nccr_mark_segment_decls(CCEprojection* p, CRnode* leaf)
{
    int i;
    int ncstat = NC_NOERR;
    NClist* path = nclistnew();
    crcollectnodepath(leaf,path);    
    /* # of path nodes must be same as number of segments */
    if(nclistlength(path) != nclistlength(p->segments))
	{THROW((ncstat = NC_EINVAL)); goto done;}
    for(i=0;i<nclistlength(path);i++) {
        CRnode* elem = (CRnode*)nclistget(path,i);
	CCEsegment* seg = (CCEsegment*)nclistget(p->segments,i);
	seg->decl = elem;
    }

done:
    nclistfree(path);
    return ncstat;
}

#ifdef IGNORE
Given a new variable, say from nc_get_vara(),
Use it to 
static int
nccr_projection_restrict(NClist* varlist, NClist* projections)
{
    int ncstat = NC_NOERR;
    int i,j,len;

#ifdef DEBUG
fprintf(stderr,"collect_variables: projections=|%s|\n",
		ccelisttostring(projections, ","));
#endif

    if(nclistlength(varlist) == 0) goto done; /* nothing to add or remove */

    /* If the projection list is empty, then add
       a projection for every variable in varlist
    */
    if(nclistlength(projections) == 0) {
        NClist* path = nclistnew();
	NClist* nodeset = NULL;

        for(i=0;i<nclistlength(varlist);i++) {
	    CCEprojection* newp;
	    CRnode* var = (CRnode*)nclistget(varlist,i);
#ifdef DEBUG
fprintf(stderr,"restriction.candidate=|%s|\n",crpathstring(var->pathname));
#endif
	    newp = (CCEprojection*)ccecreate(CES_PROJECT);
	    newp->decl = var;
	    nclistclear(path);
	    crcollectnodepath(var,path);
	    newp->segments = nclistnew();
	    for(j=0;j<nclistlength(path);j++) {
	        CRnode* node = (CRnode*)nclistget(path,j);
	        CCEsegment* newseg = (CCEsegment*)ccecreate(CES_SEGMENT);
	        newseg->name = nulldup(nccr_getname(node));
	        ccemakewholesegment(newseg,node);/*treat as simple projections*/
	        newseg->decl = node;
	        nclistpush(newp->segments,(ncelem)newseg);
	    }
	    nclistpush(projections,(ncelem)newp);
	}
	nclistfree(path);
	nclistfree(nodeset);
    } else {
       /* Otherwise, walk all the projections remove duplicates,
          (WARNING) where duplicate means same node but with possibly different
          slices
	*/
	len = nclistlength(projections);
	for(i=len-1;i>=0;i--) {/* Walk backward to facilitate removal*/
	    CCEprojection* proj = (CCEprojection*)nclistget(projections,i);
	    int found;
	    for(j=0;j<nclistlength(varlist);j++) {
		CRnode* var;
		found = 0;
		var = (CRnode*)nclistget(varlist,j);
		if(var == proj->decl) {found = 1; break;}
	    }	    
	    if(found) {
		/* suppress this projection */
		CCEprojection* p = (CCEprojection*)nclistremove(projections,i);
		ccefree((CCEnode*)p);
	    }
	}
    }
    
done:
#ifdef DEBUG
fprintf(stderr,"restriction.after=|%s|\n",
		ccelisttostring(projections,","));
#endif
    return;
}
#endif

static int
nccr_mark_visible(NCCDMR* cdmr)
{
    int ncstat = NC_NOERR;
    int i,j;

    /* Mark visible variables */
    for(i=0;i<nclistlength(cdmr->variables);i++) {
	CRnode* var = (CRnode*)nclistget(cdmr->variables,i);
	CRshape shape;
	var->flags.visible = 1;
        /* For each variable, mark any dimensions it uses as also visible */
	crextractshape(var,&shape);
	for(j=0;j<shape.rank;j++) {
	    Dimension* dim = shape.dims[j];
	    ((CRnode*)dim)->flags.visible = 1;
	}
    }
    return ncstat;
}
