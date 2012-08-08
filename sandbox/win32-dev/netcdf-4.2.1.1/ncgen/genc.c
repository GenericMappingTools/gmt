/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genc.c,v 1.6 2010/05/17 23:26:44 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include "nciter.h"
#include <ctype.h>	/* for isprint() */

#ifdef ENABLE_C

#undef TRACE

/* Forward */
static const char* groupncid(Symbol*);
static const char* typencid(Symbol*);
static const char* varncid(Symbol*);
static const char* dimncid(Symbol*);

#ifdef USE_NETCDF4
static void definectype(Symbol*);
static void genc_deftype(Symbol*);
static void genc_definespecialattributes(Symbol* vsym);
#endif

static void genc_defineattr(Symbol* asym);
static void genc_definevardata(Symbol*);
static void genc_write(Generator*,Symbol* sym, Bytebuffer* code,
                       int rank, size_t* start, size_t* count);
static void genc_writevar(Generator*,Symbol*,Bytebuffer*,int,size_t*,size_t*);
static void genc_writeattr(Generator*,Symbol*,Bytebuffer*,int,size_t*,size_t*);

/*
 * Generate C code for creating netCDF from in-memory structure.
 */
void
gen_ncc(const char *filename)
{
    int idim, ivar, iatt, maxdims;
    int ndims, nvars, natts, ngatts;
    char* cmode_string;

#ifdef USE_NETCDF4
    int igrp,ityp, ngrps, ntyps;
#endif

    ndims = listlength(dimdefs);
    nvars = listlength(vardefs);
    natts = listlength(attdefs);
    ngatts = listlength(gattdefs);
#ifdef USE_NETCDF4
    ngrps = listlength(grpdefs);
    ntyps = listlength(typdefs);
#endif /*USE_NETCDF4*/

    /* wrap in main program */
    codeline("#include <stdio.h>");
    codeline("#include <stdlib.h>");
    codeline("#include <netcdf.h>");
    codeline("");
    codeflush();

    if(specialconstants) {
	/* If the input referenced e.g. nan, inf, etc;
	   then provide definitions for them */
        codeline("");
	codeline("#define nanf (0.0f/0.0f)");
	codeline("#define nan  (0.0/0.0)");
	codeline("#define inff (1.0f/0.0f)");
	codeline("#define inf  (1.0/0.0)");
	codeline("#define infinityf inff");
	codeline("#define infinity  inf");
        codeline("");
        codeflush();
    }
    codeline("");
    codeflush();

#ifdef USE_NETCDF4

    /* Construct C type definitions*/
    if (ntyps > 0) {
	for(ityp = 0; ityp < ntyps; ityp++) {
	    Symbol* tsym = (Symbol*)listget(typdefs,ityp);
	    definectype(tsym);
	}
	codeline("");
    }
    codeflush();

    /* Construct the chunking constants*/
    if(!usingclassic) {
        for(ivar=0;ivar<nvars;ivar++) {
            Bytebuffer* tmp = bbNew();
            Symbol* var = (Symbol*)listget(vardefs,ivar);
            Specialdata* special = &var->var.special;
            if(special->flags & _CHUNKSIZES_FLAG) {
                int i;
                size_t* chunks = special->_ChunkSizes;
                if(special->nchunks == 0 || chunks == NULL) continue;
                bbClear(tmp);
                for(i=0;i<special->nchunks;i++) {
                    bbprintf(tmp,"%s%ld",
                            (i == 0?"":", "),special->_ChunkSizes[i]);
                }
                bbprintf0(stmt,"static size_t %s_chunksizes[%d] = {",
                            cname(var),special->nchunks);
                codedump(stmt);
                codedump(tmp);
                codeline("} ;");
            }
        }
	codeline("");
    }
#endif /*USE_NETCDF4*/

    /* Now construct the main procedures*/
    codeline("void");
    codeline("check_err(const int stat, const int line, const char *file) {");
    codelined(1,"if (stat != NC_NOERR) {");
    codelined(2,"(void)fprintf(stderr,\"line %d of %s: %s\\n\", line, file, nc_strerror(stat));");
    codelined(2,"fflush(stderr);");
    codelined(2,"exit(1);");
    codelined(1,"}");
    codeline("}");
    codeline("");
    codeline("int");
    bbprintf0(stmt,"%s() {/* create %s */\n", mainname, filename);
    codedump(stmt);
    /* create necessary declarations */
    codeline("");    
    codelined(1,"int  stat;  /* return status */");
    codelined(1,"int  ncid;  /* netCDF id */");
    codeflush();

#ifdef USE_NETCDF4
    /* Define variables to hold group ids*/
    if(!usingclassic) {
        codeline("");
        codelined(1,"/* group ids */");
    }
    if(!usingclassic && ngrps > 0) {    
        for(igrp = 0; igrp < ngrps; igrp++) {
	    Symbol* gsym = (Symbol*)listget(grpdefs,igrp);
	    bbprintf0(stmt,"%sint %s;\n",indented(1),groupncid(gsym));
	    codedump(stmt);
	}
    }

    /* define variables to hold type ids*/
    if(!usingclassic && ntyps > 0) {
	codeline("");
	codelined(1,"/* type ids */");
	for(ityp = 0; ityp < ntyps; ityp++) {
	    Symbol* tsym = (Symbol*)listget(typdefs,ityp);
	    bbprintf0(stmt,"%sint %s;\n",indented(1), typencid(tsym));
	    codedump(stmt);
	}
    }
    codeflush();
#endif

    if (ndims > 0) {
	codeline("");
	codelined(1,"/* dimension ids */");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    bbprintf0(stmt,"%sint %s;\n", indented(1), dimncid(dsym));
	    codedump(stmt);
	}

	codeline("");
	codelined(1,"/* dimension lengths */");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    if(dsym->dim.isunlimited) {
		bbprintf0(stmt,"%ssize_t %s_len = NC_UNLIMITED;\n",
			indented(1),cname(dsym));
	    } else {
		bbprintf0(stmt,"%ssize_t %s_len = %lu;\n",
			indented(1),
			cname(dsym),
			(unsigned long) dsym->dim.declsize);
	    }
	    codedump(stmt);
	}
    }
    codeflush();

    maxdims = 0;	/* most dimensions of any variable */
    for(ivar = 0; ivar < nvars; ivar++) {
      Symbol* vsym = (Symbol*)listget(vardefs,ivar);
      if(vsym->typ.dimset.ndims > maxdims)
	maxdims = vsym->typ.dimset.ndims;
    }

    if (nvars > 0) {
	codeline("");
	codelined(1,"/* variable ids */");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    bbprintf0(stmt,"    int %s;\n", varncid(vsym));
	    codedump(stmt);
	}

	codeline("");
	codelined(1,"/* rank (number of dimensions) for each variable */");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    bbprintf0(stmt,"#   define RANK_%s %d\n", cname(vsym),
		    vsym->typ.dimset.ndims);
	    codedump(stmt);
	}
	if (maxdims > 0) {	/* we have dimensioned variables */
	    codeline("");
	    codelined(1,"/* variable shapes */");
	    for(ivar = 0; ivar < nvars; ivar++) {
                Symbol* vsym = (Symbol*)listget(vardefs,ivar);
		if (vsym->typ.dimset.ndims > 0) {
		    bbprintf0(stmt,"    int %s_dims[RANK_%s];\n",
			    cname(vsym), cname(vsym));
		    codedump(stmt);
		}
	    }
	}
    }
    codeflush();

    /* create netCDF file, uses NC_CLOBBER mode */
    codeline("");
    codelined(1,"/* enter define mode */");

    if (!cmode_modifier) {
	cmode_string = "NC_CLOBBER";
    } else if (cmode_modifier & NC_64BIT_OFFSET) {
	cmode_string = "NC_CLOBBER|NC_64BIT_OFFSET";
#ifdef USE_NETCDF4
    } else if (cmode_modifier & NC_CLASSIC_MODEL) {
	cmode_string = "NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL";
    } else if (cmode_modifier & NC_NETCDF4) {
	cmode_string = "NC_CLOBBER|NC_NETCDF4";	
#endif
    } else {
        derror("unknown cmode modifier");
	cmode_string = "NC_CLOBBER";
    }
    bbprintf0(stmt,"    stat = nc_create(\"%s\", %s, &ncid);\n",
		 filename,cmode_string);
    codedump(stmt);
    codelined(1,"check_err(stat,__LINE__,__FILE__);");
    codeflush();
    
