/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genjjni.c,v 1.6 2010/04/04 19:39:46 dmh Exp $
 *********************************************************************/

#include "includes.h"

#ifdef ENABLE_JAVA

#undef JGDB

#undef TRACE

/*MNEMONIC*/
#define USEMEMORY 1

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward */
static void genjjni_definevardata(Symbol* vsym);
static void genjjni_primattribute(Symbol*, Bytebuffer*, unsigned long);
static void genjjni_scalarprim(Symbol* var, Bytebuffer* code);

static const char* jgroupncid(Symbol*);
static const char* jtypencid(Symbol*);
static const char* jvarncid(Symbol*);
static const char* jdimncid(Symbol*);

#ifdef USE_NETCDF4
static void definejtype(Symbol*);
static char* jprefixed(List* prefix, char* suffix, char* separator);
static void genjjni_deftype(Symbol*);
static void definespecialattributes(Symbol* vsym);
#endif

static int jputvaraprim(Putvar*, Odometer*, Bytebuffer*, size_t*);


static void genjjni_defineattribute(Symbol* asym);
static void genjjni_definevardata(Symbol*);

static void computemaxunlimited(void);

/*
Global Bytebuffer into which to store the C code output;
periodically dumped to file by jflush().
Defined in genjstd.c.
*/
extern Bytebuffer* jcode;

/*
 * Generate code for creating netCDF from in-memory structure.
 */
void
gen_ncjava_jni(const char *filename)
{
    int idim, ivar, iatt, maxdims;
    int ndims, nvars, natts, ngatts, ngrps, ntyps;
    char* cmode_string;

#ifdef USE_NETCDF4
    int igrp,ityp;
#endif

    jcode = bbNew();
    bbSetalloc(jcode,C_MAX_STMT);

    ndims = listlength(dimdefs);
    nvars = listlength(vardefs);
    natts = listlength(attdefs);
    ngatts = listlength(gattdefs);
    ngrps = listlength(grpdefs);
    ntyps = listlength(typdefs);

    /* Construct the main class */
#ifdef JGDB
    jline("import java.io.*;");
#endif
    jline("import java.util.*;");
    jline("import netcdf.*;");
    jline("import static netcdf.Constants.*;");
    jline("import static netcdf.Netcdf.*;");
    jline("");
    jpartial("public class ");
    jline(mainname);
    jline("{");

    /* Do static initializations */
    jline("");
    jline("static {");
    jlined(1,"System.loadLibrary(\"ncjni\");");
    jlined(1,"Netcdf.init();");
    jline("}");
    jline("");
    jline("static long memory = Memory.create(); /* For storing data */");

#ifdef USE_NETCDF4

    if (ntyps > 0) {
	for(ityp = 0; ityp < ntyps; ityp++) {
	    Symbol* tsym = (Symbol*)listget(typdefs,ityp);
	    definejtype(tsym);
	}
	bbCat(jcode,"");
    }
    jflush();

    /*
	Define vlen constants; For java, this is done in
	two parts. Part 1 (here) defines the constants and part 2
	fills them in. 
    */ 
    {
	jflush(); /* dump code to this point*/
	genjjni_vlenconstants(vlenconstants,jcode);
	jline("");
    }

    /* Construct the chunking constants*/
    if(!usingclassic) {
        for(ivar=0;ivar<nvars;ivar++) {
            Bytebuffer* tmp = bbNew();
            Symbol* var = (Symbol*)listget(vardefs,ivar);
            Specialdata* special = &var->var.special;
            if(special->flags & _CHUNKSIZE_FLAG) {
                int i;
                size_t* chunks = special->_ChunkSizes;
                if(special->nchunks == 0 || chunks == NULL) continue;
                bbClear(tmp);
                for(i=0;i<special->nchunks;i++) {
                    nprintf(stmt,sizeof(stmt),"%s%ld",
                            (i == 0?"":", "),special->_ChunkSizes[i]);
                    bbCat(tmp,stmt);
                }
                nprintf(stmt,sizeof(stmt),"static final long[] %s_chunksizes = {",
                            jname(var),special->nchunks);
                jpartial(stmt);
                jprint(tmp);
                jline("} ;");
            }
        }
	jline("");
    }
#endif /*USE_NETCDF4*/

    jline("static public void");
    jline("check_err(int stat) throws Exception");
    jline("{");
    jlined(1,"if (stat != NC_NOERR) {");
    jlined(2,"System.err.println(\"Failed: \"+nc_strerror(stat));");
    jlined(2,"throw new Exception(nc_strerror(stat));");
    jlined(1,"}");
    jline("}");
    jline("");

    /* Now construct the main procedure*/

    jline("static public void main(String[] argv) throws Exception");
    nprintf(stmt,sizeof(stmt), "{ /* create %s */",filename);
    jline(stmt);

    /* create necessary declarations */
    jline("");    
    jlined(1,"int  stat;  /* return status */");
    jlined(1,"int  ncid;  /* netCDF id */");
    jlined(1,"int[]  ncidp = new int[1];  /* netCDF id return */");
    jflush();

    /*
	Define vlen constants Part 2.
	The idea is to walk all the data lists
	whose variable type has a vlen and collect
	the vlen data and define a constant for it.
    */ 
    jline("");
    jflush(); /* dump code to this point*/
    genjjni_vlendata(vlenconstants,jcode);

#ifdef JGDB
jline("{");
jline("System.out.print(\"go?\");");
jline("BufferedReader rin = new BufferedReader(new InputStreamReader(System.in));");
jline("rin.readLine();");
jline("}");
#endif


#ifdef USE_NETCDF4
    /* Define variables to hold group ids*/
    if(!usingclassic) {
        jline("");
        jlined(1,"/* group ids */");
    }
    if(!usingclassic && ngrps > 0) {    
        for(igrp = 0; igrp < ngrps; igrp++) {
	    Symbol* gsym = (Symbol*)listget(grpdefs,igrp);
	    nprintf(stmt,sizeof(stmt),"int %s;",jgroupncid(gsym));
	    jlined(1,stmt);
	}
    }

    /* define variables to hold type ids*/
    if(!usingclassic && ntyps > 0) {
	jline("");
	jlined(1,"/* type ids */");
	for(ityp = 0; ityp < ntyps; ityp++) {
	    Symbol* tsym = (Symbol*)listget(typdefs,ityp);
	    nprintf(stmt,sizeof(stmt),"int %s;",jtypencid(tsym));
	    jlined(1,stmt);
	}
    }
    jflush();
#endif

    if (ndims > 0) {
	jline("");
	jlined(1,"/* dimension ids */");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    nprintf(stmt,sizeof(stmt),"int %s;",jdimncid(dsym));
	    jlined(1,stmt);
	}

	jline("");
	jlined(1,"/* dimension lengths */");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    if (dsym->dim.size == NC_UNLIMITED) {
		nprintf(stmt,sizeof(stmt),"%sfinal long %s_len = NC_UNLIMITED;",
			indented(1),jname(dsym));
	    } else {
		nprintf(stmt,sizeof(stmt),"%sfinal long %s_len = %lu;",
			indented(1),
			jname(dsym),
			(unsigned long) dsym->dim.size);
	    }
	    jline(stmt);
	}
    }
    jflush();

    maxdims = 0;	/* most dimensions of any variable */
    for(ivar = 0; ivar < nvars; ivar++) {
      Symbol* vsym = (Symbol*)listget(vardefs,ivar);
      if(vsym->typ.dimset.ndims > maxdims)
	maxdims = vsym->typ.dimset.ndims;
    }

    if (nvars > 0) {
	jline("");
	jlined(1,"/* variable ids */");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    nprintf(stmt,sizeof(stmt),"int %s;", jvarncid(vsym));
	    jlined(1,stmt);
	}

	jline("");
	jlined(1,"/* rank (number of dimensions) for each variable */");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    nprintf(stmt,sizeof(stmt),"final int RANK_%s = %d;",
		    jname(vsym),vsym->typ.dimset.ndims);
	    jlined(1,stmt);
	}
	if (maxdims > 0) {	/* we have dimensioned variables */
	    jline("");
	    jlined(1,"/* variable shapes */");
	    for(ivar = 0; ivar < nvars; ivar++) {
                Symbol* vsym = (Symbol*)listget(vardefs,ivar);
		if (vsym->typ.dimset.ndims > 0) {
		    nprintf(stmt,sizeof(stmt),"final int[] %s_dims = new int[RANK_%s];",
			    jname(vsym), jname(vsym));
		    jlined(1,stmt);
		}
	    }
	}
    }
    jflush();

    /* create netCDF file, uses NC_CLOBBER mode */
    jline("");
    jlined(1,"/* enter define mode */");

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

    nprintf(stmt,sizeof(stmt),"check_err(nc_create(\"%s\", %s, ncidp));",
		filename,cmode_string);
    jlined(1,stmt);
    jlined(1,"ncid = ncidp[0];");
    jflush();
    
