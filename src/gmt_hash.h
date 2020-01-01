/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * gmt_hash.h contains definition of the structure used for hashing.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	6 API
 */

/*!
 * \file gmt_hash.h
 * \brief Definition of the structure used for hashing
 */

#ifndef GMT_HASH_H
#define GMT_HASH_H

/*--------------------------------------------------------------------
 *			GMT HASH STRUCTURE DEFINITION
 *--------------------------------------------------------------------*/

/* To avoid lots of dynamic memory allocation for the hash lookup tables we
 * use a statically allocated structure.  By determining that the max number
 * of identical hash numbers across all the keywords is 16, we simply allocate
 * space for 16 entries for each structure.  Should later additions to GMT's
 * default parameters, colornames, etc increase this value we will be warned
 * and can change the entry GMT_HASH_MAXDEPTH below accordingly.
 */
#define GMT_HASH_MAXDEPTH	16

/*! Used to relate text keywords to array indices */
struct GMT_HASH {
	unsigned int id[GMT_HASH_MAXDEPTH];	/* Indices of corresponding keyword with identical hash value */
	unsigned int n_id;			/* Number of hash entries for this item */
	char *key[GMT_HASH_MAXDEPTH];		/* Name of these entries */
};

#endif  /* GMT_HASH_H */
