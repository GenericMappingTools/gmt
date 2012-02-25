#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
 
#include <curl/curl.h>

#include <ast.h>

#include "nclist.h"

#include "netcdf.h"
#include "nc.h"
#include "nc4internal.h"
#include "nccr.h"
#include "nccrnode.h"
#include "ncStreamx.h"

#define LINESIZE 72

/* Forward */
static ast_err nccr_dump_group(Group*, int depth);
static ast_err nccr_dump_group_body(Group*, int depth);
static ast_err nccr_dump_dimension(Dimension*, int depth);
static ast_err nccr_dump_variable(Variable*, int depth);
static ast_err nccr_dump_attribute(Attribute*, char*, int depth);
static ast_err nccr_dump_enumconst(EnumType*, int depth);
static ast_err nccr_dump_enum(EnumTypedef*, int depth);
static ast_err nccr_dump_structure(Structure*, int depth);

static char* nccr_dump_typeref(DataType datatype);
static ast_err nccr_dump_shape(size_t count, Dimension** dimset);

static ast_err nccr_data_fixed(DataType datatype, bytes_t* data);
static ast_err nccr_data_vlen(DataType datatype, bytes_t* data);

static char* indent(int depth);

/* Provide a depth marked dump of a Header */
ast_err
nccr_dumpheader(Header* hdr)
{
    if(hdr->location.defined)
	printf("location: %s\n",hdr->location.value);
    if(hdr->title.defined)
	printf("title: %s\n",hdr->title.value);
    if(hdr->id.defined)
 	printf("id: %s\n",hdr->id.value);
    if(hdr->version.defined)
	printf("version: %d\n",hdr->version.value);
    return nccr_dump_group_body(hdr->root,0);
}

static ast_err
nccr_dump_group(Group* grp, int depth)
{
    printf("%sGroup: %s",indent(depth),grp->name);
    return nccr_dump_group_body(grp,depth+1);
}

static ast_err
nccr_dump_group_body(Group* grp, int depth)
{
    int i;
    printf("%sDimensions:\n",indent(depth));
    for(i=0;i<grp->dims.count;i++)
	nccr_dump_dimension(grp->dims.values[i],depth+1);

    printf("%sTypes:\n",indent(depth));
    for(i=0;i<grp->structs.count;i++)
	nccr_dump_structure(grp->structs.values[i],depth+1);
    for(i=0;i<grp->enumTypes.count;i++)
	nccr_dump_enum(grp->enumTypes.values[i],depth+1);

    printf("%sVariables:\n",indent(depth));
    for(i=0;i<grp->vars.count;i++)
	nccr_dump_variable(grp->vars.values[i],depth+1);

    printf("%sAttributes:\n",indent(depth));
    for(i=0;i<grp->atts.count;i++)
	nccr_dump_attribute(grp->atts.values[i],"",depth+1);

    for(i=0;i<grp->groups.count;i++)
	nccr_dump_group(grp->groups.values[i],depth+1);

    return AST_NOERR;
}

enum Dimkind {LENGTH,UNLIMITED,VLEN,PRIVATE,UNDEF};

static char*
nccr_dump_dimkind(Dimension* dim)
{
    enum Dimkind dimkind = LENGTH;
    char* skind;
    static char buffer[1024];

    if(dim->isUnlimited.defined + dim->isVlen.defined
       + dim->isPrivate.defined + dim->length.defined != 1)
	fprintf(stderr,"malformed shape for dimension: %s\n",dim->name.value);

    if(dim->isUnlimited.defined && dim->isUnlimited.value != 0)
	dimkind = UNLIMITED;
    else if(dim->isVlen.defined && dim->isVlen.value != 0)
	dimkind = VLEN;
    else if(dim->isPrivate.defined && dim->isPrivate.value != 0)
	dimkind = PRIVATE;

    switch (dimkind) {
    case VLEN: skind = "*"; break;    
    case PRIVATE: skind = "<PRIVATE>"; break;    
    case LENGTH: skind = ""; break;
    case UNLIMITED: skind = "UNLIMITED"; break;
    case UNDEF: skind = ""; break;    
    }
    snprintf(buffer,sizeof(buffer),"%s(%llu)",
		skind,
		dim->length.value);
    return buffer;
}

static ast_err
nccr_dump_dimension(Dimension* dim, int depth)
{
    char* skind = nccr_dump_dimkind(dim);
    printf("%s",indent(depth));
    printf("%s = ",(dim->name.defined?dim->name.value:"<anon>"));
    printf("%s\n",skind);
    return AST_NOERR;
}