#ifdef USE_NETCDF4
    /* Define the group structure */
    /* ncid created above is also root group*/
    if(!usingclassic) {
        nprintf(stmt,sizeof(stmt),"%s = ncid;",jgroupncid(rootgroup));
        jlined(1,stmt);
        /* walking grpdefs list will do a preorder walk of all defined groups*/
        for(igrp=0;igrp<listlength(grpdefs);igrp++) {
	    Symbol* gsym = (Symbol*)listget(grpdefs,igrp);
	    if(gsym == rootgroup) continue; /* ignore root*/
	    if(gsym->container == NULL)
		PANIC("null container");
	    nprintf(stmt,sizeof(stmt),
		"check_err(nc_def_grp(%s, \"%s\", ncidp));",
		jgroupncid(gsym->container),
		gsym->name);
	    jlined(1,stmt);
	    nprintf(stmt,sizeof(stmt),"%s = ncidp[0];",jgroupncid(gsym));
	    jlined(1,stmt);
	}
        jflush();
    }
#endif

#ifdef USE_NETCDF4
    /* Construct code to define types*/
    if(ntyps > 0) {
        jline("");
	for(ityp = 0; ityp < ntyps; ityp++) {
	    Symbol* tsym = (Symbol*)listget(typdefs,ityp);
	    if(tsym->subclass == NC_PRIM
		|| tsym->subclass == NC_ARRAY) continue; /* no need to do these*/
	    genjjni_deftype(tsym);
	    jline("");
	}
    }
    jflush();
