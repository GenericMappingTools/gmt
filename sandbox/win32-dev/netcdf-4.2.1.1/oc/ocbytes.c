/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ocbytes.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULTALLOC 1024
#define ALLOCINCR 1024

static int ocbytesdebug = 1;

static long
ocbytesfail(void)
{
    fflush(stdout);
    fprintf(stderr,"bytebuffer failure\n");
    fflush(stderr);
    if(ocbytesdebug) abort();
    return FALSE;
}

OCbytes*
ocbytesnew(void)
{
  OCbytes* bb = (OCbytes*)malloc(sizeof(OCbytes));
  if(bb == NULL) return (OCbytes*)ocbytesfail();
  bb->alloc=0;
  bb->length=0;
  bb->content=NULL;
  bb->nonextendible = 0;
  return bb;
}

int
ocbytessetalloc(OCbytes* bb, unsigned int sz)
{
  char* newcontent;
  if(bb == NULL) return ocbytesfail();
  if(sz <= 0) {sz = (bb->alloc?2*bb->alloc:DEFAULTALLOC);}
  if(bb->alloc >= sz) return TRUE;
  if(bb->nonextendible) return ocbytesfail();
  newcontent=(char*)calloc(sz,sizeof(char));
  if(newcontent == NULL) return FALSE;
  if(bb->alloc > 0 && bb->length > 0 && bb->content != NULL) {
    memcpy((void*)newcontent,(void*)bb->content,sizeof(char)*bb->length);
  }
  if(bb->content != NULL) free(bb->content);
  bb->content=newcontent;
  bb->alloc=sz;
  return TRUE;
}

void
ocbytesfree(OCbytes* bb)
{
  if(bb == NULL) return;
  if(!bb->nonextendible && bb->content != NULL) free(bb->content);
  free(bb);
}

int
ocbytessetlength(OCbytes* bb, unsigned int sz)
{
  if(bb == NULL) return ocbytesfail();
  if(sz > bb->alloc) {if(!ocbytessetalloc(bb,sz)) return ocbytesfail();}
  bb->length = sz;
  return TRUE;
}

int
ocbytesfill(OCbytes* bb, char fill)
{
  unsigned int i;
  if(bb == NULL) return ocbytesfail();
  for(i=0;i<bb->length;i++) bb->content[i] = fill;
  return TRUE;
}

int
ocbytesget(OCbytes* bb, unsigned int index)
{
  if(bb == NULL) return -1;
  if(index >= bb->length) return -1;
  return bb->content[index];
}

int
ocbytesset(OCbytes* bb, unsigned int index, char elem)
{
  if(bb == NULL) return ocbytesfail();
  if(index >= bb->length) return ocbytesfail();
  bb->content[index] = elem;
  return TRUE;
}

int
ocbytesappend(OCbytes* bb, char elem)
{
  if(bb == NULL) return ocbytesfail();
  /* We need space for the char + null */
  while(bb->length+1 >= bb->alloc) {
	if(!ocbytessetalloc(bb,0)) return ocbytesfail();
  }
  bb->content[bb->length] = elem;
  bb->length++;
  bb->content[bb->length] = '\0';
  return TRUE;
}

/* This assumes s is a null terminated string*/
int
ocbytescat(OCbytes* bb, char* s)
{
    ocbytesappendn(bb,(void*)s,strlen(s)+1); /* include trailing null*/
    /* back up over the trailing null*/
    if(bb->length == 0) return ocbytesfail();
    bb->length--;
    return 1;
}

int
ocbytesappendn(OCbytes* bb, void* elem, unsigned int n)
{
  if(bb == NULL || elem == NULL) return ocbytesfail();
  if(n == 0) {n = strlen((char*)elem);}
  while(!ocbytesavail(bb,n+1)) {
    if(!ocbytessetalloc(bb,0)) return ocbytesfail();
  }
  memcpy((void*)&bb->content[bb->length],(void*)elem,n);
  bb->length += n;
  bb->content[bb->length] = '\0';
  return TRUE;
}

int
ocbytesprepend(OCbytes* bb, char elem)
{
  int i; /* do not make unsigned */
  if(bb == NULL) return ocbytesfail();
  if(bb->length >= bb->alloc) if(!ocbytessetalloc(bb,0)) return ocbytesfail();
  /* could we trust memcpy? instead */
  for(i=bb->alloc;i>=1;i--) {bb->content[i]=bb->content[i-1];}
  bb->content[0] = elem;
  bb->length++;
  return TRUE;
}

char*
ocbytesdup(OCbytes* bb)
{
    char* result = (char*)malloc(bb->length+1);
    memcpy((void*)result,(const void*)bb->content,bb->length);
    result[bb->length] = '\0'; /* just in case it is a string*/
    return result;
}

char*
ocbytesextract(OCbytes* bb)
{
    char* result = bb->content;
    bb->alloc = 0;
    bb->length = 0;
    bb->content = NULL;
    return result;
}

int
ocbytessetcontents(OCbytes* bb, char* contents, unsigned int alloc)
{
    if(bb == NULL) return ocbytesfail();
    ocbytesclear(bb);
    if(!bb->nonextendible && bb->content != NULL) free(bb->content);
    bb->content = contents;
    bb->length = 0;
    bb->alloc = alloc;
    bb->nonextendible = 1;
    return 1;
}