static ast_err
nccr_dump_variable(Variable* var, int depth)
{
    int i;
    printf("%s",indent(depth));
    printf("%s %s",nccr_dump_typeref(var->dataType),var->name);
    nccr_dump_shape(var->shape.count,var->shape.values);
    printf("\n");
    for(i=0;i<var->atts.count;i++)
	nccr_dump_attribute(var->atts.values[i],var->name,depth+1);
    return AST_NOERR;
}

static ast_err
nccr_dump_attribute(Attribute* att, char* varname, int depth)
{
    char* typeref;
    int i;

    printf("%s",indent(depth));
    typeref = nccr_dump_typeref(att->type);
    printf("%s %s:%s",typeref,varname,att->name);
    if(att->data.defined || att->sdata.count > 0) {
	printf(" = ");
        if(att->data.defined)
	    nccr_data_fixed(att->type,&att->data.value);
        for(i=0;i<att->sdata.count;i++) {
	    printf("\"%s\"",att->sdata.values[i]);
        }
    }

    printf(" ;\n");

    return AST_NOERR;
}

static ast_err
nccr_dump_enumconst(EnumType* econst, int depth)
{
    printf("%s%s = %u",indent(depth),econst->value,econst->code);
    return AST_NOERR;
}

static ast_err
nccr_dump_enum(EnumTypedef* tenum, int depth)
{
    int i;
    printf("%s",indent(depth));
    printf("enum %s\n",tenum->name);
    for(i=0;i<tenum->map.count;i++)
	nccr_dump_enumconst(tenum->map.values[i],depth+1);
    return AST_NOERR;
}

static ast_err
nccr_dump_structure(Structure* stype, int depth)
{
    int i;
    printf("%s",indent(depth));
    printf("compound %s\n",stype->name);
    for(i=0;i<stype->vars.count;i++)
	nccr_dump_variable(stype->vars.values[i],depth+1);
// todo: dump other fields as well
    return AST_NOERR;
}

static ast_err
nccr_dump_shape(size_t ndims, Dimension** dimset)
{
    int i;
    for(i=0;i<ndims;i++) {
	char* skind;
	Dimension* dim = dimset[i];
	if(dim->name.defined)
	    skind = dim->name.value;
	else
	    skind = nccr_dump_dimkind(dim);
	printf("[%s]",skind);
    }
    return AST_NOERR;
}

static char*
nccr_dump_typeref(DataType datatype)
{
    switch(datatype) {
    case CHAR: return "CHAR";
    case BYTE: return "BYTE";
    case SHORT: return "SHORT";
    case INT: return "INT";
    case INT64: return "INT64";
    case FLOAT: return "FLOAT";
    case DOUBLE: return "DOUBLE";
    case STRING: return "STRING";
    case STRUCTURE: return "STRUCTURE";
    case SEQUENCE: return "SEQUENCE";
    case ENUM1: return "ENUM1";
    case ENUM2: return "ENUM2";
    case ENUM4: return "ENUM4";
    case OPAQUE: return "OPAQUE";
    case UBYTE: return "UBYTE";
    case USHORT: return "USHORT";
    case UINT: return "UINT";
    case UINT64: return "UINT64";
    }
    return NULL;
}

static char prefix[2048];

static char*
indent(int depth)
{
    char* p = prefix;
    int plen = sizeof(prefix);
    snprintf(p,sizeof(p),"[%d]",depth);
    plen -= strlen(p);
    p += strlen(p);    
    depth++;
    if(depth >= plen) depth = plen - 1;
    memset(p,' ',depth);
    p[depth] = 0;
    return prefix;
}

/**************************************************/

static ast_err
nccr_dump_section(Section* section)
{
    int i;
    uint64_t start = 0;
    uint64_t size = 0;
    uint64_t stride = 1;
    for(i=0;i<section->range.count;i++) {
	Range* range = section->range.values[i];
	if(range->start.defined) start = range->start.value;
	size = range->size;
	if(range->stride.defined) stride = range->stride.value;
	if(stride == 1 && size == 1)
	    printf("[%llu]",start);
	else if(stride == 1)
	    printf("[%llu:%llu]",start,size);
	else
	    printf("[%llu:%llu:%llu]",start,size,stride);
    }
    return AST_NOERR;
}

/**************************************************/

static size_t
data_typesize(DataType datatype)
{
    switch (datatype) {
    case CHAR:
    case BYTE:
    case UBYTE: return 1;

    case SHORT:
    case USHORT: return 2;

    case INT:
    case UINT: return 4;

    case INT64:
    case UINT64: return 8;

    case FLOAT: return 4;
    case DOUBLE: return 8;

    default: break;

    }
    return 0;    
}