#endif

    /* define dimensions from info in dims array */
    if (ndims > 0) {
	jline("");
	jlined(1,"/* define dimensions */");
        for(idim = 0; idim < ndims; idim++) {
            Symbol* dsym = (Symbol*)listget(dimdefs,idim);
    	    nprintf(stmt,sizeof(stmt),
		"check_err(nc_def_dim(%s, \"%s\", %s_len, ncidp));",
		jgroupncid(dsym->container),
                jescapifyname(dsym->name), jname(dsym));
	    jlined(1,stmt);
	    nprintf(stmt,sizeof(stmt),"%s = ncidp[0];",jdimncid(dsym));
	    jlined(1,stmt);
       }
    }
    jflush();

    /* define variables from info in vars array */
    if (nvars > 0) {
	jline("");
	jlined(1,"/* define variables */");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            Symbol* basetype = vsym->typ.basetype;
	    Dimset* dimset = &vsym->typ.dimset;
	    jline("");
	    if(dimset->ndims > 0) {
	        for(idim = 0; idim < dimset->ndims; idim++) {
		    Symbol* dsym = dimset->dimsyms[idim];
		    nprintf(stmt,sizeof(stmt),
			    "%s_dims[%d] = %s;",
			    jname(vsym),
			    idim,
			    jdimncid(dsym));
		    jlined(1,stmt);
		}
	    }
	    nprintf(stmt,sizeof(stmt),
			"check_err(nc_def_var(%s, \"%s\", %s, RANK_%s, %s, ncidp));",
		        jgroupncid(vsym->container),
			jescapifyname(vsym->name),
			jtypencid(basetype),
			jname(vsym),
			(dimset->ndims == 0?"null":poolcat(jname(vsym),"_dims")));
	    jlined(1,stmt);
	    nprintf(stmt,sizeof(stmt),"%s = ncidp[0];",jvarncid(vsym));
	    jlined(1,stmt);
#ifdef USE_NETCDF4
	    definespecialattributes(vsym);
#endif /*USE_NETCDF4*/
	}
    }
    jflush();
    
    /* Define the global attributes*/
    if(ngatts > 0) {
	jline("");
	jlined(1,"/* assign global attributes */");
	for(iatt = 0; iatt < ngatts; iatt++) {
	    Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
	    genjjni_defineattribute(gasym);	    
	}
	jline("");
    }
    jflush();
    
    /* Define the variable specific attributes*/
    if(natts > 0) {
	jline("");
	jlined(1,"/* assign per-variable attributes */");
	for(iatt = 0; iatt < natts; iatt++) {
	    Symbol* asym = (Symbol*)listget(attdefs,iatt);
	    genjjni_defineattribute(asym);
	}
	jline("");
    }
    jflush();

    if (nofill_flag) {
        jlined(1,"/* don't initialize variables with fill values */");
	jlined(1,"check_err(nc_set_fill(%s, NC_NOFILL, 0));");
    }

    jline("");
    jlined(1,"/* leave define mode */");
    nprintf(stmt,sizeof(stmt),"check_err(nc_enddef(%s));",
	    jgroupncid(rootgroup));
    jlined(1,stmt);
    jflush();

    /* Load values into those variables with defined data */

    if(nvars > 0) {
	jline("");
	jlined(1,"/* assign variable data */");
        for(ivar = 0; ivar < nvars; ivar++) {
	    Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    if(vsym->data != NULL) genjjni_definevardata(vsym);
	}
	jline("");
	/* compute the max actual size of the unlimited dimension*/
        if(usingclassic) computemaxunlimited();
    }
    jflush();

}

void
cl_java_jni(void)
{
    nprintf(stmt,sizeof(stmt),"check_err(nc_close(%s));",
		jgroupncid(rootgroup));
    jlined(1,stmt);

    jline("}"); /* main */

    jline("}"); /* class Main */

    jflush();
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
        nprintf(stmt,sizeof(stmt),
                "%scheck_err(nc_def_var_chunking(%s, %s, %s, ",
		indented(1),
	        jgroupncid(vsym->container),
                jvarncid(vsym),
                (storage == NC_CONTIGUOUS?"NC_CONTIGUOUS":"NC_CHUNKED"));
        jpartial(stmt);
        if(special->nchunks == 0 || chunks == NULL)
            jpartial("new long[]{}");
        else {
            nprintf(stmt,sizeof(stmt),"%s_chunksizes",jname(vsym));
            jpartial(stmt);                 
        }
        jline("));");
    }   
    if(special->flags & _FLETCHER32_FLAG) {
        nprintf(stmt,sizeof(stmt),
                "check_err(nc_def_var_fletcher32(%s, %s, %d));",
                jgroupncid(vsym->container),
                jvarncid(vsym),
                special->_Fletcher32);
        jlined(1,stmt);
    }
    if(special->flags & (_DEFLATE_FLAG | _SHUFFLE_FLAG)) {
        nprintf(stmt,sizeof(stmt),
                "check_err(nc_def_var_deflate(%s, %s, %s, %d, %d));",
                jgroupncid(vsym->container),
                jvarncid(vsym),
                (special->_Shuffle == 1?"NC_SHUFFLE":"NC_NOSHUFFLE"),
                (special->_DeflateLevel >= 0?1:0),
                (special->_DeflateLevel >= 0?special->_DeflateLevel:0));
        jlined(1,stmt);
    }   
    if(special->flags & _ENDIAN_FLAG) {
        nprintf(stmt,sizeof(stmt),
                "check_err(nc_def_var_endian(%s, %s, %s));",
                jgroupncid(vsym->container),
                jvarncid(vsym),
                (special->_Endianness == NC_ENDIAN_LITTLE?"NC_ENDIAN_LITTLE"
                                                    :"NC_ENDIAN_BIG")
                );
        jlined(1,stmt);
    }   
    if(special->flags & _NOFILL_FLAG) {
        nprintf(stmt,sizeof(stmt),
                "check_err(nc_def_var_fill(%s, %s, %s));",
                jgroupncid(vsym->container),
                jvarncid(vsym),
                (special->_Fill?"NC_FILL":"NC_NOFILL")
                );
        jlined(1,stmt);
    }   
}
#endif /*USE_NETCDF4*/

/*
 * Return java type name for netCDF type, given type code.
 */
const char* 
jtype(nc_type type)
{
    switch (type) {
      case NC_CHAR: return "char";
      case NC_BYTE: return "byte";
      case NC_SHORT: return "short";
      case NC_INT: return "int";
      case NC_FLOAT: return "float";
      case NC_DOUBLE: return "double";
      case NC_UBYTE: return "long";
      case NC_USHORT: return "long";
      case NC_UINT: return "long";
      case NC_INT64: return "long";
      case NC_UINT64: return "long";
      case NC_STRING: return "String";
      case NC_ENUM: return "int";
      case NC_OPAQUE: return "String";
      default: PANIC1("ncctype: bad type code:%d",type);
    }
    return 0;
}

/*
 * Return a type name and dimensions for constant arrays
 * for netCDF type, given type code.
 */
const char* 
jarraytype(nc_type type)
{
    switch (type) {
      case NC_CHAR:
	return "String";
      case NC_BYTE:
	return "byte";
      case NC_SHORT:
	return "short";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "float";
      case NC_DOUBLE:
	return "double";
      case NC_UBYTE:
	return "long";
      case NC_USHORT:
	return "long";
      case NC_UINT:
	return "long";
      case NC_INT64:
	return "long";
      case NC_UINT64:
	return "long";
      case NC_STRING:
	return "String";
      case NC_ENUM:
	return "int";
      case NC_OPAQUE:
	return "String";
      default:
	PANIC1("ncctype: bad type code:%d",type);
    }
    return 0;
}

