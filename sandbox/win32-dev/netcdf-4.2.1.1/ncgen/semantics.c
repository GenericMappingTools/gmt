/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/semantics.c,v 1.4 2010/05/24 19:59:58 dmh Exp $ */

#include        "includes.h"
#include        "dump.h"
#include        "offsets.h"

/* Forward*/
static void filltypecodes(void);
static void processenums(void);
static void processtypes(void);
static void processtypesizes(void);
static void processvars(void);
static void processattributes(void);
static void processspecials(void);
static void processunlimiteddims(void);

static void inferattributetype(Symbol* asym);
static void checkconsistency(void);
static void validate(void);
static int tagvlentypes(Symbol* tsym);

static Symbol* uniquetreelocate(Symbol* refsym, Symbol* root);

List* vlenconstants;  /* List<Constant*>;*/
			  /* ptr to vlen instances across all datalists*/

/* Post-parse semantic checks and actions*/
void
processsemantics(void)
{
    /* Process each type and sort by dependency order*/
    processtypes();
    /* Make sure all typecodes are set if basetype is set*/
    filltypecodes();
    /* Process each type to compute its size*/
    processtypesizes();
    /* Process each var to fill in missing fields, etc*/
    processvars();
    /* If we are not allowing certain special attributes,
       but they were defined, convert them back to attributes
    */
    processspecials();
    /* Process attributes to connect to corresponding variable*/
    processattributes();
    /* Fix up enum constant values*/
    processenums();
    /* Compute the unlimited dimension sizes */
    processunlimiteddims();
    /* check internal consistency*/
    checkconsistency();
    /* do any needed additional semantic checks*/
    validate();
}

/*
Given a reference symbol, produce the corresponding
definition symbol; return NULL if there is no definition
Note that this is somewhat complicated to conform to
various scoping rules, namely:
1. look into parent hierarchy for un-prefixed dimension names.
2. look in whole group tree for un-prefixed type names;
   search is depth first. MODIFIED 5/26/2009: Search is as follows:
   a. search parent hierarchy for matching type names.
   b. search whole tree for unique matching type name
   c. complain and require prefixed name.
3. look in the same group as ref for un-prefixed variable names.
4. ditto for group references
5. look in whole group tree for un-prefixed enum constants
*/

Symbol*
locate(Symbol* refsym)
{
    Symbol* sym = NULL;
    switch (refsym->objectclass) {
    case NC_DIM:
	if(refsym->is_prefixed) {
	    /* locate exact dimension specified*/
	    sym = lookup(NC_DIM,refsym);
	} else { /* Search for matching dimension in all parent groups*/
	    Symbol* parent = lookupgroup(refsym->prefix);/*get group for refsym*/
	    while(parent != NULL) {
		/* search this parent for matching name and type*/
		sym = lookupingroup(NC_DIM,refsym->name,parent);
		if(sym != NULL) break;
		parent = parent->container;
	    }
	}		
	break;
    case NC_TYPE:
	if(refsym->is_prefixed) {
	    /* locate exact type specified*/
	    sym = lookup(NC_TYPE,refsym);
	} else {
	    Symbol* parent;
	    int i; /* Search for matching type in all groups (except...)*/
	    /* Short circuit test for primitive types*/
	    for(i=NC_NAT;i<=NC_STRING;i++) {
		Symbol* prim = basetypefor(i);
		if(prim == NULL) continue;
	        if(strcmp(refsym->name,prim->name)==0) {
		    sym = prim;
		    break;
		}
	    }
	    if(sym == NULL) {
	        /* Added 5/26/09: look in parent hierarchy first */
	        parent = lookupgroup(refsym->prefix);/*get group for refsym*/
	        while(parent != NULL) {
		    /* search this parent for matching name and type*/
		    sym = lookupingroup(NC_TYPE,refsym->name,parent);
		    if(sym != NULL) break;
		    parent = parent->container;
		}
	    }
	    if(sym == NULL) {
	        sym = uniquetreelocate(refsym,rootgroup); /* want unique */
	    }
	}		
	break;
    case NC_VAR:
	if(refsym->is_prefixed) {
	    /* locate exact variable specified*/
	    sym = lookup(NC_VAR,refsym);
	} else {
	    Symbol* parent = lookupgroup(refsym->prefix);/*get group for refsym*/
   	    /* search this parent for matching name and type*/
	    sym = lookupingroup(NC_VAR,refsym->name,parent);
	}		
        break;
    case NC_GRP:
	if(refsym->is_prefixed) {
	    /* locate exact group specified*/
	    sym = lookup(NC_GRP,refsym);
	} else {
	    Symbol* parent = lookupgroup(refsym->prefix);/*get group for refsym*/
   	    /* search this parent for matching name and type*/
	    sym = lookupingroup(NC_GRP,refsym->name,parent);
	}		
	break;

    default: PANIC1("locate: bad refsym type: %d",refsym->objectclass);
    }
    if(debug > 1) {
	char* ncname;
	if(refsym->objectclass == NC_TYPE)
	    ncname = ncclassname(refsym->subclass);
	else
	    ncname = ncclassname(refsym->objectclass);
	fdebug("locate: %s: %s -> %s\n",
		ncname,fullname(refsym),(sym?fullname(sym):"NULL"));
    }   
    return sym;
}