#ifdef USE_NETCDF4
    /* Define the group structure */
    /* ncid created above is also root group*/
    if(!usingclassic) {
        bbprintf0(stmt,"    %s = ncid;\n",groupncid(rootgroup));
        codedump(stmt);
        /* walking grpdefs list will do a preorder walk of all defined groups*/
        for(igrp=0;igrp<listlength(grpdefs);igrp++) {
	    Symbol* gsym = (Symbol*)listget(grpdefs,igrp);
	    if(gsym == rootgroup) continue; /* ignore root*/
	    if(gsym->container == NULL)
		PANIC("null container");
	    bbprintf0(stmt,
		"    stat = nc_def_grp(%s, \"%s\", &%s);\n",
		groupncid(gsym->container),
		gsym->name, groupncid(gsym));
	    codedump(stmt);
	    codelined(1,"check_err(stat,__LINE__,__FILE__);");
	}
        codeflush();
    }
#endif

#ifdef USE_NETCDF4
    /* Construct code to define types*/
    if(ntyps > 0) {
        codeline("");
	for(ityp = 0; ityp < ntyps; ityp++) {
	    Symbol* tsym = (Symbol*)listget(typdefs,ityp);
	    if(tsym->subclass == NC_PRIM
		|| tsym->subclass == NC_ARRAY) continue; /* no need to do these*/
	    genc_deftype(tsym);
	    codeline("");
	}
    }
    codeflush();
