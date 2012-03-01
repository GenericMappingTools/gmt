/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/list.h,v 1.3 2010/05/24 19:59:58 dmh Exp $ */

#ifndef LIST_H
#define LIST_H 1

/* Define the type of the elements in the sequence*/

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

typedef unsigned long elem_t;

EXTERNC int listnull(elem_t);

typedef struct List {
  unsigned int alloc;
  unsigned int length;
  elem_t* content;
} List;

EXTERNC List* listnew(void);
EXTERNC int listfree(List*);
EXTERNC int listsetalloc(List*,unsigned int);

/* Set the ith element of sq */
EXTERNC int listset(List*,unsigned int,elem_t);
/* Insert at position i of sq; will push up elements i..|seq|. */
EXTERNC int listinsert(List*,unsigned int,elem_t);

/* Tail operations */
EXTERNC int listpush(List*,elem_t); /* Add at Tail */
EXTERNC elem_t listpop(List*);
EXTERNC elem_t listtop(List*);

/* Head operations */
EXTERNC int listfpush(List*,elem_t); /* Add at Head */
EXTERNC elem_t listfpop(List*);
EXTERNC elem_t listfront(List*);
EXTERNC elem_t listremove(List* sq, unsigned int i);

/* Duplicate and return the content (null terminate) */
EXTERNC elem_t* listdup(List*);

/* Search list for a given element */
EXTERNC int listcontains(List*,elem_t);

/* Remove a list element by value (remove all instances) */
EXTERNC int listdelete(List*,elem_t);

/* Following are always "in-lined"*/
#define listclear(sq) listsetlength((sq),0U)
#define listextend(sq,len) listsetalloc((sq),(len)+(sq->alloc))
#define listcontents(sq) ((sq)->content)
#define listlength(sq)  ((sq)?(sq)->length:0U)

/* Following can be open-coded via macros */
#ifdef LINLINE

EXTERNC elem_t DATANULL;

#define listsetlength(sq,sz) \
(((sq)==NULL||(sz)<0||!listsetalloc((sq),(sz)))?0:((sq)->length=(sz),1))

#define listget(sq,index) \
(((sq)==NULL||(sz)<0||(index)<0||(index)>=(sq)->length)?DATANULL:((sq)->content[index]))

#define listpush(sq,elem) \
(((sq)==NULL||(((sz)->length >= (sq)->alloc)&&!listsetalloc((sq),0)))?0:((sq)->content[(sq)->length++]=(elem),1))

#else
EXTERNC int listsetlength(List*,unsigned int);
EXTERNC elem_t listget(List*,unsigned int);/* Return the ith element of sq */
EXTERNC int listpush(List*,elem_t); /* Add at Tail */
#endif


#endif /*LIST_H*/

