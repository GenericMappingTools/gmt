/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapattr3.c,v 1.14 2009/12/03 03:42:38 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"

#define OCCHECK(exp) if((ocstat = (exp))) {THROWCHK(ocstat); goto done;}

/* Forward */
static NCerror buildattribute(char*,nc_type,size_t,char**,NCattribute**);

/*
Invoke oc_merge_das and then extract special
attributes such as "strlen" and "dimname"
and stuff from DODS_EXTRA.
*/
int
dapmerge3(NCDAPCOMMON* nccomm, CDFnode* ddsroot, OCddsnode dasroot)
{
    int i,j;
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    NClist* allnodes;
    OClink conn;
    char* ocname = NULL;

    conn = nccomm->oc.conn;

    if(ddsroot == NULL || dasroot == NULL) return NC_NOERR;
    /* Merge the das tree onto the dds tree */ 
    ocstat = oc_merge_das(nccomm->oc.conn,dasroot,ddsroot);
    if(ocstat != OC_NOERR) goto done;

    /* Create attributes on CDFnodes */
    allnodes = nccomm->cdf.ddsroot->tree->nodes;
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(allnodes,i);
	OCddsnode ocnode = node->ocnode;
	size_t attrcount;
	OCtype ocetype;
			
	OCCHECK(oc_dds_attr_count(conn,ocnode,&attrcount));
	for(j=0;j<attrcount;j++) {
	    size_t nvalues;
	    char** values = NULL;
	    NCattribute* att = NULL;

	    if(ocname != NULL) free(ocname); /* from last loop */
	    OCCHECK(oc_dds_attr(conn,ocnode,j,&ocname,&ocetype,&nvalues,NULL));
	    if(nvalues > 0) {
	        values = (char**)malloc(sizeof(char*)*nvalues);
		if(values == NULL) {ncstat = NC_ENOMEM; goto done;}
	        OCCHECK(oc_dds_attr(conn,ocnode,j,NULL,NULL,NULL,values));
	    }
	    ncstat = buildattribute(ocname,octypetonc(ocetype),nvalues,values,&att);
	    if(ncstat != NC_NOERR) goto done;
	    if(node->attributes == NULL)
		node->attributes = nclistnew();
	    nclistpush(node->attributes,(ncelem)att);
	    if(strncmp(ocname,"DODS",strlen("DODS"))==0) {
		att->invisible = 1;
	        /* Define extra semantics associated with
                   DODS and DODS_EXTRA attributes */
		if(strcmp(ocname,"DODS.strlen")==0
		   || strcmp(ocname,"DODS_EXTRA.strlen")==0) {
		    unsigned int maxstrlen = 0;
		    if(values != NULL) {
			if(0==sscanf(values[0],"%u",&maxstrlen))
			    maxstrlen = 0;
		    }
		    node->dodsspecial.maxstrlen = maxstrlen;
#ifdef DEBUG
fprintf(stderr,"%s.maxstrlen=%d\n",node->ocname,(int)node->dodsspecial.maxstrlen);
#endif
		} else if(strcmp(ocname,"DODS.dimName")==0
		   || strcmp(ocname,"DODS_EXTRA.dimName")==0) {
		    if(values != NULL) {
		        node->dodsspecial.dimname = nulldup(values[0]);
#ifdef DEBUG
fprintf(stderr,"%s.dimname=%s\n",node->ocname,node->dodsspecial.dimname);
#endif
		    } else node->dodsspecial.dimname = NULL;
		} else if(strcmp(ocname,"DODS.Unlimited_Dimension")==0
		   || strcmp(ocname,"DODS_EXTRA.Unlimited_Dimension")==0) {
		    if(values != NULL) {
		        if(nccomm->cdf.recorddimname != NULL)
		            nclog(NCLOGWARN,"Duplicate DODS_EXTRA:Unlimited_Dimension specifications");
			else
		            nccomm->cdf.recorddimname = nulldup(values[0]);
#ifdef DEBUG
fprintf(stderr,"%s.Unlimited_Dimension=%s\n",node->ocname,nccomm->cdf.recorddimname);
#endif
		    }

		}
	    }
	    /* clean up */
	    if(values) {
		oc_reclaim_strings(nvalues,values);
		free(values);
	    }
	}
    }

