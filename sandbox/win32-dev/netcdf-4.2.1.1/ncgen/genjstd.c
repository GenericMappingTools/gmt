/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genjstd.c,v 1.4 2010/05/17 23:26:45 dmh Exp $
 *********************************************************************/

#include "includes.h"

#undef TRACE

/*MNEMONIC*/
#define USEMEMORY 1

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward */
static void genjstd_definevardata(Symbol* vsym);
static void genjstd_primattribute(Symbol*, Bytebuffer*, unsigned long);

static void genjstd_defineattribute(Symbol* asym);
static void genjstd_definevardata(Symbol*);

static void computemaxunlimited(void);

/*
Global Bytebuffer into which to store the C code output;
periodically dumped to file by jflush().
Shared with genjjni.c
*/

Bytebuffer* jcode;

/*
 * Generate code for creating netCDF from in-memory structure.
 */
void
gen_ncjava_std(const char *filename)
{
    int idim, ivar, iatt, maxdims;
    int ndims, nvars, natts, ngatts, ngrps, ntyps;
    char* cmode_string;

    jcode = bbNew();
    bbSetalloc(jcode,C_MAX_STMT);

    ndims = listlength(dimdefs);
    nvars = listlength(vardefs);
    natts = listlength(attdefs);
    ngatts = listlength(gattdefs);
    ngrps = listlength(grpdefs);
    ntyps = listlength(typdefs);

    /* Construct the main class */
    jline("import java.util.*;");
    jline("import ucar.ma2.*;");
    jline("import ucar.nc2.*;");
    jline("import ucar.nc2.NetcdfFile.*;");

    jline("");
    jpartial("public class ");
    jline(mainname);
    jline("{");

    /* Now construct the main procedure*/

    jline("");
    jline("static public void main(String[] argv) throws Exception");
    jline("{");


    /* create necessary declarations */

    if (ndims > 0) {
	jline("");
	jlined(1,"/* dimension lengths */");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    if (dsym->dim.size == NC_UNLIMITED) {
		nprintf(stmt,sizeof(stmt),"%sfinal int %s_len = 0;",
			indented(1),jname(dsym));
	    } else {
		nprintf(stmt,sizeof(stmt),"%sfinal int %s_len = %lu;",
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

    jline("");
#ifdef DOTHROW
    jlined(1,"try {");
#endif

    /* create netCDF file, uses NC_CLOBBER mode */
    jline("");
    jlined(1,"/* enter define mode */");

    if (!cmode_modifier) {
	cmode_string = "NC_CLOBBER";
    } else if (cmode_modifier & NC_64BIT_OFFSET) {
	cmode_string = "NC_CLOBBER|NC_64BIT_OFFSET";
    } else {
        derror("unknown cmode modifier");
	cmode_string = "NC_CLOBBER";
    }

    nprintf(stmt,sizeof(stmt),
                "NetcdfFileWriteable ncfile = NetcdfFileWriteable.createNew(\"%s\", %s);",
		 filename,(nofill_flag?"false":"true"));
    jlined(1,stmt);
    jflush();
    
    /* define dimensions from info in dims array */
    if (ndims > 0) {
	jline("");
	jlined(1,"/* define dimensions */");
        for(idim = 0; idim < ndims; idim++) {
            Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    if(dsym->dim.size == NC_UNLIMITED) {
                nprintf(stmt,sizeof(stmt),"Dimension %s_dim = ncfile.addUnlimitedDimension(\"%s\");",
                    jname(dsym),jescapifyname(dsym->name));
	    } else {
                nprintf(stmt,sizeof(stmt),"Dimension %s_dim = ncfile.addDimension(\"%s\", %s_len);",
                    jname(dsym),jescapifyname(dsym->name), jname(dsym));
	    }
            jlined(1,stmt);
       }
       jflush();
    }

    /* define variables from info in vars array */
    if (nvars > 0) {
        jline("");
        jlined(1,"/* define variables */");
        for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            Symbol* basetype = vsym->typ.basetype;
            Dimset* dimset = &vsym->typ.dimset;
            jline("");
            nprintf(stmt,sizeof(stmt),"ArrayList %s_dimlist = new ArrayList();",
                                        jname(vsym));
            jlined(1,stmt);
            if(dimset->ndims > 0) {
                for(idim = 0; idim < dimset->ndims; idim++) {
                    Symbol* dsym = dimset->dimsyms[idim];
                    nprintf(stmt,sizeof(stmt),"%s_dimlist.add(%s_dim);",
                            jname(vsym),jname(dsym));
                    jlined(1,stmt);
                }
	    }
            nprintf(stmt,sizeof(stmt),
                            "ncfile.addVariable(\"%s\", DataType.%s, %s_dimlist);",
                            jescapifyname(vsym->name),
                            jtypeallcaps(basetype->typ.typecode),
                            jname(vsym));
            jlined(1,stmt);
        }
        jflush();
    }
        
    /* Define the global attributes*/
    if(ngatts > 0) {
        jline("");
        jlined(1,"/* assign global attributes */");
        for(iatt = 0; iatt < ngatts; iatt++) {
            Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
            genjstd_defineattribute(gasym);            
        }
        jline("");
        jflush();
    }
    
    /* Define the variable specific attributes*/
    if(natts > 0) {
        jline("");
        jlined(1,"/* assign per-variable attributes */");
        for(iatt = 0; iatt < natts; iatt++) {
            Symbol* asym = (Symbol*)listget(attdefs,iatt);
            genjstd_defineattribute(asym);
        }
        jline("");
        jflush();
    }

    jlined(1,"ncfile.create();"); /* equiv to nc_enddef */

    /* Load values into those variables with defined data */

    if(nvars > 0) {
        jline("");
        jlined(1,"/* assign variable data */");
        for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            if(vsym->data != NULL) genjstd_definevardata(vsym);
        }
        jline("");
        /* compute the max actual size of the unlimited dimension*/
        if(usingclassic) computemaxunlimited();
    }
    jflush();

}

void
cl_java_std(void)
{
    jlined(1,"ncfile.close();");
    jline("");
#ifdef DOTHROW
    jlined(1,"} catch(Exception e) {e.printStackTrace();};");
#endif

    jline("}"); /* main */

    jline("}"); /* class Main */

    jflush();
}

/*
 * Return java type name for netCDF type, given type code.
 */
static void
genjstd_defineattribute(Symbol* asym)
{
    unsigned long len;
    Datalist* list;
    Bytebuffer* code = NULL; /* capture other decls*/

    list = asym->data;
    if(list == NULL) len = 0;
    else len = asym->att.count;

    nprintf(stmt,sizeof(stmt),"/* attribute: %s */",asym->name);
    jlined(1,stmt);

    code = bbNew();

    genjstd_attrdata(asym,code);   

    /* Handle primitives separately */
    if(asym->typ.basetype->typ.typecode != NC_CHAR) commify(code);
    genjstd_primattribute(asym, code, len);
    bbFree(code);
}

static void
genjstd_primattribute(Symbol* asym, Bytebuffer* code, unsigned long len)
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
	jlined(1,"{");
	nprintf(stmt,sizeof(stmt),"%sArray data = Array.factory(%s.class, new int[]{%lu}, ",
		indented(1),
		jtype(basetype->typ.typecode),
		len);
	jpartial(stmt);
        jprint(code);
	jline(");");
	if(asym->att.var == NULL) {
            nprintf(stmt,sizeof(stmt),"ncfile.addGlobalAttribute(\"%s\",data);",
                jescapifyname(asym->name));
	} else {
            nprintf(stmt,sizeof(stmt),"ncfile.addVariableAttribute(\"%s\",\"%s\",data);",
		jescapifyname(asym->att.var->name),
                jescapifyname(asym->name));
	}
        jlined(1,stmt);
	jlined(1,"}");
        jflush();
        break;

    case NC_CHAR:
	if(asym->att.var == NULL) {
            nprintf(stmt,sizeof(stmt),"ncfile.addGlobalAttribute(\"%s\",%s);",
                jescapifyname(asym->name),
		bbContents(code));
	} else {
            nprintf(stmt,sizeof(stmt),"ncfile.addVariableAttribute(\"%s\",\"%s\",%s);",
		jescapifyname(asym->att.var->name),
                jescapifyname(asym->name),
		bbContents(code));
	}
        jlined(1,stmt);
        jflush();
        break;

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

static void
genjstd_definevardata(Symbol* vsym)
{
    Dimset* dimset = &vsym->typ.dimset;
    Symbol* basetype = vsym->typ.basetype;
    int rank = dimset->ndims;
    int isscalar = (dimset->ndims == 0);
    Bytebuffer* code;
    nc_type typecode = basetype->typ.typecode;

    if(vsym->data == NULL) return;

    code = bbNew();

    jlined(1,"{");

    /* Handle special cases first*/
    if(isscalar) {
        /* Construct the data Array */
        nprintf(stmt,sizeof(stmt),"Array%s.D0 data = new Array%s.D0();",
		jtypecap(typecode), jtypecap(typecode));

        jlined(1,stmt);
	/* Fill it */
	genjstd_scalardata(vsym,code);
	if(typecode == NC_CHAR) {
            nprintf(stmt,sizeof(stmt),"data.set(%s.charAt(0));",
		bbContents(code));
	} else {
            nprintf(stmt,sizeof(stmt),"data.set((%s)%s);",
		jtype(typecode),bbContents(code));
	}
	jlined(1,stmt);
     } else { /* Non-scalar*/
	int i;
        Bytebuffer* dimbuf = bbNew();

	/* Store the data */
        genjstd_arraydata(vsym,NULL,code);

        /* Construct the dimension set*/
	bbCat(dimbuf,"new int[]{");
	for(i=0;i<rank;i++) {
            Symbol* dsym = dimset->dimsyms[i];
	    char tmp[32];
	    if(i==0 && dsym->dim.size == NC_UNLIMITED)
	        nprintf(tmp,sizeof(tmp),"%lu",dsym->dim.unlimitedsize);
	    else
	        nprintf(tmp,sizeof(tmp),"%lu",dsym->dim.size);
	    if(i>0) {bbCat(dimbuf,", ");}
	    bbCat(dimbuf,tmp);
	}
	bbCat(dimbuf,"}");
        /* Construct the data array and capture its index */
	if(typecode == NC_CHAR)
            nprintf(stmt,sizeof(stmt),"%sString contents = ",
			indented(1));
	else
            nprintf(stmt,sizeof(stmt),"%s%s[] contents = new %s[] {",
			indented(1),jtype(typecode),jtype(typecode));
	jpartial(stmt);
	commify(code);
        jprint(code);
        if(typecode != NC_CHAR) jpartial("}");
        jline(";");
        nprintf(stmt,sizeof(stmt),"Array%s data = new Array%s(%s);",
		jtypecap(typecode), jtypecap(typecode), bbContents(dimbuf));
        jlined(1,stmt);
        jlined(1,"IndexIterator iter = data.getIndexIterator();");
        jlined(1,"int count = 0;");
        jlined(1,"while(iter.hasNext())");
	if(typecode == NC_CHAR)
            nprintf(stmt,sizeof(stmt),
			"iter.setCharNext(contents.charAt(count++));");
	else
            nprintf(stmt,sizeof(stmt),"iter.set%sNext(contents[count++]);",
                    jtypecap(typecode));
	jlined(2,stmt);
        bbFree(dimbuf);
    }
    /* do the actual write */
    nprintf(stmt,sizeof(stmt),"ncfile.write(\"%s\",data);",
		jescapifyname(vsym->name));
    jlined(1,stmt);
    bbFree(code);    
    jlined(1,"}");
    jflush();
}

const char* 
jtypeallcaps(nc_type type)
{
    switch (type) {
      case NC_CHAR: return "CHAR";
      case NC_BYTE: return "BYTE";
      case NC_SHORT: return "SHORT";
      case NC_INT: return "INT";
      case NC_FLOAT: return "FLOAT";
      case NC_DOUBLE: return "DOUBLE";
      case NC_UBYTE: return "LONG";
      case NC_USHORT: return "LONG";
      case NC_UINT: return "LONG";
      case NC_INT64: return "LONG";
      case NC_UINT64: return "LONG";
      case NC_STRING: return "STRING";
      default: PANIC1("ncctype: bad type code:%d",type);
    }
    return 0;
}

const char* 
jtypecap(nc_type type)
{
    switch (type) {
      case NC_CHAR: return "Char";
      case NC_BYTE: return "Byte";
      case NC_SHORT: return "Short";
      case NC_INT: return "Int";
      case NC_FLOAT: return "Float";
      case NC_DOUBLE: return "Double";
      case NC_UBYTE: return "Long";
      case NC_USHORT: return "Long";
      case NC_UINT: return "Long";
      case NC_INT64: return "Long";
      case NC_UINT64: return "Long";
      case NC_STRING: return "String";
      case NC_ENUM: return "Int";
      case NC_OPAQUE: return "String";
      default: PANIC1("ncctype: bad type code:%d",type);
    }
    return 0;
}