/*
 * Return netcdf interface type name for netCDF type suffix, given type code.
 */
const char* 
jstype(nc_type nctype)
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
      case NC_OPAQUE:
	return "opaque";
      case NC_ENUM:
	return "enum";
      default:
	derror("ncstype: bad type code: %d",nctype);
	return 0;
    }
}

/* Return the group name for the specified group*/
static const char*
jgroupncid(Symbol* sym)
{
#ifdef USE_NETCDF4
    if(usingclassic) {
        return "ncid";
    } else {
        char* grptmp;
	const char* tmp1;
        if(sym == NULL) return jgroupncid(rootgroup);
        ASSERT(sym->objectclass == NC_GRP);
        tmp1 = jname(sym);
        grptmp = poolalloc(strlen(tmp1)+strlen("_grp")+1);
        strcpy(grptmp,tmp1);
        strcat(grptmp,"_grp");
        return grptmp;
    }
#else
    return "ncid";
#endif
}

/* Compute the name for a given type's id*/
/* Watch out: the result is a pool alloc*/
static const char*
jtypencid(Symbol* tsym)
{
    char* typtmp;
    const char* tmp1;
    if(tsym->subclass == NC_PRIM)
	return nctype(tsym->typ.typecode);
    tmp1 = jtypename(tsym);
    typtmp = poolalloc(strlen(tmp1)+strlen("_typ")+1);
    strcpy(typtmp,tmp1);
    strcat(typtmp,"_typ");
    return typtmp;
}

/* Compute the name for a given var's id*/
/* Watch out: the result is a static*/
static const char*
jvarncid(Symbol* vsym)
{
    const char* tmp1;
    char* vartmp;
    tmp1 = jname(vsym);
    vartmp = poolalloc(strlen(tmp1)+strlen("_id")+1);
    strcpy(vartmp,tmp1);
    strcat(vartmp,"_id");
    return vartmp;
}

/* Compute the name for a given dim's id*/
/* Watch out: the result is a static*/
static const char*
jdimncid(Symbol* dsym)
{
    const char* tmp1;
    char* dimtmp;
    tmp1 = jname(dsym);
    dimtmp = poolalloc(strlen(tmp1)+strlen("_dim")+1);
    strcpy(dimtmp,tmp1);
    strcat(dimtmp,"_dim");
    return dimtmp;
}

/* Compute the C name for a given type*/
const char*
jtypename(Symbol* tsym)
{
    const char* name;
    ASSERT(tsym->objectclass == NC_TYPE);
    if(tsym->subclass == NC_PRIM)
	name = jtype(tsym->typ.typecode);
    else
        name = jname(tsym);
    return name;
}


#ifdef USE_NETCDF4
/* 
Only generate type info for enumerations
*/
static void
definejtype(Symbol* tsym)
{
    int i;
    int nconst;

    ASSERT(tsym->objectclass == NC_TYPE);
    if(tsym->subclass != NC_ENUM) return;
    /* start enum def */
    nprintf(stmt,sizeof(stmt),"enum %s {",jtypename(tsym));
    jline(stmt);
    /* define the constants */
    nconst = listlength(tsym->subnodes);
    for(i=0;i<nconst;i++) {
	Symbol* econst = (Symbol*)listget(tsym->subnodes,i);
	ASSERT(econst->subclass == NC_ECONST);
	nprintf(stmt,sizeof(stmt),"%s%s(%s)%s",
		    indented(1),
		    jtypename(econst),
		    jconst(&econst->typ.econst),
	            (i == (nconst - 1)?";":","));
	jlined(1,stmt);
    }
    /* add boilerplate */
    nprintf(stmt,sizeof(stmt),"private final %s value;",
				jtypename(tsym->typ.basetype));
    jlined(1,stmt);
    nprintf(stmt,sizeof(stmt),"%s(%s value) {this.value = value;}",
			      jtypename(tsym),jtypename(tsym->typ.basetype));
    jlined(1,stmt);
    nprintf(stmt,sizeof(stmt),"public %s value() {return this.value;}",
		jtypename(tsym->typ.basetype));
    jlined(1,stmt);
    jline("}");
}