/*
Search for an object in all groups using preorder depth-first traversal.
Return NULL if symbol is not unique or not found at all.
*/
static Symbol*
uniquetreelocate(Symbol* refsym, Symbol* root)
{
    int i;
    Symbol* sym = NULL;
    /* search the root for matching name and major type*/
    sym = lookupingroup(refsym->objectclass,refsym->name,root);
    if(sym == NULL) {
	for(i=0;i<listlength(root->subnodes);i++) {
	    Symbol* grp = (Symbol*)listget(root->subnodes,i);
	    if(grp->objectclass == NC_GRP && !grp->is_ref) {
		Symbol* nextsym = uniquetreelocate(refsym,grp);
		if(nextsym != NULL) {
		    if(sym != NULL) return NULL; /* not unique */	
		    sym = nextsym;
		}
	    }
	}
    }
    return sym;
}


/* 1. Do a topological sort of the types based on dependency*/
/*    so that the least dependent are first in the typdefs list*/
/* 2. fill in type typecodes*/
/* 3. mark types that use vlen*/
static void
processtypes(void)
{
    int i,j,keep,added;
    List* sorted = listnew(); /* hold re-ordered type set*/
    /* Prime the walk by capturing the set*/
    /*     of types that are dependent on primitive types*/
    /*     e.g. uint vlen(*) or primitive types*/
    for(i=0;i<listlength(typdefs);i++) {
        Symbol* sym = (Symbol*)listget(typdefs,i);
	keep=0;
	switch (sym->subclass) {
	case NC_PRIM: /*ignore pre-defined primitive types*/
	    sym->touched=1;
	    break;
	case NC_OPAQUE:
	case NC_ENUM:
	    keep=1;
	    break;
        case NC_VLEN: /* keep if its basetype is primitive*/
	    if(sym->typ.basetype->subclass == NC_PRIM) keep=1;
	    break;	    	
	case NC_COMPOUND: /* keep if all fields are primitive*/
	    keep=1; /*assume all fields are primitive*/
	    for(j=0;j<listlength(sym->subnodes);j++) {
		Symbol* field = (Symbol*)listget(sym->subnodes,j);
		ASSERT(field->subclass == NC_FIELD);
		if(field->typ.basetype->subclass != NC_PRIM) {keep=0;break;}
	    }	  
	    break;
	default: break;/* ignore*/
	}
	if(keep) {
	    sym->touched = 1;
	    listpush(sorted,(elem_t)sym);
	}
    }	
    /* 2. repeated walk to collect level i types*/
    do {
        added=0;
        for(i=0;i<listlength(typdefs);i++) {
	    Symbol* sym = (Symbol*)listget(typdefs,i);
	    if(sym->touched) continue; /* ignore already processed types*/
	    keep=0; /* assume not addable yet.*/
	    switch (sym->subclass) {
	    case NC_PRIM: 
	    case NC_OPAQUE:
	    case NC_ENUM:
		PANIC("type re-touched"); /* should never happen*/
	        break;
            case NC_VLEN: /* keep if its basetype is already processed*/
	        if(sym->typ.basetype->touched) keep=1;
	        break;	    	
	    case NC_COMPOUND: /* keep if all fields are processed*/
	        keep=1; /*assume all fields are touched*/
	        for(j=0;j<listlength(sym->subnodes);j++) {
		    Symbol* field = (Symbol*)listget(sym->subnodes,j);
		    ASSERT(field->subclass == NC_FIELD);
		    if(!field->typ.basetype->touched) {keep=1;break;}
	        }	  
	        break;
	    default: break;				
	    }
	    if(keep) {
		listpush(sorted,(elem_t)sym);
		sym->touched = 1;
		added++;
	    }	    
	}
    } while(added > 0);
    /* Any untouched type => circular dependency*/
    for(i=0;i<listlength(typdefs);i++) {
	Symbol* tsym = (Symbol*)listget(typdefs,i);
	if(tsym->touched) continue;
	semerror(tsym->lineno,"Circular type dependency for type: %s",fullname(tsym));
    }
    listfree(typdefs);
    typdefs = sorted;
    /* fill in type typecodes*/
    for(i=0;i<listlength(typdefs);i++) {
        Symbol* sym = (Symbol*)listget(typdefs,i);
	if(sym->typ.basetype != NULL && sym->typ.typecode == NC_NAT)
	    sym->typ.typecode = sym->typ.basetype->typ.typecode;
    }
    /* Identify types containing vlens */
    for(i=0;i<listlength(typdefs);i++) {
        Symbol* tsym = (Symbol*)listget(typdefs,i);
	tagvlentypes(tsym);
    }
}

