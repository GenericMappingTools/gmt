/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H 1

typedef struct Bytebuffer {
  int nonextendible; /* 1 => fail if an attempt is made to extend this buffer*/
  unsigned int alloc;
  unsigned int length;
  char* content;
} Bytebuffer;

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__) || defined(__CPLUSPLUS)
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

EXTERNC Bytebuffer* bbNew(void);
EXTERNC void bbFree(Bytebuffer*);
EXTERNC int bbSetalloc(Bytebuffer*,const unsigned int);
EXTERNC int bbSetlength(Bytebuffer*,const unsigned int);
EXTERNC int bbFill(Bytebuffer*, const char fill);

/* Produce a duplicate of the contents*/
EXTERNC char* bbDup(const Bytebuffer*);

/* Return the ith char; -1 if no such char */
EXTERNC int bbGet(Bytebuffer*,unsigned int);

/* Set the ith char */
EXTERNC int bbSet(Bytebuffer*,unsigned int,char);

EXTERNC int bbAppend(Bytebuffer*,const char); /* Add at Tail */
EXTERNC int bbAppendn(Bytebuffer*,const void*,unsigned int); /* Add at Tail */

/* Insert 1 or more characters at given location */
EXTERNC int bbInsert(Bytebuffer*,const unsigned int,const char);
EXTERNC int bbInsertn(Bytebuffer*,const unsigned int,const char*,const unsigned int);

EXTERNC int bbCat(Bytebuffer*,const char*);
EXTERNC int bbCatbuf(Bytebuffer*,const Bytebuffer*);
EXTERNC int bbSetcontents(Bytebuffer*, char*, const unsigned int);
EXTERNC int bbNull(Bytebuffer*);

/* Following are always "in-lined"*/
#define bbLength(bb) ((bb)?(bb)->length:0U)
#define bbAlloc(bb) ((bb)?(bb)->alloc:0U)
#define bbContents(bb) ((bb && bb->content)?(bb)->content:(char*)"")
#define bbExtend(bb,len) bbSetalloc((bb),(len)+(bb->alloc))
#define bbClear(bb) ((void)((bb)?(bb)->length=0:0U))
#define bbNeed(bb,n) ((bb)?((bb)->alloc - (bb)->length) > (n):0U)
#define bbAvail(bb) ((bb)?((bb)->alloc - (bb)->length):0U)

#endif /*BYTEBUFFER_H*/
