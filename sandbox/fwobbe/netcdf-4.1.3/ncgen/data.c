/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/data.c,v 1.7 2010/05/24 19:59:56 dmh Exp $ */

#include        "includes.h"
#include        "offsets.h"
#include        "dump.h"

#define XVSNPRINTF vsnprintf
/*
#define XVSNPRINTF lvsnprintf
extern int lvsnprintf(char*, size_t, const char*, va_list);
*/

Constant nullconstant;
Constant fillconstant;

Bytebuffer* codebuffer;
Bytebuffer* codetmp;
Bytebuffer* stmt;


/* Forward */

/**************************************************/
/**************************************************/

/* return 1 if the next element in the datasrc is compound*/
int
issublist(Datasrc* datasrc) {return istype(datasrc,NC_COMPOUND);}

/* return 1 if the next element in the datasrc is a string*/
int
isstring(Datasrc* datasrc) {return istype(datasrc,NC_STRING);}

/* return 1 if the next element in the datasrc is a fill value*/
int
isfillvalue(Datasrc* datasrc)
{
return srcpeek(datasrc) == NULL || istype(datasrc,NC_FILLVALUE);
}

/* return 1 if the next element in the datasrc is nc_type*/
int
istype(Datasrc* datasrc , nc_type nctype)
{
    Constant* ci = srcpeek(datasrc);
    if(ci != NULL && ci->nctype == nctype) return 1;
    return 0;
}

int
isstringable(nc_type nctype)
{
    switch (nctype) {
    case NC_CHAR: case NC_STRING:
    case NC_BYTE: case NC_UBYTE:
    case NC_FILLVALUE:
	return 1;
    default: break;
    }
    return 0;
}

/**************************************************/

void
freedatasrc(Datasrc* src)
{
    efree(src);
}

Datasrc*
allocdatasrc(void)
{
    Datasrc* src;
    src = emalloc(sizeof(Datasrc));
    src->data = NULL;
    src->index = 0;
    src->length = 0;
    src->prev = NULL;
    return src;
}

Datasrc*
datalist2src(Datalist* list)
{
    Datasrc* src;
    ASSERT(list != NULL);
    src = allocdatasrc();
    src->data = list->data;
    src->index = 0;
    src->length = list->length;
    DUMPSRC(src,"#");
    return src;
}

Datasrc*
const2src(Constant* con)
{
    Datasrc* src;
    ASSERT(con != NULL);
    src = allocdatasrc();
    src->data = con;
    src->index = 0;
    src->length = 1;
    DUMPSRC(src,"#");
    return src;
}

Constant
list2const(Datalist* list)
{
    Constant con;
    ASSERT(list != NULL);
    con.nctype = NC_COMPOUND;
    con.lineno = list->data[0].lineno;
    con.value.compoundv = list;
    return con;
}

Datalist*
const2list(Constant* con)
{
    Datalist* list;
    ASSERT(con != NULL);
    list = builddatalist(1);
    if(list != NULL) {
        dlappend(list,con);
    }
    return list;
}

Constant*
srcpeek(Datasrc* ds)
{
    if(ds == NULL) return NULL;
    if(ds->index < ds->length)
	return &ds->data[ds->index];
    if(ds->spliced)
	return srcpeek(ds->prev);
    return NULL;
}

Constant*
srcnext(Datasrc* ds)
{
    DUMPSRC(ds,"!");
    if(ds == NULL) return NULL;
    if(ds->index < ds->length)
	return &ds->data[ds->index++];
    if(ds->spliced) {
	srcpop(ds);
	return srcnext(ds);
    }
    return NULL;
}

int
srcmore(Datasrc* ds)
{
    if(ds == NULL) return 0;
    if(ds->index < ds->length) return 1;
    if(ds->spliced) return srcmore(ds->prev);
    return 0;
}

int
srcline(Datasrc* ds)
{
    int index = ds->index;
    int len = ds->length;
    /* pick closest available entry*/
    if(len == 0) return 0;
    if(index >= len) index = len-1;
    return ds->data[ds->index].lineno;
}

void
srcpush(Datasrc* src)
{
    Constant* con;
    ASSERT(src != NULL);
    con = srcnext(src);
    ASSERT(con->nctype == NC_COMPOUND);
    srcpushlist(src,con->value.compoundv);
}

