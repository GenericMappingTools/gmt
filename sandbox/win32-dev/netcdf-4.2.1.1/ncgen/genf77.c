/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genf77.c,v 1.4 2010/05/17 23:26:44 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include "nciter.h"

#ifdef ENABLE_F77

#undef TRACE

/*MNEMONIC*/
#define USEMEMORY 1

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward */
static void genf77_definevardata(Symbol* vsym);
static void genf77_defineattr(Symbol* asym);
static void genf77_definevardata(Symbol*);

static void f77attrify(Symbol* asym, Bytebuffer* buf);
static const char* f77varncid(Symbol* vsym);
static const char* f77dimncid(Symbol* vsym);

static const char* nfstype(nc_type nctype);
static const char* nftype(nc_type type);
static const char* nfstype(nc_type nctype);
static const char* ncftype(nc_type type);
static const char* nfdtype(nc_type type);


static void f77skip(void);
static void f77comment(char* cmt);
static void f77fold(Bytebuffer* lines);
static void f77flush(void);

static void genf77_write(Symbol*, Bytebuffer*, Odometer*, int, int);

#ifdef USE_NETCDF4
static char* f77prefixed(List* prefix, char* suffix, char* separator);
#endif

/*
 * Generate code for creating netCDF from in-memory structure.
 */