/* Recursively check for vlens*/
static int
tagvlentypes(Symbol* tsym)
{
    int tagged = 0;
    int j;
    switch (tsym->subclass) {
        case NC_VLEN: 
	    tagged = 1;
	    tagvlentypes(tsym->typ.basetype);
	    break;	    	
	case NC_COMPOUND: /* keep if all fields are primitive*/
	    for(j=0;j<listlength(tsym->subnodes);j++) {
		Symbol* field = (Symbol*)listget(tsym->subnodes,j);
		ASSERT(field->subclass == NC_FIELD);
		if(tagvlentypes(field->typ.basetype)) tagged = 1;
	    }	  
	    break;
	default: break;/* ignore*/
    }
    if(tagged) tsym->typ.hasvlen = 1;
    return tagged;
}

/* Make sure all typecodes are set if basetype is set*/
static void
filltypecodes(void)
{
    Symbol* sym;
    for(sym=symlist;sym != NULL;sym = sym->next) {    
	if(sym->typ.basetype != NULL && sym->typ.typecode == NC_NAT)
	    sym->typ.typecode = sym->typ.basetype->typ.typecode;
    }
}

static void
processenums(void)
{
    int i,j;
    List* enumids = listnew();
    for(i=0;i<listlength(typdefs);i++) {
	Symbol* sym = (Symbol*)listget(typdefs,i);
	ASSERT(sym->objectclass == NC_TYPE);
	if(sym->subclass != NC_ENUM) continue;
	for(j=0;j<listlength(sym->subnodes);j++) {
	    Symbol* esym = (Symbol*)listget(sym->subnodes,j);
	    ASSERT(esym->subclass == NC_ECONST);
	    listpush(enumids,(elem_t)esym);
	}
    }	    
    /* Now walk set of enum ids to look for duplicates with same prefix*/
    for(i=0;i<listlength(enumids);i++) {
	Symbol* sym1 = (Symbol*)listget(enumids,i);
        for(j=i+1;j<listlength(enumids);j++) {
	   Symbol* sym2 = (Symbol*)listget(enumids,j);
	   if(strcmp(sym1->name,sym2->name) != 0) continue;
	   if(!prefixeq(sym1->prefix,sym2->prefix)) continue;
	   semerror(sym1->lineno,"Duplicate enumeration ids in same scope: %s",
		   fullname(sym1));	
	}
    }    
    /* Convert enum values to match enum type*/
    for(i=0;i<listlength(typdefs);i++) {
	Symbol* tsym = (Symbol*)listget(typdefs,i);
	ASSERT(tsym->objectclass == NC_TYPE);
	if(tsym->subclass != NC_ENUM) continue;
	for(j=0;j<listlength(tsym->subnodes);j++) {
	    Symbol* esym = (Symbol*)listget(tsym->subnodes,j);
	    Constant newec;
	    ASSERT(esym->subclass == NC_ECONST);
	    newec.nctype = esym->typ.typecode;
	    convert1(&esym->typ.econst,&newec);
	    esym->typ.econst = newec;
	}	
    }
}

