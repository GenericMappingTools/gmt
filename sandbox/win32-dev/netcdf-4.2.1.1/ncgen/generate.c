/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"
#include "nciter.h"
#include "odom.h"
#include "offsets.h"

/**************************************************/
/* Code for generating data lists*/
/**************************************************/
/* For datalist constant rules: see the rules on the man page */

/* Forward*/
static void generate_array(Symbol*,Bytebuffer*,Datalist*,Generator*,Writer);
static void generate_arrayr(Symbol*,Bytebuffer*,Datalist*,Odometer*,int,Datalist*,Generator*);
static void generate_primdata(Symbol*, Constant*, Bytebuffer*, Datalist* fillsrc, Generator*);
static void generate_fieldarray(Symbol*, Constant*, Dimset*, Bytebuffer*, Datalist* fillsrc, Generator*);

/* Mnemonics */
#define VLENLIST1
#define FIELDARRAY 1

/**************************************************/
/* Generator general procedures */

int
generator_getstate(Generator* generator ,void** statep)
{
    if(statep) *statep = (void*)generator->state;
    return 1;
}

int generator_reset(Generator* generator, void* state)
{
    generator->state = state;
    return 1;
}

#ifdef IGNORe
static void
checkodom(Odometer* odom)
{
    int i;
    for(i=0;i<odom->rank;i++) {
	ASSERT(odom->index[i] == odom->start[i]+odom->count[i]);
    }
}
#endif

/**************************************************/

void
generate_attrdata(Symbol* asym, Generator* generator, Writer writer, Bytebuffer* codebuf)
{
    Symbol* basetype = asym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;

    if(typecode == NC_CHAR) {
	gen_charattr(asym->data,codebuf);
    } else {
	int uid;
	size_t count;
        generator->listbegin(generator,LISTATTR,asym->data->length,codebuf,&uid);
        for(count=0;count<asym->data->length;count++) {
	    Constant* con = datalistith(asym->data,count);
	    generator->list(generator,LISTATTR,uid,count,codebuf);
            generate_basetype(asym->typ.basetype,con,codebuf,NULL,generator);
	}
        generator->listend(generator,LISTATTR,uid,count,codebuf);
    }
    writer(generator,asym,codebuf,0,NULL,NULL);
}

void
generate_vardata(Symbol* vsym, Generator* generator, Writer writer, Bytebuffer* code)
{
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    Symbol* basetype = vsym->typ.basetype;
    Datalist* filler = getfiller(vsym);

    if(vsym->data == NULL) return;

    /* give the buffer a running start to be large enough*/
    bbSetalloc(code, nciterbuffersize);

    if(rank == 0) {/*scalar case*/
	Constant* c0 = datalistith(vsym->data,0);
        generate_basetype(basetype,c0,code,filler,generator);
        writer(generator,vsym,code,0,NULL,NULL);
    } else {/*rank > 0*/
	generate_array(vsym,code,filler,generator,writer);
    }
}
		