#endif

    /* define dimensions from info in dims array */
    if (ndims > 0) {
	codeline("");
	codelined(1,"/* define dimensions */");
        for(idim = 0; idim < ndims; idim++) {
            Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    {
		bbprintf0(stmt,
		          "    stat = nc_def_dim(%s, \"%s\", %s_len, &%s);\n",
		          groupncid(dsym->container),
                          escapifyname(dsym->name),
                          cname(dsym),
                          dimncid(dsym));
	    }
	    codedump(stmt);
	    codelined(1,"check_err(stat,__LINE__,__FILE__);");
       }
    }
    codeflush();

    /* define variables from info in vars array */
    if (nvars > 0) {
	codeline("");
	codelined(1,"/* define variables */");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            Symbol* basetype = vsym->typ.basetype;
	    Dimset* dimset = &vsym->typ.dimset;
	    codeline("");
	    if(dimset->ndims > 0) {
	        for(idim = 0; idim < dimset->ndims; idim++) {
		    Symbol* dsym = dimset->dimsyms[idim];
		    bbprintf0(stmt,
			    "    %s_dims[%d] = %s;\n",
			    cname(vsym),
			    idim,
			    dimncid(dsym));
		    codedump(stmt);
		}
	    }
	    bbprintf0(stmt,
			"    stat = nc_def_var(%s, \"%s\", %s, RANK_%s, %s, &%s);\n",
		        groupncid(vsym->container),
			escapifyname(vsym->name),
			typencid(basetype),
			cname(vsym),
			(dimset->ndims == 0?"0":poolcat(cname(vsym),"_dims")),
			varncid(vsym));
	    codedump(stmt);
	    codelined(1,"check_err(stat,__LINE__,__FILE__);");
#ifdef USE_NETCDF4
	    genc_definespecialattributes(vsym);
#endif /*USE_NETCDF4*/
	}
    }
    codeflush();
    
    /* Define the global attributes*/
    if(ngatts > 0) {
	codeline("");
	codelined(1,"/* assign global attributes */");
	for(iatt = 0; iatt < ngatts; iatt++) {
	    Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
	    genc_defineattr(gasym);	    
	}
	codeline("");
    }
    codeflush();
    
    /* Define the variable specific attributes*/
    if(natts > 0) {
	codeline("");
	codelined(1,"/* assign per-variable attributes */");
	for(iatt = 0; iatt < natts; iatt++) {
	    Symbol* asym = (Symbol*)listget(attdefs,iatt);
	    genc_defineattr(asym);
	}
	codeline("");
    }
    codeflush();

    if (nofill_flag) {
        codelined(1,"/* don't initialize variables with fill values */");
	bbindent(stmt,1);
	bbprintf0(stmt,"stat = nc_set_fill(%s, NC_NOFILL, 0);",groupncid(rootgroup));
	codelined(1,"check_err(stat,__LINE__,__FILE__);");
    }

    codeline("");
    codelined(1,"/* leave define mode */");
    bbprintf0(stmt,"    stat = nc_enddef (%s);\n",groupncid(rootgroup));
    codedump(stmt);
    codelined(1,"check_err(stat,__LINE__,__FILE__);");
    codeflush();

    if(!header_only) {
        /* Load values into those variables with defined data */
        if(nvars > 0) {
            codeline("");
            codelined(1,"/* assign variable data */");
            for(ivar = 0; ivar < nvars; ivar++) {
                Symbol* vsym = (Symbol*)listget(vardefs,ivar);
                if(vsym->data != NULL) genc_definevardata(vsym);
            }
            codeline("");
        }
        codeflush();
    }
}