done:
    if(ocname != NULL) free(ocname);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

/* Build an NCattribute */
static NCerror
buildattribute(char* name, nc_type ptype,
               size_t nvalues, char** values, NCattribute** attp)
{
    int i;
    NCerror ncstat = NC_NOERR;
    NCattribute* att;

    att = (NCattribute*)calloc(1,sizeof(NCattribute));
    MEMCHECK(att,NC_ENOMEM);
    att->name = nulldup(name);
    att->etype = ptype;

    att->values = nclistnew();
    for(i=0;i<nvalues;i++)
	nclistpush(att->values,(ncelem)nulldup(values[i]));

    if(attp) *attp = att;

    return THROW(ncstat);
}

#if 0
/*
Given a das attribute walk it to see if it
has at least 1 actual attribute (no recursion)
*/
static int
hasattribute3(OClink conn, OCdasnode dasnode)
{
    int i;
    OCerror ocstat = OC_NOERR;
    int tf = 0; /* assume false */
    OCtype ocsubtype;
    NClist* subnodes = nclistnew();

    OCCHECK(oc_dds_octype(conn,dasnode,&ocsubtype));
    if(ocsubtype == OC_Attribute) return 1; /* this is an attribute */
    ASSERT((ocsubtype == OC_Attributeset));

    OCCHECK(collect_subnodes(conn,dasnode,subnodes));
    for(i=0;i<nclistlength(subnodes);i++) {
        OCdasnode subnode = (OCdasnode)nclistget(subnodes,i);
        OCCHECK(oc_dds_class(conn,subnode,&ocsubtype));
	if(ocsubtype == OC_Attribute) {tf=1; break;}
    }
done:
    nclistfree(subnodes);
    return tf;
}