static void
generate_array(Symbol* vsym,
               Bytebuffer* code,
               Datalist* filler,
               Generator* generator,
	       Writer writer
              )
{
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    Symbol* basetype = vsym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;
    Odometer* odom;
    nciter_t iter;

    ASSERT(rank > 0);

    /* Start by doing easy cases */

#ifdef CHARBUG
    if(typecode == NC_CHAR) { /* case: character typed variable, rank > 0 */
	Bytebuffer* charbuf = bbNew();
        gen_chararray(dimset,vsym->data,charbuf,filler);
	generator->charconstant(generator,code,charbuf);
	bbFree(charbuf);
        odom = newodometer(dimset,NULL,NULL);
        writer(generator,vsym,code,odom->rank,odom->start,odom->count);
    } else
#else /*!CHARBUG*/
    /* Case: char var && dim 1..n are not unlimited */
    if(findunlimited(dimset,1) == rank && typecode == NC_CHAR) {
	Bytebuffer* charbuf = bbNew();
	gen_leafchararray(dimset,0,vsym->data,charbuf,filler);
	generator->charconstant(generator,code,charbuf);
	bbFree(charbuf);
        odom = newodometer(dimset,NULL,NULL);
        writer(generator,vsym,code,odom->rank,odom->start,odom->count);
    } else
#endif
    /* Case 2: dim 1..n are not unlimited */
    if(findunlimited(dimset,1) == rank) {
	size_t offset = 0; /* where are we in the data list */
	size_t nelems = 0; /* # of data list items to read */
        /* Create an iterator and odometer and just walk the datalist */
        nc_get_iter(vsym,nciterbuffersize,&iter);
        odom = newodometer(dimset,NULL,NULL);
	for(;;offset+=nelems) {
	     int i,uid;
             nelems=nc_next_iter(&iter,odom->start,odom->count);
             if(nelems == 0) break;
	     bbClear(code);
             generator->listbegin(generator,LISTDATA,vsym->data->length,code,&uid);
	     for(i=0;i<nelems;i++) {
#ifdef ITERBUG
	 	Constant* con = datalistith(vsym->data,i);
#else
	 	Constant* con = datalistith(vsym->data,i+offset);
#endif	
                generator->list(generator,LISTDATA,uid,i,code);
#ifdef USE_NOFILL
		if(nofill_flag && con == NULL)
		    break;
		else
#endif
                    generate_basetype(basetype,con,code,filler,generator);
 	    }
	    generator->listend(generator,LISTDATA,uid,i,code);
#ifdef USE_NOFILL
            writer(generator,vsym,code,rank,odom->start,odom->index);
#else
            writer(generator,vsym,code,rank,odom->start,odom->count);
#endif
	}
    } else

    { /* Hard case: multiple unlimited dimensions */
        /* Setup iterator and odometer */
#ifdef CHARBUG
        nc_get_iter(vsym,nciterbuffersize,&iter);
#else
	nc_get_iter(vsym,NC_MAX_UINT,&iter); /* effectively infinite */
#endif
        odom = newodometer(dimset,NULL,NULL);
	for(;;) {/* iterate in nelem chunks */
	    /* get nelems count and modify odometer */
   	    size_t nelems=nc_next_iter(&iter,odom->start,odom->count);
	    if(nelems == 0) break;
            generate_arrayr(vsym,code,vsym->data,
			    odom,
                            /*dim index=*/0,
			    filler,generator
			   );
#ifdef USE_NOFILL
            writer(generator,vsym,code,odom->rank,odom->start,odom->index);
#else
            writer(generator,vsym,code,odom->rank,odom->start,odom->count);
#endif
        }
    }
    odometerfree(odom);
}

static void
generate_arrayr(Symbol* vsym,
               Bytebuffer* code,
               Datalist* list,
	       Odometer* odom,
               int dimindex,
               Datalist* filler,
               Generator* generator
              )
{
    Symbol* basetype = vsym->typ.basetype;
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    int lastunlimited;
    int typecode = basetype->typ.typecode;

    lastunlimited = findlastunlimited(dimset);
    if(lastunlimited == rank) lastunlimited = 0;

    ASSERT(rank > 0);
    ASSERT(dimindex >= 0 && dimindex < rank);

#ifdef CHARBUG
    ASSERT(typecode != NC_CHAR); 
#else /*!CHARBUG*/
    if(dimindex == lastunlimited && typecode == NC_CHAR) {
	Bytebuffer* charbuf = bbNew();
	gen_leafchararray(dimset,dimindex,list,charbuf,filler);
	generator->charconstant(generator,code,charbuf);
	bbFree(charbuf);
    } else
#endif /*!CHARBUG*/
    if(dimindex == lastunlimited) {
	int uid,i;
	Odometer* slabodom;
	/* build a special odometer to walk the last few dimensions
	   (similar to case 2 above)
	*/
        slabodom = newsubodometer(odom,dimset,dimindex,rank);
	/* compute the starting offset in our datalist 
	   (Assumes that slabodom->index[i] == slabodom->start[i])
        */
        generator->listbegin(generator,LISTDATA,list->length,code,&uid);
	for(i=0;odometermore(slabodom);i++) {
	    size_t offset = odometeroffset(slabodom);
	    Constant* con = datalistith(list,offset);
#ifdef USE_NOFILL
	    if(nofill_flag && con == NULL)
		break;
#endif
            generator->list(generator,LISTDATA,uid,i,code);
            generate_basetype(basetype,con,code,filler,generator);
	    odometerincr(slabodom);
        }
        generator->listend(generator,LISTDATA,uid,i,code);
	odometerfree(slabodom);
    } else {
        /* If we are strictly to the left of the next unlimited
           then our datalist is a list of compounds representing
           the next unlimited; so walk the subarray from this index
	   upto next unlimited.
        */
	int i;
	Odometer* slabodom;
        int nextunlimited = findunlimited(dimset,dimindex+1);
	ASSERT((dimindex < lastunlimited
                && (dimset->dimsyms[dimindex]->dim.isunlimited)));
	/* build a sub odometer */
        slabodom = newsubodometer(odom,dimset,dimindex,nextunlimited);
	/* compute the starting offset in our datalist 
	   (Assumes that slabodom->index[i] == slabodom->start[i])
        */
	for(i=0;odometermore(slabodom);i++) {
	    size_t offset = odometeroffset(slabodom);
	    Constant* con = datalistith(list,offset);
#ifdef USE_NOFILL
	    if(nofill_flag && con == NULL)
		break;
#endif
	    if(con == NULL || con->nctype == NC_FILL) {
		if(filler == NULL)
		    filler = getfiller(vsym);
	        generate_arrayr(vsym,code,filler,odom,nextunlimited,NULL,generator);
		
	    } else if(!islistconst(con))
	        semwarn(constline(con),"Expected {...} representing unlimited list");
	    else {
	        Datalist* sublist = con->value.compoundv;
	        generate_arrayr(vsym,code,sublist,odom,nextunlimited,filler,generator);
	    }
	    odometerincr(slabodom);
        }
	odometerfree(slabodom);
    }
    return;
}