#ifdef USE_NETCDF4
static void
genc_definespecialattributes(Symbol* vsym)
{
    Specialdata* special = &vsym->var.special;
    if(usingclassic) return;
    if(special->flags & _STORAGE_FLAG) {
        int storage = special->_Storage;
        size_t* chunks = special->_ChunkSizes;
        bbprintf0(stmt,
                "    stat = nc_def_var_chunking(%s, %s, %s, ",
                groupncid(vsym->container),
                varncid(vsym),
                (storage == NC_CONTIGUOUS?"NC_CONTIGUOUS":"NC_CHUNKED"));
        codedump(stmt);
        if(special->nchunks == 0 || chunks == NULL)
            codepartial("NULL");
        else {
            bbprintf0(stmt,"%s_chunksizes",cname(vsym));
            codedump(stmt);                 
        }
        codeline(");");
        codelined(1,"check_err(stat,__LINE__,__FILE__);");
    }   
    if(special->flags & _FLETCHER32_FLAG) {
        bbprintf0(stmt,
                "    stat = nc_def_var_fletcher32(%s, %s, %d);\n",
                groupncid(vsym->container),
                varncid(vsym),
                special->_Fletcher32);
        codedump(stmt);
        codelined(1,"check_err(stat,__LINE__,__FILE__);");
    }
    if(special->flags & (_DEFLATE_FLAG | _SHUFFLE_FLAG)) {
        bbprintf0(stmt,
                "    stat = nc_def_var_deflate(%s, %s, %s, %d, %d);\n",
                groupncid(vsym->container),
                varncid(vsym),
                (special->_Shuffle == 1?"NC_SHUFFLE":"NC_NOSHUFFLE"),
                (special->_DeflateLevel >= 0?1:0),
                (special->_DeflateLevel >= 0?special->_DeflateLevel:0));
        codedump(stmt);
        codelined(1,"check_err(stat,__LINE__,__FILE__);");
    }   
    if(special->flags & _ENDIAN_FLAG) {
        bbprintf0(stmt,
                "    stat = nc_def_var_endian(%s, %s, %s);\n",
                groupncid(vsym->container),
                varncid(vsym),
                (special->_Endianness == NC_ENDIAN_LITTLE?"NC_ENDIAN_LITTLE"
                                                    :"NC_ENDIAN_BIG")
                );
        codedump(stmt);
        codelined(1,"check_err(stat,__LINE__,__FILE__);");
    }   
    if(special->flags & _NOFILL_FLAG) {
        bbprintf0(stmt,
                "    stat = nc_def_var_fill(%s, %s, %s, NULL);\n",
                groupncid(vsym->container),
                varncid(vsym),
                (special->_Fill?"NC_FILL":"NC_NOFILL")
                );
        codedump(stmt);
        codelined(1,"check_err(stat,__LINE__,__FILE__);");
    }   
}
#endif /*USE_NETCDF4*/

void
cl_c(void)
{
    bbprintf0(stmt,"%sstat = nc_close(%s);\n",indented(1),groupncid(rootgroup));
    codedump(stmt);
    codelined(1,"check_err(stat,__LINE__,__FILE__);");
#ifndef vms
    codelined(1,"return 0;");
#else
    codelined(1,"return 1;");
#endif
    codeline("}");
    codeflush();
}

/*
 * Output a C statement
 */



/* return C name for netCDF type, given type code */
const char *
nctype(nc_type type)
{
    switch (type) {
      case NC_CHAR: return "NC_CHAR";
      case NC_BYTE: return "NC_BYTE";
      case NC_SHORT: return "NC_SHORT";
      case NC_INT: return "NC_INT";
      case NC_FLOAT: return "NC_FLOAT";
      case NC_DOUBLE: return "NC_DOUBLE";
      case NC_UBYTE: return "NC_UBYTE";
      case NC_USHORT: return "NC_USHORT";
      case NC_UINT: return "NC_UINT";
      case NC_INT64: return "NC_INT64";
      case NC_UINT64: return "NC_UINT64";
      case NC_STRING: return "NC_STRING";
      default: PANIC("nctype: bad type code");
    }
    return NULL;
}



/*
 * Return C type name for netCDF type, given type code.
 */
const char* 
ncctype(nc_type type)
{
    switch (type) {
      case NC_CHAR:
	return "char";
      case NC_BYTE:
	return "signed char";
      case NC_SHORT:
	return "short";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "float";
      case NC_DOUBLE:
	return "double";
      case NC_UBYTE:
	return "unsigned char";
      case NC_USHORT:
	return "unsigned short";
      case NC_UINT:
	return "unsigned int";
      case NC_INT64:
	return "signed long long";
      case NC_UINT64:
	return "unsigned long long";
      case NC_STRING:
	return "char*";
      default:
	PANIC1("ncctype: bad type code:%d",type);
    }
    return 0;
}

/*
 * Return C type name for netCDF type suffix, given type code.
 */
const char* 
ncstype(nc_type nctype)
{
    switch (nctype) {
      case NC_CHAR:
	return "text";
      case NC_BYTE:
	return "schar";
      case NC_SHORT:
	return "short";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "float";
      case NC_DOUBLE:
	return "double";
      case NC_UBYTE:
	return "ubyte";
      case NC_USHORT:
	return "ushort";
      case NC_UINT:
	return "uint";
      case NC_INT64:
	return "longlong";
      case NC_UINT64:
	return "ulonglong";
      case NC_STRING:
	return "string";
      default:
	derror("ncstype: bad type code: %d",nctype);
	return 0;
    }
}

/* Return the group name for the specified group*/
static const char*
groupncid(Symbol* sym)
{
#ifdef USE_NETCDF4
    if(usingclassic) {
        return "ncid";
    } else {
        char* grptmp;
	const char* tmp1;
        if(sym == NULL) return groupncid(rootgroup);
        ASSERT(sym->objectclass == NC_GRP);
        tmp1 = cname(sym);
        grptmp = poolalloc(strlen(tmp1)+strlen("_grp")+1);
        strcpy(grptmp,tmp1);
        strcat(grptmp,"_grp");
        return grptmp;
    }
#else
    return "ncid";
#endif
}