int
dapmerge3(NCDAPCOMMON* nccomm, CDFnode* ddsroot, OCddsnode dasroot)
{
    unsigned int i,j;
    NCerror ncerr = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    OClink conn = nccomm->oc.conn;
    size_t nsubnodes;
    NClist* dasglobals = nclistnew();
    NClist* dasnodes = nclistnew();
    NClist* dodsextra = nclistnew();
    NClist* varnodes = nclistnew();
    NClist* alldasnodes = nclistnew();    

    if(ddsroot == NULL || dasroot == NULL) return NC_NOERR;

    ocstat = collect_alldasnodes(conn,dasroot,alldasnodes);
    
    /* 1. collect all the relevant DAS nodes;
          namely those that contain at least one
          attribute value.
          Simultaneously look for potential ambiguities
          if found; complain but continue: result are indeterminate.
          also collect globals and DODS_EXTRA separately.
    */
    for(i=0;i<nclistlength(alldasnodes);i++) {
	OCddsnode das = (OCddsnode)nclistget(alldasnodes,i);
	OCtype octype;
        char* ocname = NULL;
	int isglobal = 0;
	int hasattributes = 0;

        OCCHECK(oc_dds_class(conn,das,&octype));
	if(octype == OC_Attribute) continue; /* ignore these for now*/

        OCCHECK(oc_dds_name(conn,das,&ocname));
	OCCHECK(oc_dds_nsubnodes(conn,das,&nsubnodes));

	isglobal = (ocname == NULL ? 0 : isglobalname3(ocname));

	/* catch DODS_EXTRA */
	if(isglobal && ocname != NULL && strcmp(ocname,"DODS_EXTRA")==0) {
	    nclistpush(dodsextra,(ncelem)das);
	    nullfree(ocname);
	    continue;
	}
	if(ocname == NULL || isglobal) {
            nclistpush(dasglobals,(ncelem)das);
	    nullfree(ocname);
	    continue;
	}
	hasattributes = hasattribute3(conn,das);
	if(hasattributes) {
	    /* Look for previously collected nodes with same name*/
            for(j=0;j<nclistlength(dasnodes);j++) {
	        OCddsnode das2 = (OCddsnode)nclistget(dasnodes,j);
		char* ocname2;
	        OCCHECK(oc_dds_name(conn,das2,&ocname2));
		if(ocname2 == NULL || ocname == NULL) goto loop;
		if(strcmp(ocname2,"DODS")==0) goto loop;
	        if(strcmp(ocname,ocname2)==0)
		        nclog(NCLOGWARN,"nc_mergedas: potentially ambiguous DAS name: %s",ocname2);
loop:
		nullfree(ocname2);
	    }
	    nclistpush(dasnodes,(ncelem)das);
	}
	nullfree(ocname);
    }

    /* 2. collect all the leaf DDS nodes (of type NC_Atomic)*/
    ocstat = collect_leaves(link,ddsroot,varnodes);

    /* 3. For each das node, locate matching DDS node(s) and attach
          attributes to the DDS node(s).
          Match means:
          1. DAS->fullname :: DDS->fullname
          2. DAS->name :: DDS->fullname (support DAS names with embedded '.'
          3. DAS->name :: DDS->name
	  4. special case for DODS. Apply 1-3 on DODS parent.
    */
    for(i=0;i<nclistlength(dasnodes);i++) {
	OCddsnode das = (OCddsnode)nclistget(dasnodes,i);
	char* ocfullname = NULL;
	char* ocbasename = NULL;

	if(das == NULL) continue;
	OCCHECK(oc_dds_name(conn,das,&ocbasename));
	if(strcmp(ocbasename,"DODS")==0) {
	    OCddsnode container;
   	    OCCHECK(oc_dds_container(conn,das,&container));
            ASSERT(container != NULL);
	    ocfullname = makeocpathstring3(conn,container,".");
	} else {
	    ocfullname = makeocpathstring3(conn,das,".");
	}
        for(j=0;j<nclistlength(varnodes);j++) {
	    CDFnode* dds = (CDFnode*)nclistget(varnodes,j);
	    char* ddsfullname = makecdfpathstring3(dds,".");
	    if(strcmp(ocfullname,ddsfullname)==0
	       || strcmp(ocbasename,ddsfullname)==0
	       || strcmp(ocbasename,dds->ocname)==0) {
		mergedas1(nccomm,conn,dds,das);
		/* remove from dasnodes list*/
		nclistset(dasnodes,i,(ncelem)NULL);
	    }
	    nullfree(ddsfullname);
	}
	nullfree(ocfullname);
	nullfree(ocbasename);
    }

    /* 4. Assign globals */
    for(i=0;i<nclistlength(dasglobals);i++) {
	OCddsnode das = (OCddsnode)nclistget(dasglobals,i);
	mergedas1(nccomm,conn,ddsroot,das);
    }

    /* 5. Assign DOD_EXTRA */
    for(i=0;i<nclistlength(dodsextra);i++) {
	OCddsnode das = (OCddsnode)nclistget(dodsextra,i);
	mergedas1(nccomm,conn,ddsroot,das);
    }

done: /* cleanup*/
    nclistfree(dasglobals);
    nclistfree(dasnodes);
    nclistfree(alldasnodes);
    nclistfree(dodsextra);
    nclistfree(varnodes);
    if(ocstat != OC_NOERR)
	ncerr = ocerrtoncerr(ocstat);
    return THROW(ncerr);
}

