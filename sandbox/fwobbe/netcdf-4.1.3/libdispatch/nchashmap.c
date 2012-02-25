/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/nchashmap.c,v 1.4 2009/09/23 22:26:08 dmh Exp $
 *********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nchashmap.h"

static ncelem ncDATANULL = (ncelem)0;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULTALLOC 31

NChashmap* nchashnew(void) {return nchashnew0(DEFAULTALLOC);}

NChashmap* nchashnew0(int alloc)
{
  NChashmap* hm;
  hm = (NChashmap*)malloc(sizeof(NChashmap));
  if(!hm) return NULL;
  hm->alloc = alloc;
  hm->table = (NClist**)malloc(hm->alloc*sizeof(NClist*));
  if(!hm->table) {free(hm); return NULL;}
  memset((void*)hm->table,0,hm->alloc*sizeof(NClist*));
  return hm;
}

int
nchashfree(NChashmap* hm)
{
  if(hm) {
    int i;
    for(i=0;i<hm->alloc;i++) {
	if(hm->table[i] != NULL) nclistfree(hm->table[i]);
    }
    free(hm->table);
    free(hm);
  }
  return TRUE;
}

/* Insert a <nchashid,ncelem> pair into the table*/
/* Fail if already there*/
int
nchashinsert(NChashmap* hm, nchashid hash, ncelem value)
{
    int i,offset,len;
    NClist* seq;
    ncelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) {seq = nclistnew(); hm->table[offset] = seq;}
    len = nclistlength(seq);
    list = nclistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) return FALSE;
    }    
    nclistpush(seq,(ncelem)hash);
    nclistpush(seq,value);
    hm->size++;
    return TRUE;
}

/* Insert a <nchashid,ncelem> pair into the table*/
/* Overwrite if already there*/
int
nchashreplace(NChashmap* hm, nchashid hash, ncelem value)
{
    int i,offset,len;
    NClist* seq;
    ncelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) {seq = nclistnew(); hm->table[offset] = seq;}
    len = nclistlength(seq);
    list = nclistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) {list[1] = value; return TRUE;}
    }    
    nclistpush(seq,(ncelem)hash);
    nclistpush(seq,value);
    hm->size++;
    return TRUE;
}

/* remove a nchashid*/
/* return TRUE if found, false otherwise*/
int
nchashremove(NChashmap* hm, nchashid hash)
{
    int i,offset,len;
    NClist* seq;
    ncelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) return TRUE;
    len = nclistlength(seq);
    list = nclistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) {
	    nclistremove(seq,i+1);
	    nclistremove(seq,i);
	    hm->size--;
	    if(nclistlength(seq) == 0) {nclistfree(seq); hm->table[offset] = NULL;}
	    return TRUE;
	}
    }    
    return FALSE;
}

/* lookup a nchashid; return DATANULL if not found*/
/* (use hashlookup if the possible values include 0)*/
ncelem
nchashget(NChashmap* hm, nchashid hash)
{
    ncelem value;
    if(!nchashlookup(hm,hash,&value)) return ncDATANULL;
    return value;
}

int
nchashlookup(NChashmap* hm, nchashid hash, ncelem* valuep)
{
    int i,offset,len;
    NClist* seq;
    ncelem* list;

    offset = (hash % hm->alloc);    
    seq = hm->table[offset];
    if(seq == NULL) return TRUE;
    len = nclistlength(seq);
    list = nclistcontents(seq);
    for(i=0;i<len;i+=2,list+=2) {
	if(*list == hash) {if(valuep) {*valuep = list[1]; return TRUE;}}
    }
    return FALSE;
}

/* Return the ith pair; order is completely arbitrary*/
/* Can be expensive*/
int
nchashith(NChashmap* hm, int index, nchashid* hashp, ncelem* elemp)
{
    int i;
    if(hm == NULL) return FALSE;
    for(i=0;i<hm->alloc;i++) {
	NClist* seq = hm->table[i];
	int len = nclistlength(seq) / 2;
	if(len == 0) continue;
	if((index - len) < 0) {
	    if(hashp) *hashp = (nchashid)nclistget(seq,index*2);
	    if(elemp) *elemp = nclistget(seq,(index*2)+1);
	    return TRUE;
	}
	index -= len;
    }
    return FALSE;
}

/* Return all the keys; order is completely arbitrary*/
/* Can be expensive*/
int
nchashkeys(NChashmap* hm, nchashid** keylist)
{
    int i,j,index;
    nchashid* keys;
    if(hm == NULL) return FALSE;
    if(hm->size == 0) {
	keys = NULL;
    } else {
        keys = (nchashid*)malloc(sizeof(nchashid)*hm->size);
        for(index=0,i=0;i<hm->alloc;i++) {
 	    NClist* seq = hm->table[i];
	    for(j=0;j<nclistlength(seq);j+=2) {	
	        keys[index++] = (nchashid)nclistget(seq,j);
	    }
	}
    }
    if(keylist) {*keylist = keys;}
    return TRUE;
}