/* Compute the C name for a given type's id*/
/* Watch out: the result is a static*/
static const char*
typencid(Symbol* tsym)
{
    char* typtmp;
    const char* tmp1;
    if(tsym->subclass == NC_PRIM)
	return nctype(tsym->typ.typecode);
    tmp1 = ctypename(tsym);
    typtmp = poolalloc(strlen(tmp1)+strlen("_typ")+1);
    strcpy(typtmp,tmp1);
    strcat(typtmp,"_typ");
    return typtmp;
}

/* Compute the C name for a given var's id*/
/* Watch out: the result is a static*/
static const char*
varncid(Symbol* vsym)
{
    const char* tmp1;
    char* vartmp;
    tmp1 = cname(vsym);
    vartmp = poolalloc(strlen(tmp1)+strlen("_id")+1);
    strcpy(vartmp,tmp1);
    strcat(vartmp,"_id");
    return vartmp;
}

/* Compute the C name for a given dim's id*/
/* Watch out: the result is a static*/
static const char*
dimncid(Symbol* dsym)
{
    const char* tmp1;
    char* dimtmp;
    tmp1 = cname(dsym);
    dimtmp = poolalloc(strlen(tmp1)+strlen("_dim")+1);
    strcpy(dimtmp,tmp1);
    strcat(dimtmp,"_dim");
    return dimtmp;
}

/* Compute the C name for a given type*/
const char*
ctypename(Symbol* tsym)
{
    const char* name;
    ASSERT(tsym->objectclass == NC_TYPE);
    if(tsym->subclass == NC_PRIM)
	name = ncctype(tsym->typ.typecode);
    else
        name = cname(tsym);
    return name;
}

#ifdef USE_NETCDF4
static void
definectype(Symbol* tsym)
{
    int i,j;

    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_PRIM: break; /* these are already taken care of*/
    case NC_OPAQUE:
	bbprintf0(stmt,"typedef unsigned char %s[%lu];\n",
		cname(tsym), tsym->typ.size);
	codedump(stmt);
	break;
    case NC_ENUM:
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* econst = (Symbol*)listget(tsym->subnodes,i);
	    Bytebuffer* econststring = bbNew();
	    ASSERT(econst->subclass == NC_ECONST);
	    c_generator->constant(c_generator,&econst->typ.econst,econststring);
	    bbNull(econststring);
	    bbprintf0(stmt,"#define %s ((%s)%s)\n",
		    cname(econst),
		    ctypename(econst->typ.basetype),
		    bbContents(econststring));
	    bbFree(econststring);
	    codedump(stmt);
	}
	bbprintf0(stmt,"typedef %s %s;\n",
		ctypename(tsym->typ.basetype), cname(tsym));
	codedump(stmt);
	break;
    case NC_VLEN:
	bbprintf0(stmt,"typedef nc_vlen_t %s;\n",
		ctypename(tsym));
	codedump(stmt);
	break;
    case NC_COMPOUND:
	bbprintf0(stmt,"typedef struct %s {\n",cname(tsym));
	codedump(stmt);
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* efield = (Symbol*)listget(tsym->subnodes,i);
	    ASSERT(efield->subclass == NC_FIELD);
	    bbprintf0(stmt,"%s%s %s",
			indented(1),ctypename(efield->typ.basetype),cname(efield));
	    codedump(stmt);
	    /* compute any dimension specification*/
	    if(efield->typ.dimset.ndims > 0) {
		Bytebuffer* dimbuf = bbNew();
	        for(j=0;j<efield->typ.dimset.ndims;j++) {
		    Symbol* dim;
		    char tmp[64];
		    bbCat(dimbuf,"[");
		    dim = efield->typ.dimset.dimsyms[j];
		    ASSERT(dim->dim.isconstant);
		    snprintf(tmp,sizeof(tmp),"%u",
			(unsigned int)dim->dim.declsize);
		    bbCat(dimbuf,tmp);
		    bbCat(dimbuf,"]");
		}
		codedump(dimbuf);
		bbFree(dimbuf);
	    }
	    codeline(";");
	}
	bbprintf0(stmt,"} %s;\n", ctypename(tsym));
	codedump(stmt);
	break;

    case NC_ARRAY:
	/* ignore: this will be handled by def_var*/
	break;

    default: panic("definectype: unexpected type subclass: %d",tsym->subclass);
    }
}

