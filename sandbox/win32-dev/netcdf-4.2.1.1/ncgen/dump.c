/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/dump.c,v 1.3 2010/05/24 19:59:57 dmh Exp $ */

#include "includes.h"
#include "dump.h"

#define DEBUGSRC

/* Forward */
static void dumpdataprim(Constant*,Bytebuffer*);

char*
indentstr(int n)
{
    static char indentline[1024];
    memset(indentline,' ',n+1);
    indentline[n+1] = '\0';
    return indentline;
}


void
dumpconstant(Constant* con, char* tag)
{
    Bytebuffer* buf = bbNew();
    Datalist* dl = builddatalist(1);
    dlappend(dl,con);
    bufdump(dl,buf);
    fprintf(stderr,"%s: %s\n",tag,bbContents(buf));
    bbFree(buf);
}

void
dumpdatalist(Datalist* list, char* tag)
{
    Bytebuffer* buf = bbNew();
    bufdump(list,buf);
    fprintf(stderr,"%s: %s\n",tag,bbContents(buf));
    bbFree(buf);
}

void
bufdump(Datalist* list, Bytebuffer* buf)
{
    int i;
    Constant* dp;
    unsigned int count;

    if(list == NULL) {
	bbCat(buf,"NULL");
	return;
    }

    count = list->length;
    for(dp=list->data,i=0;i<count;i++,dp++) {
       switch (dp->nctype) {
        case NC_COMPOUND:
	    bbCat(buf,"{");
	    bufdump(dp->value.compoundv,buf);
	    bbCat(buf,"}");
            break;
        case NC_ARRAY:
	    bbCat(buf,"[");
	    bufdump(dp->value.compoundv,buf);
	    bbCat(buf,"]");
            break;
        case NC_VLEN:
	    bbCat(buf,"{*");
	    bufdump(dp->value.compoundv,buf);
	    bbCat(buf,"}");
            break;
	default:
	    if(isprimplus(dp->nctype) || dp->nctype == NC_FILLVALUE) {
                bbCat(buf," ");
                dumpdataprim(dp,buf);
	    } else {
  	        char tmp[64];
	        sprintf(tmp,"?%d? ",dp->nctype);
   	        bbCat(buf,tmp);
            } break;
	}
    }
}


static void
dumpdataprim(Constant* ci, Bytebuffer* buf)
{
    char tmp[64];
    ASSERT(isprimplus(ci->nctype) || ci->nctype == NC_FILLVALUE);
    switch (ci->nctype) {
    case NC_CHAR: {
	bbCat(buf,"'");
	escapifychar(ci->value.charv,tmp,'\'');
	bbCat(buf,tmp);
	bbCat(buf,"'");
	} break;
    case NC_BYTE:
	sprintf(tmp,"%hhd",ci->value.int8v);
	bbCat(buf,tmp);
	break;
    case NC_SHORT:
	sprintf(tmp,"%hd",ci->value.int16v);
	bbCat(buf,tmp);
	break;
    case NC_INT:
	sprintf(tmp,"%d",ci->value.int32v);
	bbCat(buf,tmp);
	break;
    case NC_FLOAT:
	sprintf(tmp,"%g",ci->value.floatv);
	bbCat(buf,tmp);
	break;
    case NC_DOUBLE:
	sprintf(tmp,"%lg",ci->value.doublev);
	bbCat(buf,tmp);
	break;
    case NC_UBYTE:
	sprintf(tmp,"%hhu",ci->value.int8v);
	bbCat(buf,tmp);
	break;
    case NC_USHORT:
	sprintf(tmp,"%hu",ci->value.uint16v);
	bbCat(buf,tmp);
	break;
    case NC_UINT:
	sprintf(tmp,"%u",ci->value.uint32v);
	bbCat(buf,tmp);
	break;
    case NC_INT64:
	sprintf(tmp,"%lld",ci->value.int64v);
	bbCat(buf,tmp);
	break;
    case NC_UINT64:
	sprintf(tmp,"%llu",ci->value.uint64v);
	bbCat(buf,tmp);
	break;
    case NC_ECONST:
	sprintf(tmp,"%s",cname(ci->value.enumv));
	bbCat(buf,tmp);
	break;
    case NC_STRING:
	bbCat(buf,"\"");
	bbCat(buf,ci->value.stringv.stringv);
	bbCat(buf,"\"");
	break;
    case NC_OPAQUE:
	bbCat(buf,"0x");
	bbCat(buf,ci->value.opaquev.stringv);
	break;
    case NC_FILLVALUE:
	bbCat(buf,"_");
	break;
    default: PANIC1("dumpdataprim: bad type code:%d",ci->nctype);
    }
}

