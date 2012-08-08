/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCCRNODE_H
#define NCCRNODE_H

/*Forward*/
struct Group;
struct CRpath;

typedef enum Sort {
_Null		= 0,
_Attribute     	= 1,
_Dimension     	= 2,
_Variable      	= 3,
_Structure     	= 4,
_EnumTypedef   	= 5,
_EnumType      	= 6,
_Group		= 7,
_Header		= 8,
_Data		= 9,
_Range		= 10,
_Section       	= 11,
_StructureData	= 12,
_Error		= 13
} Sort;

/*
This serves as the castable supertype for all ncStream messages.
*/
typedef struct CRnode {
    uint32_t uid;
    Sort sort;
    int32_t ncid;
    struct Group* group;
    struct CRpath* pathname;
    struct {
	int isroot; /* Mark root group */
	int isdecl; /* Mark dimension decls */
	int visible; /* Mark variables as invisible */
    } flags;
    struct Dimension* dimdecl; /*sort=_Dimension; maps dimension to
				 dimension decl */
} CRnode;

#endif /*NCCRNODE_H*/
