/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bytebuffer.h"
#include "debug.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULTALLOC 1024
#define ALLOCINCR 1024

int bbdebug = 1;

/* For debugging purposes*/
static long
bbFail(void)
{
    fflush(stdout);
    fprintf(stderr,"bytebuffer failure\n");
    fflush(stderr);
    if(bbdebug) exit(1);
    return FALSE;
}

Bytebuffer*
bbNew(void)
{
  Bytebuffer* bb = (Bytebuffer*)emalloc(sizeof(Bytebuffer));
  if(bb == NULL) return (Bytebuffer*)bbFail();
  bb->alloc=0;
  bb->length=0;
  bb->content=NULL;
  bb->nonextendible = 0;
  return bb;    
}

int
bbSetalloc(Bytebuffer* bb, const unsigned int sz0)
{
  unsigned int sz = sz0;
  char* newcontent;
  if(bb == NULL) return bbFail();
  if(sz <= 0) {sz = (bb->alloc?2*bb->alloc:DEFAULTALLOC);}
  else if(bb->alloc >= sz) return TRUE;
  else if(bb->nonextendible) return bbFail();
  newcontent=(char*)ecalloc(sz,sizeof(char));
  if(bb->alloc > 0 && bb->length > 0 && bb->content != NULL) {
    memcpy((void*)newcontent,(void*)bb->content,sizeof(char)*bb->length);
  }
  if(bb->content != NULL) efree(bb->content);
  bb->content=newcontent;
  bb->alloc=sz;
  return TRUE;
}

void
bbFree(Bytebuffer* bb)
{
  if(bb == NULL) return;
  if(bb->content != NULL) efree(bb->content);
  efree(bb);
}

int
bbSetlength(Bytebuffer* bb, const unsigned int sz)
{
  if(bb == NULL) return bbFail();
  if(!bbSetalloc(bb,sz)) return bbFail();
  bb->length = sz;
  return TRUE;
}

int
bbFill(Bytebuffer* bb, const char fill)
{
  unsigned int i;
  if(bb == NULL) return bbFail();
  for(i=0;i<bb->length;i++) bb->content[i] = fill;  
  return TRUE;
}

int
bbGet(Bytebuffer* bb, unsigned int index)
{
  if(bb == NULL) return -1;
  if(index >= bb->length) return -1;
  return bb->content[index];
}

int
bbSet(Bytebuffer* bb, unsigned int index, char elem)
{
  if(bb == NULL) return bbFail();
  if(index >= bb->length) return bbFail();
  bb->content[index] = elem;
  return TRUE;
}

int
bbAppend(Bytebuffer* bb, char elem)
{
  if(bb == NULL) return bbFail();
  /* We need space for the char + null */
  while(bb->length+1 >= bb->alloc) if(!bbSetalloc(bb,0)) return bbFail();
  bb->content[bb->length] = elem;
  bb->length++;
  bb->content[bb->length] = '\0';
  return TRUE;
}

/* This assumes s is a null terminated string*/
int
bbCat(Bytebuffer* bb, const char* s)
{
    bbAppendn(bb,(void*)s,strlen(s)+1); /* include trailing null*/
    /* back up over the trailing null*/
    if(bb->length == 0) return bbFail();
    bb->length--;
    return 1;
}

int
bbCatbuf(Bytebuffer* bb, const Bytebuffer* s)
{
    if(bbLength(s) > 0)
	bbAppendn(bb,bbContents(s),bbLength(s));
    bbNull(bb);
    return 1;
}

int
bbAppendn(Bytebuffer* bb, const void* elem, const unsigned int n0)
{
  unsigned int n = n0;
  if(bb == NULL || elem == NULL) return bbFail();
  if(n == 0) {n = strlen((char*)elem);}
  while(!bbNeed(bb,(n+1))) {if(!bbSetalloc(bb,0)) return bbFail();}
  memcpy((void*)&bb->content[bb->length],(void*)elem,n);
  bb->length += n;
  bb->content[bb->length] = '\0';
  return TRUE;
}

int
bbInsert(Bytebuffer* bb, const unsigned int index, const char elem)
{
  char tmp[2];
  tmp[0]=elem;
  return bbInsertn(bb,index,tmp,1);
}

int
bbInsertn(Bytebuffer* bb, const unsigned int index, const char* elem, const unsigned int n)
{
  unsigned int i;
  int j;
  unsigned int newlen = bb->length + n;

  if(bb == NULL) return bbFail();
  if(newlen >= bb->alloc) {
    if(!bbExtend(bb,n)) return bbFail();
  }
/*
index=0
n=3
len=3
newlen=6
a b c
x y z a b c
-----------
0 1 2 3 4 5

i=0 1 2
j=5 4 3
  2 1 0
*/
  for(j=newlen-1,i=index;i<bb->length;i++) {
    bb->content[j]=bb->content[j-n];
  }
  memcpy((void*)(bb->content+index),(void*)elem,n);
  bb->length += n;
  return TRUE;
}

int
bbHeadpop(Bytebuffer* bb, char* pelem)
{
  if(bb == NULL) return bbFail();
  if(bb->length == 0) return bbFail();
  *pelem = bb->content[0];
  memcpy((void*)&bb->content[0],(void*)&bb->content[1],
        sizeof(char)*(bb->length - 1));
  bb->length--;  
  return TRUE;  
}

int
bbTailpop(Bytebuffer* bb, char* pelem)
{
  if(bb == NULL) return bbFail();
  if(bb->length == 0) return bbFail();
  *pelem = bb->content[bb->length-1];
  bb->length--;  
  return TRUE;  
}

int
bbHeadpeek(Bytebuffer* bb, char* pelem)
{
  if(bb == NULL) return bbFail();
  if(bb->length == 0) return bbFail();
  *pelem = bb->content[0];
  return TRUE;  
}

int
bbTailpeek(Bytebuffer* bb, char* pelem)
{
  if(bb == NULL) return bbFail();
  if(bb->length == 0) return bbFail();
  *pelem = bb->content[bb->length - 1];
  return TRUE;  
}

char*
bbDup(const Bytebuffer* bb)
{
    char* result = (char*)emalloc(bb->length+1);
    memcpy((void*)result,(const void*)bb->content,bb->length);
    result[bb->length] = '\0'; /* just in case it is a string*/
    return result;
}

int
bbSetcontents(Bytebuffer* bb, char* contents, const unsigned int alloc)
{
    if(bb == NULL) return bbFail();
    bbClear(bb);
    if(!bb->nonextendible && bb->content != NULL) efree(bb->content);
    bb->content = contents;
    bb->length = 0;
    bb->alloc = alloc;
    bb->nonextendible = 1;
    return 1;
}

/* Add invisible NULL terminator */
int
bbNull(Bytebuffer* bb)
{
    bbAppend(bb,'\0');
    bb->length--;
    return 1;
}
