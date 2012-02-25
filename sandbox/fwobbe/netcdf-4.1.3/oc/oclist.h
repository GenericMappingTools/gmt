/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */
#ifndef OCLIST_H
#define OCLIST_H 1

/* Define the type of the elements in the list*/

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

typedef unsigned long ocelem;

EXTERNC int oclistnull(ocelem);

typedef struct OClist {
  unsigned int alloc;
  unsigned int length;
  ocelem* content;
} OClist;

EXTERNC OClist* oclistnew(void);
EXTERNC int oclistfree(OClist*);
EXTERNC int oclistsetalloc(OClist*,unsigned int);
EXTERNC int oclistsetlength(OClist*,unsigned int);

/* Set the ith element */
EXTERNC int oclistset(OClist*,unsigned int,ocelem);
/* Get value at position i */
EXTERNC ocelem oclistget(OClist*,unsigned int);/* Return the ith element of l */
/* Insert at position i; will push up elements i..|seq|. */
EXTERNC int oclistinsert(OClist*,unsigned int,ocelem);
/* Remove element at position i; will move higher elements down */
EXTERNC ocelem oclistremove(OClist* l, unsigned int i);

/* Tail operations */
EXTERNC int oclistpush(OClist*,ocelem); /* Add at Tail */
EXTERNC ocelem oclistpop(OClist*);
EXTERNC ocelem oclisttop(OClist*);

/* Duplicate and return the content (null terminate) */
EXTERNC ocelem* oclistdup(OClist*);

/* Look for value match */
EXTERNC int oclistcontains(OClist*, ocelem);

/* Following are always "in-lined"*/
#define oclistclear(l) oclistsetlength((l),0U)
#define oclistextend(l,len) oclistsetalloc((l),(len)+(l->alloc))
#define oclistcontents(l) ((l)->content)
#define oclistlength(l)  ((l)?(l)->length:0U)

#endif /*OCLIST_H*/