void
gen_ncf77(const char *filename)
{
    int idim, ivar, iatt;
    int ndims, nvars, natts, ngatts, ngrps, ntyps;
    char* cmode_string;

    ndims = listlength(dimdefs);
    nvars = listlength(vardefs);
    natts = listlength(attdefs);
    ngatts = listlength(gattdefs);
    ngrps = listlength(grpdefs);
    ntyps = listlength(typdefs);

    /* Construct the main program */

    f77skip();
    bbprintf0(stmt,"program %s\n", mainname, filename);
    codedump(stmt);
    bbprintf0(stmt,"* input file %s", filename);
    codeline("include 'netcdf.inc'");
    f77comment("error status return");
    codeline("integer stat");
    f77comment("netCDF ncid");
    codeline("integer  ncid");

    /* create necessary declarations */

    if (ndims > 0) {
	f77skip();
	f77comment("dimension lengths");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    bbprintf0(stmt,"integer %s_len\n",f77name(dsym));
	    codedump(stmt);
	    if(dsym->dim.declsize == NC_UNLIMITED) {
	        bbprintf0(stmt,"parameter (%s_len = NF_UNLIMITED)\n",
			f77name(dsym));
	    } else {
		bbprintf0(stmt,"parameter (%s_len = %lu)\n",
			f77name(dsym),
			(unsigned long) dsym->dim.declsize);
	    }
	    codedump(stmt);
	}
    }
    f77flush();

    /* Now create the dimension id's */
    if (ndims > 0) {
	f77skip();
	f77comment("dimension ids");
	for(idim = 0; idim < ndims; idim++) {
	    Symbol* dsym = (Symbol*)listget(dimdefs,idim);
	    bbprintf0(stmt,"integer %s_dim\n",f77name(dsym));
	    codedump(stmt);
	}
    }

    if (nvars > 0) {
	f77skip();
	f77comment("variable ids");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    bbprintf0(stmt,"integer %s;\n", f77varncid(vsym));
	    codedump(stmt);
	}

	f77skip();
	f77comment("rank (number of dimensions) for each variable");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    bbprintf0(stmt,"integer %s_rank\n", f77name(vsym));
	    codedump(stmt);
	    bbprintf0(stmt,"parameter (%s_rank = %d)\n",
		    f77name(vsym),
		    vsym->typ.dimset.ndims);
	    codedump(stmt);
	}
        f77skip();
        f77comment("variable shapes");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    if(vsym->typ.dimset.ndims > 0) {
		bbprintf0(stmt,"integer %s_dims(%s_rank)\n",
			    f77name(vsym), f77name(vsym));
		codedump(stmt);
	    }
	}
	/* variable declarations (for scalar and fixed sized only) */
        f77skip();
        f77comment("variable declarations");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    nc_type typecode = vsym->typ.basetype->typ.typecode;
	    if(vsym->data == NULL) continue;
	    if(typecode == NC_CHAR) continue;
	    if(vsym->typ.dimset.ndims == 0) {/* scalar */
                bbprintf0(stmt,"%s %s\n",
			nfdtype(typecode),f77name(vsym));
                codedump(stmt);
	    } else if(vsym->typ.dimset.dimsyms[0]->dim.declsize != NC_UNLIMITED) {
		int i;
		Bytebuffer* dimstring = bbNew();
		Dimset* dimset = &vsym->typ.dimset;
                /* Compute the dimensions (in reverse order for fortran) */
                for(i=dimset->ndims-1;i>=0;i--) {
	            char tmp[32];
	            Symbol* dsym = dimset->dimsyms[i];
	            nprintf(tmp,sizeof(tmp)," %lu",
			(unsigned long)dsym->dim.declsize);
	            bbCat(dimstring,tmp);
		}
  	        commify(dimstring);
                bbprintf0(stmt,"%s %s(%s)\n",
			    nfdtype(typecode),
			    f77name(vsym),
			    bbContents(dimstring));
	        codedump(stmt);
                bbFree(dimstring);
	    }
	}
    }
    f77flush();

    /* F77 (as defined for ncgen3) requires per-type vectors for attributes */
    if(ngatts > 0 || natts > 0) {
	nc_type nctype;
	int pertypesizes[NC_DOUBLE+1];
	for(nctype=0;nctype<=NC_DOUBLE;nctype++) {pertypesizes[nctype] = 0;}
	if(ngatts > 0) {
    	    for(iatt = 0; iatt < ngatts; iatt++) {
	        Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
		int count = gasym->data->length;
		int typecode = gasym->typ.basetype->typ.typecode;
	        if(count == 0) continue;
		if(pertypesizes[typecode] < count)
		    pertypesizes[typecode] = count; /* keep max */
	    }
	}
	if(natts > 0) {
    	    for(iatt = 0; iatt < natts; iatt++) {
	        Symbol* asym = (Symbol*)listget(attdefs,iatt);
		int count = asym->data->length;
		int typecode = asym->typ.basetype->typ.typecode;
	        if(count == 0) continue;
		if(pertypesizes[typecode] < count)
		    pertypesizes[typecode] = count; /* keep max */
	    }
	}
	/* Now, define the per-type vectors */
        f77skip();
        f77comment("attribute vectors");
	for(nctype=NC_BYTE;nctype <= NC_DOUBLE;nctype++) {
	    char* basetype = "integer";
            if(nctype == NC_FLOAT) basetype = "real";
            else if(nctype == NC_DOUBLE) basetype = "double precision";
	    if(pertypesizes[nctype] > 0) {
	        bbprintf0(stmt,"%s %sval(%d)\n",
			basetype, ncftype(nctype),
			pertypesizes[nctype]);
	        codedump(stmt);
	    }
	}
    }

    /* create netCDF file, uses NC_CLOBBER mode */
    f77skip();
    f77skip();
    f77comment("enter define mode");

    if (!cmode_modifier) {
	cmode_string = "nf_clobber";
    } else if (cmode_modifier & NC_64BIT_OFFSET) {
	cmode_string = "nf_clobber|nf_64bit_offset";
    } else {
        derror("unknown cmode modifier: %d",cmode_modifier);
	cmode_string = "nf_clobber";
    }
    bbprintf0(stmt,"stat = nf_create('%s', %s, ncid);\n",
		 filename,cmode_string);
    codedump(stmt);
    codeline("call check_err(stat)");
    f77flush();
    
    /* define dimensions from info in dims array */
    if (ndims > 0) {
	f77skip();
	f77comment("define dimensions");
        for(idim = 0; idim < ndims; idim++) {
            Symbol* dsym = (Symbol*)listget(dimdefs,idim);
    	    bbprintf0(stmt,
		"stat = nf_def_dim(ncid, %s, %s_len, %s);\n",
                f77escapifyname(dsym->name), f77name(dsym), f77dimncid(dsym));
	    codedump(stmt);
	    codeline("call check_err(stat)");
       }
    }
    f77flush();

    /* define variables from info in vars array */
    if (nvars > 0) {
	f77skip();
	f77comment("define variables");
	for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            Symbol* basetype = vsym->typ.basetype;
	    Dimset* dimset = &vsym->typ.dimset;
	    f77skip();
	    if(dimset->ndims > 0) {
		/* Remember; FORTRAN dimension order is reversed */
	        for(idim = 0; idim < dimset->ndims; idim++) {
		    int reverse = (dimset->ndims - idim) - 1;
		    Symbol* dsym = dimset->dimsyms[reverse];
		    bbprintf0(stmt,
			    "%s_dims(%d) = %s\n",
			    f77name(vsym),
			    idim+1,
			    f77dimncid(dsym));
		    codedump(stmt);
		}
	    }
	    bbprintf0(stmt,
			"stat = nf_def_var(ncid, %s, %s, %s_rank, %s, %s);\n",
			f77escapifyname(vsym->name),
			nftype(basetype->typ.typecode),
			f77name(vsym),
			(dimset->ndims == 0?"0":poolcat(f77name(vsym),"_dims")),
			f77varncid(vsym));
	    codedump(stmt);
	    codeline("call check_err(stat)");
	}
    }
    f77flush();
    
    /* Define the global attributes*/
    if(ngatts > 0) {
	f77skip();
	f77comment("assign global attributes");
	for(iatt = 0; iatt < ngatts; iatt++) {
	    Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
	    genf77_defineattr(gasym);	    
	}
	f77skip();
    }
    f77flush();
    
    /* Define the variable specific attributes*/
    if(natts > 0) {
	f77skip();
	f77comment("assign per-variable attributes");
	for(iatt = 0; iatt < natts; iatt++) {
	    Symbol* asym = (Symbol*)listget(attdefs,iatt);
	    genf77_defineattr(asym);
	}
	f77skip();
    }
    f77flush();

    if (nofill_flag) {
        f77comment("don't initialize variables with fill values");
	codeline("stat = nf_set_fill(ncid, NC_NOFILL, 0);");
	codeline("call check_err(stat)");
    }

    f77skip();
    f77comment("leave define mode");
    codeline("stat = nf_enddef(ncid);");
    codeline("call check_err(stat)");
    f77flush();

    /* Assign scalar variable data and non-unlimited arrays in-line */
    if(nvars > 0) {
	f77skip();
	f77skip();
	f77comment("assign scalar and fixed dimension variable data");
        for(ivar = 0; ivar < nvars; ivar++) {
	    Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    if(vsym->data == NULL) continue;
	    if(vsym->typ.dimset.ndims == 0)
	        genf77_definevardata(vsym);
	}
	f77skip();
    }

    /* Invoke write procedures */
    if(nvars > 0) {
	f77skip();
	f77skip();
	f77comment("perform variable data writes");
        for(ivar = 0; ivar < nvars; ivar++) {
	    Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    /* Call the procedures for writing unlimited variables */
	    if(vsym->data != NULL
		&& vsym->typ.dimset.ndims > 0) {
	        bbprintf0(stmt,"call write_%s(ncid,%s_id)\n",
	        		f77name(vsym),f77name(vsym));
		codedump(stmt);
	    }
	}
    }

    /* Close the file */
    codeline("stat = nf_close(ncid)");
    codeline("call check_err(stat)");
    codeline("end");

    /* Generate the write procedures */
    if(nvars > 0) {
	f77skip();
        for(ivar = 0; ivar < nvars; ivar++) {
	    Symbol* vsym = (Symbol*)listget(vardefs,ivar);
	    if(vsym->data == NULL) continue;
	    if(vsym->typ.dimset.ndims > 0)
	        genf77_definevardata(vsym);
	}
	f77skip();
    }
    f77flush();

    /* Generate the check_err procedure */
    f77skip();
    codeline("subroutine check_err(stat)");
    codeline("integer stat");
    codeline("include 'netcdf.inc'");
    codeline("if (stat .ne. NF_NOERR) then");
    codeline("print *, nf_strerror(stat)");
    codeline("stop");
    codeline("endif");
    codeline("end");
    f77flush();

}

