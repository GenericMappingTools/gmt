#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include <ast_runtime.h>
#include <ast_internal.h>
#include <ast_debug.h>

/**
 * Define MAXWIRELEN to be at least big enough
 * to hold instance of any possible primitives
 * except string and byte and as encoded using any
 * of the possible encodings (e.g. int32, int64, varint, etc)
 * => Must hold at least lub(64/7) = 10 */
#define MAXWIRELEN 16

/* max length of a 32 varint Must hold at least lub(32/7) = 5 */
#define VARINTMAX32 5
/* max length of a 64 varint Must hold at least lub(64/7) = 10 */
#define VARINTMAX64 10

#define VARINTMAX VARINTMAX64

/* Define the maximum c-type size;
   should be max(sizeof(bytes_t),sizeof(uint64_t)
*/
#define MAXCTYPESIZE (sizeof(bytes_t)>sizeof(uint64_t)?sizeof(bytes_t):sizeof(uint64_t))


/* Map the primitives (ast_sort) to the associated wiretype */
static int sort_wiretype_map[] = {
ast_64bit,   /*ast_double*/
ast_32bit,   /*ast_float*/
ast_varint,  /*ast_int32*/
ast_varint,  /*ast_int64*/
ast_varint,  /*ast_uint32*/
ast_varint,  /*ast_uint64*/
ast_varint,  /*ast_sint32*/
ast_varint,  /*ast_sint64*/
ast_32bit,   /*ast_fixed32*/
ast_64bit,   /*ast_fixed64*/
ast_32bit,   /*ast_sfixed32*/
ast_64bit,   /*ast_sfixed64*/
ast_counted, /*ast_string*/
ast_counted, /*ast_bytes*/
ast_varint,  /*ast_bool*/
ast_varint,  /*ast_enum*/
};


enum Sortsize { sizecount=0, size32=32, size64=64 };

/* Map sorts to their value sizes; 0=>counted */
#ifdef IGNORE
static enum Sortsize sort_size[] = {
size64,     /*ast_double*/
size32,     /*ast_float*/
size32,     /*ast_int32*/
size64,     /*ast_int64*/
size32,     /*ast_uint32*/
size64,     /*ast_uint64*/
size32,     /*ast_sint32*/
size64,     /*ast_sint64*/
size32,     /*ast_fixed32*/
size64,     /*ast_fixed64*/
size32,     /*ast_sfixed32*/
size32,     /*ast_sfixed64*/
sizecount,  /*ast_string*/
sizecount,  /*ast_bytes*/
size32,     /*ast_bool*/
size32,     /*ast_enum*/
};
#endif

/* Define a null value for bytes_t */
bytes_t bytes_t_null = {0,NULL};

static int ast_readvarint(ast_runtime* rt, uint8_t* buffer, size_t* countp);

/* Invoke the ast_runtime operators */

static size_t
ast_write(ast_runtime* rt, size_t count, uint8_t* data)
{
    return rt->ops->write(rt,count,data);
}

static size_t
ast_read(ast_runtime* rt, size_t count, uint8_t* buffer)
{
    return rt->ops->read(rt,count,buffer);
}

ast_err
ast_mark(ast_runtime* rt, size_t avail)
{
    return rt->ops->mark(rt,avail);
}

ast_err
ast_unmark(ast_runtime* rt)
{
    return rt->ops->unmark(rt);
}

ast_err
ast_reclaim(ast_runtime* rt)
{
    return rt->ops->reclaim(rt);
}

void*
ast_alloc(ast_runtime* rt, size_t len)
{
    if(len == 0) return NULL;
    return rt->ops->alloc(rt,len);
}

void
ast_free(ast_runtime* rt, void* mem)
{
    if(mem == NULL) return;
    return rt->ops->free(rt,mem);
}

/* Define the primitive W/R/F functions */

/* Read the tag and return the relevant info */
ast_err
ast_read_tag(ast_runtime* rt, int* wiretypep, int* fieldnop)
{
    ast_err status = AST_NOERR;
    uint8_t buffer[MAXWIRELEN];
    size_t count;
    uint32_t key, wiretype, fieldno;

    /* Extract the wiretype + index */
    status = ast_readvarint(rt,buffer,&count);
    if(status != AST_NOERR) ATHROW(status,done);

    /* convert from varint */
    key = uint32_decode(count,buffer);

    /* Extract the wiretype and fieldno */
    wiretype = (key & 0x7);
    fieldno = (key >> 3);

    /* return the fieldno and wiretype */
    if(fieldnop) *fieldnop = fieldno;
    if(wiretypep) *wiretypep = wiretype;

done:
    return ACATCH(status);
}

