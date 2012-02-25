/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapdump.c,v 1.21 2010/05/26 21:43:31 dmh Exp $
 *********************************************************************/
#include "config.h"
#ifdef USE_PARALLEL
#include "netcdf_par.h"
#endif
#include "ncdap3.h"
#include "dapdump.h"

#define CHECK(n) if((n) != NC_NOERR) {return (n);} else {}

int
dumpmetadata(int ncid, NChdr** hdrp)
{
    int stat,i,j,k;
    NChdr* hdr = (NChdr*)calloc(1,sizeof(NChdr));
    MEMCHECK(hdr,NC_ENOMEM);
    hdr->ncid = ncid;
    hdr->content = ncbytesnew();
    if(hdrp) *hdrp = hdr;

    stat = nc_inq(hdr->ncid,
		  &hdr->ndims,
		  &hdr->nvars,
		  &hdr->ngatts,
		  &hdr->unlimid);
    CHECK(stat);
    if(ncdap3debug > 0) {
        fprintf(stdout,"ncid=%d ngatts=%d ndims=%d nvars=%d unlimid=%d\n",
		hdr->ncid,hdr->ngatts,hdr->ndims,hdr->nvars,hdr->unlimid);
    }
    hdr->gatts = (NCattribute*)calloc(1,hdr->ngatts*sizeof(NCattribute));
    MEMCHECK(hdr->gatts,NC_ENOMEM);
    if(hdr->ngatts > 0)
	fprintf(stdout,"global attributes:\n");
    for(i=0;i<hdr->ngatts;i++) {
	NCattribute* att = &hdr->gatts[i];
        char attname[NC_MAX_NAME];
	nc_type nctype;
	size_t typesize;
        size_t nvalues;

        stat = nc_inq_attname(hdr->ncid,NC_GLOBAL,i,attname);
        CHECK(stat);
	att->name = nulldup(attname);
	stat = nc_inq_att(hdr->ncid,NC_GLOBAL,att->name,&nctype,&nvalues);
        CHECK(stat);
	att->etype = nctypetodap(nctype);
 	typesize = nctypesizeof(att->etype);
	fprintf(stdout,"\t[%d]: name=%s type=%s values(%lu)=",
			i,att->name,nctypetostring(octypetonc(att->etype)),
                        (unsigned long)nvalues);
	if(nctype == NC_CHAR) {
	    size_t len = typesize*nvalues;
	    char* values = (char*)malloc(len+1);/* for null terminate*/
	    MEMCHECK(values,NC_ENOMEM);
	    stat = nc_get_att(hdr->ncid,NC_GLOBAL,att->name,values);
            CHECK(stat);
	    values[len] = '\0';
	    fprintf(stdout," '%s'",values);
	} else {
	    size_t len = typesize*nvalues;
	    char* values = (char*)malloc(len);
	    MEMCHECK(values,NC_ENOMEM);
	    stat = nc_get_att(hdr->ncid,NC_GLOBAL,att->name,values);
            CHECK(stat);
	    for(k=0;k<nvalues;k++) {
		fprintf(stdout," ");
		dumpdata1(octypetonc(att->etype),k,values);
	    }
	}
	fprintf(stdout,"\n");
    }

    hdr->dims = (Dim*)malloc(hdr->ndims*sizeof(Dim));
    MEMCHECK(hdr->dims,NC_ENOMEM);
    for(i=0;i<hdr->ndims;i++) {
	hdr->dims[i].dimid = i;
        stat = nc_inq_dim(hdr->ncid,
	                  hdr->dims[i].dimid,
	                  hdr->dims[i].name,
	                  &hdr->dims[i].size);
        CHECK(stat);
	fprintf(stdout,"dim[%d]: name=%s size=%lu\n",
		i,hdr->dims[i].name,(unsigned long)hdr->dims[i].size);
    }    
    hdr->vars = (Var*)malloc(hdr->nvars*sizeof(Var));
    MEMCHECK(hdr->vars,NC_ENOMEM);
    for(i=0;i<hdr->nvars;i++) {
	Var* var = &hdr->vars[i];
	nc_type nctype;
	var->varid = i;
        stat = nc_inq_var(hdr->ncid,
	                  var->varid,
	                  var->name,
			  &nctype,
			  &var->ndims,
			  var->dimids,
	                  &var->natts);
        CHECK(stat);
	var->nctype = (nctype);
	fprintf(stdout,"var[%d]: name=%s type=%s |dims|=%d",
		i,
		var->name,
		nctypetostring(var->nctype),
		var->ndims);
	fprintf(stdout," dims={");
	for(j=0;j<var->ndims;j++) {
	    fprintf(stdout," %d",var->dimids[j]);
	}
	fprintf(stdout,"}\n");
	var->atts = (NCattribute*)malloc(var->natts*sizeof(NCattribute));
        MEMCHECK(var->atts,NC_ENOMEM);
        for(j=0;j<var->natts;j++) {
	    NCattribute* att = &var->atts[j];
	    char attname[NC_MAX_NAME];
	    size_t typesize;
	    char* values;
	    nc_type nctype;
	    size_t nvalues;
            stat = nc_inq_attname(hdr->ncid,var->varid,j,attname);
	    CHECK(stat);
	    att->name = nulldup(attname);
	    stat = nc_inq_att(hdr->ncid,var->varid,att->name,&nctype,&nvalues);
	    CHECK(stat);
	    att->etype = nctypetodap(nctype);
	    typesize = nctypesizeof(att->etype);
	    values = (char*)malloc(typesize*nvalues);
	    MEMCHECK(values,NC_ENOMEM);
	    stat = nc_get_att(hdr->ncid,var->varid,att->name,values);
            CHECK(stat);
	    fprintf(stdout,"\tattr[%d]: name=%s type=%s values(%lu)=",
			j,att->name,nctypetostring(octypetonc(att->etype)),(unsigned long)nvalues);
	    for(k=0;k<nvalues;k++) {
		fprintf(stdout," ");
		dumpdata1(octypetonc(att->etype),k,values);
	    }
	    fprintf(stdout,"\n");
	}
    }    
    fflush(stdout);
    return NC_NOERR;
}