/*
Generate the code for defining a given type
*/
static void
genjjni_deftype(Symbol* tsym)
{
    int i;

    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_PRIM: break; /* these are already taken care of*/
    case NC_OPAQUE:
	nprintf(stmt,sizeof(stmt),"check_err(nc_def_opaque(%s, %lu, \"%s\", ncidp));",
		jgroupncid(tsym->container),
		tsym->typ.size,
		jescapifyname(tsym->name));
	jlined(1,stmt);
	nprintf(stmt,sizeof(stmt),"%s = ncidp[0];",jtypencid(tsym));
	jlined(1,stmt);
	break;

    case NC_ENUM:
	jlined(1,"{");
	jlined(1,stmt);
	jlined(1,"long econst;");
	nprintf(stmt,sizeof(stmt),"check_err(nc_def_enum(%s, %s, \"%s\", ncidp));",
		jgroupncid(tsym->container),
		nctype(tsym->typ.basetype->typ.typecode),
		jname(tsym));
	jlined(1,stmt);
	nprintf(stmt,sizeof(stmt),"%s = ncidp[0];",jtypencid(tsym));
	jlined(1,stmt);
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* econst = (Symbol*)listget(tsym->subnodes,i);
	    ASSERT(econst->subclass == NC_ECONST);
	    nprintf(stmt,sizeof(stmt),"econst = (long)%s.%s.value();",
		jname(tsym),
		jname(econst));
	    jlined(1,stmt);
	    nprintf(stmt,sizeof(stmt),"check_err(nc_insert_enum(%s, %s, \"%s\", econst));",
		    jgroupncid(tsym->container), jtypencid(tsym),
		    jescapifyname(econst->name));
	    jlined(1,stmt);
	}
	jlined(1,"}");
	break;

    case NC_VLEN:
	nprintf(stmt,sizeof(stmt),"stat = nc_def_vlen(%s, \"%s\", %s, ncidp);",
		jgroupncid(tsym->container),
		jescapifyname(tsym->name),
		jtypencid(tsym->typ.basetype));
	jlined(1,stmt);
	nprintf(stmt,sizeof(stmt),"%s = ncidp[0];",jtypencid(tsym));
	jlined(1,stmt);
	break;

    case NC_COMPOUND:
	/* We cannot use sizeof(), so use the computed size */
	nprintf(stmt,sizeof(stmt),"check_err(nc_def_compound(%s, %ld, \"%s\", ncidp));",
		jgroupncid(tsym->container),
		(unsigned long)tsym->typ.size,
		jtypename(tsym),
		tsym->name);
	jlined(1,stmt);
	nprintf(stmt,sizeof(stmt),"%s = ncidp[0];",jtypencid(tsym));
	jlined(1,stmt);
	/* Generate the field dimension constants*/
	jlined(1,"{");
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    int j;
	    Symbol* efield = (Symbol*)listget(tsym->subnodes,i);
	    ASSERT(efield->subclass == NC_FIELD);
	    if(efield->typ.dimset.ndims == 0) continue;	    
	    nprintf(stmt,sizeof(stmt),"final int %s_dims[] = new int[] {",
			jname(efield));
	    for(j=0;j<efield->typ.dimset.ndims;j++) {
		char tmp[256];
	        Symbol* e = efield->typ.dimset.dimsyms[j];
		ASSERT(e->dim.isconstant);
		sprintf(tmp,"%u",e->dim.size);
		strcat(stmt,(j==0?"":", "));
		strcat(stmt,tmp);
	    }
	    strcat(stmt,"};");
	    jlined(1,stmt);
	}
	for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* efield = (Symbol*)listget(tsym->subnodes,i);
	    char tmp[1024];
	    ASSERT(efield->subclass == NC_FIELD);
	    sprintf(tmp,"%lu",efield->typ.offset);
	    if(efield->typ.dimset.ndims > 0){ 
	        nprintf(stmt,sizeof(stmt),"check_err(nc_insert_array_compound(%s, %s, \"%s\", %s, %s, %d, %s_dims));",
		    jgroupncid(tsym->container),
		    jtypencid(tsym),
		    jescapifyname(efield->name),
		    tmp,
		    jtypencid(efield->typ.basetype),
		    efield->typ.dimset.ndims,
		    jname(efield));
	    } else {
	        nprintf(stmt,sizeof(stmt),"check_err(nc_insert_compound(%s, %s, \"%s\", %s, %s));",
		    jgroupncid(tsym->container),
		    jtypencid(tsym),
		    jescapifyname(efield->name),
		    tmp,
		    jtypencid(efield->typ.basetype));
	    }
	    jlined(1,stmt);
	}
	jlined(1,"}");
	break;

    case NC_ARRAY:
	/* ignore: this will be handled by def_var*/
	break;

    default: panic("genjjni_deftype: unexpected type subclass: %d",tsym->subclass);
    }
}
#endif

static void
genjjni_defineattribute(Symbol* asym)
{
    unsigned long len;
    Datalist* list;
    Symbol* basetype = asym->typ.basetype;
    Bytebuffer* code = NULL; /* capture other decls*/

    list = asym->data;
    if(list == NULL) PANIC("empty attribute list");
    len = asym->att.count;
    if(len == 0) PANIC("empty attribute list");

    nprintf(stmt,sizeof(stmt),"/* attribute: %s */",asym->name);
    jlined(1,stmt);

    code = bbNew();

    genjjni_attrdata(asym,code);	

    /* Handle primitives separately */
    if(isprimplus(basetype->typ.typecode)) {
	if(basetype->typ.typecode != NC_CHAR) commify(code);
	genjjni_primattribute(asym, code, len);
    } else { /* User defined type */
#ifndef USE_NETCDF4
        verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
#else /* USE_NETCDF4 */
	if(usingclassic && !isclassicprim(basetype->typ.typecode)) {
            verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
	    return;
	}
	/* Dump the generation code */
        jprint(code);
        nprintf(stmt,sizeof(stmt),"check_err(nc_put_att_memory(%s, %s, \"%s\", %s, %lu, memory));",
		jgroupncid(asym->container),
		(asym->att.var == NULL?"NC_GLOBAL"
			              :jvarncid(asym->att.var)),
		jescapifyname(asym->name),
		jtypencid(basetype),
	        len);
        jlined(1,stmt);
#endif
    }
    bbFree(code);
}

static void
genjjni_primattribute(Symbol* asym, Bytebuffer* code, unsigned long len)
{
    Symbol* basetype = asym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;

    /* Handle NC_CHAR specially */
    if(typecode == NC_CHAR) {
        /* revise the length count */
	len = bbLength(code);
	if(len == 0) {bbAppend(code,'\0'); len++;}
	jquotestring(code,'"');
    } else {
        /* Convert to constant */
	char* code2 = bbDup(code);
        bbClear(code);
        nprintf(stmt,sizeof(stmt),"new %s[]",
		jarraytype(typecode));
        bbCat(code,stmt);
        bbCat(code,"{");
        bbCat(code,code2);
        bbCat(code,"}");
	efree(code2);
    }
    switch (typecode) {
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_FLOAT:
    case NC_DOUBLE:
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_att_%s(%s, %s, \"%s\", %s, %lu, ",
		indented(1),
                jstype(basetype->typ.typecode),
                jgroupncid(asym->container),
                (asym->att.var == NULL?"NC_GLOBAL"
                                      :jvarncid(asym->att.var)),
                jescapifyname(asym->name),
                jtypencid(basetype),             
                len);
        jpartial(stmt);
	jprint(code);
	jline("));");
	jflush();
        break;

    case NC_CHAR:
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_att_%s(%s, %s, \"%s\", %lu, ",
		indented(1),
                jstype(basetype->typ.typecode),
                jgroupncid(asym->container),
                (asym->att.var == NULL?"NC_GLOBAL"
                                      :jvarncid(asym->att.var)),
                jescapifyname(asym->name),
                len);
        jpartial(stmt);
	jprint(code);
	jline("));");
	jflush();
        break;

#ifdef USING_NETCDF4
    /* !usingclassic only (except NC_STRING) */
    case NC_UBYTE:
    case NC_USHORT:
    case NC_UINT:
    case NC_INT64:
    case NC_UINT64:
    case NC_STRING:
        if(usingclassic) {
            verror("Non-classic type: %s",njtypename(basetype->typ.typecode));
            return;
        }
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_att_%s(%s, %s, \"%s\", %s, %lu, ",
		indented(1),
                jstype(basetype->typ.typecode),
                jgroupncid(asym->container),
                (asym->att.var == NULL?"NC_GLOBAL"
                                      :jvarncid(asym->att.var)),
                jescapifyname(asym->name),
                jtypencid(basetype),             
                len);
        jpartial(stmt);
	jprint(code);
	jline("));");
	jflush();
        break;
#endif
    default: break;
    }
}

