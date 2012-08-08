#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include <ast_runtime.h>
#include <ast_internal.h>

#define DFALTALLOC 1024

/* Should be exactly eight characters long */
static const char BYTEIO_UIDSTRING[8] = "byteio  ";
static uint64_t BYTEIO_UID = 0;

/**************************************************/
/* Byte IO */
/**************************************************/

/* Create a runtime readers and writers backed by a byte buffer */

/* Define the data kept in the stream field of ast_runtime */
struct _ast_bytestream {
    bool_t extendible; // 0 => cannot extend
    size_t alloc;
    size_t pos;
    uint8_t* buffer;
    struct _ast_stack { /* Only used when reading */
        size_t maxpos;
        struct _ast_stack* stack;
    } stack;
};

static size_t ast_byteio_write(ast_runtime* rt, size_t len, uint8_t* data);
static size_t ast_byteio_read(ast_runtime* rt, size_t len, uint8_t* data);
static int ast_byteio_mark(ast_runtime* rt, size_t count);
static int ast_byteio_unmark(ast_runtime* rt);
static int ast_byteio_reclaim(ast_runtime* rt);
static void* ast_byteio_alloc(ast_runtime* rt, size_t len);
static void ast_byteio_free(ast_runtime* rt, void* mem);

static ast_runtime_ops byteops = {
ast_byteio_write,
ast_byteio_read,
ast_byteio_mark,
ast_byteio_unmark,
ast_byteio_reclaim,
ast_byteio_alloc,
ast_byteio_free
};

static size_t
ast_byteio_write(ast_runtime* rt, size_t len, uint8_t* data)
{
    if(rt == NULL || rt->uid != BYTEIO_UID || rt->mode != AST_WRITE)
	return AST_EFAIL;
    if(len == 0 || data == NULL) return AST_NOERR;
    struct _ast_bytestream*  stream = (struct _ast_bytestream*)rt->stream;
    while(stream->pos+len >= stream->alloc) {
	if(stream->extendible) {
	    char* newbuffer = NULL;
   	    size_t delta = DFALTALLOC;
	    if(stream->buffer == NULL) {
		stream->pos = 0;
	        stream->alloc += delta;		
		newbuffer = malloc(stream->alloc);
		if(newbuffer == NULL) return AST_ENOMEM;
	    } else {
		stream->alloc += delta;
		newbuffer = realloc(stream->buffer,stream->alloc);
	    }
  	    if(newbuffer == NULL) return AST_ENOMEM;
	} else return AST_EFAIL; /* Cannot extend */
    } /*while*/
    memcpy(stream->buffer+stream->pos,(void*)data,len);
    stream->pos += len;
    return len;
}

static size_t
ast_byteio_read(ast_runtime* rt, size_t len, uint8_t* data)
{
    struct _ast_bytestream*  stream = NULL;
    if(rt == NULL || rt->uid != BYTEIO_UID || rt->mode != AST_READ)
	return AST_EFAIL;
    if(len == 0 || data == NULL) return AST_NOERR;
    stream = (struct _ast_bytestream*)rt->stream;    
    if(stream->pos+len > stream->stack.maxpos) {
	rt->err = AST_EOF;
	len = 0;
	stream->pos = stream->stack.maxpos;
    } else if(stream->pos+len > stream->alloc) {
	rt->err = AST_EOF;
	len = 0;
	stream->pos = stream->alloc;
    } else {
        memcpy((void*)data,stream->buffer+stream->pos,len);
        stream->pos += len;
    }
    return len;
}

/* limit reads to next n bytes */
static int
ast_byteio_mark(ast_runtime* rt, size_t count)
{
    struct _ast_bytestream* stream = NULL;
    struct _ast_stack* node = NULL;
    if(rt == NULL || rt->uid != BYTEIO_UID || rt->mode != AST_READ)
	return AST_EFAIL;
    stream = (struct _ast_bytestream*)rt->stream;
    node = ast_alloc(rt,sizeof(struct _ast_stack));
    if(node == NULL) return AST_ENOMEM;
    *node = stream->stack;
    stream->stack.stack = node;
    stream->stack.maxpos = stream->pos+count;
    /* stream maxpos must be <= alloc */
    if(stream->stack.maxpos > stream->alloc)
	assert(0);
/*        stream->stack.maxpos = stream->alloc; */
    return AST_NOERR;
}