static int
mergedas1(NCDAPCOMMON* nccomm, OClink conn, CDFnode* dds, OCddsnode das)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    unsigned int i,j,k;
    unsigned int nsubnodes;
    OCobject* subnodes = NULL;
    OCobject* dodsnodes = NULL;
    unsigned int ndodsnodes;

    if(dds == NULL || das == OCNULL) return NC_NOERR; /* nothing to do */
    if(dds->attributes == NULL) dds->attributes = nclistnew();
    /* assign the simple attributes in the das set to this dds node*/
    OCCHECK(oc_inq_nsubnodes(conn,das,&nsubnodes));
    OCCHECK(oc_inq_subnodes(conn,das,&subnodes));
    for(i=0;i<nsubnodes;i++) {
	OCobject attnode = subnodes[i];
	OCtype octype, ocetype;
	char* ocname = NULL;
	unsigned int ocnvalues;
        OCCHECK(oc_inq_name(conn,attnode,&ocname));	
        OCCHECK(oc_inq_class(conn,attnode,&octype));
	if(octype == OC_Attribute) {
	    NCattribute* att = NULL;
	    NClist* stringvalues;
            OCCHECK(oc_inq_primtype(conn,attnode,&ocetype));	
	    OCCHECK(oc_inq_dasattr_nvalues(conn,attnode,&ocnvalues));
	    stringvalues = nclistnew();
	    for(j=0;j<ocnvalues;j++) {
		char* stringval;
	        OCCHECK(oc_inq_dasattr(conn,attnode,j,&ocetype,&stringval));
	        nclistpush(stringvalues,(ncelem)stringval);
	    }
	    ncstat = buildattribute(ocname,
				    octypetonc(ocetype),
				    stringvalues,
				    &att);				
	    if(ncstat) goto done;
            nclistpush(dds->attributes,(ncelem)att);
	} else if(octype == OC_Attributeset
		  && (strcmp(ocname,"DODS")==0
		      || strcmp(ocname,"DODS_EXTRA")==0)) {
	    /* Turn the DODS special attributes into into
               special attributes for dds node */
	    OCCHECK(oc_inq_nsubnodes(conn,attnode,&ndodsnodes));
	    OCCHECK(oc_inq_subnodes(conn,attnode,&dodsnodes));
	    for(j=0;j<ndodsnodes;j++) {
		char* dodsname = NULL;
		char newname[4096];
	        OCobject attnode = dodsnodes[j];
	        NCattribute* att = NULL;
	        NClist* stringvalues;
	        OCCHECK(oc_inq_class(conn,attnode,&octype));
		if(octype != OC_Attribute) continue;
                OCCHECK(oc_inq_primtype(conn,attnode,&ocetype));	
	        OCCHECK(oc_inq_dasattr_nvalues(conn,attnode,&ocnvalues));
	        stringvalues = nclistnew();
	        for(k=0;k<ocnvalues;k++) {
		    char* stringval;
	            OCCHECK(oc_inq_dasattr(conn,attnode,k,&ocetype,&stringval));
	            nclistpush(stringvalues,(ncelem)stringval);
		}
	        OCCHECK(oc_inq_name(conn,attnode,&dodsname));
		/* Compute new special name */
		strcpy(newname,"_DODS_");
		strcat(newname,dodsname);
	        ncstat = buildattribute(newname,
				        octypetonc(ocetype),
				        stringvalues,
				        &att);				
		if(ncstat) goto done;
		att->invisible = 1;
            	nclistpush(dds->attributes,(ncelem)att);

		/* Define extra semantics associated with DODS and DODS_EXTRA attribute */
		if(strcmp(dodsname,"strlen")==0) {
		    unsigned int maxstrlen = 0;
		    if(nclistlength(stringvalues) > 0) {
		        char* stringval = (char*)nclistget(stringvalues,0);
			if(0==sscanf(stringval,"%u",&maxstrlen)) maxstrlen = 0;
		    }
		    dds->dodsspecial.maxstrlen = maxstrlen;
#ifdef DEBUG
fprintf(stderr,"%s.maxstrlen=%d\n",dds->ocname,(int)dds->dodsspecial.maxstrlen);
#endif
		} else if(strcmp(dodsname,"dimName")==0) {
		    if(nclistlength(stringvalues) > 0) {
		        char* stringval = (char*)nclistget(stringvalues,0);
		        dds->dodsspecial.dimname = nulldup(stringval);
#ifdef DEBUG
fprintf(stderr,"%s.dimname=%s\n",dds->ocname,dds->dodsspecial.dimname);
#endif
		    } else dds->dodsspecial.dimname = NULL;
		} else if(strcmp(dodsname,"Unlimited_Dimension")==0) {
		    if(nccomm->cdf.recorddimname != NULL) {
		        nclog(NCLOGWARN,"Duplicate DODS_EXTRA:Unlimited_Dimension specifications");
		    } else if(nclistlength(stringvalues) > 0) {
		        char* stringval = (char*)nclistget(stringvalues,0);
			nccomm->cdf.recorddimname = nulldup(stringval);
#ifdef DEBUG
fprintf(stderr,"%s.Unlimited_Dimension=%s\n",dds->ocname,nccomm->cdf.recorddimname);
#endif
		    }
		} /* else ignore */
	        nullfree(dodsname);
	    }
	    nullfree(dodsnodes);
	}
        nullfree(ocname);
    }