void
dumpdata1(nc_type nctype, size_t index, char* data)
{
    switch (nctype) {
    case NC_CHAR:
	fprintf(stdout,"'%c' %hhd",data[index],data[index]);
	break;
    case NC_BYTE:
	fprintf(stdout,"%hdB",((signed char*)data)[index]);
	break;
    case NC_UBYTE:
	fprintf(stdout,"%huB",((unsigned char*)data)[index]);
	break;
    case NC_SHORT:
	fprintf(stdout,"%hdS",((short*)data)[index]);
	break;
    case NC_USHORT:
	fprintf(stdout,"%hdUS",((unsigned short*)data)[index]);
	break;
    case NC_INT:
	fprintf(stdout,"%d",((int*)data)[index]);
	break;
    case NC_UINT:
	fprintf(stdout,"%uU",((unsigned int*)data)[index]);
	break;
    case NC_FLOAT:
	fprintf(stdout,"%#gF",((float*)data)[index]);
	break;
    case NC_DOUBLE:
	fprintf(stdout,"%#gD",((double*)data)[index]);
	break;
    case NC_STRING:
	fprintf(stdout,"\"%s\"",((char**)data)[index]);
	break;
    default:
	fprintf(stdout,"Unknown type: %i",nctype);
	break;
    }
    fflush(stdout);
}

/* Following should be kept consistent with
   the makeXXXstring3 routines in constraints3.c
*/

/* Convert an NCprojection instance into a string
   that can be used with the url
*/
char*
dumpprojections(NClist* projections)
{
    return dcelisttostring(projections,",");
}

char*
dumpprojection(DCEprojection* proj)
{
    return dcetostring((DCEnode*)proj);
}

char*
dumpselections(NClist* selections)
{
    return dcelisttostring(selections,"&");
}

char*
dumpselection(DCEselection* sel)
{
    return dcetostring((DCEnode*)sel);
}

char*
dumpconstraint(DCEconstraint* con)
{
    return dcetostring((DCEnode*)con);
}

char*
dumpsegments(NClist* segments)
{
    return dcelisttostring(segments,".");
}

char*
dumppath(CDFnode* leaf)
{
    NClist* path = nclistnew();
    NCbytes* buf = ncbytesnew();
    char* result;
    int i;

    if(leaf == NULL) return nulldup("");
    collectnodepath3(leaf,path,!WITHDATASET);
    for(i=0;i<nclistlength(path);i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(i > 0) ncbytescat(buf,".");
	ncbytescat(buf,node->name);
    }
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    nclistfree(path);
    return result;
}

static void
dumpindent(int indent, NCbytes* buf)
{
    static char* indentstr = "  ";
    int i;
    for(i=0;i<indent;i++) ncbytescat(buf,indentstr);
}

static void dumptreer(CDFnode* root, NCbytes* buf, int indent, int visible);