void
cl_f77(void)
{
   /* already done above */
}

/* Compute the name for a given var's id*/
/* Watch out: the result is a static*/
static const char*
f77varncid(Symbol* vsym)
{
    const char* tmp1;
    char* vartmp;
    tmp1 = f77name(vsym);
    vartmp = poolalloc(strlen(tmp1)+strlen("_id")+1);
    strcpy(vartmp,tmp1);
    strcat(vartmp,"_id");
    return vartmp;
}

/* Compute the name for a given dim's id*/
/* Watch out: the result is a static*/
static const char*
f77dimncid(Symbol* dsym)
{
    const char* tmp1;
    char* dimtmp;
    tmp1 = f77name(dsym);
    dimtmp = poolalloc(strlen(tmp1)+strlen("_dim")+1);
    strcpy(dimtmp,tmp1);
    strcat(dimtmp,"_dim");
    return dimtmp;
}

/* Compute the name for a given type*/
const char*
f77typename(Symbol* tsym)
{
    const char* name;
    ASSERT(tsym->objectclass == NC_TYPE);
    if(tsym->subclass == NC_PRIM)
	name = nftype(tsym->typ.typecode);
    else
        name = f77name(tsym);
    return name;
}

/* Compute the name for a given symbol*/
/* Cache in symbol->lname*/
const char*
f77name(Symbol* sym)
{
    if(sym->lname == NULL) {
	char* name = pooldup(sym->name);
#ifdef USE_NETCDF4
	if(sym->subclass == NC_FIELD || sym->subclass == NC_ECONST) {
	     sym->lname = nulldup(decodify(name));
	} else
#endif
	if(sym->objectclass == NC_ATT && sym->att.var != NULL) {
	    /* Attribute name must be prefixed with the f77name of the*/
	    /* associated variable*/
	    const char* vname = f77name(sym->att.var);
	    const char* aname = decodify(name);
	    sym->lname = (char*)emalloc(strlen(vname)
					+strlen(aname)
					+1+1);
	    sym->lname[0] = '\0';
            strcpy(sym->lname,vname);
	    strcat(sym->lname,"_");
	    strcat(sym->lname,aname);
	} else {
            /* convert to language form*/
#ifdef USE_NETCDF4
            sym->lname = nulldup(decodify(f77prefixed(sym->prefix,name,"_")));
#else
            sym->lname = nulldup(decodify(name)); /* convert to usable form*/
#endif
	}
    }
    return sym->lname;
}

