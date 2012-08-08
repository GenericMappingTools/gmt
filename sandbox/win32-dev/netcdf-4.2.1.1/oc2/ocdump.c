/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information.
*/

#include "config.h"
#include <sys/stat.h>

#ifdef NETINET_IN_H
#include <netinet/in.h>
#endif

#include "ocinternal.h"
#include "ocdebug.h"

#define MAXLEVEL 1

/*Forward*/
static void dumpocnode1(OCnode* node, int depth);
static void dumpdimensions(OCnode* node);
static void dumpattvalue(OCtype nctype, char** aset, int index);

static char* sindent = NULL;

static char*
dent(int n)
{
    if(sindent == NULL) {
	sindent = (char*)ocmalloc(102);
	MEMCHECK(sindent,NULL);
	memset((void*)sindent,(int)' ',(size_t)101);
	sindent[101] = '\0';
    }
    if(n > 100) n = 100;
    return sindent+(100-n);
}

/* support [dd] leader*/
static char*
dent2(int n) {return dent(n+4);}

static void
tabto(int pos, OCbytes* buffer)
{
    int bol,len,pad;
    len = ocbyteslength(buffer);
    /* find preceding newline */
    for(bol=len-1;;bol--) {
	int c = ocbytesget(buffer,bol);
	if(c < 0) break;
	if(c == '\n') {bol++; break;}
    }
    len = (len - bol);
    pad = (pos - len);
    while(pad-- > 0) ocbytescat(buffer," ");
}

void
ocdumpnode(OCnode* node)
{
    if(node != NULL) {
        dumpocnode1(node,0);
    } else {
	fprintf(stdout,"<NULL>\n");
    }
    fflush(stdout);
}

static void
dumpocnode1(OCnode* node, int depth)
{
    unsigned int n;
    switch (node->octype) {
    case OC_Atomic: {
        fprintf(stdout,"[%2d]%s ",depth,dent(depth));
	if(node->name == NULL) OCPANIC("prim without name");
	fprintf(stdout,"%s %s",octypetostring(node->etype),node->name);
	dumpdimensions(node);
	fprintf(stdout," &%lx",(unsigned long)node);
	fprintf(stdout,"\n");
    } break;

    case OC_Dataset: {
        fprintf(stdout,"[%2d]%s ",depth,dent(depth));
	fprintf(stdout,"dataset %s\n",
		(node->name?node->name:""));
	for(n=0;n<oclistlength(node->subnodes);n++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,n),depth+1);
	}
    } break;

    case OC_Structure: {
        fprintf(stdout,"[%2d]%s ",depth,dent(depth));
	fprintf(stdout,"struct %s",
		(node->name?node->name:""));
	dumpdimensions(node);
	fprintf(stdout," &%lx",(unsigned long)node);
	fprintf(stdout,"\n");
	for(n=0;n<oclistlength(node->subnodes);n++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,n),depth+1);
	}
    } break;

    case OC_Sequence: {
        fprintf(stdout,"[%2d]%s ",depth,dent(depth));
	fprintf(stdout,"sequence %s",
		(node->name?node->name:""));
	dumpdimensions(node);
	fprintf(stdout," &%lx",(unsigned long)node);
	fprintf(stdout,"\n");
	for(n=0;n<oclistlength(node->subnodes);n++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,n),depth+1);
	}
    } break;

    case OC_Grid: {
	unsigned int i;
        fprintf(stdout,"[%2d]%s ",depth,dent(depth));
	fprintf(stdout,"grid %s",
		(node->name?node->name:""));
	dumpdimensions(node);
	fprintf(stdout," &%lx",(unsigned long)node);
	fprintf(stdout,"\n");
	fprintf(stdout,"%sarray:\n",dent2(depth+1));
	dumpocnode1((OCnode*)oclistget(node->subnodes,0),depth+2);
	fprintf(stdout,"%smaps:\n",dent2(depth+1));
	for(i=1;i<oclistlength(node->subnodes);i++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,i),depth+2);
	}
    } break;

    case OC_Attribute: {
        fprintf(stdout,"[%2d]%s ",depth,dent(depth));
	if(node->name == NULL) OCPANIC("Attribute without name");
	fprintf(stdout,"%s %s",octypetostring(node->etype),node->name);
	for(n=0;n<oclistlength(node->att.values);n++) {
	    char* value = (char*)oclistget(node->att.values,n);
	    if(n > 0) fprintf(stdout,",");
	    fprintf(stdout," %s",value);
	}
	fprintf(stdout," &%lx",(unsigned long)node);
	fprintf(stdout,"\n");
    } break;

    case OC_Attributeset: {
        fprintf(stdout,"[%2d]%s ",depth,dent(depth));
	fprintf(stdout,"%s:\n",node->name?node->name:"Attributes");
	for(n=0;n<oclistlength(node->subnodes);n++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,n),depth+1);
	}
    } break;

    default:
	OCPANIC1("encountered unexpected node type: %x",node->octype);
    }

    if(node->attributes != NULL) {
	unsigned int i;
	for(i=0;i<oclistlength(node->attributes);i++) {
	    OCattribute* att = (OCattribute*)oclistget(node->attributes,i);
	    fprintf(stdout,"%s[%s=",dent2(depth+2),att->name);
	    if(att->nvalues == 0)
		OCPANIC("Attribute.nvalues == 0");
	    if(att->nvalues == 1) {
		dumpattvalue(att->etype,att->values,0);
	    } else {
		unsigned int j;
	        fprintf(stdout,"{");
		for(j=0;j<att->nvalues;j++) {
		    if(j>0) fprintf(stdout,", ");
		    dumpattvalue(att->etype,att->values,j);
		}
	        fprintf(stdout,"}");
	    }
	    fprintf(stdout,"]\n");
	}
    }
}