/* Generate an instance of the basetype */
void
generate_basetype(Symbol* tsym, Constant* con, Bytebuffer* codebuf, Datalist* filler, Generator* generator)
{
    Datalist* data;

    switch (tsym->subclass) {

    case NC_ENUM:
    case NC_OPAQUE:
    case NC_PRIM:
	if(islistconst(con)) {
	    semerror(constline(con),"Expected primitive found {..}");
	}
	generate_primdata(tsym,con,codebuf,filler,generator);
	break;

    case NC_COMPOUND: {
	int i,uid;
	if(con == NULL || isfillconst(con)) {
	    Datalist* fill = (filler==NULL?getfiller(tsym):filler);
	    ASSERT(fill->length == 1);
	    con = &fill->data[0];
	    if(!islistconst(con))
	        semerror(con->lineno,"Compound data fill value is not enclosed in {..}");
	}
	if(!islistconst(con)) {/* fail on no compound*/
	    semerror(constline(con),"Compound data must be enclosed in {..}");
        }
	data = con->value.compoundv;
	generator->listbegin(generator,LISTCOMPOUND,listlength(tsym->subnodes),codebuf,&uid);
        for(i=0;i<listlength(tsym->subnodes);i++) {
            Symbol* field = (Symbol*)listget(tsym->subnodes,i);
	    con = datalistith(data,i);
	    generator->list(generator,LISTCOMPOUND,uid,i,codebuf);
            generate_basetype(field,con,codebuf,NULL,generator);
	}
	generator->listend(generator,LISTCOMPOUND,uid,i,codebuf);
	} break;

    case NC_VLEN: {
	Bytebuffer* vlenbuf;
        int uid;
	size_t count;

	if(con == NULL || isfillconst(con)) {
	    Datalist* fill = (filler==NULL?getfiller(tsym):filler);
	    ASSERT(fill->length == 1);
	    con = &fill->data[0];
	    if(con->nctype != NC_COMPOUND) {
	        semerror(con->lineno,"Vlen data fill value is not enclosed in {..}");
	    }
	}

	if(!islistconst(con)) {
	    semerror(constline(con),"Vlen data must be enclosed in {..}");
        }
        data = con->value.compoundv;
        /* generate the nc_vlen_t instance*/
	vlenbuf = bbNew();	
	if(tsym->typ.basetype->typ.typecode == NC_CHAR) {
	    gen_charvlen(data,vlenbuf);
	    generator->vlenstring(generator,vlenbuf,&uid,&count);
	} else {
    	    generator->listbegin(generator,LISTVLEN,data->length,codebuf,&uid);
            for(count=0;count<data->length;count++) {
   	        generator->list(generator,LISTVLEN,uid,count,vlenbuf);
                generate_basetype(tsym->typ.basetype,datalistith(data,count),vlenbuf,NULL,generator);
	    }
   	    generator->listend(generator,LISTVLEN,uid,count,codebuf,(void*)vlenbuf);
	}
	generator->vlendecl(generator,codebuf,tsym,uid,count,vlenbuf);
	bbFree(vlenbuf);
        } break;

    case NC_FIELD:
	if(tsym->typ.dimset.ndims > 0) {
	    /* Verify that we have a sublist (or fill situation) */
	    if(con != NULL && !isfillconst(con) && !islistconst(con)) 
		semerror(constline(con),"Dimensioned fields must be enclose in {...}");
            generate_fieldarray(tsym->typ.basetype,con,&tsym->typ.dimset,codebuf,filler,generator);
	} else {
	    generate_basetype(tsym->typ.basetype,con,codebuf,NULL,generator);
	}
	break;

    default: PANIC1("generate_basetype: unexpected subclass %d",tsym->subclass);
    }
}