static void
computemaxunlimited(void)
{
    int i;
    unsigned long maxsize;
    Symbol* udim = rootgroup->grp.unlimiteddim;
    if(udim == NULL) return; /* there is no unlimited dimension*/
    /* Look at each variable and see what*/
    /* size it gives to the unlimited dim (if any)*/
    maxsize = 0;
    for(i=0;i<listlength(vardefs);i++) {
	Symbol* dim;
	Symbol* var = (Symbol*)listget(vardefs,i);	
	if(var->typ.dimset.ndims == 0) continue; /* rank == 0*/
	dim = var->typ.dimset.dimsyms[0];
	if(dim->dim.size != NC_UNLIMITED) continue; /* var does not use unlimited*/
	if(var->typ.dimset.dimsyms[0]->dim.size > maxsize)
	    maxsize = var->typ.dimset.dimsyms[0]->dim.size;
    }
}


/* Define the put_vara closure function for C data*/
/*
Important assumptions:
1. The leftmost changed index controls the count set.
2. All indices to the right of #1 are assumed to be at there
   max values.
*/

static int
jputvara(struct Putvar* closure, Odometer* odom, Bytebuffer* databuf)
{
    int i;
    int stat = NC_NOERR;
    size_t startset[NC_MAX_VAR_DIMS];
    size_t countset[NC_MAX_VAR_DIMS];
    Symbol* vsym = closure->var;
    Symbol* basetype = vsym->typ.basetype;
    Dimset* dimset = &vsym->typ.dimset;
    Bytebuffer* code = closure->code;
    char dimstring[NC_MAX_VAR_DIMS*2+1];
    size_t count;
    nc_type typecode = basetype->typ.typecode;

    memset((void*)startset,0,sizeof(startset));
    memset((void*)countset,0,sizeof(countset));

#ifdef TRACE
fprintf(stderr,"putvara: %s: ",vsym->name);
fprintf(stderr,"odom = %s\n",odometerprint(odom));
fprintf(stderr,"initial startset = [");
for(i=0;i<closure->rank;i++) fprintf(stderr,"%s%u",(i>0?", ":""),closure->startset[i]);
fprintf(stderr,"]\n");
fflush(stderr);
#endif

    /* Compute base on a change in the 0th dimension*/
    for(i=1;i<closure->rank;i++) {
        startset[i] = 0;
        countset[i] = odom->dims[i].datasize;
    }

    startset[0] = closure->startset[0];
    countset[0] = odom->dims[0].index - startset[0];

#ifdef TRACE
{
    fprintf(stderr,"startset = [");
    for(i=0;i<closure->rank;i++) fprintf(stderr,"%s%u",(i>0?", ":""),startset[i]);
    fprintf(stderr,"] ");
    fprintf(stderr,"countset = [");
    for(i=0;i<closure->rank;i++) fprintf(stderr,"%s%u",(i>0?", ":""),countset[i]);
    fprintf(stderr,"]\n");
    fflush(stderr);
}
#endif

    /* generate constants for data*/
    count = 1;
    dimstring[0] = '\0';
    for(i=0;i<dimset->ndims;i++) {
	count *= countset[i];
	strcat(dimstring,"[]");
    }

    /* define a block to avoid name clashes*/
    nprintf(stmt,sizeof(stmt),"%s{\n",indented(1));
    bbCat(code,stmt);    

    /* generate constants for startset, countset*/
    nprintf(stmt,sizeof(stmt),"%slong[] %s_startset = new long[] {",
			    indented(1),
			    jname(vsym));
    bbCat(code,stmt);
    for(i=0;i<closure->rank;i++) {
        nprintf(stmt,sizeof(stmt),"%s%lu",(i>0?", ":""),startset[i]);
	bbCat(code,stmt);
    }
    bbCat(code,"} ;\n");

    nprintf(stmt,sizeof(stmt),"%slong[] %s_countset = new long[] {",
			    indented(1),
			    jname(vsym));
    bbCat(code,stmt);
    for(i=0;i<closure->rank;i++) {
        nprintf(stmt,sizeof(stmt),"%s%lu",(i>0?", ":""),countset[i]);
	bbCat(code,stmt);
    }
    bbCat(code,"} ;\n");

    /* Handle primitives separately */
    if(isprimplus(typecode)) {
	jputvaraprim(closure, odom,databuf,countset);
    } else { /* User defined type */
#ifndef USE_NETCDF4
        verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
#else /* USE_NETCDF4 */
	if(usingclassic && !isclassicprim(basetype->typ.typecode)) {
            verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
	    return NC_EINVAL;
	}
	/* Dump the generation code */
	bbCat(code,bbContents(databuf));
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_vara_memory(%s, %s, %s_startset, %s_countset, memory));\n",
		indented(1),
		jgroupncid(vsym->container),
		jvarncid(vsym),
		jname(vsym),
		jname(vsym));
	bbCat(code,stmt);