static void
dumpdimensions(OCnode* node)
{
    unsigned int i;
    for(i=0;i<node->array.rank;i++) {
        OCnode* dim = (OCnode*)oclistget(node->array.dimensions,i);
        fprintf(stdout,"[%s=%lu]",
			(dim->name?dim->name:"?"),
			(unsigned long)dim->dim.declsize);
    }
}

static void
dumpattvalue(OCtype nctype, char** strings, int index)
{
    if(nctype == OC_String || nctype == OC_URL) {
        fprintf(stdout,"\"%s\"",strings[index]);
    } else {
        fprintf(stdout,"%s",strings[index]);
    }
}

void
ocdumpslice(OCslice* slice)
{
    fprintf(stdout,"[");
    fprintf(stdout,"%lu",(unsigned long)slice->first);
    if(slice->stride > 1) fprintf(stdout,":%lu",(unsigned long)slice->stride);
    fprintf(stdout,":%lu",(unsigned long)(slice->first+slice->count)-1);
    fprintf(stdout,"]");
}

void
ocdumpclause(OCprojectionclause* ref)
{
    unsigned int i;
    OClist* path = oclistnew();
    occollectpathtonode(ref->node,path);
    for(i=0;i<oclistlength(path);i++) {
        OClist* sliceset;
	OCnode* node = (OCnode*)oclistget(path,i);
	if(node->tree != NULL) continue; /* leave off the root node*/
	fprintf(stdout,"%s%s",(i>0?PATHSEPARATOR:""),node->name);
	sliceset = (OClist*)oclistget(ref->indexsets,i);
	if(sliceset != NULL) {
	    unsigned int j;
	    for(j=0;j<oclistlength(sliceset);j++) {
	        OCslice* slice = (OCslice*)oclistget(sliceset,j);
	        ocdumpslice(slice);
	    }
	}
    }
}


static void
addfield(char* field, char* line, int align)
{
    int len,rem;
    strcat(line,"|");
    strcat(line,field);
    len = strlen(field);
    rem = (align - len);
    while(rem-- > 0) strcat(line," ");
}