static void
data_dumpelement(DataType datatype, bytes_t* data, size_t offset, char* buffer)
{
    uint8_t* pos = data->bytes + offset;

    switch (datatype) {

    case CHAR: {char x = (char) pos[0]; sprintf(buffer,"%c",x);} break;
    case BYTE: {char x = (char) pos[0]; sprintf(buffer,"%hhd",x);} break;

    case UBYTE: {unsigned char x = (unsigned char) pos[0]; sprintf(buffer,"%hhu",x);} break;

    case SHORT: {short x = *(short*)pos; sprintf(buffer,"%hd",x);} break;
    case USHORT: {unsigned short x = *(unsigned short*)pos; sprintf(buffer,"%hu",x);} break;

    case INT: {int x = *(int*)pos; sprintf(buffer,"%d",x);} break;
    case UINT: {unsigned int x = *(unsigned int*)pos; sprintf(buffer,"%u",x);} break;

    case INT64: {long long x = *(long long*)pos; sprintf(buffer,"%lld",x);} break;
    case UINT64: {unsigned long long x = *(unsigned long long*)pos; sprintf(buffer,"%llu",x);} break;

    case FLOAT: {float x = *(float*)pos; sprintf(buffer,"%g",x);} break;
    case DOUBLE: {double x = *(double*)pos; sprintf(buffer,"%g",x);} break;

    default: break;
    }
}

static DataType
mark_signedness(DataType datatype, int isunsigned)
{
    switch (datatype) {
    case CHAR: datatype = UBYTE; break;
    case BYTE: datatype = UBYTE; break;
    case SHORT: datatype = USHORT; break;
    case INT: datatype = UINT; break;
    case INT64: datatype = UINT64; break;
    default: break;
    }
    return datatype;
}

ast_err
nccr_data_dump(Data* dataset, Variable* var, int bigendian, bytes_t* data)
{
    ast_err status = AST_NOERR;
    DataType datatype = dataset->dataType;

    /* Fixed datatype signedness */
    if(var->unsigned_.defined)
        datatype = mark_signedness(datatype,var->unsigned_.value);

    printf("dataset: %s",dataset->varName);
    if(dataset->section.defined) {
	status = nccr_dump_section(dataset->section.value);
        if(status != AST_NOERR) goto done;
    }
    printf("\n");

    if(dataset->dataType == STRING || dataset->dataType == OPAQUE) {
	status = nccr_data_vlen(dataset->dataType,data);
    } else { /* simple integer-like types */
	status = nccr_data_fixed(dataset->dataType,data);
    }
    printf("\n");

done:
    return AST_NOERR;
}

static ast_err
nccr_data_fixed(DataType datatype, bytes_t* data)
{
    ast_err status = AST_NOERR;
    int typesize = data_typesize(datatype);
    size_t offset;
    char buffer[1024];

    if(typesize > 0) { /* validate data length */
        if(data->nbytes % typesize != 0) {
            fprintf(stderr,"*** FAIL: nbytes %% data != 0\n");
            status = AST_EFAIL;
            goto done;
        }
    }
    for(offset=0;offset<data->nbytes;offset+=typesize) {
	if(offset > 0) printf(", ");
        data_dumpelement(datatype, data, offset, buffer);
	printf("%s",buffer);
    }
done:
    return status;
}

static ast_err
nccr_data_vlen(DataType datatype, bytes_t* data)
{
    size_t size;
    uint64_t nobjects;
    int i,j;

    /* extract the count of the number of objects */
    nobjects = varint_decode(10,data->bytes,&size);
    data->nbytes -= size;
    data->bytes += size;

    for(i=0;i<nobjects;i++) {
	bytes_t bytes;
	uint64_t len;
        /* extract the count + bytes */
        len = varint_decode(10,data->bytes,&size);
        data->nbytes -= size;
        data->bytes += size;
	/* pull the bytestring */
	bytes.nbytes = len;
	bytes.bytes = (uint8_t*)malloc(len+1);
	memcpy(bytes.bytes,data->bytes,len);
	bytes.bytes[data->nbytes] = '\0';
        data->nbytes -= len;
        data->bytes += len;
	if(datatype == STRING) {
	    printf("\"%s\"",bytes.bytes);
	} else {
	    printf("(%d)0x",(int)len);
	    for(j=0;j<len;j++) {
		printf("%2x",bytes.bytes[j]);
	    }
	}
    }
    return AST_NOERR;
}