static ast_err
ast_read_primitive_data(ast_runtime* rt, const ast_sort sort, void* valuep)
{
    ast_err status = AST_NOERR;
    uint8_t buffer[MAXWIRELEN];
    size_t count,len;
    uint32_t wiretype;

    if(valuep == NULL) AERR(status,AST_EFAIL,done);

    /* compute the wiretype from the sort */
    wiretype = sort_wiretype_map[sort];

    /* Based on the wiretype, extract the proper number of bytes */
    switch (wiretype) {
    case ast_varint:
        status = ast_readvarint(rt,buffer,&len);
        if(status != AST_NOERR) ATHROW(status,done);
	break;
    case ast_32bit:
	len = 4;
        if(ast_read(rt,len,buffer) != len) AERR(status,AST_EOF,done);
	break;
    case ast_64bit:
	len = 8;
        if(ast_read(rt,len,buffer) != len) AERR(status,AST_EOF,done);
	break;
    case ast_counted: /* get the count */
        status = ast_readvarint(rt,buffer,&len);
        if(status != AST_NOERR) ATHROW(status,done);
	count = (size_t)uint64_decode(len,buffer);	
	break;
    default: ATHROW(status,done);
    }

    /* Now extract the value */
    switch (sort) {
    case ast_enum: /* fall thru */
    case ast_int32:
	*((int32_t*)valuep) = int32_decode(len,buffer);
	break;
    case ast_int64:
	*((int64_t*)valuep) = int64_decode(len,buffer);
	break;
    case ast_uint32:
	*((uint32_t*)valuep) = uint32_decode(len,buffer);
	break;
    case ast_uint64:
	*((uint64_t*)valuep) = uint64_decode(len,buffer);
	break;
    case ast_sint32:
	*((int32_t*)valuep) = unzigzag32(uint32_decode(len,buffer));
	break;
    case ast_sint64:
	*((int64_t*)valuep) = unzigzag64(uint64_decode(len,buffer));
	break;
    case ast_bool:
	*((bool_t*)valuep) = boolean_decode(len,buffer);
	break;
    case ast_fixed32:
	*((uint32_t*)valuep) = fixed32_decode(buffer);
	break;
    case ast_sfixed32:
	*((int32_t*)valuep) = fixed32_decode(buffer);
	break;
    case ast_fixed64:
	*((uint64_t*)valuep) = fixed64_decode(buffer);
	break;
    case ast_sfixed64:
	*((int64_t*)valuep) = fixed64_decode(buffer);
	break;
    case ast_float: {
	uint32_t value = fixed32_decode(buffer);
	memcpy(valuep,&value,sizeof(float));
	} break;
    case ast_double: {
	uint64_t value = fixed64_decode(buffer);
	memcpy(valuep,&value,sizeof(double));
	} break;
    case ast_string: {
	/* Count already holds the length of the string */
	char* stringvalue = ast_alloc(rt,count+1);
	if(stringvalue == NULL) AERR(status,AST_ENOMEM,done);
	if(ast_read(rt,count,(uint8_t*)stringvalue) != count) AERR(status,AST_EOF,done);
	stringvalue[count] = '\0';
	*((char**)valuep) = stringvalue;
	} break;
    case ast_bytes: {
	/* Count already holds the length of the byte string */
	uint8_t* bytestring = ast_alloc(rt,count);
	if(bytestring == NULL) AERR(status,AST_ENOMEM,done);
	if(ast_read(rt,count,bytestring) != count) AERR(status,AST_EOF,done);
	((bytes_t*)valuep)->nbytes = count;
	((bytes_t*)valuep)->bytes = bytestring;
	} break;
    default:
	ATHROW(status,done);
	break;
    }

done:
    return ACATCH(status);
}

ast_err
ast_read_primitive(ast_runtime* rt, const ast_sort sort,
                         const int fieldno,void* valuep)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype;

    if(valuep == NULL) AERR(status,AST_EFAIL,done);

    /* compute the wiretype from the sort */
    wiretype = sort_wiretype_map[sort];
    status = ast_read_primitive_data(rt,sort,valuep);
    if(status != AST_NOERR) goto done;
