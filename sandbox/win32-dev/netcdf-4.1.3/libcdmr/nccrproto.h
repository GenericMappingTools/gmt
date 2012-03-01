/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCCRPROTO_H
#define NCCRPROTO_H

extern int nccr_cvtasterr(ast_err err);
extern int nccr_decodeheader(bytes_t* packet, Header** hdrp);
extern int nccr_walk_Header(Header* node, NClist* nodes);
extern int nccr_compute_pathnames(NClist* nodes);
extern int nccr_map_dimensions(NClist* nodes);
extern void nccr_deref_dimensions(NClist* nodes);

#endif /*NCCRPROTO_H*/