/* Compute type sizes and compound offsets*/
void
computesize(Symbol* tsym)
{
    int i;
    int offset = 0;
    unsigned long totaldimsize;
    if(tsym->touched) return;
    tsym->touched=1;
    switch (tsym->subclass) {
        case NC_VLEN: /* actually two sizes for vlen*/
	    computesize(tsym->typ.basetype); /* first size*/
	    tsym->typ.size = ncsize(tsym->typ.typecode);
	    tsym->typ.alignment = nctypealignment(tsym->typ.typecode);
	    tsym->typ.nelems = 1; /* always a single compound datalist */
	    break;
	case NC_PRIM:
	    tsym->typ.size = ncsize(tsym->typ.typecode);
	    tsym->typ.alignment = nctypealignment(tsym->typ.typecode);
	    tsym->typ.nelems = 1;
	    break;
	case NC_OPAQUE:
	    /* size and alignment already assigned*/
	    tsym->typ.nelems = 1;
	    break;
	case NC_ENUM:
	    computesize(tsym->typ.basetype); /* first size*/
	    tsym->typ.size = tsym->typ.basetype->typ.size;
	    tsym->typ.alignment = tsym->typ.basetype->typ.alignment;
	    tsym->typ.nelems = 1;
	    break;
	case NC_COMPOUND: /* keep if all fields are primitive*/
	    /* First, compute recursively, the size and alignment of fields*/
	    for(i=0;i<listlength(tsym->subnodes);i++) {
		Symbol* field = (Symbol*)listget(tsym->subnodes,i);
		ASSERT(field->subclass == NC_FIELD);
		computesize(field);
		/* alignment of struct is same as alignment of first field*/
		if(i==0) tsym->typ.alignment = field->typ.alignment;
	    }	  
	    /* now compute the size of the compound based on*/
	    /* what user specified*/
	    offset = 0;
	    for(i=0;i<listlength(tsym->subnodes);i++) {
		Symbol* field = (Symbol*)listget(tsym->subnodes,i);
		/* only support 'c' alignment for now*/
		int alignment = field->typ.alignment;
		offset += getpadding(offset,alignment);
		field->typ.offset = offset;
		offset += field->typ.size;
	    }
	    tsym->typ.size = offset;
	    break;
        case NC_FIELD: /* Compute size assume no unlimited dimensions*/
	    if(tsym->typ.dimset.ndims > 0) {
	        computesize(tsym->typ.basetype);
	        totaldimsize = crossproduct(&tsym->typ.dimset,0,0);
	        tsym->typ.size = tsym->typ.basetype->typ.size * totaldimsize;
	        tsym->typ.alignment = tsym->typ.basetype->typ.alignment;
	        tsym->typ.nelems = 1;
	    } else {
	        tsym->typ.size = tsym->typ.basetype->typ.size;
	        tsym->typ.alignment = tsym->typ.basetype->typ.alignment;
	        tsym->typ.nelems = tsym->typ.basetype->typ.nelems;
	    }
	    break;
	default:
	    PANIC1("computesize: unexpected type class: %d",tsym->subclass);
	    break;
    }
}

void
processvars(void)
{
    int i,j;
    for(i=0;i<listlength(vardefs);i++) {
	Symbol* vsym = (Symbol*)listget(vardefs,i);
	Symbol* tsym = vsym->typ.basetype;
	/* fill in the typecode*/
	vsym->typ.typecode = tsym->typ.typecode;
	for(j=0;j<tsym->typ.dimset.ndims;j++) {
	    /* deref the dimensions*/
	    tsym->typ.dimset.dimsyms[j] = tsym->typ.dimset.dimsyms[j];
#ifndef USE_NETCDF4
	    /* UNLIMITED must only be in first place*/
	    if(tsym->typ.dimset.dimsyms[j]->dim.declsize == NC_UNLIMITED) {
		if(j != 0)
		    semerror(vsym->lineno,"Variable: %s: UNLIMITED must be in first dimension only",fullname(vsym));
	    }
#endif
	}	
    }
}

static void
processtypesizes(void)
{
    int i;
    /* use touch flag to avoid circularity*/
    for(i=0;i<listlength(typdefs);i++) {
	Symbol* tsym = (Symbol*)listget(typdefs,i);
	tsym->touched = 0;
    }
    for(i=0;i<listlength(typdefs);i++) {
	Symbol* tsym = (Symbol*)listget(typdefs,i);
	computesize(tsym); /* this will recurse*/
    }
}

