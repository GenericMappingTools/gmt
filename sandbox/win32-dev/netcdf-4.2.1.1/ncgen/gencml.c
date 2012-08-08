/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/gencml.c,v 1.5 2010/04/04 19:39:46 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include <ctype.h>	/* for isprint() */

#ifdef ENABLE_CML

#undef TRACE

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward */
/*
static const char* ncstype(nc_type);
static const char* nctype(nc_type);
static const char* ncctype(nc_type);
static const char* groupncid(Symbol*);
static const char* typencid(Symbol*);
static const char* varncid(Symbol*);
static const char* dimncid(Symbol*);
*/

static void gencml_group(Symbol* group);

static char* ncxtype(nc_type type);

#ifdef USE_NETCDF4
static void definextype(Symbol*);
static char* xprefixed(List* prefix, char* suffix, char* separator);
static void definespecialattributes(Symbol* vsym);
#endif

static void defineattribute(Symbol* asym);
static void definevardata(Symbol*);

#ifdef USE_NETCDF4
static void xprint(Bytebuffer* buf);
#endif
static void xpartial(char* line);
static void xline(char* line);
static void xflush(void);


/*
Global Bytebuffer into which to store the C code output;
periodically dumped to file by xflush().
*/
Bytebuffer* xcode;

/*
 * Generate C code for creating netCDF from in-memory structure.
 */
void
gen_nccml(const char *filename)
{
    xcode = bbNew();
    bbSetalloc(xcode,C_MAX_STMT);

    /* Dump XML header */
    xline("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    xline("<netcdf xmlns=\"http://www.unidata.ucar.edu/namespaces/netcdf/ncml-2.2\">"); 

    xline("");
    xline("<explicit/>");
    xline("");

    xflush();

    /* Recursively dump all groups via the rootgroup */
    gencml_group(rootgroup);
}

void
cl_cml(void)
{
    xline("</netcdf>");
    xflush();
}

static void
gencml_group(Symbol* group)
{
    int idim, ivar, iatt;
    int ndims, nvars, natts, ngatts, ngrps, ntyps;

#ifdef USE_NETCDF4
    int igrp,ityp;
#endif

    ndims = listlength(dimdefs);
    nvars = listlength(vardefs);
    natts = listlength(attdefs);
    ngatts = listlength(gattdefs);
    ngrps = listlength(grpdefs);
    ntyps = listlength(typdefs);

#ifdef USE_NETCDF4
    if(group != rootgroup) {
	xline("");
	xpartial("<group name=");
	xpartial("\"");
	xpartial(xname(group));
	xline("\" >");
    }
#endif

#ifdef USE_NETCDF4
    /* Construct code to define types*/
    if(ntyps > 0) {
	xline("<!-- define types -->");
	xline("<types>");
	for(ityp = 0; ityp < ntyps; ityp++) {
	    Symbol* tsym = (Symbol*)listget(typdefs,ityp);
	    xline("");
	    if(tsym->subclass == NC_PRIM
	       || tsym->subclass == NC_ARRAY) continue;/*no need to do these*/
	    definextype(tsym);
	}
	xline("");
	xline("</types>");
    }
    xflush();
#endif

    /* define dimensions from info in dims array */
    if (ndims > 0) {
	xline("");
	xline("<!-- define dimensions -->");
        for(idim = 0; idim < ndims; idim++) {
            Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    unsigned long declsize = dsym->dim.size;
	    char* utag = "";
	    if(declsize == 0)
		utag = " isUnlimited=\"true\"";
    	    nprintf(stmt,sizeof(stmt),
		"<dimension name=\"%s\" length=\"%lu\"%s />",
                xname(dsym),declsize,utag);
	    xline(stmt);
       }
    }
    xflush();

    /* Define the global attributes*/
    if(ngatts > 0) {
	xline("");
	xline("<!-- Define global attributes -->");
	for(iatt = 0; iatt < ngatts; iatt++) {
	    Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
	    if(gasym->att.var == NULL) /* => global */
		defineattribute(gasym);	    
	}
	xline("");
    }
    xflush();
    
    /* define variables from info in vars array */
    if (nvars > 0) {
	xline("");
	xline("<!-- define variables -->");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    Dimset* dimset = &vsym->typ.dimset;
	    if(ivar > 0) xline("");
	    nprintf(stmt,sizeof(stmt),"<variable name=\"%s\"",xname(vsym));
  	    xpartial(stmt);
	    if(dimset->ndims > 0) {
		xpartial(" shape=\"");
	        for(idim = 0; idim < dimset->ndims; idim++) {
		    Symbol* dsym = dimset->dimsyms[idim];
		    if(idim > 0) xpartial(" ");
		    xpartial(xname(dsym));
		}
		xpartial("\"");
	    }
	    xpartial(" type=\"");
	    xpartial(xname(vsym->typ.basetype));	    
	    xline(" >");
#ifdef USE_NETCDF4
	    definespecialattributes(vsym);
#endif /*USE_NETCDF4*/
            if(natts > 0) {
	        for(iatt = 0; iatt < natts; iatt++) {
	            Symbol* asym = (Symbol*)listget(attdefs,iatt);
		    if(asym->att.var == vsym) defineattribute(asym);
		}
	    }
	    if(vsym->data != NULL) definevardata(vsym);
  	    xline("</variable>");
	}
    }
    xflush();

