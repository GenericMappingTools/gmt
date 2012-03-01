/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genj.c,v 1.2 2010/05/17 23:26:44 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include "nciter.h"

#ifdef ENABLE_JAVA

#undef EXCEPTWRAP

#undef TRACE

/*MNEMONIC*/
#define USEMEMORY 1

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward */
static void genj_definevardata(Symbol* vsym);

static void genj_defineattribute(Symbol* asym);
static void genj_definevardata(Symbol*);

static void genj_write(Symbol*, Bytebuffer*, Odometer*, int, int);

static const char* jtypeallcaps(nc_type type);
static const char* jtypecap(nc_type type);
static const char* jtype(nc_type type);
static const char* jarraytype(nc_type type);
static const char* jname(Symbol* sym);

static void computemaxunlimited(void);

/*
 * Generate code for creating netCDF from in-memory structure.
 */
void
gen_ncjava(const char *filename)
{
    int idim, ivar, iatt, maxdims;
    int ndims, nvars, natts, ngatts, ngrps, ntyps;
    char* cmode_string;

    ndims = listlength(dimdefs);
    nvars = listlength(vardefs);
    natts = listlength(attdefs);
    ngatts = listlength(gattdefs);
    ngrps = listlength(grpdefs);
    ntyps = listlength(typdefs);

    /* Construct the main class */
    codeline("import java.util.*;");
    codeline("import ucar.ma2.*;");
    codeline("import ucar.nc2.*;");
    codeline("import ucar.nc2.NetcdfFile.*;");

    codeline("");
    codepartial("public class ");
    codeline(mainname);
    codeline("{");

    /* Now construct the main procedure*/

    codeline("");
    codeline("static public void main(String[] argv) throws Exception");
    codeline("{");

    /* create necessary declarations */

    if(ndims > 0) {
	codeline("");
	codelined(1,"/* dimension lengths */");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    if(dsym->dim.declsize == NC_UNLIMITED) {
		bbprintf0(stmt,"%sfinal int %s_len = 0;\n",
			indented(1),jname(dsym));
	    } else {
		bbprintf0(stmt,"%sfinal int %s_len = %lu;\n",
			indented(1),
			jname(dsym),
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

    codeline("");
#ifdef EXCEPTWRAP
    codelined(1,"try {");
#endif

    /* create netCDF file, uses NC_CLOBBER mode */
    codeline("");
    codelined(1,"/* enter define mode */");

    if(!cmode_modifier) {
	cmode_string = "NC_CLOBBER";
    } else if(cmode_modifier & NC_64BIT_OFFSET) {
	cmode_string = "NC_CLOBBER|NC_64BIT_OFFSET";
    } else {
        derror("unknown cmode modifier");
	cmode_string = "NC_CLOBBER";
    }

    bbprintf0(stmt,
                "%sNetcdfFileWriteable ncfile = NetcdfFileWriteable.createNew(\"%s\", %s);\n",
		 indented(1),filename,(nofill_flag?"false":"true"));
    codedump(stmt);
    codeflush();
    
    /* define dimensions from info in dims array */
    if(ndims > 0) {
	codeline("");
	codelined(1,"/* define dimensions */");
        for(idim = 0; idim < ndims; idim++) {
            Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    if(dsym->dim.declsize == NC_UNLIMITED) {
                bbprintf0(stmt,"%sDimension %s_dim = ncfile.addUnlimitedDimension(\"%s\");\n",
                    indented(1),jname(dsym),jescapifyname(dsym->name));
	    } else {
                bbprintf0(stmt,"%sDimension %s_dim = ncfile.addDimension(\"%s\", %s_len);\n",
                    indented(1),jname(dsym),jescapifyname(dsym->name), jname(dsym));
	    }
            codedump(stmt);
       }
       codeflush();
    }

    /* define variables from info in vars array */
    if(nvars > 0) {
        codeline("");
        codelined(1,"/* define variables */");
        for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            Symbol* basetype = vsym->typ.basetype;
            Dimset* dimset = &vsym->typ.dimset;
            codeline("");
            bbprintf0(stmt,"%sArrayList %s_dimlist = new ArrayList();\n",
                                        indented(1),jname(vsym));
            codedump(stmt);
            if(dimset->ndims > 0) {
                for(idim = 0; idim < dimset->ndims; idim++) {
                    Symbol* dsym = dimset->dimsyms[idim];
                    bbprintf0(stmt,"%s%s_dimlist.add(%s_dim);\n",
                            indented(1),jname(vsym),jname(dsym));
                    codedump(stmt);
                }
	    }
            bbprintf0(stmt,
                            "%sncfile.addVariable(\"%s\", DataType.%s, %s_dimlist);\n",
			    indented(1),
                            jescapifyname(vsym->name),
                            jtypeallcaps(basetype->typ.typecode),
                            jname(vsym));
            codedump(stmt);
        }
        codeflush();
    }
        
    /* Define the global attributes*/
    if(ngatts > 0) {
        codeline("");
        codelined(1,"/* assign global attributes */");
        for(iatt = 0; iatt < ngatts; iatt++) {
            Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
            genj_defineattribute(gasym);            
        }
        codeline("");
        codeflush();
    }
    
    /* Define the variable specific attributes*/
    if(natts > 0) {
        codeline("");
        codelined(1,"/* assign per-variable attributes */");
        for(iatt = 0; iatt < natts; iatt++) {
            Symbol* asym = (Symbol*)listget(attdefs,iatt);
            genj_defineattribute(asym);
        }
        codeline("");
        codeflush();
    }

    codelined(1,"ncfile.create();"); /* equiv to nc_enddef */

    /* Load values into those variables with defined data */

    if(nvars > 0) {
        codeline("");
        codelined(1,"/* assign variable data */");
        for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            if(vsym->data != NULL) genj_definevardata(vsym);
        }
        codeline("");
        /* compute the max actual size of the unlimited dimension*/
        if(usingclassic) computemaxunlimited();
    }
    codeflush();

}

void
cl_java(void)
{
    codelined(1,"ncfile.close();");
    codeline("");
#ifdef EXCEPTIONWRAP
    codelined(1,"} catch(Exception e) {e.printStackTrace();};");
#endif
    codelined(1,"}"); /* main */
    codeline("}"); /* class Main */
    codeflush();
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
        if(dim->dim.declsize != NC_UNLIMITED) continue; /* var does not use unlimited*/
        if(var->typ.dimset.dimsyms[0]->dim.declsize > maxsize)
            maxsize = var->typ.dimset.dimsyms[0]->dim.declsize;
    }
}