/*
Generate the C code for defining a given type
*/
static void
genc_deftype(Symbol* tsym)
{
    int i;

    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_PRIM: break; /* these are already taken care of*/
    case NC_OPAQUE:
	bbprintf0(stmt,"%sstat = nc_def_opaque(%s, %lu, \"%s\", &%s);\n",
		indented(1),
		groupncid(tsym->container),
		tsym->typ.size,
		tsym->name,
		typencid(tsym));
	codedump(stmt);
	codelined(1,"check_err(stat,__LINE__,__FILE__);");
	break;
    case NC_ENUM:
	codelined(1,"{");
	bbprintf0(stmt,"%s%s econst;\n",
		indented(1),
		ncctype(tsym->typ.basetype->typ.typecode));
	codedump(stmt);
	bbprintf0(stmt,"%sstat = nc_def_enum(%s, %s, \"%s\", &%s);\n",
		indented(1),
		groupncid(tsym->container),
		nctype(tsym->typ.basetype->typ.typecode),
		tsym->name,
		typencid(tsym));
	codedump(stmt);
	codelined(1,"check_err(stat,__LINE__,__FILE__);");
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* econst = (Symbol*)listget(tsym->subnodes,i);
	    Bytebuffer* econststring = bbNew();
	    ASSERT(econst->subclass == NC_ECONST);
	    c_generator->constant(c_generator,&econst->typ.econst,econststring);
	    bbNull(econststring);
	    bbprintf0(stmt,"%seconst = %s;\n",
		indented(1),bbContents(econststring));
	    bbFree(econststring);
	    codedump(stmt);
	    bbprintf0(stmt,"%sstat = nc_insert_enum(%s, %s, \"%s\", &econst);\n",
		    indented(1),
		    groupncid(tsym->container),
		    typencid(tsym),
		    escapifyname(econst->name));
	    codedump(stmt);
	}
	codelined(1,"}");
	break;
    case NC_VLEN:
	bbprintf0(stmt,"%sstat = nc_def_vlen(%s, \"%s\", %s, &%s);",
		indented(1),
		groupncid(tsym->container),
		escapifyname(tsym->name),
		typencid(tsym->typ.basetype),
		typencid(tsym));
	codedump(stmt);
	codelined(1,"check_err(stat,__LINE__,__FILE__);");
	break;
    case NC_COMPOUND:
	bbprintf0(stmt,"%sstat = nc_def_compound(%s, sizeof(%s), \"%s\", &%s);",
		indented(1),
		groupncid(tsym->container),
		ctypename(tsym),
		escapifyname(tsym->name),
		typencid(tsym));
	codedump(stmt);
	codelined(1,"check_err(stat,__LINE__,__FILE__);");
	/* Generate the field dimension constants*/
	codelined(1,"{");
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    int j;
	    Symbol* efield = (Symbol*)listget(tsym->subnodes,i);
	    ASSERT(efield->subclass == NC_FIELD);
	    if(efield->typ.dimset.ndims == 0) continue;	    
	    bbprintf0(stmt,"%sstatic int %s_dims[%d] = {\n",
			indented(1),
			cname(efield),efield->typ.dimset.ndims);
	    for(j=0;j<efield->typ.dimset.ndims;j++) {
		char tmp[256];
	        Symbol* e = efield->typ.dimset.dimsyms[j];
		ASSERT(e->dim.isconstant);
		snprintf(tmp,sizeof(tmp),"%u",(unsigned int)e->dim.declsize);
		bbCat(stmt,(j==0?"":", "));
		bbCat(stmt,tmp);
	    }
	    bbCat(stmt,"};");
	    codedump(stmt);
	}
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* efield = (Symbol*)listget(tsym->subnodes,i);
	    char tmp[1024];
	    ASSERT(efield->subclass == NC_FIELD);
#ifdef TESTALIGNMENT
	    snprintf(tmp,sizeof(tmp),"%lu",efield->typ.offset);
#else
	    snprintf(tmp,sizeof(tmp),"NC_COMPOUND_OFFSET(%s,%s)",
			ctypename(tsym), cname(efield));
#endif
	    if(efield->typ.dimset.ndims > 0){ 
	        bbprintf0(stmt,"%sstat = nc_insert_array_compound(%s, %s, \"%s\", %s, %s, %d, %s_dims);",
		    indented(1),
		    groupncid(tsym->container),
		    typencid(tsym),
		    escapifyname(efield->name),
		    tmp,
		    typencid(efield->typ.basetype),
		    efield->typ.dimset.ndims,
		    cname(efield));
	    } else {
	        bbprintf0(stmt,"%sstat = nc_insert_compound(%s, %s, \"%s\", %s, %s);",
		    indented(1),
		    groupncid(tsym->container),
		    typencid(tsym),
		    escapifyname(efield->name),
		    tmp,
		    typencid(efield->typ.basetype));
	    }
	    codedump(stmt);
	    codelined(1,"check_err(stat,__LINE__,__FILE__);");
	}
	codelined(1,"}");
	break;

    case NC_ARRAY:
	/* ignore: this will be handled by def_var*/
	break;

    default: panic("genc_deftype: unexpected type subclass: %d",tsym->subclass);
    }
}