done:
    return ACATCH(status);
}

static ast_err
ast_write_primitive_data(ast_runtime* rt, const ast_sort sort,
			  const int fieldno,
			  const void* valuep)
{
    ast_err status = AST_NOERR;
    uint8_t buffer[MAXWIRELEN];
    size_t count;

    /* Write the data in proper wiretype format using the sort */
    switch (sort) {
    case ast_int32:
	count = int32_encode(*(int32_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done)};
	break;
    case ast_int64:
	count = int64_encode(*(int64_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_uint32:
	count = uint32_encode(*(uint32_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_uint64:
	count = uint64_encode(*(uint64_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_sint32:
	count = sint32_encode(*(int32_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_sint64:
	count = sint64_encode(*(int64_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_bool:
	count = boolean_encode(*(bool_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_enum:
	count = int32_encode(*(int32_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_fixed32: /* fall thru */
    case ast_sfixed32: /* fall thru */
    case ast_float:
	count = fixed32_encode(*(uint32_t*)valuep,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_fixed64:  /* fall thru */
    case ast_sfixed64: /* fall thru */
    case ast_double:
	count = fixed64_encode(*(uint64_t*)valuep,buffer);
        if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	break;
    case ast_string: {
	char* stringvalue = *(char**)valuep;
	size_t len = strlen(stringvalue);
	count = uint32_encode(len,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	if(ast_write(rt,len,(uint8_t*)stringvalue) != count) {status = AST_EIO; ATHROW(status,done);}
	} break;
    case ast_bytes: {
	bytes_t* bytevalue = (bytes_t*)valuep;		
	count = uint32_encode(bytevalue->nbytes,buffer);
	if(ast_write(rt,count,buffer) != count) {status = AST_EIO; ATHROW(status,done);}
	if(ast_write(rt,bytevalue->nbytes,bytevalue->bytes) != bytevalue->nbytes) {status = AST_EIO; ATHROW(status,done);}
	} break;
    default:
	ATHROW(status,done);
	break;
    }
done:
    return ACATCH(status);
}

static size_t
ast_encode_tag(ast_runtime* rt,
		const uint32_t wiretype, const uint32_t fieldno,
		uint8_t* buffer/*[MAXWIRELEN]*/)
{
    int key;
    size_t count;

    key = (wiretype | (fieldno << 3));
    /* convert key to varint */
    count = uint32_encode(key,(uint8_t*)buffer);
    return count;
}

ast_err
ast_write_tag(ast_runtime* rt, const uint32_t wiretype, const uint32_t fieldno)
{
    size_t count;
    uint8_t buffer[MAXWIRELEN];

    count = ast_encode_tag(rt,wiretype,fieldno,buffer);
    /* Write it out */
    if(ast_write(rt,count,buffer) != count) return AST_EIO;
    return AST_NOERR;
}

/* Procedure to write out count */
ast_err
ast_write_count(ast_runtime* rt, const size_t count)
{
    uint8_t buffer[VARINTMAX64];
    /* write count as varint */
    size_t len = uint64_encode((uint64_t)count,buffer);
    if(ast_write(rt,len,buffer) != len) return AST_EIO;
    return AST_NOERR;
}

ast_err
ast_write_primitive(ast_runtime* rt, const ast_sort sort, const int fieldno, const void* valuep)
{
    ast_err status = AST_NOERR;
    ast_wiretype wiretype;

    wiretype = sort_wiretype_map[sort];

    status = ast_write_tag(rt,wiretype,fieldno);
    if(status != AST_NOERR) goto done;

    /* write the data part */
    status = ast_write_primitive_data(rt,sort,fieldno,valuep);
    if(status != AST_NOERR) goto done;
done:
    return status;
}

ast_err
ast_read_count(ast_runtime* rt, size_t* countp)
{
    uint64_t count = 0;
    uint8_t buffer[VARINTMAX64];
    size_t len;
    ast_err status = ast_readvarint(rt,buffer,&len);
    if(status != AST_NOERR) ATHROW(status,done);
    count = uint64_decode(len,buffer);
    if(countp) *countp = (size_t)count;
done:
    return ACATCH(status);
}

/* Append + extend a REPEAT field */
ast_err
ast_repeat_append(ast_runtime* rt, ast_sort sort,void* repeater0, void* value)
{
    size_t sortsize = ast_ctypesize(rt,sort);
    ast_repeated_field* repeater = (ast_repeated_field*)repeater0;
    char* target;

    if(repeater->count == 0 || repeater->values == NULL) {
	repeater->count = 0;
	repeater->values = ast_alloc(rt,sortsize);
	if(repeater->values == NULL) return ACATCH(AST_ENOMEM);
    } else {
	void* newmem = ast_alloc(rt,(repeater->count+1)*sortsize);
	if(newmem == NULL) return ACATCH(AST_ENOMEM);
	memcpy(newmem,repeater->values,(repeater->count)*sortsize);
	repeater->values = newmem;
    }
    target = ((char*)repeater->values) + (repeater->count * sortsize);
    memcpy((void*)target,value,sortsize);
    repeater->count++;
    return AST_NOERR;
}

size_t
ast_ctypesize(ast_runtime* rt, ast_sort sort)
{
    switch(sort) {
    case ast_double: return sizeof(double);
    case ast_float: return sizeof(float);

    case ast_enum:
    case ast_bool:
    case ast_int32:
    case ast_sint32:
    case ast_fixed32:
    case ast_sfixed32:
    case ast_uint32: return sizeof(uint32_t);

    case ast_int64:
    case ast_sint64:
    case ast_fixed64:
    case ast_sfixed64:
    case ast_uint64: return sizeof(uint64_t);

    case ast_string: return sizeof(char*);
    case ast_bytes: return sizeof(bytes_t);

    case ast_message:return sizeof(void*);

    default: assert(0);
    }
    return 0;
}

size_t
ast_get_tagsize(ast_runtime* rt, ast_sort sort, int fieldno)
{
    size_t count;
    int wiretype;
    uint8_t buffer[MAXWIRELEN];

    wiretype = sort_wiretype_map[sort];
    count = ast_encode_tag(rt,wiretype,fieldno,buffer);
    return count;
}


/* Return on the wire # of bytes for this value */ 
size_t
ast_get_size(ast_runtime* rt, ast_sort sort, void* valuep)
{
    size_t count = 0;
    switch (sort) {
    case ast_enum: /* fall thru */
    case ast_int32:
	count = int32_size(*(int32_t*)valuep);
	break;
    case ast_int64:
	count = int64_size(*(int64_t*)valuep);
	break;
    case ast_uint32:
	count = uint32_size(*(uint32_t*)valuep);
	break;
    case ast_uint64:
	count = uint64_size(*(uint64_t*)valuep);
	break;
    case ast_sint32:
	count = sint32_size(*(int32_t*)valuep);
	break;
    case ast_sint64:
	count = sint64_size(*(int64_t*)valuep);
	break;
    case ast_bool:
	count = 1;
	break;
    case ast_fixed32:
	count = 4;
	break;
    case ast_sfixed32:
	count = 4;
	break;
    case ast_fixed64:
	count = 8;
	break;
    case ast_sfixed64:
	count = 8;
	break;
    case ast_float:
	count = 4;
	break;
    case ast_double:
	count = 8;
	break;
    case ast_string:
	/* string count is size for length counter + strlen(string) */
	count = 0;
	if(valuep != NULL) {
	    char* stringvalue = (char*)valuep;
	    uint32_t slen = strlen(stringvalue);
	    count = uint32_size(slen);
	    count += slen;
	}
	break;
    case ast_bytes:
	count = 0;
	if(valuep != NULL) {
	    bytes_t* bytedata = (bytes_t*)valuep;
	    count = uint32_size(bytedata->nbytes);
	    count += bytedata->nbytes;
	}
	break;
    default:
	break;
    }
    return count;
}

/* Procedure to extract arbitrary varint bytes from input stream */
static ast_err
ast_readvarint(ast_runtime* rt, uint8_t* buffer, size_t* countp)
{
    size_t i=0;
    uint8_t byte[1];
    while(i<VARINTMAX64) {
	size_t count = ast_read(rt,1,byte);
	if(count < 1) return rt->err;
	buffer[i++] = 0x7f & byte[0];
	if((0x80 & byte[0]) == 0) break;
    }
    if(countp) *countp = i;
    return AST_NOERR;
}

/* Given an unknown field, skip past it */
ast_err
ast_skip_field(ast_runtime* rt, int wiretype, int fieldno)
{
    ast_err status = AST_NOERR;
    uint8_t buffer[MAXWIRELEN];
    size_t len;
    switch (wiretype) {
    case ast_varint:
        status = ast_readvarint(rt,buffer,&len);
        if(status != AST_NOERR) ATHROW(status,done);
	break;
    case ast_32bit:
	len = 4;
        status = ast_read(rt,len,buffer);
        if(status != AST_NOERR) ATHROW(status,done);
	break;
    case ast_64bit:
	len = 8;
        status = ast_read(rt,len,buffer);
        if(status != AST_NOERR) ATHROW(status,done);
	break;
    case ast_counted:
        status = ast_readvarint(rt,buffer,&len);
        if(status != AST_NOERR) ATHROW(status,done);
        /* get the count */
	len = (size_t)uint64_decode(len,buffer);	
	/* Now skip "count" bytes */
	while(len > 0) {
	    size_t n = (len > sizeof(buffer) ? sizeof(buffer) : len);
	    size_t actual = ast_read(rt,n,buffer);
	    if(actual < n) {status = rt->err; ATHROW(status,done);}
	}
	break;
    default: status = AST_EFAIL; break;
    }
done:
    return ACATCH(status);
}

ast_err
ast_write_primitive_packed(ast_runtime* rt, const ast_sort sort,
                           const int fieldno, const void* valuep)
{
    ast_err status = AST_NOERR;
    int i;
    uint8_t buffer[MAXWIRELEN];
    size_t count,size;
    const ast_repeated_field* repeater = (const ast_repeated_field*)valuep;
    char* p;
    int typesize;

    if(repeater->count == 0) goto done;

    typesize = ast_ctypesize(rt,sort);

    /* wiretype is always ast_counted */
    status = ast_write_tag(rt,ast_counted,fieldno);
    if(status != AST_NOERR) goto done;

    /* Compute the size of what is to be written */
    for(p=repeater->values,size=0,i=0;i<repeater->count;i++,p+=typesize) {
	size += ast_get_size(rt,sort,(void*)p);
    }
    /* convert size to varint and write it out */
    count = uint32_encode(size,buffer);
    if(ast_write(rt,count,buffer) != count) {status = AST_EIO; goto done;}
    /* Now write each value */
    for(p=repeater->values,i=0;i<repeater->count;i++,p+=typesize) {
	status = ast_write_primitive_data(rt,sort,fieldno,(void*)p);
        if(status != AST_NOERR) goto done;
    }

done:
    return ACATCH(status);
}

ast_err
ast_read_primitive_packed(ast_runtime* rt, const ast_sort sort, const int fieldno,
                   void* valuep)
{
    ast_err status = AST_NOERR;
    size_t count,len;
    const ast_repeated_field* repeater = (const ast_repeated_field*)valuep;
    int typesize = ast_ctypesize(rt,sort);
    char* p = NULL;
    uint8_t varintbuf[VARINTMAX64];
    uint8_t cbuffer[MAXCTYPESIZE];

    if(valuep == NULL) return ACATCH(AST_EFAIL);

    /* wire type should always be ast_counted */
    status = ast_readvarint(rt,varintbuf,&len);
    if(status != AST_NOERR) ATHROW(status,done);
    /* extract the count */
    count = (size_t)uint64_decode(len,varintbuf);	

    /* mark the runtime */
    status = ast_mark(rt,count);
    if(status != AST_NOERR) ATHROW(status,done);

    /* Read until we hit the end of the mark */
    for(count=0,p=(char*)valuep;status == AST_NOERR;count++,p+=typesize) {
	status = ast_read_primitive_data(rt,sort,cbuffer);
        if(status != AST_NOERR) break;
	status = ast_repeat_append(rt,sort,(void*)repeater,(void*)cbuffer);
    }
    if(status == AST_EOF) status = AST_NOERR;
    if(status != AST_NOERR) ATHROW(status,done);
    status = ast_unmark(rt);

done:
    return ACATCH(status);
}

ast_err
ast_reclaim_string(ast_runtime* rt, char* value)
{
    ast_free(rt,value);
    return AST_NOERR;
}

ast_err
ast_reclaim_bytes(ast_runtime* rt, bytes_t* value)
{
    ast_free(rt,value->bytes);
    return AST_NOERR;
}

ast_runtime_ops*
ast_byteio_getopts(ast_runtime* rt)
{
    return rt->ops;
}

ast_err
ast_byteio_setopts(ast_runtime* rt, ast_runtime_ops* ops)
{
    rt->ops = ops;
}
