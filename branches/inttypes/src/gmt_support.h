/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
	GMT_LONG n_arg;	/* How many commands this macro represents */
	char *name;	/* The macro name */
	char **arg;	/* List of those commands */
};

/* Macros to reallocate memory for groups of 2, 3 or 4 arrays at a time of the same size/type */
#ifdef DEBUG
#define GMT_malloc(C,a,n,n_alloc,type) GMT_malloc_func(C,a,n,n_alloc,sizeof(type),__FILE__,__LINE__)
#else
#define GMT_malloc(C,a,n,n_alloc,type) GMT_malloc_func(C,a,n,n_alloc,sizeof(type),"",0)
#endif
/* The k = *n_alloc below is needed to ensure only the final GMT_malloc call changes n_alloc */
#define GMT_malloc2(C,a,b,n,n_alloc,type) { size_t __k = *n_alloc; a = GMT_malloc(C,a,n,&__k,type); b = GMT_malloc(C,b,n,n_alloc,type); }
#define GMT_malloc3(C,a,b,c,n,n_alloc,type) { size_t __k = *n_alloc; a = GMT_malloc(C,a,n,&__k,type); __k = *n_alloc; b = GMT_malloc(C,b,n,&__k,type); c = GMT_malloc(C,c,n,n_alloc,type); }
#define GMT_malloc4(C,a,b,c,d,n,n_alloc,type) { size_t __k = *n_alloc; a = GMT_malloc(C,a,n,&__k,type); __k = *n_alloc; b = GMT_malloc(C,b,n,&__k,type); __k = *n_alloc; c = GMT_malloc(C,c,n,&__k,type); d = GMT_malloc(C,d,n,n_alloc,type); }

/* Convenience macro for GMT_memory_func */
#ifdef DEBUG
#define GMT_memory(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),__FILE__,__LINE__)
#else
#define GMT_memory(C,ptr,n,type) GMT_memory_func(C,ptr,n,sizeof(type),"",0)
#endif

/* Convenience macro for GMT_free_func */
#ifdef DEBUG
#define GMT_free(C,ptr) (GMT_free_func(C,ptr,__FILE__,__LINE__),(ptr)=NULL)
#else
#define GMT_free(C,ptr) (GMT_free_func(C,ptr,"",0),(ptr)=NULL)
#endif

#ifdef DEBUG
#define MEM_TXT_LEN	128

struct MEMORY_ITEM {
	size_t size;	/* Size of memory allocated */
	GMT_LONG line;	/* Line number where things were initially allocated */
	void *ptr;	/* Memory pointer */
#ifdef NEW_DEBUG
	char *name;	/* File name */
	struct MEMORY_ITEM *l, *r;
#else
	char name[MEM_TXT_LEN];	/* File name */
#endif
};

struct MEMORY_TRACKER {
	GMT_LONG active;	/* Normally TRUE but can be changed to focus on just some allocations */
	GMT_LONG search;	/* Normally TRUE but can be changed to skip searching when we know we add a new item */
	uint64_t n_ptr;		/* Number of unique pointers to allocated memory */
	uint64_t n_allocated;	/* Number of items allocated by GMT_memory */
	uint64_t n_reallocated;	/* Number of items reallocated by GMT_memory */
	uint64_t n_freed;	/* Number of items freed by GMT_free */
	uint64_t current;	/* Memory allocated at current time */
	uint64_t maximum;	/* Highest memory count during execution */
	uint64_t largest;	/* Highest memory allocation to a single variable */
	size_t n_alloc;		/* Allocated size of memory pointer array */
#ifdef NEW_DEBUG
	struct MEMORY_ITEM *list_head, *list_tail;
#else
	struct MEMORY_ITEM *item;	/* Memory item array */
#endif
};

#endif

#endif /* _GMT_SUPPORT_H */
