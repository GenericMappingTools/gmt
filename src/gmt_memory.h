/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

/*!
 * \file gmt_M_memory.h
 * \brief 
 */

#ifndef GMT_MEMORY_H
#define GMT_MEMORY_H

enum GMT_enum_mem_alloc {	/* Initial memory for 2 double columns is 32 Mb */
	GMT_INITIAL_MEM_COL_ALLOC	= 2U,
	GMT_INITIAL_MEM_ROW_ALLOC	= 2097152U	/* 2^21 */	
};

/*! Macros to reallocate memory for groups of 2, 3 or 4 arrays at a time of the same size/type */
#if defined (DEBUG) || defined (MEMDEBUG)
#define gmt_M_malloc(C,a,n,n_alloc,type) gmt_malloc_func(C,a,n,n_alloc,sizeof(type),__SOURCE_LINE_FUNC)
#else
#define gmt_M_malloc(C,a,n,n_alloc,type) gmt_malloc_func(C,a,n,n_alloc,sizeof(type),__func__)
#endif
/* The __kp = n_alloc below is needed since NULL may be passed. __k is used to ensure only the final gmt_M_malloc call changes n_alloc (unless it is NULL) */
#define gmt_M_malloc2(C,a,b,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = gmt_M_malloc(C,a,n,&__k,type); b = gmt_M_malloc(C,b,n,n_alloc,type); }
#define gmt_M_malloc3(C,a,b,c,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = gmt_M_malloc(C,a,n,&__k,type); __k = (__kp) ? *__kp : 0U; b = gmt_M_malloc(C,b,n,&__k,type); c = gmt_M_malloc(C,c,n,n_alloc,type); }
#define gmt_M_malloc4(C,a,b,c,d,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = gmt_M_malloc(C,a,n,&__k,type); __k = (__kp) ? *__kp : 0U; b = gmt_M_malloc(C,b,n,&__k,type); __k = (__kp) ? *__kp : 0U; c = gmt_M_malloc(C,c,n,&__k,type); d = gmt_M_malloc(C,d,n,n_alloc,type); }

/*! Convenience macro for gmt_memory_func */
#if defined (DEBUG) || defined (MEMDEBUG)
#define gmt_M_memory(C,ptr,n,type) gmt_memory_func(C,ptr,n,sizeof(type),false,__SOURCE_LINE_FUNC)
#define gmt_M_memory_aligned(C,ptr,n,type) gmt_memory_func(C,ptr,n,sizeof(type),true,__SOURCE_LINE_FUNC)
#else
#define gmt_M_memory(C,ptr,n,type) gmt_memory_func(C,ptr,n,sizeof(type),false,__func__)
#define gmt_M_memory_aligned(C,ptr,n,type) gmt_memory_func(C,ptr,n,sizeof(type),true,__func__)
#endif

/*! Convenience macro for gmt_free_func */
#if defined (DEBUG) || defined (MEMDEBUG)
#define gmt_M_free(C,ptr) (gmt_free_func(C,ptr,false,__SOURCE_LINE_FUNC),(ptr)=NULL)
#define gmt_M_free_aligned(C,ptr) (gmt_free_func(C,ptr,true,__SOURCE_LINE_FUNC),(ptr)=NULL)
#else
#define gmt_M_free(C,ptr) (gmt_free_func(C,ptr,false,__func__),(ptr)=NULL)
#define gmt_M_free_aligned(C,ptr) (gmt_free_func(C,ptr,true,__func__),(ptr)=NULL)
#endif

/*! Convenience macro for free that explicitly sets freed pointer to NULL */
#define gmt_M_str_free(ptr) (free((void *)(ptr)),(ptr)=NULL)

#ifdef MEMDEBUG

struct MEMORY_ITEM {
	size_t size;	/* Size of memory allocated */
	void *ptr;	/* Memory pointer */
	char *name;	/* Source filename and line or function name */
	size_t ID;	/* Unique ID for this allocation */
	struct MEMORY_ITEM *l, *r;
};

struct MEMORY_TRACKER {
#ifdef MEMDEBUG
	bool active;	/* Normally true but can be changed to focus on just some allocations */
	bool search;	/* Normally true but can be changed to skip searching when we know we add a new item */
	bool do_log;	/* true if we wish to write detailed alloc/free log */
	uint64_t n_ptr;		/* Number of unique pointers to allocated memory */
	uint64_t n_allocated;	/* Number of items allocated by gmt_M_memory */
	uint64_t n_reallocated;	/* Number of items reallocated by gmt_M_memory */
	uint64_t n_freed;	/* Number of items freed by gmt_M_free */
	uint64_t n_ID;		/* Running number assigned to new allocations */
	uint64_t find;		/* If > 0 then we look for this ID to be allocated */
	size_t current;		/* Memory allocated at current time */
	size_t maximum;		/* Highest memory count during execution */
	size_t largest;		/* Highest memory allocation to a single variable */
	size_t n_alloc;		/* Allocated size of memory pointer array */
	struct MEMORY_ITEM *root; /* Pointer to splay tree */
	FILE *fp;	/* For logging if GMT_TRACK_MEMORY is 2 */
#endif
};

/* Items needed if -DMEMDEBUG is in effect */
EXTERN_MSC int gmt_memtrack_init (struct GMT_CTRL *GMT);
EXTERN_MSC void gmt_memtrack_report (struct GMT_CTRL *GMT);

#endif

#endif /* GMT_MEMORY_H */