static void
genf77_defineattr(Symbol* asym)
{
    unsigned long len;
    Datalist* list;
    Symbol* basetype = asym->typ.basetype;
    Bytebuffer* code = NULL; /* capture other decls*/

    list = asym->data;
    len = list==NULL?0:list->length;

    bbprintf0(stmt,"* define %s\n",asym->name);
    codedump(stmt);

    code = bbNew();
    f77data_attrdata(asym,code);	

    /* Use the specialized put_att_XX routines if possible*/
    switch (basetype->typ.typecode) {
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_FLOAT:
    case NC_DOUBLE:
	f77attrify(asym,code);
	codedump(code);
        bbClear(code);
        bbprintf0(stmt,"stat = nf_put_att_%s(ncid, %s, %s, %s, %lu, %sval)\n",
		nfstype(basetype->typ.typecode),
		(asym->att.var == NULL?"NF_GLOBAL"
			              :f77varncid(asym->att.var)),
		f77escapifyname(asym->name),
		nftype(basetype->typ.typecode),		
	 	len,
		ncftype(basetype->typ.typecode));
	codedump(stmt);
	break;

    case NC_CHAR:
	len = bbLength(code);
	f77quotestring(code);	
        bbprintf0(stmt,"stat = nf_put_att_text(ncid, %s, %s, %lu, ",
		(asym->att.var == NULL?"NF_GLOBAL"
			              :f77varncid(asym->att.var)),
		f77escapifyname(asym->name),
	 	len);
	codedump(stmt);
	codedump(code);
	codeline(")");
	break;


    default: /* User defined type */
        verror("Non-classic type: %s",nctypename(basetype->typ.typecode));
	break;
    }

    bbFree(code);
    codeline("call check_err(stat)");
}

static void
f77skip(void)
{
    codeline("");
}

static void
f77comment(char* cmt)
{
    codepartial("* ");
    codeline(cmt);
}

static void
f77fold(Bytebuffer* lines)
{
    char* s;
    char* line0;
    char* linen;
    int linesize;
    static char trimchars[] = " \t\r\n";

    s = bbDup(lines);
    bbClear(lines);
    linesize = 0;
    line0 = s;
    /* Start by trimming leading blanks and empty lines */
    while(*line0 && strchr(trimchars,*line0) != NULL) line0++;
    if(*line0 == '\0') return;
    for(;;) {
	size_t linelen;
	linen = line0;
	/* collect a single line */
	while(*linen != '\n' && *linen != '\0') linen++;
	if(*linen == '\0') break;
	linen++; /* include trailing newline */
	linelen = (linen - line0);
	/* handle comments and empty lines */
	if(*line0 == '*' || linelen == 1) {
	    bbAppendn(lines,line0,linelen);
	    line0 = linen;
	    continue;
	}
	/* Not a comment */
        /* check to see if we need to fold it (watch out for newline)*/
	if(linelen <= (F77_MAX_STMT+1)) { /* no folding needed */
	    bbCat(lines,"      "); /*indent*/
	    bbAppendn(lines,line0,linelen);
	    line0 = linen;
	    continue;
	}	 
	/* We need to fold */
        bbCat(lines,"      "); /*indent first line */
	while(linelen > F77_MAX_STMT) {
	    int incr = F77_MAX_STMT;
	    /* Check to ensure we are folding at a legal point */
	    if(*(line0+(incr-1)) == '\\') incr--;
	    bbAppendn(lines,line0,incr);
	    bbCat(lines,"\n     1"); /* comment extender */
	    line0 += incr;
	    linelen -= incr;
	}
	/* Do last part of the line */
	bbAppendn(lines,line0,linelen);
	line0 = linen;
    }
}