static void
makespecial(int tag, Symbol* vsym, nc_type typ, Datalist* dlist)
{
    Symbol* attr = install(specialname(tag));
    attr->objectclass = NC_ATT;
    attr->data = dlist;
    if(vsym) {
	Symbol* grp = vsym->container;
	if(grp) listpush(grp->subnodes,(elem_t)attr);
	attr->container = grp;
    }
    attr->att.var = vsym;
    attr->typ.basetype = primsymbols[typ==NC_STRING?NC_CHAR:typ];
    listpush(attdefs,(elem_t)attr);
}
	
static void
processspecial1(Symbol* vsym)
{
    unsigned long flags = vsym->var.special.flags;
    int i,tag;
    Constant con;
    Datalist* dlist;
    if(flags == 0) return; /* no specials defined */
    con = nullconstant;
    if((tag=(flags & _CHUNKSIZES_FLAG))) {
	dlist = builddatalist(vsym->var.special.nchunks);
        for(i=0;i<vsym->var.special.nchunks;i++) {
            con = nullconstant;
            con.nctype = NC_INT;
            con.value.int32v = (int)vsym->var.special._ChunkSizes[i];
            dlappend(dlist,&con);
        }
        makespecial(tag,vsym,con.nctype,dlist);
    } else if((tag=(flags & _STORAGE_FLAG))) {
        con.nctype = NC_STRING;
        con.value.stringv.stringv
            = (vsym->var.special._Storage == NC_CHUNKED? "chunked"
                                                       : "contiguous");
        con.value.stringv.len = strlen(con.value.stringv.stringv);
        dlist = builddatalist(1);
        dlappend(dlist,&con);
        makespecial(tag,vsym,con.nctype,dlist);
    }
    if((tag=(flags & _FLETCHER32_FLAG))) {
        con.nctype = NC_STRING;
        con.value.stringv.stringv
            = (vsym->var.special._Fletcher32 == 1? "true"
                                                 : "false");
        con.value.stringv.len = strlen(con.value.stringv.stringv);
        dlist = builddatalist(1);
        dlappend(dlist,&con);
        makespecial(tag,vsym,con.nctype,dlist);
    }
    if((tag=(flags & _DEFLATE_FLAG))) {
        con.nctype = NC_INT;
        con.value.int32v = vsym->var.special._DeflateLevel;
        dlist = builddatalist(1);
        dlappend(dlist,&con);
        makespecial(tag,vsym,con.nctype,dlist);
    }
    if((tag=(flags & _SHUFFLE_FLAG))) {
        con.nctype = NC_STRING;
        con.value.stringv.stringv
            = (vsym->var.special._Shuffle == 1? "true"
                                              : "false");
        con.value.stringv.len = strlen(con.value.stringv.stringv);
        dlist = builddatalist(1);
        dlappend(dlist,&con);
        makespecial(tag,vsym,con.nctype,dlist);
    }
    if((tag=(flags & _ENDIAN_FLAG))) {
        con.nctype = NC_STRING;
        con.value.stringv.stringv
            = (vsym->var.special._Endianness == 1? "little"
                                                 :"big");
        con.value.stringv.len = strlen(con.value.stringv.stringv);
        dlist = builddatalist(1);
        dlappend(dlist,&con);
        makespecial(tag,vsym,con.nctype,dlist);
    }
    if((tag=(flags & _NOFILL_FLAG))) {
        con.nctype = NC_STRING;
        /* Watch out: flags is NOFILL, but we store FILL */
	if(vsym->var.special._Fill == 1) {
            con.value.stringv.stringv = "false";
	} else {
	    nofill_flag = 1;
            con.value.stringv.stringv = "true";
        }
        con.value.stringv.len = strlen(con.value.stringv.stringv);
        dlist = builddatalist(1);
        dlappend(dlist,&con);
        makespecial(tag,vsym,con.nctype,dlist);
    }
}

static void
processspecials(void)
{
    int i;
    for(i=0;i<listlength(vardefs);i++) {
	Symbol* vsym = (Symbol*)listget(vardefs,i);
	processspecial1(vsym);
    }
}