#endif /*USE_NETCDF4*/

static void
genc_defineattr(Symbol* asym)
{
    /* we need to capture vlen strings for dumping */
    Bytebuffer* save = bbNew(); /* capture so we can dump
                                   vlens first */
    List* oldstate = NULL;
    generator_getstate(c_generator,(void*)&oldstate);
    listfree(oldstate);
    generator_reset(c_generator,(void*)listnew());
    generate_attrdata(asym,c_generator,(Writer)genc_write,save);
    bbFree(save);
}

static void
genc_definevardata(Symbol* vsym)
{
    Bytebuffer* save; /* capture so we can dump vlens first */
    List* oldstate = NULL;
    if(vsym->data == NULL) return;
    save = bbNew();
    generator_getstate(c_generator,(void*)&oldstate);
    listfree(oldstate);
    generator_reset(c_generator,(void*)listnew());
    generate_vardata(vsym,c_generator,(Writer)genc_write,save);
    bbFree(save);
}

static void
genc_write(Generator* generator, Symbol* sym, Bytebuffer* code,
           int rank, size_t* start, size_t* count)
{
    if(sym->objectclass == NC_ATT)
	genc_writeattr(generator,sym,code,rank,start,count);
    else if(sym->objectclass == NC_VAR)
	genc_writevar(generator,sym,code,rank,start,count);
    else
	PANIC("illegal symbol for genc_write");
}

static void
genc_writevar(Generator* generator, Symbol* vsym, Bytebuffer* code,
           int rank, size_t* start, size_t* count)
{
    Symbol* basetype = vsym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;
    List* vlendecls;

    /* define a block to avoid name clashes*/
    codeline("");
    codelined(1,"{");

    /* Dump any vlen decls first */
    generator_getstate(generator,(void**)&vlendecls);
    if(vlendecls != NULL && listlength(vlendecls) > 0) {
	int i;
	for(i=0;i<listlength(vlendecls);i++) {
	   Bytebuffer* decl = (Bytebuffer*)listget(vlendecls,i);
	   codelined(1,bbContents(decl));
	   bbFree(decl);
	}
	listfree(vlendecls);
	generator_reset(generator,NULL);
    }       

    if(rank == 0) {
	codelined(1,"size_t zero = 0;");
	    /* We make the data be an array so we do not need to
               ampersand it later => we need an outer pair of braces
            */
	    commify(code); /* insert commas at proper places */
            bbprintf0(stmt,"%sstatic %s %s_data[1] = {%s};\n",
			    indented(1),
			    ctypename(basetype),
			    cname(vsym),
			    bbContents(code));
	codedump(stmt);
        bbprintf0(stmt,"%sstat = nc_put_var1(%s, %s, &zero, %s_data);\n",
		indented(1),
		groupncid(vsym->container),
		varncid(vsym),
		cname(vsym));
        codedump(stmt);
        codelined(1,"check_err(stat,__LINE__,__FILE__);");
	codeflush();
    } else { /* rank > 0 */
	int i;
        size_t length = 0;
        if(typecode == NC_CHAR) {
	    length = bbLength(code);
	    /* generate data constant */
	    bbprintf(stmt,"%schar* %s_data = ",
			indented(1),
			cname(vsym),
			(unsigned long)length);
	    codedump(stmt);
	    cquotestring(code,'"');
	    codedump(code);
	    codeline(" ;");
	} else {
	    /* Compute total size */ 
	    length = 1;
    	    for(i=0;i<rank;i++) length *= count[i];
	    /* generate data constant */
	    commify(code); /* insert commas at proper places */
	    bbprintf(stmt,"%s%s %s_data[%lu] = ",
			indented(1),
			ctypename(basetype),
			cname(vsym),
			(unsigned long)length);
	    codedump(stmt);
    	    /* C requires an outer set of braces on datalist constants */
	    codepartial("{");
	    codedump(code);
	    codeline("} ;");
	}
	
	/* generate constants for startset, countset*/
	bbprintf0(stmt,"%ssize_t %s_startset[%lu] = {",
			indented(1),
			cname(vsym),
			rank);
	for(i=0;i<rank;i++) {
	    bbprintf(stmt,"%s%lu",(i>0?", ":""),start[i]);
	}
	codedump(stmt);
	codeline("} ;");
	
	bbprintf0(stmt,"%ssize_t %s_countset[%lu] = {",
			indented(1),
			cname(vsym),
			rank);
	for(i=0;i<rank;i++) {
	    bbprintf(stmt,"%s%lu",(i>0?", ":""),count[i]);
	}
	codedump(stmt);
	codeline("};");
	
	bbprintf0(stmt,"%sstat = nc_put_vara(%s, %s, %s_startset, %s_countset, %s_data);\n",
			indented(1),
			groupncid(vsym->container), varncid(vsym),
			cname(vsym),
			cname(vsym),
			cname(vsym));
	codedump(stmt);
	codelined(1,"check_err(stat,__LINE__,__FILE__);");
	
    }
    /* end defined block*/
    codelined(1,"}\n");
    codeflush();
}
	