static void
dumpfield(int index, char* n8, int isxdr)
{
    char line[1024];
    char tmp[32];

    union {
	unsigned int uv;
	int sv;
	char cv[4];
	float fv;
    } form;
    union {
	char cv[8];
        unsigned long long ll;
        double d;
    } dform;

    line[0] = '\0';

    /* offset */
    sprintf(tmp,"%6d",index);
    addfield(tmp,line,5);

    memcpy(form.cv,n8,4);

    /* straight hex*/
    sprintf(tmp,"%08x",form.uv);
    addfield(tmp,line,8);

    if(isxdr) {swapinline32(&form.uv);}

    /* unsigned integer */
    sprintf(tmp,"%12u",form.uv);
    addfield(tmp,line,12);

    /* signed integer */
    sprintf(tmp,"%12d",form.sv);
    addfield(tmp,line,12);

    /* float */
    sprintf(tmp,"%#g",form.fv);
    addfield(tmp,line,12);

    /* char[4] */
    {
        /* use raw form (i.e. n8)*/
        int i;
	tmp[0] = '\0';
        for(i=0;i<4;i++) {
	    char stmp[64];
	    unsigned int c = (n8[i] & 0xff);
	    if(c < ' ' || c > 126)
                sprintf(stmp,"\\%02x",c);
	    else
                sprintf(stmp,"%c",c);
	    strcat(tmp,stmp);
        }
    }

    addfield(tmp,line,16);

    /* double */
    memcpy(dform.cv,n8,2*XDRUNIT);
    if(isxdr) xxdrntohdouble(dform.cv,&dform.d);
    sprintf(tmp,"%#g",dform.d);
    addfield(tmp,line,12);

    fprintf(stdout,"%s\n",line);
}

static void
typedmemorydump(char* memory, size_t len, int fromxdr)
{
    unsigned int i,count,rem;
    char line[1024];
    char* pmem;
    char mem[8];

    assert(memory[len] == 0);

    /* build the header*/
    line[0] = '\0';
    addfield("offset",line,6);
    addfield("hex",line,8);
    addfield("uint",line,12);
    addfield("int",line,12);
    addfield("float",line,12);
    addfield("char[4]",line,16);
    addfield("double",line,12);
    strcat(line,"\n");
    fprintf(stdout,"%s",line);

    count = (len / sizeof(int));
    rem = (len % sizeof(int));

    for(pmem=memory,i=0;i<count;i++,pmem+=4) {
	memset(mem,0,8);
	if(i<(count-1))
	    memcpy(mem,pmem,8);
	else
	    memcpy(mem,pmem,4);
	dumpfield(i*sizeof(unsigned int),mem,fromxdr);
    }
    if(rem > 0) {
	memset(mem,0,8);
	memcpy(mem,pmem,4);
	dumpfield(i*sizeof(unsigned int),mem,fromxdr);
    }
    fflush(stdout);
}

static void
simplememorydump(char* memory, size_t len, int fromxdr)
{
    unsigned int i,count,rem;
    int* imemory;
    char tmp[32];
    char line[1024];

    assert(memory[len] == 0);

    /* build the header*/
    line[0] = '\0';
    addfield("offset",line,6);
    addfield("XDR (hex)",line,9);
    addfield("!XDR (hex)",line,10);
    fprintf(stdout,"%s\n",line);

    count = (len / sizeof(int));
    rem = (len % sizeof(int));
    if(rem != 0)
	fprintf(stderr,"ocdump: |mem|%%4 != 0\n");
    imemory = (int*)memory;

    for(i=0;i<count;i++) {
	unsigned int vx = (unsigned int)imemory[i];
	unsigned int v = vx;
	if(!xxdr_network_order) swapinline32(&v);
        line[0] = '\0';
        sprintf(tmp,"%6d",i);
        addfield(tmp,line,6);
        sprintf(tmp,"%08x",vx);
        addfield(tmp,line,9);
        sprintf(tmp,"%08x",v);
        addfield(tmp,line,10);
        fprintf(stdout,"%s\n",line);
    }
    fflush(stdout);
}

void
ocdumpmemory(char* memory, size_t len, int xdrencoded, int level)
{
    if(level > MAXLEVEL) level = MAXLEVEL;
    switch (level) {
    case 1: /* Do a multi-type dump */
        typedmemorydump(memory,len,xdrencoded);
	break;
    case 0: /* Dump a simple linear list of the contents of the memory as 32-bit hex and decimal */
    default:
        simplememorydump(memory,len,xdrencoded);
	break;
    }
}