void
srcpushlist(Datasrc* src, Datalist* dl)
{
    Datasrc* newsrc;
    ASSERT(src != NULL && dl != NULL);
    newsrc = allocdatasrc();
    *newsrc = *src;
    src->prev = newsrc;
    src->index = 0;
    src->data = dl->data;
    src->length = dl->length;
    DUMPSRC(src,">!");
}

void
srcpop(Datasrc* src)
{
    if(src != NULL) {
        Datasrc* prev = src->prev;
	*src = *prev;
        freedatasrc(prev);
    }
    DUMPSRC(src,"<");
}

void
srcsplice(Datasrc* ds, Datalist* list)
{
    srcpushlist(ds,list);
    ds->spliced = 1;    
}

void
srcmove(Datasrc* ds, size_t delta)
{
    srcmoveto(ds,ds->index+delta);
}

void
srcmoveto(Datasrc* ds, size_t pos)
{
    if(pos >= ds->length)
	ds->index = ds->length;
    else
        ds->index = pos;
}

void
srcsetfill(Datasrc* ds, Datalist* list)
{
    if(ds->index >= ds->length) PANIC("srcsetfill: no space");
    if(ds->data[ds->index].nctype != NC_FILLVALUE) PANIC("srcsetfill: not fill");
    ds->data[ds->index].nctype = NC_COMPOUND;
    ds->data[ds->index].value.compoundv = list;
}


/**************************************************/
#ifdef DEBUG
void
report(char* lead, Datalist* list)
{
extern void bufdump(Datalist*,Bytebuffer*);
Bytebuffer* buf = bbNew();
bufdump(list,buf);
fprintf(stderr,"\n%s::%s\n",lead,bbContents(buf));
fflush(stderr);
bbFree(buf);
}

void
report0(char* lead, Datasrc* src, int index)
{
if(debug == 0) return;
fprintf(stderr,"%s src ",lead);
if(index >=0 ) fprintf(stderr,"(%d)",index);
fprintf(stderr,":: ");
dumpdatasrc(src);
fprintf(stderr,"\n");
fflush(stderr);
}
#endif

/**************************************************/

/* Shallow constant cloning*/
Constant
cloneconstant(Constant* con)
{
    Constant newcon = *con;
    char* s;
    switch (newcon.nctype) {
    case NC_STRING:
	s = (char*)emalloc(newcon.value.stringv.len+1);
	memcpy(s,newcon.value.stringv.stringv,newcon.value.stringv.len);
	s[newcon.value.stringv.len] = '\0';
	newcon.value.stringv.stringv = s;
	break;
    case NC_OPAQUE:
	s = (char*)emalloc(newcon.value.opaquev.len+1);
	memcpy(s,newcon.value.opaquev.stringv,newcon.value.opaquev.len);
	s[newcon.value.opaquev.len] = '\0';
	newcon.value.opaquev.stringv = s;
	break;
    default: break;
    }
    return newcon;
}

/**************************************************/

Datalist*
datalistclone(Datalist* dl)
{
    int i;
    Datalist* clone = builddatalist(dl->length);
    for(i=0;i<dl->length;i++) {
	clone->data[i] = cloneconstant(dl->data+i);
    }
    return clone;
}

Datalist*
datalistconcat(Datalist* dl1, Datalist* dl2)
{
    Constant* vector;
    ASSERT(dl1 != NULL);
    if(dl2 == NULL) return dl1;
    vector = (Constant*)erealloc(dl1->data,sizeof(Constant)*(dl1->length+dl2->length));
    if(vector == NULL) return NULL;
    memcpy((void*)(vector+dl1->length),dl2->data,sizeof(Constant)*(dl2->length));
    dl1->data = vector;
    return dl1;
}

Datalist*
datalistappend(Datalist* dl, Constant* con)
{
    Constant* vector;
    ASSERT(dl != NULL);
    if(con == NULL) return dl;
    vector = (Constant*)erealloc(dl->data,sizeof(Constant)*(dl->length+1));
    if(vector == NULL) return NULL;
    vector[dl->length] = *con;
    dl->length++;
    dl->data = vector;
    return dl;
}