/*
 * Return java type name for netCDF type, given type code.
 */
static void
genj_defineattribute(Symbol* asym)
{
    unsigned long len;
    Symbol* basetype = asym->typ.basetype;
    Datalist* list;
    Bytebuffer* code = NULL; /* capture other decls*/
    nc_type typecode = basetype->typ.typecode;

    list = asym->data;
    len = list == NULL?0:list->length;

    codeprintf("%s/* attribute: %s */\n",indented(1),asym->name);

    code = bbNew();

    if(typecode == NC_CHAR) {
        gen_charattr(asym,code);
    } else {
        Datasrc* src = datalist2src(asym->data);
        while(srcmore(src)) {
            jdata_basetype(asym->typ.basetype,src,code,NULL);
	}
    }

    /* Handle NC_CHAR specially */
    if(typecode == NC_CHAR) {
        /* revise the length count */
        len = bbLength(code);
        if(len == 0) {
	    bbAppend(code,'\0'); len++;
	    bbClear(code);
	    bbCat(code,"\"\"");
	} else
            jquotestring(code,'"');
	bbNull(code);
    } else {
        char* code2;
	commify(code);
        /* Convert to constant */
        code2 = bbDup(code);
        bbClear(code);
        bbprintf0(stmt,"new %s[]",
                jarraytype(typecode));
        bbCatbuf(code,stmt);
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
	codelined(1,"{");
	bbprintf0(stmt,"%sArray data = Array.factory(%s.class, new int[]{%lu}, ",
		indented(1),
		jtype(basetype->typ.typecode),
		len);
	codedump(stmt);
        codedump(code);
	codeline(");");
	if(asym->att.var == NULL) {
            bbprintf0(stmt,"%sncfile.addGlobalAttribute(\"%s\",data);\n",
                indented(1),jescapifyname(asym->name));
	} else {
            bbprintf0(stmt,"%sncfile.addVariableAttribute(\"%s\",\"%s\",data);\n",
		indented(1),
		jescapifyname(asym->att.var->name),
                jescapifyname(asym->name));
	}
        codedump(stmt);
	codelined(1,"}");
        codeflush();
        break;

    case NC_CHAR:
	if(asym->att.var == NULL) {
            bbprintf0(stmt,"%sncfile.addGlobalAttribute(\"%s\",%s);\n",
		indented(1),
                jescapifyname(asym->name),
		bbContents(code));
	} else {
            bbprintf0(stmt,"%sncfile.addVariableAttribute(\"%s\",\"%s\",%s);\n",
		indented(1),
		jescapifyname(asym->att.var->name),
                jescapifyname(asym->name),
		bbContents(code));
	}
        codedump(stmt);
        codeflush();
        break;

    default: break;
    }
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

static const char* 
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
static const char*
jname(Symbol* sym)
{
    if(sym->lname == NULL) {
	if(sym->objectclass == NC_ATT && sym->att.var != NULL) {
	    /* Attribute name must be prefixed with the jname of the*/
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
            sym->lname = nulldup(jdecodify(sym->name)); /* convert to usable form*/
	}
    }
    return sym->lname;
}


/*
 * Return java type name for netCDF type, given type code.
 */
static const char* 
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
static const char* 
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

static void
genj_definevardata(Symbol* vsym)
{
    Dimset* dimset = &vsym->typ.dimset;
    int isscalar = (dimset->ndims == 0);
    Bytebuffer* code;
    Datasrc* src = NULL;
    Datalist* fillsrc = NULL;
    nciter_t iter;
    Odometer* odom = NULL;
    size_t nelems;
    int chartype = (vsym->typ.basetype->typ.typecode == NC_CHAR);

    if(vsym->data == NULL) return;

    code = bbNew();
    /* give the buffer a running start to be large enough*/
    bbSetalloc(code, nciterbuffersize);

    if(!isscalar && chartype) {
        gen_chararray(vsym,code,fillsrc);
        genj_write(vsym,code,NULL,0,0);
    } else { /* not character constant */
        src = datalist2src(vsym->data);
        fillsrc = vsym->var.special._Fillvalue;
    
        /* Handle special cases first*/
        if(isscalar) {
            jdata_basetype(vsym->typ.basetype,src,code,fillsrc);
            commify(code);
            genj_write(vsym,code,NULL,1,0);
         } else { /* Non-scalar*/
            int index;
            /* Create an iterator to generate blocks of data */
            nc_get_iter(vsym,nciterbuffersize,&iter);
            /* Fill in the local odometer instance */
            odom = newodometer(&vsym->typ.dimset,NULL,NULL);
            for(index=0;;index++) {
                nelems=nc_next_iter(&iter,odom->start,odom->count);
                if(nelems == 0) break;
                jdata_array(vsym,code,src,odom,/*index=*/0,fillsrc);
                genj_write(vsym,code,odom,0,index);
            }
        }
        odometerfree(odom);
    }
    bbFree(code);
}

static void
genj_write(Symbol* vsym, Bytebuffer* code, Odometer* odom, int isscalar, int index)
{
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    int typecode = vsym->typ.basetype->typ.typecode;
    int chartype = (typecode == NC_CHAR);
    int i;

    codeline("");
    codelined(1,"{"); /* Enclose in {...} for scoping */

    if(isscalar) {
        /* Construct the data Array */
        bbprintf0(stmt,"%sArray%s.D0 data = new Array%s.D0();\n",
		indented(1),jtypecap(typecode), jtypecap(typecode));
        codedump(stmt);
	if(chartype) {
#ifdef IGNORE
	    jquotestring(code,'"');
            bbprintf0(stmt,"%sdata.set(%s.charAt(0));\n",
		indented(1),bbContents(code));
#else
            bbprintf0(stmt,"%sdata.set((char)%s);\n",
		indented(1),bbContents(code));
#endif
	} else {
            bbprintf0(stmt,"%sdata.set((%s)%s);\n",
		indented(1),jtype(typecode),bbContents(code));
	}
	codedump(stmt);
        /* do the actual write */
        bbprintf0(stmt,"%sncfile.write(\"%s\",data);\n",
		indented(1),jescapifyname(vsym->name));
	codedump(stmt);
    } else { /* array */
	Bytebuffer* dimbuf = bbNew();
        /* Construct the dimension set*/
	bbCat(dimbuf,"new int[]{");
	for(i=0;i<rank;i++) {
            Symbol* dsym = dimset->dimsyms[i];
	    char tmp[32];
	    if(i==0 && dsym->dim.declsize == NC_UNLIMITED)
	        nprintf(tmp,sizeof(tmp),"%lu",dsym->dim.unlimitedsize);
	    else
	        nprintf(tmp,sizeof(tmp),"%lu",dsym->dim.declsize);
	    if(i>0) {bbCat(dimbuf,", ");}
	    bbCat(dimbuf,tmp);
	}
	bbCat(dimbuf,"}");
        /* Construct the data array and capture its index */
	if(typecode == NC_CHAR) {
	    jquotestring(code,'"');
            bbprintf0(stmt,"%sString contents = ",
			indented(1));
	} else {
            bbprintf0(stmt,"%s%s[] contents = new %s[] {",
			indented(1),jtype(typecode),jtype(typecode));
	    commify(code);
	}
	codedump(stmt);
        codedump(code);
        if(typecode != NC_CHAR) codepartial("}");
        codeline(";");
        bbprintf0(stmt,"%sArray%s data = new Array%s(%s);\n",
		indented(1),
		jtypecap(typecode),
		jtypecap(typecode),
		bbContents(dimbuf));
        codedump(stmt);
        codelined(1,"IndexIterator iter = data.getIndexIterator();");
        codelined(1,"int count = 0;");
        codelined(1,"while(iter.hasNext())");
	if(typecode == NC_CHAR)
            bbprintf0(stmt,
			"%siter.setCharNext(contents.charAt(count++));\n",indented(2));
	else
            bbprintf0(stmt,"%siter.set%sNext(contents[count++]);\n",
                    indented(2),jtypecap(typecode));
	codedump(stmt);
        bbFree(dimbuf);
	/* Construct the origin set from the odometer start set */
        bbprintf0(stmt,"%sint[] origin = new int[]{",indented(1));
	for(i=0;i<rank;i++) {
	    bbprintf(stmt,"%s%lu",(i>0?", ":""),odom->start[i]);
	}
	bbCat(stmt,"};\n");
	codedump(stmt);
        /* do the actual write */
        bbprintf0(stmt,"%sncfile.write(\"%s\",origin,data);\n",
		indented(1),jescapifyname(vsym->name));
	codedump(stmt);
    }
    codelined(1,"}"); /* Enclose in {...} for scoping */
}
#endif /*ENABLE_JAVA*/