static int
ocreadfile(FILE* file, int datastart, char** memp, size_t* lenp)
{
    char* mem;
    size_t len;
    size_t pos;
    size_t red;
    struct stat stats;

    pos = ftell(file);
    fseek(file,0,SEEK_SET);
    fseek(file,datastart,SEEK_SET);

    fstat(fileno(file),&stats);
    len = stats.st_size;
    len -= datastart;
    
    mem = (char*)calloc(len+1,1);
    if(mem == NULL) return 0;

    /* Read only the data part */
    red = fread(mem,1,len,file);
    if(red < len) {
	fprintf(stderr,"ocreadfile: short file\n");
	return 0;
    }	
    fseek(file,pos,SEEK_SET); /* leave it as we found it*/
    if(memp) *memp = mem;
    if(lenp) *lenp = len;
    return 1;
}

void
ocdd(OCstate* state, OCnode* root, int xdrencoded, int level)
{
    char* mem;
    size_t len;
    if(root->tree->data.file != NULL) {
        if(!ocreadfile(root->tree->data.file,root->tree->data.bod,&mem,&len)) {
    	    fprintf(stderr,"ocdd could not read data file\n");
	    return;
	}
        ocdumpmemory(mem,len,xdrencoded,level);
        free(mem);
    } else {
        mem = root->tree->data.memory;
        mem += root->tree->data.bod;
        len = root->tree->data.datasize;
        len -= root->tree->data.bod;
        ocdumpmemory(mem,len,xdrencoded,level);
    }
}

void
ocdumpdata(OCstate* state, OCdata* data, OCbytes* buffer, int frominstance)
{
    char tmp[1024];
    OCnode* template = data->template;
    snprintf(tmp,sizeof(tmp),"%lx:",(unsigned long)data);
    ocbytescat(buffer,tmp);
    if(!frominstance) {
        ocbytescat(buffer," node=");
        ocbytescat(buffer,template->name);
    }
    snprintf(tmp,sizeof(tmp)," xdroffset=%ld",(unsigned long)data->xdroffset);
    ocbytescat(buffer,tmp);
    if(data->template->octype == OC_Atomic) {
        snprintf(tmp,sizeof(tmp)," xdrsize=%ld",(unsigned long)data->xdrsize);
        ocbytescat(buffer,tmp);
    }
    if(iscontainer(template->octype)) {
        snprintf(tmp,sizeof(tmp)," ninstances=%d",(int)data->ninstances);
        ocbytescat(buffer,tmp);
    } else if(template->etype == OC_String || template->etype == OC_URL) {
        snprintf(tmp,sizeof(tmp)," nstrings=%d",(int)data->nstrings);
        ocbytescat(buffer,tmp);
    }
    ocbytescat(buffer," container=");
    snprintf(tmp,sizeof(tmp),"%lx",(unsigned long)data->container);
    ocbytescat(buffer,tmp);
    ocbytescat(buffer," mode=");
    ocbytescat(buffer,ocdtmodestring(data->datamode,0));
}

/*
Depth Offset   Index Flags Size Type      Name
0123456789012345678901234567890123456789012345
0         1         2         3         4
[001] 00000000 0000  FS    0000 Structure person
*/
static const int tabstops[] = {0,6,15,21,27,32,42};
static const char* header =
"Depth Offset   Index Flags Size Type     Name\n";