/* Pop out of the current mark */
static int
ast_byteio_unmark(ast_runtime* rt)
{
    struct _ast_bytestream* stream = NULL;
    struct _ast_stack* node = NULL;
    if(rt == NULL || rt->uid != BYTEIO_UID || rt->mode != AST_READ)
	return AST_EFAIL;
    stream = (struct _ast_bytestream*)rt->stream;
    node = stream->stack.stack;
    stream->stack.maxpos = node->maxpos;
    if(stream->stack.maxpos > stream->alloc)
	assert(0);
    stream->stack.stack = node->stack;
    ast_free(rt,node);
    return AST_NOERR;
}


static int
ast_byteio_reclaim(ast_runtime* rt)
{
    struct _ast_bytestream* stream = NULL;
    if(rt == NULL) return AST_NOERR;
    if(rt->uid != BYTEIO_UID) return AST_EFAIL;
    stream = (struct _ast_bytestream*)rt->stream;
    if(stream->extendible && stream->buffer != NULL) free(stream->buffer);
    struct _ast_stack* curr =  stream->stack.stack;
    while(curr != NULL) {
        struct _ast_stack* next =  stream->stack.stack;
        ast_free(rt,curr);
	curr = next;	
    }
    free(rt);
    return AST_NOERR;
}

/* Extract the current position from a byteio runtime */
ast_err
ast_byteio_count(ast_runtime* rt, size_t* countp)
{
    struct _ast_bytestream* stream = NULL;
    if(rt == NULL || rt->uid != BYTEIO_UID)
	return AST_EFAIL;
    stream = (struct _ast_bytestream*)rt->stream;
    if(countp) *countp = stream->pos;
    return AST_NOERR;
}

/* Extract the buffer from a byteio runtime */
ast_err
ast_byteio_content(ast_runtime* rt, bytes_t* result)
{
    bytes_t content;
    struct _ast_bytestream* stream = NULL;
    if(result == NULL) return AST_NOERR;
    if(rt == NULL || rt->uid != BYTEIO_UID || rt->mode != AST_WRITE)
	return AST_EFAIL;
    stream = (struct _ast_bytestream*)rt->stream;
    content.nbytes = stream->pos;
    content.bytes = stream->buffer;
    /* prevent later writes */
    stream->alloc = 0;
    stream->pos = 0;
    stream->buffer = NULL;
    if(result) *result = content;
    return AST_NOERR;
}


int
ast_byteio_new(ast_iomode mode, void* buf, size_t len, ast_runtime** rtp)
{
    int status = AST_NOERR;
    ast_runtime* rt = NULL;
    struct _ast_bytestream*  stream = NULL;

    if(BYTEIO_UID == 0)
	BYTEIO_UID = ast_create_unique_id(BYTEIO_UIDSTRING);
    rt = malloc(sizeof(ast_runtime));
    if(rt == NULL) {status = AST_ENOMEM; goto fail;}
    memset(rt,0,sizeof(ast_runtime));

    stream = malloc(sizeof(struct _ast_bytestream));
    if(stream == NULL) {status = AST_ENOMEM; goto fail;}
    memset(stream,0,sizeof(struct _ast_bytestream));

    rt->stream = (void*)stream;
    rt->ops = &byteops;
    rt->uid = BYTEIO_UID;
    rt->mode = mode;

    if(mode == AST_READ) {
        if(buf == NULL || len == 0) {status = AST_EFAIL; goto fail;}
	stream->extendible = 0;
	stream->alloc = len;
	stream->pos = 0;
	stream->buffer = buf;
	stream->stack.maxpos = stream->alloc;
    } else if(mode == AST_WRITE) {
	if(buf == NULL) {
	    stream->extendible = 1;
	    /* Use default initial length */
	    len = DFALTALLOC;
	    buf = malloc(len);
	    if(buf == NULL) {status = AST_ENOMEM; goto fail;}
	} else {
	    stream->extendible = 0;
	}
	stream->alloc = len;
	stream->pos = 0;
	stream->buffer = buf;
    } else {status = AST_EFAIL; goto fail;}
    if(rtp) *rtp = rt;
    return AST_NOERR;

fail:
    if(rt != NULL) free(rt);
    if(stream != NULL) free(stream);
    if(stream->buffer != buf && mode == AST_WRITE) free(stream->buffer);
    return status;
}


/* Wrap calloc and free */
static void*
ast_byteio_alloc(ast_runtime* rt, size_t len)
{
    if(len == 0) return NULL;
    return calloc(1,len);
}

static void
ast_byteio_free(ast_runtime* rt, void* mem)
{
    if(mem != NULL)
	free(mem);
}