static void
dumptreer1(CDFnode* root, NCbytes* buf, int indent, char* tag, int visible)
{
    int i;
    dumpindent(indent,buf);
    ncbytescat(buf,tag);
    ncbytescat(buf," {\n");
    for(i=0;i<nclistlength(root->subnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(root->subnodes,i);
	if(visible && !root->visible) continue;
	if(root->nctype == NC_Grid) {
	    if(i==0) {
		dumpindent(indent+1,buf);
	        ncbytescat(buf,"Array:\n");
	    } else if(i==1) {
		dumpindent(indent+1,buf);
	        ncbytescat(buf,"Maps:\n");
	    }
	    dumptreer(node,buf,indent+2,visible);
	} else {
	    dumptreer(node,buf,indent+1,visible);
	}
    }
    dumpindent(indent,buf);
    ncbytescat(buf,"} ");
    ncbytescat(buf,root->name);
}

static void
dumptreer(CDFnode* root, NCbytes* buf, int indent, int visible)
{
    int i;
    char* primtype = NULL;
    if(visible && !root->visible) return;
    switch (root->nctype) {
    case NC_Dataset:
	dumptreer1(root,buf,indent,"Dataset",visible);
	break;
    case NC_Sequence:
	dumptreer1(root,buf,indent,"Sequence",visible);
	break;
    case NC_Structure:
	dumptreer1(root,buf,indent,"Structure",visible);
	break;
    case NC_Grid:
	dumptreer1(root,buf,indent,"Grid",visible);
	break;
    case NC_Primitive:
	switch (root->etype) {
	case NC_BYTE: primtype = "byte"; break;
	case NC_CHAR: primtype = "char"; break;
	case NC_SHORT: primtype = "short"; break;
	case NC_INT: primtype = "int"; break;
	case NC_FLOAT: primtype = "float"; break;
	case NC_DOUBLE: primtype = "double"; break;
	case NC_UBYTE: primtype = "ubyte"; break;
	case NC_USHORT: primtype = "ushort"; break;
	case NC_UINT: primtype = "uint"; break;
	case NC_INT64: primtype = "int64"; break;
	case NC_UINT64: primtype = "uint64"; break;
	case NC_STRING: primtype = "string"; break;
	default: break;
	}
	dumpindent(indent,buf);
	ncbytescat(buf,primtype);
	ncbytescat(buf," ");
	ncbytescat(buf,root->name);
	break;
    default: break;    
    }

    if(nclistlength(root->array.dimensions) > 0) {
	for(i=0;i<nclistlength(root->array.dimensions);i++) {
	    CDFnode* dim = (CDFnode*)nclistget(root->array.dimensions,i);
	    char tmp[64];
	    ncbytescat(buf,"[");
	    if(dim->name != NULL) {
		ncbytescat(buf,dim->name);
	        ncbytescat(buf,"=");
	    }
	    snprintf(tmp,sizeof(tmp),"%lu",(unsigned long)dim->dim.declsize);
	    ncbytescat(buf,tmp);
	    ncbytescat(buf,"]");
	}
    }
    ncbytescat(buf,";\n");
}

char*
dumptree(CDFnode* root)
{
    NCbytes* buf = ncbytesnew();
    char* result;
    dumptreer(root,buf,0,0);
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

char*
dumpvisible(CDFnode* root)
{
    NCbytes* buf = ncbytesnew();
    char* result;
    dumptreer(root,buf,0,1);
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

/* Provide detailed data on a CDFnode */
char*
dumpnode(CDFnode* node)
{
    NCbytes* buf = ncbytesnew();
    char* result;
    int i;
    char* nctype = NULL;
    char* primtype = NULL;
    char tmp[1024];

    switch (node->nctype) {
    case NC_Dataset: nctype = "Dataset"; break;
    case NC_Sequence: nctype = "Sequence"; break;
    case NC_Structure: nctype = "Structure"; break;
    case NC_Grid: nctype = "Grid"; break;
    case NC_Primitive:
	switch (node->etype) {
	case NC_BYTE: primtype = "byte"; break;
	case NC_CHAR: primtype = "char"; break;
	case NC_SHORT: primtype = "short"; break;
	case NC_INT: primtype = "int"; break;
	case NC_FLOAT: primtype = "float"; break;
	case NC_DOUBLE: primtype = "double"; break;
	case NC_UBYTE: primtype = "ubyte"; break;
	case NC_USHORT: primtype = "ushort"; break;
	case NC_UINT: primtype = "uint"; break;
	case NC_INT64: primtype = "int64"; break;
	case NC_UINT64: primtype = "uint64"; break;
	case NC_STRING: primtype = "string"; break;
	default: break;
	}
	break;
    default: break;    
    }
    snprintf(tmp,sizeof(tmp),"%s %s {\n",
		(nctype?nctype:primtype),node->name);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"dds=%lx\n",(unsigned long)node->dds);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"container=%s\n",
		(node->container?node->container->name:"null"));
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"root=%s\n",
		(node->root?node->root->name:"null"));
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"ncbasename=%s\n",node->ncbasename);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"ncfullname=%s\n",node->ncfullname);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"|subnodes|=%d\n",nclistlength(node->subnodes));
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"externaltype=%d\n",node->externaltype);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"ncid=%d\n",node->ncid);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"maxstringlength=%ld\n",node->maxstringlength);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"sequencelimit=%ld\n",node->sequencelimit);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"usesequence=%d\n",node->usesequence);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"elided=%d\n",node->elided);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"visible=%d\n",node->visible);
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"attachment=%s\n",
		(node->attachment?node->attachment->name:"null"));
    ncbytescat(buf,tmp);
    snprintf(tmp,sizeof(tmp),"rank=%u\n",nclistlength(node->array.dimensions));
    ncbytescat(buf,tmp);
    for(i=0;i<nclistlength(node->array.dimensions);i++) {
	CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,i);
        snprintf(tmp,sizeof(tmp),"dims[%d]={\n",i);
        ncbytescat(buf,tmp);
	snprintf(tmp,sizeof(tmp),"    name=%s\n",dim->name);
        ncbytescat(buf,tmp);
	snprintf(tmp,sizeof(tmp),"    dimflags=%u\n",
			(unsigned int)dim->dim.dimflags);
        ncbytescat(buf,tmp);
	snprintf(tmp,sizeof(tmp),"    declsize=%lu\n",
		    (unsigned long)dim->dim.declsize);
        ncbytescat(buf,tmp);
	snprintf(tmp,sizeof(tmp),"    declsize0=%lu\n",
		    (unsigned long)dim->dim.declsize0);
        ncbytescat(buf,tmp);
        snprintf(tmp,sizeof(tmp),"    }\n");
        ncbytescat(buf,tmp);
    }

    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

