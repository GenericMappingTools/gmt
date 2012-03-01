/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#define CRUDE

#include "config.h"
#include <sys/stat.h>

#ifdef NETINET_IN_H
#include <netinet/in.h>
#endif

#include "ocinternal.h"
#include "ocdata.h"
#include "ocdebug.h"

/*Forward*/
static void dumpocnode1(OCnode* node, int depth);
static void dumpdimensions(OCnode* node);
static void dumpattvalue(OCtype nctype, char** aset, int index);
static void ocdumpmemdata1(OCmemdata* memdata, OCbytes* buf, int depth);

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

void
ocdumpnode(OCnode* node)
{
    if(node != NULL) {
        dumpocnode1(node,0);
    } else {
	fprintf(stderr,"<NULL>\n");
    }
    fflush(stderr);
}

static void
dumpocnode1(OCnode* node, int depth)
{
    unsigned int n;
    switch (node->octype) {
    case OC_Primitive: {
        fprintf(stderr,"[%2d]%s ",depth,dent(depth));
	if(node->name == NULL) OCPANIC("prim without name");
	fprintf(stderr,"%s %s",octypetostring(node->etype),node->name);
	dumpdimensions(node);
	fprintf(stderr," @%lx",(unsigned long)node);
	fprintf(stderr,"\n");
    } break;

    case OC_Dataset: {
        fprintf(stderr,"[%2d]%s ",depth,dent(depth));
	fprintf(stderr,"dataset %s\n",
		(node->name?node->name:""));
	for(n=0;n<oclistlength(node->subnodes);n++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,n),depth+1);
	}
    } break;

    case OC_Structure: {
        fprintf(stderr,"[%2d]%s ",depth,dent(depth));
	fprintf(stderr,"struct %s",
		(node->name?node->name:""));
	dumpdimensions(node);
	fprintf(stderr," @%lx",(unsigned long)node);
	fprintf(stderr,"\n");
	for(n=0;n<oclistlength(node->subnodes);n++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,n),depth+1);
	}
    } break;

    case OC_Sequence: {
        fprintf(stderr,"[%2d]%s ",depth,dent(depth));
	fprintf(stderr,"sequence %s",
		(node->name?node->name:""));
	dumpdimensions(node);
	fprintf(stderr," @%lx",(unsigned long)node);
	fprintf(stderr,"\n");
	for(n=0;n<oclistlength(node->subnodes);n++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,n),depth+1);
	}
    } break;

    case OC_Grid: {
	unsigned int i;
        fprintf(stderr,"[%2d]%s ",depth,dent(depth));
	fprintf(stderr,"grid %s",
		(node->name?node->name:""));
	dumpdimensions(node);
	fprintf(stderr," @%lx",(unsigned long)node);
	fprintf(stderr,"\n");
	fprintf(stderr,"%sarray:\n",dent2(depth+1));
	dumpocnode1((OCnode*)oclistget(node->subnodes,0),depth+2);
	fprintf(stderr,"%smaps:\n",dent2(depth+1));
	for(i=1;i<oclistlength(node->subnodes);i++) {
	    dumpocnode1((OCnode*)oclistget(node->subnodes,i),depth+2);
	}
    } break;

    case OC_Attribute: {
        fprintf(stderr,"[%2d]%s ",depth,dent(depth));
	if(node->name == NULL) OCPANIC("Attribute without name");
	fprintf(stderr,"%s %s",octypetostring(node->etype),node->name);
	for(n=0;n<oclistlength(node->att.values);n++) {
	    char* value = (char*)oclistget(node->att.values,n);
	    if(n > 0) fprintf(stderr,",");
	    fprintf(stderr," %s",value);
	}
	fprintf(stderr," @%lx",(unsigned long)node);
	fprintf(stderr,"\n");
    } break;

    case OC_Attributeset: {
        fprintf(stderr,"[%2d]%s ",depth,dent(depth));
	fprintf(stderr,"%s:\n",node->name?node->name:"Attributes");
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
	    fprintf(stderr,"%s[%s=",dent2(depth+2),att->name);
	    if(att->nvalues == 0)
		OCPANIC("Attribute.nvalues == 0");
	    if(att->nvalues == 1) {
		dumpattvalue(att->etype,att->values,0);
	    } else {
		unsigned int j;
	        fprintf(stderr,"{");
		for(j=0;j<att->nvalues;j++) {
		    if(j>0) fprintf(stderr,", ");
		    dumpattvalue(att->etype,att->values,j);
		}
	        fprintf(stderr,"}");
	    }
	    fprintf(stderr,"]\n");
	}
    }
}

static void
dumpdimensions(OCnode* node)
{
    unsigned int i;
    for(i=0;i<node->array.rank;i++) {
        OCnode* dim = (OCnode*)oclistget(node->array.dimensions,i);
        fprintf(stderr,"[%s=%lu]",
			(dim->name?dim->name:"?"),
			(unsigned long)dim->dim.declsize);
    }
}