Datalist*
datalistreplace(Datalist* dl, unsigned int index, Constant* con)
{
    ASSERT(dl != NULL);
    ASSERT(index < dl->length);
    ASSERT(con != NULL);
    dl->data[index] = *con;
    return dl;
}

int
datalistline(Datalist* ds)
{
    if(ds == NULL || ds->length == 0) return 0;
    return ds->data[0].lineno;
}


/* Go thru a databuf of possibly nested constants
   and insert commas as needed; ideally, this
   operation should be idempotent so that
   the caller need not worry about it having already
   been applied.
*/

static char* commifyr(char* p, Bytebuffer* buf);
static char* wordstring(char* p, Bytebuffer* buf, int quote);

void
commify(Bytebuffer* buf)
{
    char* list,*p;

    if(bbLength(buf) == 0) return;
    list = bbDup(buf);
    p = list;
    bbClear(buf);
    commifyr(p,buf);
    bbNull(buf);
    efree(list);
}

static char*
commifyr(char* p, Bytebuffer* buf)
{
    int comma = 0;
    int c;
    while((c=*p++)) {
	if(c == ' ') continue;
	if(c == ',') continue;
	else if(c == '}') break;
	if(comma) bbCat(buf,", "); else comma=1;
	if(c == '{') {
	    bbAppend(buf,'{');
	    p = commifyr(p,buf);
	    bbAppend(buf,'}');
	} else if(c == '\'' || c == '\"') {
	    p = wordstring(p,buf,c);
	} else {
	    bbAppend(buf,c);
	    p=word(p,buf);
	}
    }    
    return p;
}

char*
word(char* p, Bytebuffer* buf)
{
    int c;
    while((c=*p++)) {
	if(c == '}' || c == ' ' || c == ',') break;
	if(c == '\\') {
	    bbAppend(buf,c);
	    c=*p++;
	    if(!c) break;
	}
	bbAppend(buf,(char)c);	
    }	
    p--; /* leave terminator for parent */
    return p;
}

static char*
wordstring(char* p, Bytebuffer* buf, int quote)
{
    int c;
    bbAppend(buf,quote);
    while((c=*p++)) {	    
	if(c == '\\') {
	    bbAppend(buf,c);
	    c = *p++;
	    if(c == '\0') return --p;
	} else if(c == quote) {
	    bbAppend(buf,c);
	    return p;
	}
	bbAppend(buf,c);
    }
    return p;
}


static const char zeros[] =
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
void
alignbuffer(Constant* prim, Bytebuffer* buf)
{
    int alignment,pad,offset;

    if(prim->nctype == NC_ECONST)
        alignment = nctypealignment(prim->value.enumv->typ.typecode);
    else if(usingclassic && prim->nctype == NC_STRING)
        alignment = nctypealignment(NC_CHAR);
    else if(prim->nctype == NC_CHAR)
        alignment = nctypealignment(NC_CHAR);
    else
        alignment = nctypealignment(prim->nctype);
    offset = bbLength(buf);
    pad = getpadding(offset,alignment);
    if(pad > 0) {
	bbAppendn(buf,(void*)zeros,pad);
    }
}



/*
Following routines are in support of language-oriented output
*/

void
codedump(Bytebuffer* buf)
{
   bbCatbuf(codebuffer,buf);
}

void
codepartial(const char* txt)
{
    bbCat(codebuffer,txt);
}

void
codeline(const char* line)
{
    codepartial(line);
    codepartial("\n");
}

void
codelined(int n, const char* txt)
{
    bbindent(codebuffer,n);
    bbCat(codebuffer,txt);
    codepartial("\n");
}

void
codeflush(void)
{
    if(bbLength(codebuffer) > 0) {
        bbNull(codebuffer);
        fputs(bbContents(codebuffer),stdout);
        fflush(stdout);
        bbClear(codebuffer);
    }
}

void
bbindent(Bytebuffer* buf, const int n)
{
    bbCat(buf,indented(n));
}

/* Provide an restrict snprintf that writes to an expandable buffer */
/* Simulates a simple snprintf because apparently
   the IRIX one is broken wrt return value.
   Supports only %u %d %f %s and %% specifiers
   with optional leading hh or ll.
*/

