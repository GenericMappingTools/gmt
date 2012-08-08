/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCCRPROTO_H
#define NCCRPROTO_H

/*Forward*/
struct NClist;

extern int nccr_cvtasterr(ast_err err);
extern int nccr_decodeheadermessage(bytes_t* packet, Header** hdrp);
extern int nccr_walk_Header(Header* node, struct NClist* nodes);
extern int nccr_compute_pathnames(struct NClist* nodes);
extern int nccr_map_dimensions(struct NClist* nodes);
extern void nccr_deref_dimensions(struct NClist* nodes);

extern int nccr_decodedatamessage(bytes_t* buf, Data** datahdrp, size_t* posp);
extern int nccr_decodedatacount(bytes_t* packet, size_t* offsetp, size_t* countp);

extern char* nccr_getname(CRnode* node);
extern DataType nccr_gettype(CRnode* node);

#endif /*NCCRPROTO_H*/