void
dumpgroup(Symbol* g)
{
    if(debug <= 1) return; 
    fdebug("group %s {\n",(g==NULL?"null":g->name));
    if(g != NULL && g->subnodes != NULL) {    
	int i;
	for(i=0;i<listlength(g->subnodes);i++) {
	    Symbol* sym = (Symbol*)listget(g->subnodes,i);
	    char* tname;
	    if(sym->objectclass == NC_PRIM
	       || sym->objectclass == NC_TYPE) {
		tname = nctypename(sym->subclass);
	    } else
		tname = nctypename(sym->objectclass);
	    fdebug("    %3d:  %s\t%s\t%s\n",
		i,
		sym->name,
		tname,
		(sym->is_ref?"ref":"")
		);
	}
    }
    fdebug("}\n");
}

void
dumpconstant1(Constant* con)
{
    switch (con->nctype) {
    case NC_COMPOUND: {
	Datalist* dl = con->value.compoundv;
	Bytebuffer* buf = bbNew();
	bufdump(dl,buf);
/*	fprintf(stderr,"(0x%lx){",(unsigned long)dl);*/
	fprintf(stderr,"{%s}",bbDup(buf));
	bbFree(buf);
	} break;	
    case NC_STRING:
	if(con->value.stringv.len > 0 && con->value.stringv.stringv != NULL)
	    fprintf(stderr,"\"%s\"",con->value.stringv.stringv);
	else
	    fprintf(stderr,"\"\"");
	break;
    case NC_OPAQUE:
	if(con->value.opaquev.len > 0 && con->value.opaquev.stringv != NULL)
	    fprintf(stderr,"0x%s",con->value.opaquev.stringv);
	else
	    fprintf(stderr,"0x--");
	break;
    case NC_ECONST:
	fprintf(stderr,"%s",(con->value.enumv==NULL?"?":con->value.enumv->name));
	break;
    case NC_FILLVALUE:
	fprintf(stderr,"_");
	break;
    case NC_CHAR:
	fprintf(stderr,"'%c'",con->value.charv);
	break;
    case NC_BYTE:
	fprintf(stderr,"%hhd",con->value.int8v);
	break;
    case NC_UBYTE:
	fprintf(stderr,"%hhu",con->value.uint8v);
	break;
    case NC_SHORT:
	fprintf(stderr,"%hd",con->value.int16v);
	break;
    case NC_USHORT:
	fprintf(stderr,"%hu",con->value.uint16v);
	break;
    case NC_INT:
	fprintf(stderr,"%d",con->value.int32v);
	break;
    case NC_UINT:
	fprintf(stderr,"%u",con->value.uint32v);
	break;
    case NC_INT64:
	fprintf(stderr,"%lld",con->value.int64v);
	break;
    case NC_UINT64:
	fprintf(stderr,"%llu",con->value.uint64v);
	break;
    case NC_FLOAT:
	fprintf(stderr,"%g",con->value.floatv);
	break;
    case NC_DOUBLE:
	fprintf(stderr,"%g",con->value.doublev);
	break;
    default:
	fprintf(stderr,"<unknown>");
	break;
    }
    fflush(stderr);
}

#define MAXELEM 8
#define MAXDEPTH 4

void
dumpsrc0(Datasrc* src,char* tag)
{
    int i, count, index, depth;
    depth = MAXDEPTH;
    count = src->length;
    index = src->index;
    if(count > MAXELEM) count = MAXELEM;
    if(index > count) index = count;
    fprintf(stderr,"%s:: ",(tag?tag:""));
    do {
        fprintf(stderr,"[%d/%d]",src->index,src->length);
	for(i=0;i<index;i++) {
	    fprintf(stderr," ");
	    dumpconstant1(&src->data[i]);
	}
	fprintf(stderr,"^");
	for(i=index;i<count;i++) {
	    fprintf(stderr," ");
	    dumpconstant1(&src->data[i]);
	}
        if(count < src->length) fprintf(stderr,"...");
	fprintf(stderr," | ");	
        src = src->prev;
    } while(src != NULL && depth > 0);
    if(src != NULL) fprintf(stderr,"---");
    fprintf(stderr,"\n");
    fflush(stderr);
}

void
dumpsrc(Datasrc* src,char* tag)
{
#ifndef DEBUGSRC
    if(debug == 0) return;
#endif
    dumpsrc0(src,tag);
}