#endif
    }

    /* end defined block*/
    nprintf(stmt,sizeof(stmt),"%s}\n",indented(1));
    bbCat(code,stmt);    

    for(i=0;i<closure->rank;i++) {
        closure->startset[i] = startset[i] + countset[i];
    }

    bbClear(databuf);
    return stat;
}


static int
jputvaraprim(struct Putvar* closure, Odometer* odom, Bytebuffer* databuf, size_t* countset)
{
    int i;
    Symbol* vsym = closure->var;
    Symbol* basetype = vsym->typ.basetype;
    Dimset* dimset = &vsym->typ.dimset;
    Bytebuffer* code = closure->code;
    size_t len;
    nc_type typecode = basetype->typ.typecode;

    /* generate constants for data*/
    len = 1;
    for(i=0;i<dimset->ndims;i++) {
	len *= countset[i];
    }

    /* Handle NC_CHAR specially */
    if(typecode == NC_CHAR) {
        /* revise the length count */
	len = bbLength(databuf);
	if(len == 0) {bbAppend(databuf,'\0'); len++;}
	jquotestring(databuf,'"');
    } else {
        /* Convert to constant */
	char* databuf2;
        commify(databuf);
	databuf2 = bbDup(databuf);
        bbClear(databuf);
        nprintf(stmt,sizeof(stmt),"new %s[]",
		jarraytype(typecode));
        bbCat(databuf,stmt);
        bbCat(databuf,"{");
        bbCat(databuf,databuf2);
        bbCat(databuf,"}");
	efree(databuf2);
    }
    switch (typecode) {
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_FLOAT:
    case NC_DOUBLE:
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_vara_%s(%s, %s, %s_startset, %s_countset, ",
		indented(1),
                jstype(basetype->typ.typecode),
		jgroupncid(vsym->container),
		jvarncid(vsym),
		jname(vsym),
		jname(vsym));
        bbCat(code,stmt);
	bbCat(code,bbContents(databuf));
	bbCat(code,"));\n");
        break;

    case NC_CHAR:
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_vara_%s(%s, %s, %s_startset, %s_countset, ",
		indented(1),
                jstype(basetype->typ.typecode),
                jgroupncid(vsym->container),
		jvarncid(vsym),
		jname(vsym),
		jname(vsym));
        bbCat(code,stmt);
	bbCat(code,bbContents(databuf));
	bbCat(code,"));\n");
        break;

#ifdef USE_NETCDF4
    /* !usingclassic only (except NC_STRING) */
    case NC_UBYTE:
    case NC_USHORT:
    case NC_UINT:
    case NC_INT64:
    case NC_UINT64:
    case NC_STRING:
        if(usingclassic) {
            verror("Non-classic type: %s",nctypename(typecode));
            return NC_EINVAL;
        }
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_vara_%s(%s, %s, %s_startset, %s_countset, ",
		indented(1),
                jstype(basetype->typ.typecode),
                jgroupncid(vsym->container),
		jvarncid(vsym),
		jname(vsym),
		jname(vsym));
        bbCat(code,stmt);
	bbCat(code,bbContents(databuf));
	bbCat(code,"));\n");
        break;

    case NC_ENUM:
    case NC_OPAQUE:
        if(usingclassic) {
            verror("Non-classic type: %s",nctypename(typecode));
            return NC_EINVAL;
        }
	/* We use some augmented vara procedures */
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_vara_%s(%s, %s, %s_startset, %s_countset, ",
		indented(1),
		(typecode == NC_ENUM?"enum":"opaque"),
                jgroupncid(vsym->container),
		jvarncid(vsym),
		jname(vsym),
		jname(vsym));
        bbCat(code,stmt);
	bbCat(code,bbContents(databuf));
	bbCat(code,"));\n");
        break;

#endif
    default:
        verror("Non-classic type: %s",nctypename(typecode));
    }
    bbClear(databuf);
    return NC_NOERR;
}

static void
genjjni_definevardata(Symbol* vsym)
{
    Dimset* dimset = &vsym->typ.dimset;
    Symbol* basetype = vsym->typ.basetype;
    int rank = dimset->ndims;
    int isscalar = (dimset->ndims == 0);
    Bytebuffer* code;
    nc_type typecode = basetype->typ.typecode;

    if(vsym->data == NULL) return;

    code = bbNew();

    /* Handle special cases first*/
    if(isscalar) {
	genjjni_scalardata(vsym,code);
	if(isprimplus(typecode)) {
	    genjjni_scalarprim(vsym,code);
	} else {
            nprintf(stmt,sizeof(stmt),"check_err(nc_put_var_memory(%s, %s, memory));",
		jgroupncid(vsym->container),
		jvarncid(vsym));
            jlined(1,stmt);
	}
    } else { /* Non-scalar*/
        Bytebuffer* databuf;
        /* build a closure*/
	Putvar closure;
        closure.putvar = jputvara;
        closure.rank = rank;
	closure.code = code;
        closure.var = vsym;
        memset(closure.startset,0,sizeof(closure.startset));

	/* Most complex case; use closure as needed*/
	/* Use a separate data buffer to capture the data list*/
        databuf = bbNew();
	genjjni_arraydata(vsym,&closure,databuf);
	jprint(code);
	bbFree(databuf);

    }
    bbFree(code);    
}

static void
genjjni_scalarprim(Symbol* var, Bytebuffer* code)
{
    Symbol* basetype = var->typ.basetype;
    nc_type typecode = basetype->typ.typecode;

    switch (typecode) {
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_FLOAT:
    case NC_DOUBLE:
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_var_%s(%s, %s, (%s)",
		indented(1),
                ncstype(basetype->typ.typecode),
                jgroupncid(var->container),
		jvarncid(var),
		jtype(basetype->typ.typecode));
        jpartial(stmt);
	jprint(code);
	jline("));");
	jflush();
        break;

    case NC_CHAR:
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_var_%s(%s, %s, ",
		indented(1),
                ncstype(basetype->typ.typecode),
                jgroupncid(var->container),
		jvarncid(var));
        jpartial(stmt);
	jprint(code);
	jline("));");
	jflush();
        break;

#ifdef USING_NETCDF4
    /* !usingclassic only (except NC_STRING) */
    case NC_UBYTE:
    case NC_USHORT:
    case NC_UINT:
    case NC_INT64:
    case NC_UINT64:
    case NC_STRING:
        if(usingclassic) {
            verror("Non-classic type: %s",njtypename(basetype->typ.typecode));
            return;
        }
        nprintf(stmt,sizeof(stmt),"%scheck_err(nc_put_vara_%s(%s, %s, new long[]{0}, new long[]{1}, ",
		indented(1),
                ncstype(basetype->typ.typecode),
                jgroupncid(var->container),
		jvarncid(var));
        jpartial(stmt);
	jprint(code);
	jline("));");
	jflush();
        break;
#endif
    default: break;
    }
}
/*
 * Output a statement
 */