#ifdef USE_NETCDF4
    /* Recursively define subgroups */
    for(igrp=0;igrp<ngrps;igrp++) {
	Symbol* gsym = (Symbol*)listget(grpdefs,igrp);
	if(gsym->container != group) continue;
	gencml_group(gsym);
    }
#endif    

#ifdef USE_NETCDF4
    if(group != rootgroup) {
	xline("</group>");
    }
#endif
}

#ifdef USE_NETCDF4
static void
definespecialattributes(Symbol* vsym)
{
    Specialdata* special = &vsym->var.special;
    if(usingclassic) return;
    if(special->flags & _STORAGE_FLAG) {
        int storage = special->_Storage;
        size_t* chunks = special->_ChunkSizes;
        nprintf(stmt,sizeof(stmt),"<attribute name=\"_Chunksizes\" storage=\"%s\"",
                (storage == NC_CONTIGUOUS?"contiguous":"chunked"));
        xpartial(stmt);                 
        if(special->nchunks != 0 && chunks != NULL) {
	    int i;
            xpartial(" value=\"");
            for(i=0;i<special->nchunks;i++) {
		if(i > 0) xpartial(" ");
                nprintf(stmt,sizeof(stmt),"%ld",special->_ChunkSizes[i]);
		xpartial(stmt);
            }
            xpartial("\"");
        }
        xline(" />");
    }
    if(special->flags & _FLETCHER32_FLAG) {
        nprintf(stmt,sizeof(stmt),
                "<attribute name=\"_Fletcher32\" value=\"%s\" />",
		(special->_Fletcher32?"true":"false"));
        xline(stmt);
    }
    if(special->flags & (_DEFLATE_FLAG | _SHUFFLE_FLAG)) {
        nprintf(stmt,sizeof(stmt),
                "<attribute name=\"_DeflateLevel\" shuffle=\"%s\" deflate=\"%s\" value=\"%d\" />",
		(special->_Shuffle==1?"shuffle":"noshuffle"),
		(special->_DeflateLevel >= 0?"true":"false"),
		(special->_DeflateLevel >= 0?special->_DeflateLevel:0));
        xline(stmt);
    }   
    if(special->flags & _ENDIAN_FLAG) {
        nprintf(stmt,sizeof(stmt),
                "<attribute name=\"_Endianness\" value=\"%s\" />",
		(special->_Endianness == NC_ENDIAN_LITTLE?"little":"big"));
        xline(stmt);
    }   
    if(special->flags & _NOFILL_FLAG) {
        nprintf(stmt,sizeof(stmt),
                "<attribute name=\"_Fill\" value=\"%s\" />",
		(special->_Fill?"fill":"nofill"));
        xline(stmt);
    }   
}