static void
processattributes(void)
{
    int i,j;
    /* process global attributes*/
    for(i=0;i<listlength(gattdefs);i++) {
	Symbol* asym = (Symbol*)listget(gattdefs,i);
	/* If the attribute has a zero length, then default it */
	if(asym->data == NULL || asym->data->length == 0) {
	    asym->data = builddatalist(1);
	    emptystringconst(asym->lineno,&asym->data->data[asym->data->length]);
	    /* force type to be NC_CHAR */
	    asym->typ.basetype = primsymbols[NC_CHAR];
	}
	if(asym->typ.basetype == NULL) inferattributetype(asym);
        /* fill in the typecode*/
	asym->typ.typecode = asym->typ.basetype->typ.typecode;
    }
    /* process per variable attributes*/
    for(i=0;i<listlength(attdefs);i++) {
	Symbol* asym = (Symbol*)listget(attdefs,i);
	/* If the attribute has a zero length, then default it */
	if(asym->data == NULL || asym->data->length == 0) {
	    asym->data = builddatalist(1);
	    emptystringconst(asym->lineno,&asym->data->data[asym->data->length]);
	    /* force type to be NC_CHAR */
	    asym->typ.basetype = primsymbols[NC_CHAR];
	}
	/* If no basetype is specified, then try to infer it;
           the exception if _Fillvalue, whose type is that of the
           containing variable.
        */
        if(strcmp(asym->name,specialname(_FILLVALUE_FLAG)) == 0) {
	    /* This is _Fillvalue */
	    asym->typ.basetype = asym->att.var->typ.basetype; /* its basetype is same as its var*/
	    /* put the datalist into the specials structure */
	    if(asym->data == NULL) {
		/* Generate a default fill value */
	        asym->data = getfiller(asym->typ.basetype);
	    }
	    asym->att.var->var.special._Fillvalue = asym->data;
	} else if(asym->typ.basetype == NULL) {
	    inferattributetype(asym);
	}
	/* fill in the typecode*/
	asym->typ.typecode = asym->typ.basetype->typ.typecode;
    }
    /* collect per-variable attributes per variable*/
    for(i=0;i<listlength(vardefs);i++) {
	Symbol* vsym = (Symbol*)listget(vardefs,i);
	List* list = listnew();
        for(j=0;j<listlength(attdefs);j++) {
	    Symbol* asym = (Symbol*)listget(attdefs,j);
	    ASSERT(asym->att.var != NULL);
	    if(asym->att.var != vsym) continue;	    
            listpush(list,(elem_t)asym);
	}
	vsym->var.attributes = list;
    }
}

/*
 Look at the first primitive value of the
 attribute's datalist to infer the type of the attribute.
 There is a potential ambiguity when that value is a string.
 Is the attribute type NC_CHAR or NC_STRING?
 The answer is we always assume it is NC_CHAR in order to
 be back compatible with ncgen.
*/

static nc_type
inferattributetype1(Datasrc* src)
{
    nc_type result = NC_NAT;
    /* Recurse down any enclosing compound markers to find first non-fill "primitive"*/
    while(result == NC_NAT && srcmore(src)) {
	if(issublist(src)) {
	    srcpush(src);
	    result = inferattributetype1(src);
	    srcpop(src);
	} else {	
	    Constant* con = srcnext(src);
	    if(isprimplus(con->nctype)) result = con->nctype;
	    /* else keep looking*/
	}
    }
    return result;
}

static void
inferattributetype(Symbol* asym)
{
    Datalist* datalist;
    Datasrc* src;
    nc_type nctype;
    ASSERT(asym->data != NULL);
    datalist = asym->data;
    if(datalist->length == 0) {
        /* Default for zero length attributes */
	asym->typ.basetype = basetypefor(NC_CHAR);
	return;
    }
    src = datalist2src(datalist);
    nctype = inferattributetype1(src);    
    freedatasrc(src);
    /* get the corresponding primitive type built-in symbol*/
    /* special case for string*/
    if(nctype == NC_STRING)
        asym->typ.basetype = basetypefor(NC_CHAR);
    else if(usingclassic) {
        /* If we are in classic mode, then restrict the inferred type
           to the classic types */
	switch (nctype) {
	case NC_UBYTE:
	    nctype = NC_SHORT;
	    break;	
	case NC_USHORT:
	case NC_UINT:
	case NC_INT64:
	case NC_UINT64:
	case NC_OPAQUE:
	case NC_ENUM:
	    nctype = NC_INT;
	    break;
	default: /* leave as is */
	    break;
	}
	asym->typ.basetype = basetypefor(nctype);
    } else
	asym->typ.basetype = basetypefor(nctype);
}

