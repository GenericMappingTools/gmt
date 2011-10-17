/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

#ifndef _GMT_SUPPORT_H
#define _GMT_SUPPORT_H

/* The 6 basic 1, 2, 4, 4|8, 4, 8-byte types */
#define GMT_N_TYPES	6
#define GMT_CHAR_TYPE	0
#define GMT_SHORT_TYPE	1
#define GMT_INT_TYPE	2
#define GMT_LONG_TYPE	3
#define GMT_FLOAT_TYPE	4
#define GMT_DOUBLE_TYPE	5

/* Return codes from GMT_inonout */
#define GMT_OUTSIDE	0
#define GMT_ONEDGE	1
#define GMT_INSIDE	2

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
#define GMT_malloc2(C,a,b,n,n_alloc,type) { GMT_LONG k = *n_alloc; a = GMT_malloc(C,a,n,&k,type); b = GMT_malloc(C,b,n,n_alloc,type); }
#define GMT_malloc3(C,a,b,c,n,n_alloc,type) { GMT_LONG k = *n_alloc; a = GMT_malloc(C,a,n,&k,type); k = *n_alloc; b = GMT_malloc(C,b,n,&k,type); c = GMT_malloc(C,c,n,n_alloc,type); }
#define GMT_malloc4(C,a,b,c,d,n,n_alloc,type) { GMT_LONG k = *n_alloc; a = GMT_malloc(C,a,n,&k,type); k = *n_alloc; b = GMT_malloc(C,b,n,&k,type); k = *n_alloc; c = GMT_malloc(C,c,n,&k,type); d = GMT_malloc(C,d,n,n_alloc,type); }

/* Convenience macro for GMT_memory_func */
#ifdef DEBUG
#define GMT_memory(C,ptr,n,type) (type*) GMT_memory_func(C,ptr,(GMT_LONG)(n),sizeof(type),__FILE__,__LINE__)
#else
#define GMT_memory(C,ptr,n,type) (type*) GMT_memory_func(C,ptr,(GMT_LONG)(n),sizeof(type),"",0)
#endif

/* Convenience macro for GMT_free_func */
#ifdef DEBUG
#define GMT_free(C,array) { if (array) /* Do not try to free a NULL pointer! */ \
{ \
	GMT_free_func(C,(void *)array,__FILE__,__LINE__); \
	array = NULL; /* Cleanly set the freed pointer to NULL */ \
} }
#else
#define GMT_free(C,array) { if (array) /* Do not try to free a NULL pointer! */ \
{ \
	GMT_free_func(C,(void *)array,"",0); \
	array = NULL; /* Cleanly set the freed pointer to NULL */ \
} }
#endif

#ifdef DEBUG
#define MEM_TXT_LEN	64

struct MEMORY_ITEM {
	GMT_LONG size;	/* Size of memory allocated */
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
	GMT_LONG active;		/* Normally TRUE but can be changed to focus on just some allocations */
	GMT_LONG search;		/* Normally TRUE but can be changed to skip searching when we know we add a new item */
	GMT_LONG n_ptr;		/* Number of unique pointers to allocated memory */
	GMT_LONG n_allocated;	/* Number of items allocated by GMT_memory */
	GMT_LONG n_reallocated;	/* Number of items reallocated by GMT_memory */
	GMT_LONG n_freed;	/* Number of items freed by GMT_free */
	GMT_LONG current;	/* Memory allocated at current time */
	GMT_LONG maximum;	/* Highest memory count during execution */
	GMT_LONG largest;	/* Highest memory allocation to a single variable */
	GMT_LONG n_alloc;	/* Allocated size of memory pointer array */
#ifdef NEW_DEBUG
	struct MEMORY_ITEM *list_head, *list_tail;
#else
	struct MEMORY_ITEM *item;	/* Memory item array */
#endif
};

#endif

#endif /* _GMT_SUPPORT_H */