char*
dumpalign(NCalignment* ncalign)
{
    char* result;
    char tmp[1024];
    if(ncalign == NULL)
	result = nulldup("NCalignment{size=-- alignment=-- offset=--}");
    else {
        snprintf(tmp,sizeof(tmp),"NCalignment{size=%lu alignment=%lu offset=%lu}",
		 ncalign->size,ncalign->alignment,ncalign->offset);
        result = nulldup(tmp);
    }
    return result;
}

char*
dumpcachenode(NCcachenode* node)
{
    char* result = NULL;
    char tmp[8192];
    int i;
    NCbytes* buf;

    if(node == NULL) return strdup("cachenode{null}");
    buf = ncbytesnew();
    snprintf(tmp,sizeof(tmp),"cachenode%s(%lx){size=%lu; constraint=%s; vars=",
		node->prefetch?"*":"",
		(unsigned long)node,
		(unsigned long)node->xdrsize,
		buildconstraintstring3(node->constraint));
    ncbytescat(buf,tmp);
    if(nclistlength(node->vars)==0)
	ncbytescat(buf,"null");
    else for(i=0;i<nclistlength(node->vars);i++) {
	CDFnode* var = (CDFnode*)nclistget(node->vars,i);
	if(i > 0) ncbytescat(buf,",");
	ncbytescat(buf,makesimplepathstring3(var));
    }
    ncbytescat(buf,"}");
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

char*
dumpcache(NCcache* cache)
{
    char* result = NULL;
    char tmp[8192];
    int i;
    NCbytes* buf;

    if(cache == NULL) return strdup("cache{null}");
    buf = ncbytesnew();
    snprintf(tmp,sizeof(tmp),"cache{limit=%lu; size=%lu;\n",
		(unsigned long)cache->cachelimit,
		(unsigned long)cache->cachesize);
    ncbytescat(buf,tmp);
    if(cache->prefetch) {
	ncbytescat(buf,"\tprefetch=");
	ncbytescat(buf,dumpcachenode(cache->prefetch));
	ncbytescat(buf,"\n");
    }
    if(nclistlength(cache->nodes) > 0) {
        for(i=0;i<nclistlength(cache->nodes);i++) {
   	    NCcachenode* node = (NCcachenode*)nclistget(cache->nodes,i);
	    ncbytescat(buf,"\t");
	    ncbytescat(buf,dumpcachenode(node));
	    ncbytescat(buf,"\n");
	}
    }
    ncbytescat(buf,"}");
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

/* This should be consistent with makeslicestring3 in constraints3.c */
char*
dumpslice(DCEslice* slice)
{
    return dcetostring((DCEnode*)slice);
}

char*
dumpslices(DCEslice* slice, unsigned int rank)
{
    int i;
    NCbytes* buf;
    char* result = NULL;

    buf = ncbytesnew();
    for(i=0;i<rank;i++,slice++) {
        ncbytescat(buf,dumpslice(slice));
    }
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}