static void
definextype(Symbol* tsym)
{
    int i;

    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_PRIM: break; /* these are already taken care of*/
    case NC_OPAQUE:
	nprintf(stmt,sizeof(stmt),"<opaque name=\"%s\" size=\"%lu\" />",
		xname(tsym),(unsigned long)tsym->typ.size);
	xline(stmt);
	break;
    case NC_ENUM:
	nprintf(stmt,sizeof(stmt),"<enum name=\"%s\" type=\"%s\" >",
		xname(tsym),xtypename(tsym->typ.basetype));
	xline(stmt);
	stmt[0] = 0;
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* econst = (Symbol*)listget(tsym->subnodes,i);
	    ASSERT(econst->subclass == NC_ECONST);
	    nprintf(stmt,sizeof(stmt),"<enumconst name=\"%s\" value=\"%s\" />",
		    xname(econst),
		    xconst(&econst->typ.econst));
	    xline(stmt);
	}
	xline("</enum>");
	break;
    case NC_VLEN:
	nprintf(stmt,sizeof(stmt),"<vlen name=\"%s\" type=\"%s\" />",
		xname(tsym),xtypename(tsym));
	xline(stmt);
	break;
    case NC_COMPOUND:
	nprintf(stmt,sizeof(stmt),"<compound name=\"%s\" >",
		xname(tsym));
	xline(stmt);
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* efield = (Symbol*)listget(tsym->subnodes,i);
	    ASSERT(efield->subclass == NC_FIELD);
	    nprintf(stmt,sizeof(stmt),"<field name=\"%s\"",xname(efield));
	    xpartial(stmt);
	    /* compute any dimension specification*/
	    if(efield->typ.dimset.ndims > 0) {
		Bytebuffer* dimbuf = bbNew();
		int j;
		bbCat(dimbuf," shape=\"");
	        for(j=0;j<efield->typ.dimset.ndims;j++) {
		    char tmp[32];
		    Symbol* dim;
		    if(j > 0) bbCat(dimbuf," ");
		    dim = efield->typ.dimset.dimsyms[j];
		    ASSERT(dim->dim.isconstant);
		    sprintf(tmp,"%lu",(unsigned long)dim->dim.size);
		    bbCat(dimbuf,tmp);
		}
		bbCat(dimbuf,"\"");
		xprint(dimbuf);
		bbFree(dimbuf);
	    }
	    nprintf(stmt,sizeof(stmt)," type=\"%s\" />",
		    xtypename(efield->typ.basetype));
	    xline(stmt);
	}
	xline("</compound>");
	xflush();
	break;

    case NC_ARRAY:
	/* ignore: this will be handled by def_var*/
	break;

    default: panic("definextype: unexpected type subclass: %d",tsym->subclass);
    }
}

#endif /*USE_NETCDF4*/

static void
defineattribute(Symbol* asym)
{
    unsigned long len;
    Datalist* list;
    Symbol* basetype = asym->typ.basetype;
    Bytebuffer* code = NULL; /* capture other decls*/

    list = asym->data;
    if(list == NULL) PANIC("empty attribute list");
    len = asym->att.count;
    if(len == 0) PANIC("empty attribute list");

    code = bbNew();

    gencml_attrdata(asym,code);	

    /* Handle NC_CHAR specially */
    if(basetype->typ.typecode == NC_CHAR) {
	/* revise the length count */
	len = bbLength(code);
	if(len == 0) {bbAppend(code,'\0'); len++;}
	xquotestring(code);
    }

    if(isprimplus(basetype->typ.typecode)) {
        nprintf(stmt,sizeof(stmt),"<attribute name=\"%s\" type=\"%s\" value=\"%s\" />",
		xname(asym),xtypename(asym->typ.basetype),
		bbContents(code));
        xline(stmt);
    } else {
        nprintf(stmt,sizeof(stmt),"<attribute name=\"%s\" type=\"%s\" >",
		xname(asym),xtypename(asym->typ.basetype));
        xline(stmt);
	xpartial("<values>");
	xpartial(bbContents(code));
	xline("</values>");
        xline("</attribute>");
    }    
    xflush();
}