void
ocdumpdatatree(OCstate* state, OCdata* data, OCbytes* buffer, int depth)
{
    size_t i,rank;
    OCnode* template;
    char tmp[1024];
    size_t crossproduct;
    int tabstop = 0;
    const char* typename;

    /* If this is the first call, then dump a header line */
    if(depth == 0) {
	ocbytescat(buffer,header);
    }

    /* get info about the template */
    template = data->template;
    rank = template->array.rank;

    /* get total dimension size */
    if(rank > 0)
        crossproduct = octotaldimsize(template->array.rank,template->array.sizes);

    /* Dump the depth first */
    snprintf(tmp,sizeof(tmp),"[%03d]",depth);
    ocbytescat(buffer,tmp);

    tabto(tabstops[++tabstop],buffer);

    snprintf(tmp,sizeof(tmp),"%08lu",(unsigned long)data->xdroffset);
    ocbytescat(buffer,tmp);

    tabto(tabstops[++tabstop],buffer);

    /* Dump the Index wrt to parent, if defined */
    if(fisset(data->datamode,OCDT_FIELD)
       || fisset(data->datamode,OCDT_ELEMENT)
       || fisset(data->datamode,OCDT_RECORD)) {
        snprintf(tmp,sizeof(tmp),"%04lu ",(unsigned long)data->index);
        ocbytescat(buffer,tmp);
    }
   
    tabto(tabstops[++tabstop],buffer);

    /* Dump the mode flags in compact form */
    ocbytescat(buffer,ocdtmodestring(data->datamode,1));
   
    tabto(tabstops[++tabstop],buffer);

    /* Dump the size or ninstances */
    if(fisset(data->datamode,OCDT_ARRAY)
       || fisset(data->datamode,OCDT_SEQUENCE)) {
        snprintf(tmp,sizeof(tmp),"%04lu",(unsigned long)data->ninstances);
    } else {
        snprintf(tmp,sizeof(tmp),"%04lu",(unsigned long)data->xdrsize);
    }
    ocbytescat(buffer,tmp);

    tabto(tabstops[++tabstop],buffer);

    if(template->octype == OC_Atomic) {
	typename = octypetoddsstring(template->etype);
    } else { /*must be container*/
	typename = octypetoddsstring(template->octype);
    }
    ocbytescat(buffer,typename);

    tabto(tabstops[++tabstop],buffer);

    snprintf(tmp,sizeof(tmp),"%s",template->name);
    ocbytescat(buffer,tmp);

    if(rank > 0) {
	snprintf(tmp,sizeof(tmp),"[%lu]",(unsigned long)crossproduct);
	ocbytescat(buffer,tmp);
    }
    ocbytescat(buffer,"\n");

    /* dump the sub-instance, which might be fields, records, or elements */
    if(!fisset(data->datamode,OCDT_ATOMIC)) {
        for(i=0;i<data->ninstances;i++)
	    ocdumpdatatree(state,data->instances[i],buffer,depth+1);
    }
}

void
ocdumpdatapath(OCstate* state, OCdata* data, OCbytes* buffer)
{
    int i;
    OCdata* path[1024];
    char tmp[1024];
    OCdata* pathdata;
    OCnode* template;
    int isrecord;

    path[0] = data;
    for(i=1;;i++) {
	OCdata* next = path[i-1];
	if(next->container == NULL) break;
	path[i] = next->container;
    }    	        
    /* Path is in reverse order */
    for(i=i-1;i>=0;i--) {
	pathdata = path[i];
	template = pathdata->template;
	ocbytescat(buffer,"/");
	ocbytescat(buffer,template->name);
	/* Check the mode of the next step in path */
	if(i > 0) {
	    OCdata* next = path[i-1];
	    if(fisset(next->datamode,OCDT_FIELD)
		|| fisset(next->datamode,OCDT_ELEMENT)
		|| fisset(next->datamode,OCDT_RECORD)) {
		snprintf(tmp,sizeof(tmp),".%lu",next->index);
	        ocbytescat(buffer,tmp);
	    }
	}
	if(template->octype == OC_Atomic) {
	    if(template->array.rank > 0) {
	        off_t xproduct = octotaldimsize(template->array.rank,template->array.sizes);
	        snprintf(tmp,sizeof(tmp),"[0..%lu]",(unsigned long)xproduct-1);
	        ocbytescat(buffer,tmp);
	    }
	}
	isrecord = 0;
	if(template->octype == OC_Sequence) {
	    /* Is this a record or a sequence ? */
	    isrecord = (fisset(pathdata->datamode,OCDT_RECORD) ? 1 : 0);
	}
    }
    /* Add suffix to path */
    if(iscontainer(template->octype)) {
        /* add the container type, except distinguish record and sequence */
	ocbytescat(buffer,":");
	if(isrecord)
	    ocbytescat(buffer,"Record");
	else
	    ocbytescat(buffer,octypetoddsstring(template->octype));
    } else if(isatomic(template->octype)) {
	/* add the atomic etype */
	ocbytescat(buffer,":");
	ocbytescat(buffer,octypetoddsstring(template->etype));
    }
    snprintf(tmp,sizeof(tmp),"->0x%0lx",(unsigned long)pathdata);
    ocbytescat(buffer,tmp);	
}