static void
f77flush(void)
{
    if(bbLength(codebuffer) > 0) {
        bbNull(codebuffer);
	f77fold(codebuffer);
        codeflush();
    }
}

static char* f77attrifyr(Symbol*, char* p, Bytebuffer* buf);

static void
f77attrify(Symbol* asym, Bytebuffer* buf)
{
    char* list,*p;

    if(bbLength(buf) == 0) return;
    list = bbDup(buf);
    p = list;
    bbClear(buf);
    f77attrifyr(asym,p,buf);
    bbNull(buf);
    efree(list);
}

static char*
f77attrifyr(Symbol* asym, char* p, Bytebuffer* buf)
{
    Symbol* basetype = asym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;
    int c;
    int index;
    char where[1024];

    nprintf(where,sizeof(where),"%sval",ncftype(typecode));
    for(index=1;(c=*p);) {
	if(c == ' ' || c == ',') {p++; continue;}
	bbprintf0(stmt,"%s(%d) = ",where,index);
	bbCatbuf(buf,stmt);
        p=word(p,buf);
        bbCat(buf,"\n");
	index++;
    }    
    return p;
}

#ifdef USE_NETCDF4
/* Result is pool alloc'd*/
static char*
f77prefixed(List* prefix, char* suffix, char* separator)
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

/* return FORTRAN name for netCDF type, given type code */
static const char*
nftype(nc_type type)
{
    switch (type) {
      case NC_CHAR: return "nf_char";
      case NC_BYTE: return "nf_byte";
      case NC_SHORT: return "nf_short";
      case NC_INT: return "nf_int";
      case NC_FLOAT: return "nf_float";
      case NC_DOUBLE: return "nf_double";
      default: PANIC("nctype: bad type code");
    }
    return NULL;
}

/* return FORTRAN declaration type for given type code */
static const char*
nfdtype(nc_type type)
{
    switch (type) {
      case NC_CHAR: return "integer";
      case NC_BYTE: return "integer";
      case NC_SHORT: return "integer";
      case NC_INT: return "integer";
      case NC_FLOAT: return "real ";
      case NC_DOUBLE: return "double precision";
      default: PANIC("nctype: bad type code");
    }
    return NULL;
}

/*
 * Return proper _put_var_ suffix for given nc_type
 */
static const char* 
nfstype(nc_type nctype)
{
    switch (nctype) {
      case NC_CHAR:
	return "text";
      case NC_BYTE:
	return "int";
      case NC_SHORT:
	return "int";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "real";
      case NC_DOUBLE:
	return "double";
      default:
	derror("ncstype: bad type code: %d",nctype);
	return 0;
    }
}

/*
 * Return FORTRAN type name for netCDF attribute type
 */
static const char* 
ncftype(nc_type type)
{
    switch (type) {
      case NC_CHAR:
	return "text";
      case NC_BYTE:
	return "int1";
      case NC_SHORT:
	return "int2";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "real";
      case NC_DOUBLE:
	return "double";
      default:
	PANIC1("ncctype: bad type code:%d",type);
    }
    return 0;
}

static void
genf77_definevardata(Symbol* vsym)
{
    Dimset* dimset = &vsym->typ.dimset;
    int isscalar = (dimset->ndims == 0);
    Bytebuffer* code = NULL;
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
        genf77_write(vsym,code,NULL,0,0);
    } else { /* not character constant */
        src = datalist2src(vsym->data);
        fillsrc = vsym->var.special._Fillvalue;
    
        /* Handle special cases first*/
        if(isscalar) {
            f77data_basetype(vsym->typ.basetype,src,code,fillsrc);
            commify(code);
            genf77_write(vsym,code,NULL,1,0);
        } else { /* Non-scalar*/
            int index;
            /* Create an iterator to generate blocks of data */
            nc_get_iter(vsym,nciterbuffersize,&iter);
            /* Fill in the local odometer instance */
            odom = newodometer(&vsym->typ.dimset,NULL,NULL);
            for(index=0;;index++) {
                nelems=nc_next_iter(&iter,odom->start,odom->count);
                if(nelems == 0) break;
                f77data_array(vsym,code,src,odom,/*index=*/0,fillsrc);
                genf77_write(vsym,code,odom,0,index);
            }
        }
        odometerfree(odom);
    }
    bbFree(code);
}

