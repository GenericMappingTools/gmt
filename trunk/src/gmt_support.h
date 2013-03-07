/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#ifndef _GMT_SUPPORT_H
#define _GMT_SUPPORT_H

/* Return codes from GMT_inonout */
enum GMT_enum_inside {
	GMT_OUTSIDE = 0,
	GMT_ONEDGE,
	GMT_INSIDE};

/* Here are definition of MATH_MACRO and some functions used by grdmath and gmtmath */
struct MATH_MACRO {
	unsigned int n_arg;	/* How many commands this macro represents */
	char *name;	/* The macro name */
	char **arg;	/* List of those commands */
};

/* Macros to reallocate memory for groups of 2, 3 or 4 arrays at a time of the same size/type */
#ifdef DEBUG
#define GMT_malloc(C,a,n,n_alloc,type) GMT_malloc_func(C,a,n,n_alloc,sizeof(type),__SOURCE_LINE)
#else
#define GMT_malloc(C,a,n,n_alloc,type) GMT_malloc_func(C,a,n,n_alloc,sizeof(type),__func__)
#endif
/* The __kp = n_alloc below is needed since NULL may be passed. __k is used to ensure only the final GMT_malloc call changes n_alloc (unless it is NULL) */
#define GMT_malloc2(C,a,b,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = GMT_malloc(C,a,n,&__k,type); b = GMT_malloc(C,b,n,n_alloc,type); }
#define GMT_malloc3(C,a,b,c,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = GMT_malloc(C,a,n,&__k,type); __k = (__kp) ? *__kp : 0U; b = GMT_malloc(C,b,n,&__k,type); c = GMT_malloc(C,c,n,n_alloc,type); }
#define GMT_malloc4(C,a,b,c,d,n,n_alloc,type) { size_t __k, *__kp = n_alloc; __k = (__kp) ? *__kp : 0U; a = GMT_malloc(C,a,n,&__k,type); __k = (__kp) ? *__kp : 0U; b = GMT_malloc(C,b,n,&__k,type); __k = (__kp) ? *__kp : 0U; c = GMT_malloc(C,c,n,&__k,type); d = GMT_malloc(C,d,n,n_alloc,type); }

/* Convenience macro for GMT_memory_func */
#ifdef DEBUG
#define GMT_memory(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),false,__SOURCE_LINE)
#define GMT_memory_aligned(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),true,__SOURCE_LINE)
#else
#define GMT_memory(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),false,__func__)
#define GMT_memory_aligned(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),true,__func__)
#endif

/* Convenience macro for GMT_free_func */
#ifdef DEBUG
#define GMT_free(C,ptr) (GMT_free_func(C,ptr,false,__SOURCE_LINE),(ptr)=NULL)
#define GMT_free_aligned(C,ptr) (GMT_free_func(C,ptr,true,__SOURCE_LINE),(ptr)=NULL)
#else
#define GMT_free(C,ptr) (GMT_free_func(C,ptr,false,__func__),(ptr)=NULL)
#define GMT_free_aligned(C,ptr) (GMT_free_func(C,ptr,true,__func__),(ptr)=NULL)
#endif

#ifdef MEMDEBUG

struct MEMORY_ITEM {
	size_t size; /* Size of memory allocated */
	void *ptr;   /* Memory pointer */
	char *name;  /* Source filename and line or function name */
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
	size_t current;		/* Memory allocated at current time */
	size_t maximum;		/* Highest memory count during execution */
	size_t largest;		/* Highest memory allocation to a single variable */
	size_t n_alloc;		/* Allocated size of memory pointer array */
	struct MEMORY_ITEM *list_head, *list_tail;
	FILE *fp;	/* For logging if GMT_TRACK_MEMORY is 2 */
};

/* Items needed if -DMEMDEBUG is in effect */
EXTERN_MSC void GMT_memtrack_init (struct GMT_CTRL *C, struct MEMORY_TRACKER *M);
EXTERN_MSC void GMT_memtrack_report (struct GMT_CTRL *C, struct MEMORY_TRACKER *M);
EXTERN_MSC void GMT_memtrack_on (struct GMT_CTRL *C, struct MEMORY_TRACKER *M);
EXTERN_MSC void GMT_memtrack_off (struct GMT_CTRL *C, struct MEMORY_TRACKER *M);

/* external struct to avoid having to pass it */
EXTERN_MSC struct MEMORY_TRACKER g_mem_keeper;
#endif

#endif /* _GMT_SUPPORT_H */