static void
vbbprintf(Bytebuffer* buf, const char* fmt, va_list argv)
{
    char tmp[128];
    const char* p;
    int c;
    int hcount;
    int lcount;

    char* text;

    for(p=fmt;(c=*p++);) {
	hcount = 0; lcount = 0;
	switch (c) {
	case '%':
retry:	    switch ((c=*p++)) {
	    case '\0': bbAppend(buf,'%'); p--; break;
	    case '%': bbAppend(buf,c); break;
	    case 'h':
		hcount++;
		while((c=*p) && (c == 'h')) {hcount++; p++;}
		if(hcount > 2) hcount = 2;
		goto retry;	        
	    case 'l':
		lcount++;
		while((c=*p) && (c == 'l')) {
		    lcount++;
		    p++;
		}
		if(lcount > 2) lcount = 2;
		goto retry;	        
	    case 'u':
		if(hcount == 2) {
   	            snprintf(tmp,sizeof(tmp),"%hhu",
			(unsigned int)va_arg(argv,unsigned int));
		} else if(hcount == 1) {
   	            snprintf(tmp,sizeof(tmp),"%hu",
			(unsigned int)va_arg(argv,unsigned int));
		} else if(lcount == 2) {
   	            snprintf(tmp,sizeof(tmp),"%llu",
			(unsigned long long)va_arg(argv,unsigned long long));
		} else if(lcount == 1) {
   	            snprintf(tmp,sizeof(tmp),"%lu",
			(unsigned long)va_arg(argv,unsigned long));
		} else {
   	            snprintf(tmp,sizeof(tmp),"%u",
			(unsigned int)va_arg(argv,unsigned int));
		}
		bbCat(buf,tmp);
		break;
	    case 'd':
		if(hcount == 2) {
   	            snprintf(tmp,sizeof(tmp),"%hhd",
			(signed int)va_arg(argv,signed int));
		} else if(hcount == 1) {
   	            snprintf(tmp,sizeof(tmp),"%hd",
			(signed int)va_arg(argv,signed int));
		} else if(lcount == 2) {
   	            snprintf(tmp,sizeof(tmp),"%lld",
			(signed long long)va_arg(argv,signed long long));
		} else if(lcount == 1) {
   	            snprintf(tmp,sizeof(tmp),"%ld",
			(signed long)va_arg(argv,signed long));
		} else {
   	            snprintf(tmp,sizeof(tmp),"%d",
			(signed int)va_arg(argv,signed int));
		}
		bbCat(buf,tmp);
		break;
            case 'f':
		if(lcount > 0) {
   	            snprintf(tmp,sizeof(tmp),"%.16g",
			(double)va_arg(argv,double));
		} else {
   	            snprintf(tmp,sizeof(tmp),"%.8g",
			(double)va_arg(argv,double));
		}
		bbCat(buf,tmp);
	        break;
	    case 's':
		text = va_arg(argv,char*);
		bbCat(buf,text);
		break;		
            default:
		PANIC1("vbbprintf: unknown specifier: %c",(char)c);
	    }
	    break;
	default: 
	    bbAppend(buf,c);
	}
    }
}

void
bbprintf(Bytebuffer* buf, const char *fmt, ...)
{
    va_list argv;
    va_start(argv,fmt);
    vbbprintf(buf,fmt,argv);
}

void
bbprintf0(Bytebuffer* buf, const char *fmt, ...)
{
    va_list argv;
    va_start(argv,fmt);
    bbClear(buf);
    vbbprintf(buf,fmt,argv);
}

void
codeprintf(const char *fmt, ...)
{
    va_list argv;
    va_start(argv,fmt);
    vbbprintf(codebuffer,fmt,argv);
}

Constant*
emptycompoundconst(int lineno, Constant* c)
{
    ASSERT(c != NULL);
    c->lineno = lineno;
    c->nctype = NC_COMPOUND;
    c->value.compoundv = builddatalist(0);
    return c;    
}

Constant*
emptystringconst(int lineno, Constant* c)
{
    ASSERT(c != NULL);
    c->lineno = lineno;
    c->nctype = NC_STRING;
    c->value.stringv.len = 0;
    c->value.stringv.stringv = NULL;
    return c;    
}