/* Note: no closure is used for CML */
static void
definevardata(Symbol* vsym)
{
    Dimset* dimset = &vsym->typ.dimset;
    int isscalar = (dimset->ndims == 0);
    Bytebuffer* code;
    nc_type typecode = vsym->typ.basetype->typ.typecode;

    if(vsym->data == NULL) return;

    code = bbNew();

    /* Handle special cases first*/
    if(isscalar) {
	gencml_scalardata(vsym,code);
    } else { /* Non-scalar*/
	gencml_arraydata(vsym,code);
    }
    if(typecode == NC_STRING) {
        xpartial("<values>");
        xpartial(bbContents(code));
        xline("</values>");
    } else {
        xline("<values>");
        xpartial(bbContents(code));
        if(isprimplus(vsym->typ.basetype->typ.typecode)) xline("");
        xline("</values>");
    }
    bbFree(code);    
}

/* return CML name for netCDF type, given type code */
static char *
ncxtype(nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_CHAR: return "char";
      case NC_BYTE: return "byte";
      case NC_SHORT: return "short";
      case NC_INT: return "int";
      case NC_FLOAT: return "float";
      case NC_DOUBLE: return "double";
      case NC_UBYTE: return "ubyte";
      case NC_USHORT: return "ushort";
      case NC_UINT: return "uint";
      case NC_INT64: return "int64";
      case NC_UINT64: return "uint64";
      case NC_STRING: return "string";
      default: PANIC("nctype: bad type code");
    }
    return NULL;
}

/* Compute the CML name for a given type*/
char*
xtypename(Symbol* tsym)
{
    char* name;
    ASSERT(tsym->objectclass == NC_TYPE);
    if(tsym->subclass == NC_PRIM)
	name = ncxtype(tsym->typ.typecode);
    else
        name = xname(tsym);
    return name;
}

/* Compute the CML name for a given symbol*/
/* Cache in symbol->lname*/
char*
xname(Symbol* sym)
{
    if(sym->lname == NULL) {
	char* name = pooldup(sym->name);
#ifdef USE_NETCDF4
	if(sym->subclass == NC_FIELD || sym->subclass == NC_ECONST) {
	     sym->lname = nulldup(decodify(name));
	} else
            sym->lname = nulldup(decodify(xprefixed(sym->prefix,name,"/")));
#else
            sym->lname = nulldup(decodify(name)); /* convert to usable form*/
#endif
    }
    return sym->lname;
}

#ifdef USE_NETCDF4
/* Result is pool alloc'd*/
static char*
xprefixed(List* prefix, char* suffix, char* separator)
{
    int slen;
    int plen;
    int i;
    char* result;

    ASSERT(suffix != NULL);
    plen = prefixlen(prefix);
    if(prefix == NULL || plen == 0) return decodify(suffix);
    /* plen > 0*/
    slen = 0;
    for(i=0;i<plen;i++) {
	Symbol* sym = (Symbol*)listget(prefix,i);
	slen += (strlen(sym->name)+strlen(separator));
    }
    slen += strlen(suffix);
    slen++; /* for null terminator*/
    result = poolalloc(slen);
    result[0] = '\0';
    /* Leave off the root*/
    i = (rootgroup == (Symbol*)listget(prefix,0))?1:0;
    for(;i<plen;i++) {
	Symbol* sym = (Symbol*)listget(prefix,i);
        strcat(result,sym->name); /* append "<prefix[i]/>"*/
	strcat(result,separator);
    }    
    strcat(result,suffix); /* append "<suffix>"*/
    return result;
}
#endif

/**************************************************/
/*
 * Output a CML statement
 */

#ifdef USE_NETCDF4
static void
xprint(Bytebuffer* buf)
{
   bbAppendn(xcode,bbContents(buf),bbLength(buf));
}
#endif

static void
xpartial(char* line)
{
    bbCat(xcode,line);
}

static void
xline(char* line)
{
    xpartial(line);
    bbCat(xcode,"\n");
}

static void
xflush(void)
{
    if(bbLength(xcode) > 0) {
        bbAppend(xcode,'\0');
        fputs(bbContents(xcode),stdout);
        fflush(stdout);
        bbClear(xcode);
    }
}

#endif /*ENABLE_CML*/
