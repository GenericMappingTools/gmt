/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

#ifndef _GMT_MEMORY_H
#define _GMT_MEMORY_H

enum GMT_enum_mem_alloc {	/* Initial memory for 2 double columns is 32 Mb */
	GMT_INITIAL_MEM_COL_ALLOC	= 2U,
	GMT_INITIAL_MEM_ROW_ALLOC	= 2097152U	/* 2^21 */	
};

/* Macros to reallocate memory for groups of 2, 3 or 4 arrays at a time of the same size/type */
#if defined (DEBUG) || defined (MEMDEBUG)
#define GMT_malloc(C,a,n,n_alloc,type) GMT_malloc_func(C,a,n,n_alloc,sizeof(type),__SOURCE_LINE_FUNC)
#else
#define GMT_malloc(C,a,n,n_alloc,type) GMT_malloc_func(C,a,n,n_alloc,sizeof(type),__func__)
#endif
/* The __kp = n_alloc below is needed since NULL may be passed. __k is used to ensure only the final GMT_malloc call changes n_alloc (unless it is NULL) */
#define GMT_malloc2(C,a,b,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = GMT_malloc(C,a,n,&__k,type); b = GMT_malloc(C,b,n,n_alloc,type); }
#define GMT_malloc3(C,a,b,c,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = GMT_malloc(C,a,n,&__k,type); __k = (__kp) ? *__kp : 0U; b = GMT_malloc(C,b,n,&__k,type); c = GMT_malloc(C,c,n,n_alloc,type); }
#define GMT_malloc4(C,a,b,c,d,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = GMT_malloc(C,a,n,&__k,type); __k = (__kp) ? *__kp : 0U; b = GMT_malloc(C,b,n,&__k,type); __k = (__kp) ? *__kp : 0U; c = GMT_malloc(C,c,n,&__k,type); d = GMT_malloc(C,d,n,n_alloc,type); }

/* Convenience macro for GMT_memory_func */
#if defined (DEBUG) || defined (MEMDEBUG)
#define GMT_memory(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),false,__SOURCE_LINE_FUNC)
#define GMT_memory_aligned(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),true,__SOURCE_LINE_FUNC)
#else
#define GMT_memory(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),false,__func__)
#define GMT_memory_aligned(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),true,__func__)
#endif

/* Convenience macro for GMT_free_func */
#if defined (DEBUG) || defined (MEMDEBUG)
#define GMT_free(C,ptr) (GMT_free_func(C,ptr,false,__SOURCE_LINE_FUNC),(ptr)=NULL)
#define GMT_free_aligned(C,ptr) (GMT_free_func(C,ptr,true,__SOURCE_LINE_FUNC),(ptr)=NULL)
#else
#define GMT_free(C,ptr) (GMT_free_func(C,ptr,false,__func__),(ptr)=NULL)
#define GMT_free_aligned(C,ptr) (GMT_free_func(C,ptr,true,__func__),(ptr)=NULL)
#endif

#ifdef MEMDEBUG

struct MEMORY_ITEM {
	size_t size;	/* Size of memory allocated */
	void *ptr;	/* Memory pointer */
	char *name;	/* Source filename and line or function name */
	size_t ID;	/* Unique ID for this allocation */
	struct MEMORY_ITEM *l, *r;
};

struct MEMORY_TRACKER {
	bool active;	/* Normally true but can be changed to focus on just some allocations */
	bool search;	/* Normally true but can be changed to skip searching when we know we add a new item */
	bool do_log;	/* true if we wish to write detailed alloc/free log */
	uint64_t n_ptr;		/* Number of unique pointers to allocated memory */
	uint64_t n_allocated;	/* Number of items allocated by GMT_memory */
	uint64_t n_reallocated;	/* Number of items reallocated by GMT_memory */
	uint64_t n_freed;	/* Number of items freed by GMT_free */
	uint64_t n_ID;		/* Running number assigned to new allocations */
	uint64_t find;		/* If > 0 then we look for this ID to be allocated */
	size_t current;		/* Memory allocated at current time */
	size_t maximum;		/* Highest memory count during execution */
	size_t largest;		/* Highest memory allocation to a single variable */
	size_t n_alloc;		/* Allocated size of memory pointer array */
	struct MEMORY_ITEM *root; /* Pointer to splay tree */
	FILE *fp;	/* For logging if GMT_TRACK_MEMORY is 2 */
};

/* Items needed if -DMEMDEBUG is in effect */
EXTERN_MSC int GMT_memtrack_init (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_memtrack_report (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_memtrack_on (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_memtrack_off (struct GMT_CTRL *GMT);

#endif

#endif /* _GMT_MEMORY_H */