static void
genc_writeattr(Generator* generator, Symbol* asym, Bytebuffer* code,
               int rank, size_t* start, size_t* count)
{
    Symbol* basetype = asym->typ.basetype;
    int typecode = basetype->typ.typecode;
    size_t len = asym->data->length; /* default assumption */

    /* define a block to avoid name clashes*/
    codeline("");
    codelined(1,"{");

    /* Handle NC_CHAR specially */
    if(typecode == NC_CHAR) {
        len = bbLength(code); /* presumably before quoting */
	/* Revise length if length == 0 */
	if(len == 0) len++;
	cquotestring(code,'"');
    } else {
        /* All other cases */
	/* Dump any vlen decls first */
	List* vlendecls;
	generator_getstate(generator,(void**)&vlendecls);
	if(vlendecls != NULL && listlength(vlendecls) > 0) {
	    int i;
	    for(i=0;i<listlength(vlendecls);i++) {
	        Bytebuffer* decl = (Bytebuffer*)listget(vlendecls,i);
	        codelined(1,bbContents(decl));
	        bbFree(decl);
	    }
	    listfree(vlendecls);
	    generator_reset(generator,NULL);
	}       

        commify(code);
        bbprintf0(stmt,"%sstatic const %s %s_att[%ld] = ",
			indented(1),
			ctypename(basetype),
			cname(asym),
			asym->data->length
			);
        codedump(stmt);
        codepartial("{");
        codedump(code);
        codepartial("}");
        codeline(" ;");
        bbClear(code);
    }

    /* Use the specialized put_att_XX routines if possible*/
    switch (basetype->typ.typecode) {
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_FLOAT:
    case NC_DOUBLE:
        bbprintf0(stmt,"%sstat = nc_put_att_%s(%s, %s, \"%s\", %s, %lu, %s_att);\n",
		indented(1),
		ncstype(basetype->typ.typecode),
		groupncid(asym->container),
		(asym->att.var == NULL?"NC_GLOBAL"
			              :varncid(asym->att.var)),
		escapifyname(asym->name),
		typencid(basetype),		
	 	len,
		cname(asym));
	codedump(stmt);
	break;

    case NC_CHAR:
	/* Include the string constant in-line */
        bbprintf0(stmt,"%sstat = nc_put_att_%s(%s, %s, \"%s\", %lu, %s);\n",
		indented(1),
		ncstype(basetype->typ.typecode),
		groupncid(asym->container),
		(asym->att.var == NULL?"NC_GLOBAL"
			              :varncid(asym->att.var)),
		escapifyname(asym->name),
	 	len,
		bbContents(code));
	codedump(stmt);
	break;

#ifdef USE_NETCDF4
    /* !usingclassic only (except NC_STRING) */
    case NC_UBYTE:
    case NC_USHORT:
    case NC_UINT:
    case NC_INT64:
    case NC_UINT64:
	if(usingclassic) {
	    verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
	    return;
	}
        bbprintf0(stmt,"%sstat = nc_put_att_%s(%s, %s, \"%s\", %s, %lu, %s_att);",
		indented(1),
		ncstype(basetype->typ.typecode),
		groupncid(asym->container),
		(asym->att.var == NULL?"NC_GLOBAL"
			              :varncid(asym->att.var)),
		escapifyname(asym->name),
		typencid(basetype),		
	 	len,
		cname(asym));
	codedump(stmt);
	break;

    case NC_STRING:
	if(usingclassic) {
	    verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
	    return;
	}
        bbprintf0(stmt,"%sstat = nc_put_att_%s(%s, %s, \"%s\", %lu, %s_att);",
		indented(1),
		ncstype(basetype->typ.typecode),
		groupncid(asym->container),
		(asym->att.var == NULL?"NC_GLOBAL"
			              :varncid(asym->att.var)),
		escapifyname(asym->name),
	 	len,
		cname(asym));
	codedump(stmt);
	break;
#endif

    default: /* User defined type */
#ifndef USE_NETCDF4
        verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
#else /* USE_NETCDF4 */
	if(usingclassic && !isclassicprim(basetype->typ.typecode)) {
            verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
	}
        bbprintf0(stmt,"%sstat = nc_put_att(%s, %s, \"%s\", %s, %lu, %s_att);\n",
		indented(1),
		groupncid(asym->container),
		(asym->att.var == NULL?"NC_GLOBAL"
			              :varncid(asym->att.var)),
		escapifyname(asym->name),
		typencid(basetype),
	        len,
		cname(asym));
        codedump(stmt);
#endif
	break;
    }

    codelined(1,"check_err(stat,__LINE__,__FILE__);");
    codelined(1,"}");
}
#endif /*ENABLE_C*/