void
jprint(Bytebuffer* buf)
{
   bbAppendn(jcode,bbContents(buf),bbLength(buf));
}

void
jpartial(char* line)
{
    bbCat(jcode,line);
}

void
jline(char* line)
{
    jpartial(line);
    bbCat(jcode,"\n");
}

void
jlined(int n, char* line)
{
    bbCat(jcode,indented(n));
    jline(line);
}

void
jflush(void)
{
    if(bbLength(jcode) > 0) {
        bbAppend(jcode,'\0');
        fputs(bbContents(jcode),stdout);
        fflush(stdout);
        bbClear(jcode);
    }
}

/* Result is a pool string or a constant => do not free*/
char*
jconst(Constant* ci)
{
    char tmp[64];
    tmp[0] = '\0';
    switch (ci->nctype) {
    case NC_CHAR:
	{
	    strcpy(tmp,"'");
	    escapifychar(ci->value.charv,tmp+1,'\'');
	    strcat(tmp,"'");
	}
	break;
    case NC_BYTE:
	    sprintf(tmp,"%hhd",ci->value.int8v);
	break;
    case NC_SHORT:
	sprintf(tmp,"%hd",ci->value.int16v);
	break;
    case NC_INT:
	sprintf(tmp,"%d",ci->value.int32v);
	break;
    case NC_FLOAT:
	sprintf(tmp,"%.8g",ci->value.floatv);
	break;
    case NC_DOUBLE:
	sprintf(tmp,"%.16g",ci->value.doublev);
	break;
    case NC_UBYTE:
	    sprintf(tmp,"%hhu",ci->value.uint8v);
	break;
    case NC_USHORT:
	sprintf(tmp,"%hu",ci->value.uint16v);
	break;
    case NC_UINT:
	sprintf(tmp,"%uL",ci->value.uint32v); /* upgrade to long */
	break;
    case NC_INT64:
	sprintf(tmp,"%lldL",ci->value.int64v);
	break;
    case NC_UINT64: {
	/* HACK to handle unsigned values */
	long long l = (long)ci->value.uint64v;
	sprintf(tmp,"%lldL",l);
	} break;
    case NC_ECONST:
	sprintf(tmp,"%s",jname(ci->value.enumv));
	break;
    case NC_STRING:
	{
	    char* escaped = escapify(ci->value.stringv.stringv,
				 '"',ci->value.stringv.len);
	    char* result = poolalloc(1+2+strlen(escaped));
	    strcpy(result,"\"");
	    strcat(result,escaped);
	    strcat(result,"\"");
	    return result;
	}
	break;
    case NC_OPAQUE: {
	char* bstring;
	char* p;
	int bslen;
	bslen=(4*ci->value.opaquev.len);
	bstring = poolalloc(bslen+2+1);
	strcpy(bstring,"\"");
	p = ci->value.opaquev.stringv;
	while(*p) {
	    strcat(bstring,"\\x");
	    strncat(bstring,p,2);	    	    
	    p += 2;	
	}
	strcat(bstring,"\"");
	return bstring;
	} break;

    default: PANIC1("ncstype: bad type code: %d",ci->nctype);
    }
    return pooldup(tmp); /*except for NC_STRING and NC_OPAQUE*/
}

void
jquotestring(Bytebuffer* databuf, char quote)
{
    char* escaped = jescapify(bbContents(databuf),'"',bbLength(databuf));
    bbClear(databuf);
    bbAppend(databuf,quote);
    if(escaped != NULL) bbCat(databuf,escaped);
    bbAppend(databuf,quote);
}

/* Compute the name for a given symbol*/
/* Cache in symbol->lname*/
const char*
jname(Symbol* sym)
{
    if(sym->lname == NULL) {
#ifdef USE_NETCDF4
	if(sym->subclass == NC_FIELD || sym->subclass == NC_ECONST) {
	     sym->lname = nulldup(jdecodify(sym->name));
	} else
#endif
	if(sym->objectclass == NC_ATT && sym->att.var != NULL) {
	    /* Attribute name must be prefixed with the cname of the*/
	    /* associated variable*/
	    char* lname;
	    lname = (char*)emalloc(strlen(sym->att.var->name)
					+strlen(sym->name)
					+1+1);
	    lname[0] = '\0';
            strcpy(lname,sym->att.var->name);
	    strcat(lname,"_");
	    strcat(lname,sym->name);
	    /* Now convert to java acceptable name */
	    sym->lname = nulldup(jdecodify(lname));
	} else {
            /* convert to language form*/
#ifdef USE_NETCDF4
            sym->lname = nulldup(jdecodify(jprefixed(sym->prefix,sym->name,"_")));
#else
            sym->lname = nulldup(jdecodify(sym->name)); /* convert to usable form*/
#endif
	}
    }
    return sym->lname;
}

#ifdef USE_NETCDF4
/* Result is pool alloc'd*/
static char*
jprefixed(List* prefix, char* suffix, char* separator)
{
    int slen;
    int plen;
    int i;
    char* result;

    ASSERT(suffix != NULL);
    plen = prefixlen(prefix);
    if(prefix == NULL || plen == 0) return pooldup(suffix);
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
#endif /*USE_NETCDF4*/


#endif /*ENABLE_JAVA*/