/* Find name within group structure*/
Symbol*
lookupgroup(List* prefix)
{
#ifdef USE_NETCDF4
    if(prefix == NULL || listlength(prefix) == 0)
	return rootgroup;
    else
	return (Symbol*)listtop(prefix);
#else
    return rootgroup;
#endif
}

/* Find name within given group*/
Symbol*
lookupingroup(nc_class objectclass, char* name, Symbol* grp)
{
    int i;
    if(name == NULL) return NULL;
    if(grp == NULL) grp = rootgroup;
dumpgroup(grp);
    for(i=0;i<listlength(grp->subnodes);i++) {
	Symbol* sym = (Symbol*)listget(grp->subnodes,i);
	if(sym->is_ref) continue;
	if(sym->objectclass != objectclass) continue;
	if(strcmp(sym->name,name)!=0) continue;
	return sym;
    }
    return NULL;
}

/* Find symbol within group structure*/
Symbol*
lookup(nc_class objectclass, Symbol* pattern)
{
    Symbol* grp;
    if(pattern == NULL) return NULL;
    grp = lookupgroup(pattern->prefix);
    if(grp == NULL) return NULL;
    return lookupingroup(objectclass,pattern->name,grp);
}


/* return internal size for values of specified netCDF type */
size_t
nctypesize(
     nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_BYTE: return sizeof(char);
      case NC_CHAR: return sizeof(char);
      case NC_SHORT: return sizeof(short);
      case NC_INT: return sizeof(int);
      case NC_FLOAT: return sizeof(float);
      case NC_DOUBLE: return sizeof(double);
      case NC_UBYTE: return sizeof(unsigned char);
      case NC_USHORT: return sizeof(unsigned short);
      case NC_UINT: return sizeof(unsigned int);
      case NC_INT64: return sizeof(long long);
      case NC_UINT64: return sizeof(unsigned long long);
      case NC_STRING: return sizeof(char*);
      default:
	PANIC("nctypesize: bad type code");
    }
    return 0;
}

static int
sqContains(List* seq, Symbol* sym)
{
    int i;
    if(seq == NULL) return 0;
    for(i=0;i<listlength(seq);i++) {
        Symbol* sub = (Symbol*)listget(seq,i);
	if(sub == sym) return 1;
    }
    return 0;
}

static void
checkconsistency(void)
{
    int i;
    for(i=0;i<listlength(grpdefs);i++) {
	Symbol* sym = (Symbol*)listget(grpdefs,i);
	if(sym == rootgroup) {
	    if(sym->container != NULL)
	        PANIC("rootgroup has a container");
	} else if(sym->container == NULL && sym != rootgroup)
	    PANIC1("symbol with no container: %s",sym->name);
	else if(sym->container->is_ref != 0)
	    PANIC1("group with reference container: %s",sym->name);
	else if(sym != rootgroup && !sqContains(sym->container->subnodes,sym))
	    PANIC1("group not in container: %s",sym->name);
	if(sym->subnodes == NULL)
	    PANIC1("group with null subnodes: %s",sym->name);
    }
    for(i=0;i<listlength(typdefs);i++) {
	Symbol* sym = (Symbol*)listget(typdefs,i);
        if(!sqContains(sym->container->subnodes,sym))
	    PANIC1("type not in container: %s",sym->name);
    }
    for(i=0;i<listlength(dimdefs);i++) {
	Symbol* sym = (Symbol*)listget(dimdefs,i);
        if(!sqContains(sym->container->subnodes,sym))
	    PANIC1("dimension not in container: %s",sym->name);
    }
    for(i=0;i<listlength(vardefs);i++) {
	Symbol* sym = (Symbol*)listget(vardefs,i);
        if(!sqContains(sym->container->subnodes,sym))
	    PANIC1("variable not in container: %s",sym->name);
	if(!(isprimplus(sym->typ.typecode)
	     || sqContains(typdefs,sym->typ.basetype)))
	    PANIC1("variable with undefined type: %s",sym->name);
    }
}