/* Used only for structure field arrays*/
static void
generate_fieldarray(Symbol* basetype, Constant* con, Dimset* dimset,
		 Bytebuffer* codebuf, Datalist* filler, Generator* generator)
{
    int i;
    int chartype = (basetype->typ.typecode == NC_CHAR);
    Datalist* data;

    ASSERT(dimset->ndims > 0);

    if(con != NULL && !isfillconst(con))
        data = con->value.compoundv;
    else
	data = NULL;

    if(chartype) {
	/* Collect the char field in a separate buffer */
	Bytebuffer* charbuf = bbNew();
        gen_chararray(dimset,data,charbuf,filler);	
	generator->charconstant(generator,codebuf,charbuf);
	bbFree(charbuf);
    } else {
	int uid;
	size_t xproduct = crossproduct(dimset,0,0); /* compute total number of elements */
        generator->listbegin(generator,LISTFIELDARRAY,xproduct,codebuf,&uid);
        for(i=0;i<xproduct;i++) {
	    con = (data == NULL ? NULL : datalistith(data,i));
	    generator->list(generator,LISTFIELDARRAY,uid,i,codebuf);
            generate_basetype(basetype,con,codebuf,NULL,generator);
	}
        generator->listend(generator,LISTFIELDARRAY,uid,i,codebuf);
    }
}


/* An opaque string value might not conform
   to the size of the opaque to which it is being
   assigned. Normalize it to match the required
   opaque length (in bytes).
   Note that the string is a sequence of nibbles (4 bits).
*/
static void
normalizeopaquelength(Constant* prim, unsigned long nbytes)
{
    int nnibs = 2*nbytes;
    ASSERT(prim->nctype==NC_OPAQUE);
    if(prim->value.opaquev.len == nnibs) { 
        /* do nothing*/
    } else if(prim->value.opaquev.len > nnibs) { /* truncate*/
	prim->value.opaquev.stringv[nnibs] = '\0';
	prim->value.opaquev.len = nnibs;
    } else {/* prim->value.opaquev.len < nnibs => expand*/
        char* s;
	s = (char*)emalloc(nnibs+1);
	memset(s,'0',nnibs);
	memcpy(s,prim->value.opaquev.stringv,prim->value.opaquev.len);
	s[nnibs] = '\0';
	efree(prim->value.opaquev.stringv);
	prim->value.opaquev.stringv=s;
	prim->value.opaquev.len = nnibs;
    }
}

static void
generate_primdata(Symbol* basetype, Constant* prim, Bytebuffer* codebuf,
		  Datalist* filler, Generator* generator)
{
    Constant target;

    if(prim == NULL || isfillconst(prim)) {
	Datalist* fill = (filler==NULL?getfiller(basetype):filler);
	ASSERT(fill->length == 1);
	prim = datalistith(fill,0);
    }

    ASSERT(prim->nctype != NC_COMPOUND);

    target.nctype = basetype->typ.typecode;

    if(target.nctype != NC_ECONST) {
	convert1(prim,&target);
    }

    switch (target.nctype) {
    case NC_ECONST:
        if(basetype->subclass != NC_ENUM) {
	    semerror(constline(prim),"Conversion to enum not supported (yet)");
	} break;
     case NC_OPAQUE: 
	normalizeopaquelength(&target,basetype->typ.size);
	break;
    default:
	break;
    }
    generator->constant(generator,&target,codebuf);
    
    return;
}