done:
    nullfree(subnodes);
    if(ocstat != OC_NOERR) ncstat = ocerrtoncerr(ocstat);
    return THROW(ncstat);
}

static int
isglobalname3(char* name)
{
    int len = strlen(name);
    int glen = strlen("global");
    char* p;
    if(len < glen) return 0;
    p = name + (len - glen);
    if(strcasecmp(p,"global") != 0)
	return 0;
    return 1;
}


static OCerror
collect_alldasnodes(OClink link, OCddsnode dasnode, NClist* alldasnodes)
{
    size_t nsubnodes,i;
    OCerror ocstat = OC_NOERR;
    nclistpush(alldasnodes,(ncelem)dasnode);
    ocstat = oc_dds_nsubnodes(link,dasnode,&nsubnodes);
    if(ocstat != OC_NOERR) goto done;
    for(i=0;i<nsubnodes;i++) {
	OCddsnode subnode;
	ocstat = oc_dds_ithsubnode(link,dasnode,i,&subnode);
        if(ocstat != OC_NOERR) goto done;
	ocstat = collect_alldasnodes(link,subnode,alldasnodes);
        if(ocstat != OC_NOERR) goto done;	
    }
    
done:
    return ocstat;
}

static OCerror
collect_leaves(OClink link, OCddsnode ddsnode, NClist* leaves)
{
    size_t nsubnodes,i;
    OCerror ocstat = OC_NOERR;
    OCtype octype;
    ocstat = oc_dds_octype(link,ddsnode,&octype);
    if(ocstat != OC_NOERR) goto done;
    if(octype == OC_Atomic) {
        nclistpush(leaves,(ncelem)ddsnode);
    } else {
        ocstat = oc_dds_nsubnodes(link,ddsnode,&nsubnodes);
        if(ocstat != OC_NOERR) goto done;
        for(i=0;i<nsubnodes;i++) {
	    OCddsnode subnode;
	    ocstat = oc_dds_ithsubnode(link,ddsnode,i,&subnode);
            if(ocstat != OC_NOERR) goto done;
	    ocstat = collect_leaves(link,subnode,leaves);
            if(ocstat != OC_NOERR) goto done;	
	}
    }
    
done:
    return ocstat;
}

static OCerror
collect_subnodes(OClink link, OCddsnode ddsnode, NClist* subnodes)
{
    size_t nsubnodes,i;
    OCerror ocstat = OC_NOERR;
    ocstat = oc_dds_nsubnodes(link,ddsnode,&nsubnodes);
    if(ocstat != OC_NOERR) goto done;
    for(i=0;i<nsubnodes;i++) {
        OCddsnode subnode;
	ocstat = oc_dds_ithsubnode(link,ddsnode,i,&subnode);
        if(ocstat != OC_NOERR) goto done;
	nclistpush(subnodes,(ncelem)subnode);
    }
    
done:
    return ocstat;
}
#endif /*0*/