static void
validate(void)
{
    int i;
    for(i=0;i<listlength(vardefs);i++) {
	Symbol* sym = (Symbol*)listget(vardefs,i);
	if(sym->var.special._Fillvalue != NULL) {
	}
    }
}
static void
computeunlimitedsizes(Dimset* dimset, int dimindex, Datalist* data, int ischar)
{
    int i;
    size_t xproduct, unlimsize;
    int nextunlim,lastunlim;
    Symbol* thisunlim = dimset->dimsyms[dimindex];
    size_t length;
    
    ASSERT(thisunlim->dim.isunlimited);
    nextunlim = findunlimited(dimset,dimindex+1);
    lastunlim = (nextunlim == dimset->ndims);

    xproduct = crossproduct(dimset,dimindex+1,nextunlim);

    if(!lastunlim) {
	/* Compute candiate size of this unlimited */
        length = data->length;
	unlimsize = length / xproduct;
	if(length % xproduct != 0)
	    unlimsize++; /* => fill requires at some point */
#ifdef DEBUG2
fprintf(stderr,"unlimsize: dim=%s declsize=%lu xproduct=%lu newsize=%lu\n",
thisunlim->name,
(unsigned long)thisunlim->dim.declsize,
(unsigned long)xproduct,
(unsigned long)unlimsize);
#endif
	if(thisunlim->dim.declsize < unlimsize) /* want max length of the unlimited*/
            thisunlim->dim.declsize = unlimsize;
        /*!lastunlim => data is list of sublists, recurse on each sublist*/
	for(i=0;i<data->length;i++) {
	    Constant* con = data->data+i;
	    ASSERT(con->nctype == NC_COMPOUND);
	    computeunlimitedsizes(dimset,nextunlim,con->value.compoundv,ischar);
	}
    } else {			/* lastunlim */
	if(ischar) {
	    /* Char case requires special computations;
	       compute total number of characters */
	    length = 0;
	    for(i=0;i<data->length;i++) {
		Constant* con = &data->data[i];
		switch (con->nctype) {
	        case NC_CHAR: case NC_BYTE: case NC_UBYTE:
		    length++;
		    break;
		case NC_STRING:
		    length += con->value.stringv.len;
	            break;
		case NC_COMPOUND:
		    semwarn(datalistline(data),"Expected character constant, found {...}");
		    break;
		default:
		    semwarn(datalistline(data),"Illegal character constant: %d",con->nctype);
	        }
	    }
	} else { /* Data list should be a list of simple non-char constants */
   	    length = data->length;
	}
	unlimsize = length / xproduct;
	if(length % xproduct != 0)
	    unlimsize++; /* => fill requires at some point */
#ifdef DEBUG2
fprintf(stderr,"unlimsize: dim=%s declsize=%lu xproduct=%lu newsize=%lu\n",
thisunlim->name,
(unsigned long)thisunlim->dim.declsize,
(unsigned long)xproduct,
(unsigned long)unlimsize);
#endif
	if(thisunlim->dim.declsize < unlimsize) /* want max length of the unlimited*/
            thisunlim->dim.declsize = unlimsize;
    }
}

static void
processunlimiteddims(void)
{
    int i;
    /* Set all unlimited dims to size 0; */
    for(i=0;i<listlength(dimdefs);i++) {
	Symbol* dim = (Symbol*)listget(dimdefs,i);
	if(dim->dim.isunlimited)
	    dim->dim.declsize = 0;
    }
    /* Walk all variables */
    for(i=0;i<listlength(vardefs);i++) {
	Symbol* var = (Symbol*)listget(vardefs,i);
	int first,ischar;
	Dimset* dimset = &var->typ.dimset;
	if(dimset->ndims == 0) continue; /* ignore scalars */
	if(var->data == NULL) continue; /* no data list to walk */
	ischar = (var->typ.basetype->typ.typecode == NC_CHAR);
	first = findunlimited(dimset,0);
	if(first == dimset->ndims) continue; /* no unlimited dims */
	if(first == 0) {
	    computeunlimitedsizes(dimset,first,var->data,ischar);
	} else {
	    for(i=0;i<var->data->length;i++) {
	        Constant* con = var->data->data+i;
	        ASSERT(con->nctype == NC_COMPOUND);
	        computeunlimitedsizes(dimset,first,con->value.compoundv,ischar);
	    }
	}
    }
#ifdef DEBUG1
    /* print unlimited dim size */
    if(listlength(dimdefs) == 0)
        fprintf(stderr,"unlimited: no unlimited dimensions\n");
    else for(i=0;i<listlength(dimdefs);i++) {
	Symbol* dim = (Symbol*)listget(dimdefs,i);
	if(dim->dim.isunlimited)
	    fprintf(stderr,"unlimited: %s = %lu\n",
		    dim->name,
	            (unsigned long)dim->dim.declsize);
    }
#endif
}
