/*
 * Copyright 1993-1996 University Corporation for Atmospheric Research/Unidata
 * 
 * Portions of this software were developed by the Unidata Program at the 
 * University Corporation for Atmospheric Research.
 * 
 * Access and use of this software shall impose the following obligations
 * and understandings on the user. The user is granted the right, without
 * any fee or cost, to use, copy, modify, alter, enhance and distribute
 * this software, and any derivative works thereof, and its supporting
 * documentation for any purpose whatsoever, provided that this entire
 * notice appears in all copies of the software, derivative works and
 * supporting documentation.  Further, UCAR requests that the user credit
 * UCAR/Unidata in any publications that result from the use of this
 * software or in any product that includes this software. The names UCAR
 * and/or Unidata, however, may not be used in any advertising or publicity
 * to endorse or promote any products or commercial entity unless specific
 * written permission is obtained from UCAR/Unidata. The user also
 * understands that UCAR/Unidata is not obligated to provide the user with
 * any support, consulting, training or assistance of any kind with regard
 * to the use, operation and performance of this software nor to provide
 * the user with any updates, revisions, new versions or "bug fixes."
 * 
 * THIS SOFTWARE IS PROVIDED BY UCAR/UNIDATA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL UCAR/UNIDATA BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE ACCESS, USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef NC_META_H
#define NC_META_H 1

/**
 Provide an in-memory representation
 of a type system associated with a netcdf
 file. This is required for e.g. rpc so
 that it can serialize data arguments to
 e.g. nc_put_vara
 */

/**
 Use a single node type to represent
 meta-data definition tree.
 */

/**
 * Define the primary node types
 * by extending the existing entries in
 * netcdf.h
 */

/* Start defs at a good offset from NC_COMPOUND;
   ok if it overlaps with NC_FIRSTUSERTYPEID.
*/
#define NC_META_TYPE_OFFSET 32

/* Start with a root node to hold various things */
#define NC_ATOMIC (NC_META_TYPE_OFFSET+0)
#define NC_FIELD (NC_META_TYPE_OFFSET+1)
#define NC_ROOT (NC_META_TYPE_OFFSET+2) 
#define NC_GROUP (NC_META_TYPE_OFFSET+3)
#define NC_VAR (NC_META_TYPE_OFFSET+4)
#define NC_DIM (NC_META_TYPE_OFFSET+5) /* does not include field dims */

typedef int nc_meta;

typedef struct MetaNode {
    nc_meta nodeclass;
    nc_meta subclass;
    int       ncid;
    nc_type   typeid;
    char      name[NC_MAX_NAME];
    size_t    size;                     /* for opaque, dims, etc */
    MetaNode* basetype;                 /* vlen or enum or basetype */
    int       nelems;                   /* # fields ,econsts, dims, etc. */
    struct {
	MetaNode** nodeset;
	MetaNode*  rootgroup;
    } root;
    struct {
	MetaNode** typeset; /* types in group */
	MetaNode** varset;  /* vars in group */
	MetaNode** dimrset; /* dims in group */
	MetaNode** groups;  /* (sub)groups in group */
    } group;
    struct {                             /* compound node specific info */
	MetaNode** fields;
    } compound;
    struct {                             /* var node specific info */
        MetaNode*  dims[NC_MAX_VAR_DIMS];
    } var;
    struct {
	size_t actualsize;               /* actual size for unlimited */
    } dim;
    struct {                             /* field node specific info */
        size_t    dims[NC_MAX_VAR_DIMS];
        size_t    offset;
        size_t    alignment;
    } field;				 /* compound fields */
    /* Don't bother with separate econst nodes */
    struct MetaEconst {                  /* enum constants */
        char      name[NC_MAX_NAME];
        char      value[16];		 /* actual value size is unknown */
    } *econsts;
} MetaNode;

/* Meta Methods */
extern MetaNode*  NC_meta_alloc(nc_meta nodeclass);
extern MetaNode** NC_meta_allocn(nc_meta nodeclass, int count);
extern void       NC_meta_free(MetaNode*);

#endif /*NC_META_H*/