static void
dumpattvalue(OCtype nctype, char** strings, int index)
{
    if(nctype == OC_String || nctype == OC_URL) {
        fprintf(stderr,"\"%s\"",strings[index]);
    } else {
        fprintf(stderr,"%s",strings[index]);
    }
}

void
ocdumpslice(OCslice* slice)
{
    fprintf(stderr,"[");
    fprintf(stderr,"%lu",(unsigned long)slice->first);
    if(slice->stride > 1) fprintf(stderr,":%lu",(unsigned long)slice->stride);
    fprintf(stderr,":%lu",(unsigned long)(slice->first+slice->count)-1);
    fprintf(stderr,"]");
}

void
ocdumpclause(OCprojectionclause* ref)
{
    unsigned int i;
    OClist* path = oclistnew();
    collectpathtonode(ref->node,path);
    for(i=0;i<oclistlength(path);i++) {
        OClist* sliceset;
	OCnode* node = (OCnode*)oclistget(path,i);
	if(node->tree != NULL) continue; /* leave off the root node*/
	fprintf(stderr,"%s%s",(i>0?PATHSEPARATOR:""),node->name);
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
dumpmem2(char* s, char* accum, int align)
{
    int len,rem;
    strcat(accum,"|");
    strcat(accum,s);
    len = strlen(s);
    rem = (align - len);
    while(rem-- > 0) strcat(accum," ");
}

static void
dumpmem1(int index, unsigned int n, unsigned int n1)
{
    char s[1024];
    char tmp[32];
    union {
	unsigned int uv;
	int sv;
	unsigned char cv[4];
	float fv;
    } form;
#ifndef CRUDE
    union {
	unsigned int uv[2];
	double dv;
    } dform;
    int i;
#endif
    form.uv = n;
    s[0] = '\0';
#ifndef CRUDE
    sprintf(tmp,"%6d",index);
    dumpmem2(tmp,s,5);
#endif
    sprintf(tmp,"%08x",form.uv);
    dumpmem2(tmp,s,8);
#ifndef CRUDE
    sprintf(tmp,"%12u",form.uv);
    dumpmem2(tmp,s,12);
    sprintf(tmp,"%12d",form.sv);
    dumpmem2(tmp,s,12);
    sprintf(tmp,"%#g",form.fv);
    dumpmem2(tmp,s,12);
    tmp[0] = '\0';
    for(i=0;i<4;i++) {
	char stmp[64];
	if(form.cv[i] < ' ' || form.cv[i] > 126)
            sprintf(stmp,"\\%02x",(unsigned int)form.cv[i]);
	else
            sprintf(stmp,"%c",form.cv[i]);
	strcat(tmp,stmp);
    }
    dumpmem2(tmp,s,16);
    dform.uv[1] = n;
    dform.uv[0] = n1;
    sprintf(tmp,"%#g",dform.dv);
    dumpmem2(tmp,s,12);
#else
    tmp[0] = '\0';
#endif
    strcat(s,"\n");
    fprintf(stderr,"%s",s);
}

static void
dumpmemory0(char* memory, int len, int fromxdr, int bod)
{
    unsigned int i,count,rem;
    int* imemory;
#ifndef CRUDE
    char hdr[1024];
#endif

    assert(memory[len] == 0);

#ifndef CRUDE
    /* build the header*/
    hdr[0] = '\0';
    dumpmem2("offset",hdr,6);
    dumpmem2("hex",hdr,8);
    dumpmem2("uint",hdr,12);
    dumpmem2("int",hdr,12);
    dumpmem2("float",hdr,12);
    dumpmem2("char[4]",hdr,16);
    dumpmem2("double",hdr,12);
    strcat(hdr,"\n");
    fprintf(stderr,"%s",hdr);
#endif

    count = (len / sizeof(int));
    rem = (len % sizeof(int));
    imemory = (int*)memory;

    for(i=0;i<count;i++) {
	unsigned int tmp0 = (unsigned int)imemory[i];
	unsigned int tmp1 = (unsigned int)(i<count?imemory[i+1]:0);
	if(fromxdr) {tmp0 = ocntoh(tmp0); tmp1 = ocntoh(tmp1);}
	dumpmem1(i*sizeof(unsigned int)+bod,tmp0,tmp1);
    }
    if(rem > 0) {
	unsigned int tmp = 0;
	memcpy((void*)&tmp,(void*)(imemory+(sizeof(len)*count)),rem);
	if(fromxdr) tmp = ocntoh(tmp);
	dumpmem1(count*sizeof(unsigned int)+bod,tmp,0);
    }
    fflush(stderr);
}

void
ocdumppacket(char* memory, int len, int bod)
{
    dumpmemory0(memory,len,1,bod);
}

void
ocdumpmemory(char* memory, int len, int bod)
{
    dumpmemory0(memory,len,0,bod);
}

void
ocdumpfile(FILE* file, int datastart)
{
    int i,count,rem,len;
    long pos;
    char dds[4096];
    char hdr[1024];
    struct stat stats;
    unsigned int imemory;
    unsigned int imemory1;

    pos = ftell(file);
    fseek(file,0,SEEK_SET);

    fstat(fileno(file),&stats);
    len = stats.st_size;

    fprintf(stderr,"\nlength=%d datastart=%d\n",len,datastart);

    if(datastart > 0) {
        fread(dds,1,datastart,file);
        dds[datastart] = '\0';
        fprintf(stderr,"DDS:\n");
        fprintf(stderr,"====================\n");
        fprintf(stderr,"%s\n",dds);
    } else {
	fprintf(stderr,"DDS: none specified\n");
    }
    fprintf(stderr,"====================\n");

    /* build the header*/
    hdr[0] = '\0';
    dumpmem2("offset",hdr,5);
    dumpmem2("hex",hdr,8);
    dumpmem2("uint",hdr,12);
    dumpmem2("int",hdr,12);
    dumpmem2("float",hdr,12);
    dumpmem2("char[4]",hdr,16);
    dumpmem2("double",hdr,12);
    strcat(hdr,"\n");
    fprintf(stderr,"%s",hdr);

    len -= datastart;
    count = (len / sizeof(unsigned int));
    rem = (len % sizeof(unsigned int));

    for(i=0;i<count;i++) {
	long pos;
	fread(&imemory,sizeof(unsigned int),1,file);
	pos = ftell(file);
	fread(&imemory1,sizeof(unsigned int),1,file);
	fseek(file,pos,SEEK_SET);
	imemory = ocntoh(imemory);
	imemory1 = ocntoh(imemory1);
	dumpmem1(i*4+datastart,imemory,imemory1);
    }
    if(rem > 0) {
	fprintf(stderr,">>>>remainder=%d\n",rem);
    }
    fflush(stderr);
    fseek(file,pos,SEEK_SET); /* leave it as we found it*/
}

void
ocdumpmemdata(OCmemdata* memdata, OCbytes* buf)
{
    if(memdata == NULL || buf == NULL) return;
    ocdumpmemdata1(memdata,buf,0);
}

static char*
ocmodestr(OCmode mode)
{
    switch (mode) {
    case Emptymode: return "Empty";
    case Nullmode: return "Null";
    case Dimmode: return "Dim";
    case Recordmode: return "Record";
    case Fieldmode: return "Field";
    case Datamode: return "Data";
    }
    return "?";
}

static void
ocdumpmemdata1(OCmemdata* md, OCbytes* buf, int depth)
{
    OCmemdata** mdp;
    unsigned int i;
    char tmp[1024];

    switch ((OCtype)md->octype) {

    case OC_Sequence:
    case OC_Grid:    
    case OC_Structure:
    case OC_Dataset:
	sprintf(tmp,"%s%s/%s (%lu) {\n",dent(depth),
		octypetostring((OCtype)md->octype),
		ocmodestr(md->mode),
		(unsigned long)md->count);
	ocbytescat(buf,tmp);
	mdp = (OCmemdata**)md->data.data;
	switch ((OCmode)md->mode) {
        case Fieldmode:
	    for(i=0;i<md->count;i++) {
	        sprintf(tmp,"%s[%u]",dent(depth+1),i);
	        ocbytescat(buf,tmp);
	        ocdumpmemdata1(mdp[i],buf,depth+1);
	    }	    
	    break;

	case Dimmode:
	    for(i=0;i<md->count;i++) {
	        sprintf(tmp,"%s(%u)",dent(depth+1),i);
	        ocbytescat(buf,tmp);
	        ocdumpmemdata1(mdp[i],buf,depth+1);
	    }	    
	    break;

	case Recordmode:
	    for(i=0;i<md->count;i++) {
	        sprintf(tmp,"%s{%u}",dent(depth+1),i);
	        ocbytescat(buf,tmp);
	        ocdumpmemdata1(mdp[i],buf,depth+1);
	    }
	    break;

        default: break;
	}
	sprintf(tmp,"%s}\n",dent(depth));
	break;
	
    case OC_Primitive: {
	OCtype etype = (OCtype)md->etype;
        char* data = md->data.data;
	sprintf(tmp,"%s%s/%s (%lu) {",dent(depth),
		octypetostring(etype),ocmodestr(md->mode),(unsigned long)md->count);
	ocbytescat(buf,tmp);
        for(i=0;i<md->count;i++) {
	    char* p = data + (octypesize(etype)*i);
	    ocbytescat(buf," ");
	    octypeprint(etype,tmp,sizeof(tmp),(void*)p);
	    ocbytescat(buf,tmp);
	}
        ocbytescat(buf," }\n");
    } break;

    default: break;
    }
}

void
ocdd(OCstate* state, OCnode* root)
{
#ifdef OC_DISK_STORAGE
    ocdumpfile(root->tree->data.file,root->tree->data.bod);
#else
    ocdumpmemory(root->tree->data.xdrdata,
		 root->tree->data.datasize,
		 root->tree->data.bod);
#endif
}