static void
genf77_write(Symbol* vsym, Bytebuffer* code, Odometer* odom, int isscalar,
		int index)
{
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    int typecode = vsym->typ.basetype->typ.typecode;
    int chartype = (typecode == NC_CHAR);
    int i;

    if(isscalar) {
	if(chartype) {
            bbprintf0(stmt,"stat = nf_put_var_%s(ncid, %s, %s)\n",
	        nfstype(typecode),
		f77varncid(vsym),
		bbContents(code));
            codedump(stmt);
	} else {
            bbprintf0(stmt,"data %s /%s/\n",
			    f77name(vsym),bbContents(code));
	    codedump(stmt);
            bbprintf0(stmt,"stat = nf_put_var_%s(ncid, %s, %s)\n",
	        nfstype(typecode),
		f77varncid(vsym),
		f77name(vsym));
            codedump(stmt);
	}
        codeline("call check_err(stat)");
	f77skip();
    } else { /* array */
        char* dimstring;

        /* Construct the procedure body */
        f77skip();
	if(index == 0)
            bbprintf0(stmt,"subroutine write_%s(ncid,%s_id)\n",
                        f77name(vsym),f77name(vsym));
	else
            bbprintf0(stmt,"subroutine write_%s_%d(ncid,%s_id)\n",
                        f77name(vsym),f77name(vsym),index);
        codedump(stmt);
        codeline("integer ncid");
        bbprintf0(stmt,"integer %s_id\n",f77name(vsym));
        codedump(stmt);
        codeline("include 'netcdf.inc'");
        codeline("integer stat");
        f77skip();
        bbprintf0(stmt,"integer %s_start(%u)\n",
                        f77name(vsym),(unsigned int)rank);
        codedump(stmt);
        bbprintf0(stmt,"integer %s_count(%u)\n",
                        f77name(vsym),(unsigned int)rank);
        codedump(stmt);
        f77skip();

	if(typecode != NC_CHAR) {
            /* Compute the dimensions (in reverse order for fortran) */
	    bbClear(stmt);
            for(i=rank-1;i>=0;i--) {
                char tmp[32];
                nprintf(tmp,sizeof(tmp),"%s%lu",
			(i==(rank-1)?"":","),
			odom->count[i]);
                bbCat(stmt,tmp);
            }
            dimstring = bbDup(stmt);
            commify(code);

            bbprintf0(stmt,"%s %s(%s)\n",
                                nfdtype(typecode),
                                f77name(vsym),
                                dimstring);
            efree(dimstring);
	    codedump(stmt);

            /* Generate the data // statement */
            bbprintf0(stmt,"data %s /",f77name(vsym));
            bbCatbuf(stmt,code);
            bbCat(stmt,"/\n");
	    codedump(stmt);
        }

        /* Set the values for the start and count sets
           but in reverse order
        */
        for(i=0;i<dimset->ndims;i++) {
            int reverse = (dimset->ndims - i) - 1;
            bbprintf0(stmt,"%s_start(%d) = %lu\n",
	            f77name(vsym),
	            i+1,
	            odom->start[reverse]+1); /* +1 for FORTRAN */
            codedump(stmt);
        }
        for(i=0;i<dimset->ndims;i++) {
            int reverse = (dimset->ndims - i) - 1;
            bbprintf0(stmt,"%s_count(%d) = %lu\n",
                f77name(vsym),
                i+1,
                odom->count[reverse]);
            codedump(stmt);
        }
        bbprintf0(stmt,"stat = nf_put_vara_%s(ncid, %s, %s_start, %s_count, ",
                nfstype(typecode),
                f77varncid(vsym),
                f77name(vsym),
                f77name(vsym));
        codedump(stmt);
        if(typecode == NC_CHAR) {
            f77quotestring(code);
            codedump(code);
        } else {
            codeprintf("%s",f77name(vsym));
        }
        codeline(")");
        codeline("call check_err(stat)");
        /* Close off the procedure */
        codeline("end");
    }
}

#endif /*ENABLE_F77*/
