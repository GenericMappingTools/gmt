/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *  Management of internal temporary memory.
 *
 * Author:	P. Wessel, F. Wobbe
 * Date:	1-SEPT-2013
 * Version:	5.x
 *
 * To avoid lots of alloc and realloc calls we prefer to allocate a sizeable array
 * per coordinate axes once, then use that temprary space for reading and
 * calculations, and then alloc permanent space elsewhere and call memcpy to
 * place the final memory there.  We assume that for most purposes we will
 * need GMT_INITIAL_MEM_COL_ALLOC columns [3] and allocate GMT_INITIAL_MEM_ROW_ALLOC
 * [1048576U] rows for each column.  This is 24 Mb for double precision data.
 * These arrays are expected to hardly ever beeing reallocated as that would
 * only happen for very long segments, a rare occurance. For most typical data
 * we may have lots of smaller segments but rarely do any segment exceed the
 * 1048576U length initialized above.  Thus, reallocs are generally avoided.
 * Note: (1) Each column maintains its own n_alloc counter and the code belows will
 *           check each length whenever things need to be extended.
 *	 (2) We chose to maintain a small set of column vectors rather than a single
 *	     item since GMT tends to use columns vectors and thus the book-keeping is
 *	     simpler and the number of columns is typically very small (2-3).
 */

/* Two functions called elsewhere:
   GMT_prep_tmp_arrays	: Called wherever temporary column vectors are needed, such
			  as in data reading and fix_up_path and elsewhere.
   GMT_free_tmp_arrays  : Called when ready to free up stuff
 */

#include "gmt_dev.h"

enum GMT_enum_mem_alloc {	/* Initial memory for 3 double columns are 24 Mb */
	GMT_INITIAL_MEM_COL_ALLOC	= 3U,
	GMT_INITIAL_MEM_ROW_ALLOC	= 1048576U	/* 2^20 */	
};

void gmt_init_tmp_arrays (struct GMT_CTRL *GMT, size_t n_cols)
{
	/* Initialization of GMT coordinate temp arrays - this is called at most once per GMT session  */

	size_t col;

	if (n_cols < GMT_INITIAL_MEM_COL_ALLOC) n_cols = GMT_INITIAL_MEM_COL_ALLOC;	/* Allocate at least this many */
	GMT->current.io.mem_coord  = GMT_memory (GMT, NULL, n_cols, double *);		/* These are all NULL */
	GMT->current.io.mem_rows = GMT_memory (GMT, NULL, n_cols, size_t);		/* These are all 0 */
	GMT->current.io.mem_cols = n_cols;		/* How many columns we have initialized */
	for (col = 0; col < n_cols; col++) {	/* For each column, reallocate space for n_rows */
		GMT->current.io.mem_coord[col] = GMT_memory (GMT, NULL, GMT_INITIAL_MEM_ROW_ALLOC, double);
		GMT->current.io.mem_rows[col] = GMT_INITIAL_MEM_ROW_ALLOC;
	}
}

void GMT_free_tmp_arrays (struct GMT_CTRL *GMT)
{
	/* Free temporary coordinate memory used by this session */
	size_t col;

	for (col = 0; col < GMT->current.io.mem_cols; col++)	/* For each column, free an array */
		GMT_free (GMT, GMT->current.io.mem_coord[col]);
	GMT_free (GMT, GMT->current.io.mem_coord);
	GMT_free (GMT, GMT->current.io.mem_rows);
}

void GMT_prep_tmp_arrays (struct GMT_CTRL *GMT, size_t row, size_t n_cols)
{
	size_t col;

	/* Check if this is the very first time, if so we initialize the arrays */
	if (GMT->current.io.mem_coord == NULL)
		gmt_init_tmp_arrays (GMT, n_cols);	/* First time we get here */

	/* Check if we are exceeding our column count so far, if so we must allocate more columns */
	else if (n_cols > GMT->current.io.mem_cols) {	/* Must allocate more columns, this is expected to happen rarely */
		GMT->current.io.mem_coord = GMT_memory (GMT, GMT->current.io.mem_coord, n_cols, double *);	/* New ones are NOT NULL */
		GMT->current.io.mem_rows  = GMT_memory (GMT, GMT->current.io.mem_rows, n_cols, size_t);		/* New ones are NOT 0 */
		for (col = GMT->current.io.mem_cols; col < n_cols; col++) {	/* Explicitly zero out the new additions */
			GMT->current.io.mem_coord[col] = NULL;
			GMT->current.io.mem_rows[col] = 0;
		}
		GMT->current.io.mem_cols = n_cols;		/* Updated count */
	}

	/* Check if we are exceeding our allocated count for this column.  If so allocate more rows */

	for (col = 0; col < n_cols; col++) {	/* Handle each column separately */
		if (row >= GMT->current.io.mem_rows[col]) {	/* Must allocate more rows, this is expected to happen rarely given the large initial allocation */
			while (row > GMT->current.io.mem_rows[col]) GMT->current.io.mem_rows[col] <<= 1;	/* Double up */
			GMT->current.io.mem_coord[col] = GMT_memory (GMT, GMT->current.io.mem_coord[col], GMT->current.io.mem_rows[col], double);
		}
	}
	/* Note: Any additions to these arrays are not guaranteed to be set to zero */
}
