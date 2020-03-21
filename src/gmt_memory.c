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
 *  Management of internal temporary memory.
 *
 * Author:	P. Wessel, F. Wobbe
 * Date:	1-SEPT-2013
 * Version:	5.x
 *
 */

/* Two functions called elsewhere:
   gmt_prep_tmp_arrays	: Called wherever temporary column vectors are needed, such
			  as in data reading and fix_up_path and elsewhere.
   gmtlib_free_tmp_arrays  : Called when ready to free up stuff
   gmt_M_malloc              Memory management
   gmt_M_memory              Memory allocation/reallocation
   gmt_M_free                Memory deallocation
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

#ifdef HAVE_MEMALIGN
#	include <malloc.h>
#endif

/* Local functions */

/* To avoid lots of alloc and realloc calls we prefer to allocate a sizeable array
 * per coordinate axes once, then use that temporary space for reading and
 * calculations, and then alloc permanent space elsewhere and call memcpy to
 * place the final memory there.  We assume that for most purposes we will
 * need GMT_INITIAL_MEM_COL_ALLOC columns [2] and allocate GMT_INITIAL_MEM_ROW_ALLOC
 * [2097152U] rows for each column.  This is 32 Mb for double precision data.
 * These arrays are expected to hardly ever being reallocated as that would
 * only happen for very long segments, a rare occurrence. For most typical data
 * we may have lots of smaller segments but rarely do any segment exceed the
 * 1048576U length initialized above.  Thus, reallocs are generally avoided.
 * Note: (1) All columns share a single n_alloc counter and the code belows will
 *           check whenever arrays need to be extended.
 *	 (2) We chose to maintain a small set of column vectors rather than a single
 *	     item since GMT tends to use columns vectors and thus the book-keeping is
 *	     simpler and the number of columns is typically very small (2-3).
 */

GMT_LOCAL void memory_init_tmp_arrays (struct GMT_CTRL *GMT, int direction, size_t n_cols) {
	/* Initialization of GMT coordinate temp arrays - this is called at most once per GMT session  */

	if (!GMT->hidden.mem_set) {
		if (n_cols == 0 && (direction == GMT_NOTSET || (GMT->current.io.record_type[direction] & GMT_READ_DATA))) n_cols = GMT_INITIAL_MEM_COL_ALLOC;	/* Allocate at least this many */
	}
	if (n_cols) {	/* Records have numerical content */
		size_t col;
		GMT->hidden.mem_coord  = gmt_M_memory (GMT, GMT->hidden.mem_coord, n_cols, double *);	/* These are all NULL */
		GMT->hidden.mem_cols = n_cols;	/* How many columns we have initialized */
		for (col = 0; col < n_cols; col++)	/* For each column, reallocate space for n_rows */
			GMT->hidden.mem_coord[col] = gmt_M_memory (GMT, NULL, GMT_INITIAL_MEM_ROW_ALLOC, double);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT memory: Initialize %" PRIuS " temporary column double arrays, each of length : %" PRIuS "\n", GMT->hidden.mem_cols, GMT->hidden.mem_rows);
		GMT->hidden.mem_rows = GMT_INITIAL_MEM_ROW_ALLOC;
	}
	if (direction != GMT_NOTSET && GMT->current.io.record_type[direction] & GMT_READ_TEXT) {	/* For text or mixed records */
		GMT->hidden.mem_txt = gmt_M_memory (GMT, NULL, GMT_INITIAL_MEM_ROW_ALLOC, char *);
		GMT->hidden.mem_rows = GMT_INITIAL_MEM_ROW_ALLOC;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT memory: Initialize a temporary column char * array of length : %" PRIuS "\n", GMT->hidden.mem_rows);
	}
	GMT->hidden.mem_set = true;
}

GMT_LOCAL int memory_die_if_memfail (struct GMT_CTRL *GMT, size_t nelem, size_t size, const char *where) {
	/* Handle reporting and aborting if memory allocation fails */
	double mem = ((double)nelem) * ((double)size);
	unsigned int k = 0;
	static char *m_unit[4] = {"bytes", "kb", "Mb", "Gb"};
	while (mem >= 1024.0 && k < 3) mem /= 1024.0, k++;
	gmtlib_report_func (GMT, GMT_MSG_WARNING, where, "Could not reallocate memory [%.2f %s, %" PRIuS " items of %" PRIuS " bytes]\n", mem, m_unit[k], nelem, size);
#ifdef DEBUG
	gmtlib_report_func (GMT, GMT_MSG_WARNING, where, "gmt_M_memory [realloc] called\n");
#endif
	GMT_exit (GMT, GMT_MEMORY_ERROR); return GMT_MEMORY_ERROR;
}

#ifdef MEMDEBUG
/* Memory tracking used to assist in finding memory leaks.  We internally keep track
 * of all memory allocated by gmt_M_memory and subsequently freed with gmt_M_free.  If
 * upon exit there are unreleased memory we issue a report of how many items were
 * not freed and where they were first allocated.  This is only used by the developers
 * and if -DMEMDEBUG is not set then all of this is left out.
 * The environmental variable GMT_TRACK_MEMORY controls the memory tracking:
 * a. Unset GMT_TRACK_MEMORY or set to '0' to deactivate the memory tracking.
 * b. Set GMT_TRACK_MEMORY to 1 (or any but 0) to activate the memory tracking.
 * c. Set GMT_TRACK_MEMORY to 2 to activate the memory tracking and to write a
 *    detailed log of all transactions taking place during a session to the file
 *    gmt_memtrack_<pid>.log
 *
 * Paul Wessel, Latest revision June 2012.
 * Florian Wobbe, Latest revision August 2013.
 * Splay tree manipulation functions are modified after Sleator and Tarjan, 1985:
 * Self-adjusting binary search trees. JACM, 32(3), doi:10.1145/3828.3835 */

static inline double gmt_memtrack_mem (size_t mem, unsigned int *unit) {
	/* Report the memory in the chosen unit */
	unsigned int k = 0;
	double val = mem / 1024.0;	/* Kb */
	if (val > 1024.0) {val /= 1024.0; k++;}	/* Now in Mb */
	if (val > 1024.0) {val /= 1024.0; k++;}	/* Now in Gb */
	*unit = k;
	return (val);
}

int gmt_memtrack_init (struct GMT_CTRL *GMT) {
	/* Called in gmt_begin() */
	time_t now = time (NULL);
	char *env = getenv ("GMT_TRACK_MEMORY"); /* 0: off; any: track; 2: log to file */
	struct MEMORY_TRACKER *M = calloc (1, sizeof (struct MEMORY_TRACKER));
	GMT->hidden.mem_keeper = M;
	M->active = ( env && strncmp (env, "0", 1) != 0 ); /* track if GMT_TRACK_MEMORY != 0 */
	M->do_log = ( env && strncmp (env, "2", 1) == 0 ); /* log if GMT_TRACK_MEMORY == 2 */
	if (M->active) {
		size_t ID;
		ID = atoi (env);
		if (ID > 2) M->find = ID;
	}
	M->search = true;
	if (!M->do_log) /* Logging not requested */ {
		if (M->active) M->fp = stderr;
		return GMT_OK;
	}
	else
	{
		int pid = getpid();
		char logfile[GMT_LEN32];
		snprintf (logfile, GMT_LEN32, "gmt_memtrack_%d.log", pid);
		if ((M->fp = fopen (logfile, "w")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Could not create log file gmt_memtrack_%d.log\n", pid);
			GMT_exit (GMT, GMT_ERROR_ON_FOPEN); return GMT_ERROR_ON_FOPEN;
		}
	}
	fprintf (M->fp, "# %s", ctime (&now));
	fprintf (M->fp, "#           addr     size_b total_k module line/function\n");
	return (GMT_OK);
}

static inline struct MEMORY_ITEM * gmt_treesplay (struct MEMORY_ITEM *t, void *addr) {
	/* Simple top-down splay, not requiring addr to be in the tree t.
	 * This function searches for an item with key addr in the tree rooted at t. If
	 * it's there, it is splayed to the root. If it isn't there, then the node put
	 * at the root is the last one before NULL that would have been reached in
	 * a normal binary search for addr. (It's a neighbor of addr in the tree.) */
	struct MEMORY_ITEM N, *l, *r, *y;

	if (t == NULL) return t;
	N.l = N.r = NULL;
	l = r = &N;

	for (;;) {
		if (addr < t->ptr) {
			if (t->l == NULL) break;
			if (addr < t->l->ptr) {
				y = t->l; /* rotate right */
				t->l = y->r;
				y->r = t;
				t = y;
				if (t->l == NULL) break;
			}
			r->l = t;   /* link right */
			r = t;
			t = t->l;
		}
		else if (addr > t->ptr) {
			if (t->r == NULL) break;
			if (addr > t->r->ptr) {
				y = t->r; /* rotate left */
				t->r = y->l;
				y->l = t;
				t = y;
				if (t->r == NULL) break;
			}
			l->r = t;   /* link left */
			l = t;
			t = t->r;
		}
		else
			break;
	}
	l->r = t->l;    /* assemble */
	r->l = t->r;
	t->l = N.r;
	t->r = N.l;
	return t;
}

static inline struct MEMORY_ITEM * gmt_treeinsert (struct MEMORY_ITEM *t, void *addr) {
	/* Insert addr into the tree t, unless it's already there.
	 * Return a pointer to the resulting tree. */
	struct MEMORY_ITEM *new = calloc (1, sizeof (struct MEMORY_ITEM));

	new->ptr = addr;
	if (t == NULL)
		return new;
	t = gmt_treesplay (t, addr);
	if (addr < t->ptr) {
		new->l = t->l;
		new->r = t;
		t->l = NULL;
		return new;
	} else if (addr > t->ptr) {
		new->r = t->r;
		new->l = t;
		t->r = NULL;
		return new;
	} else {
		/* We get here if addr is already in the tree. Don't add it again. */
		gmt_M_str_free (new);
		return t;
	}
}

static inline struct MEMORY_ITEM * gmt_treefind (struct MEMORY_ITEM **t, void *addr) {
	/* Splay item with addr to the root and update tree pointer t. Return
	 * pointer to the resulting tree. If it isn't there, then return NULL. */
	struct MEMORY_ITEM *x = *t;
	x = gmt_treesplay (x, addr);
	*t = x;
	return (x != NULL && x->ptr == addr) ? x : NULL;
}

static inline struct MEMORY_ITEM * gmt_treedelete (struct MEMORY_ITEM *t, void *addr) {
	/* Delete addr from the tree t, if it's there.
	 * Return a pointer to the resulting tree. */
	struct MEMORY_ITEM *x;

	if (t==NULL) return NULL;
	t = gmt_treesplay(t, addr);
	if (addr == t->ptr) { /* found it */
		if (t->l == NULL) {
			x = t->r;
		} else {
			x = gmt_treesplay (t->l, addr);
			x->r = t->r;
		}
		gmt_M_str_free (t->name);
		gmt_M_str_free (t);
		return x;
	}
	return t; /* It wasn't there */
}

static inline void gmt_treedestroy (struct MEMORY_ITEM **t) {
	/* Remves all items from the tree rooted at t. */
	struct MEMORY_ITEM *x = *t;
	if (x != NULL) {
		gmt_treedestroy (&x->l);
		gmt_treedestroy (&x->r);
		gmt_M_str_free (x->name);
		gmt_M_str_free (x);
		*t = NULL;
	}
}

static inline void gmt_memtrack_add (struct GMT_CTRL *GMT, const char *where, void *ptr, void *prev_ptr, size_t size) {
	/* Called from gmt_M_memory to update current list of memory allocated */
	size_t old, diff;
	void *use = NULL;
	struct MEMORY_ITEM *entry = NULL;
	static const char *mode[3] = {"INI", "ADD", "SET"};
	int kind;
	struct MEMORY_TRACKER *M = GMT->hidden.mem_keeper;

	use = (prev_ptr) ? prev_ptr : ptr;
	entry = (M->search) ? gmt_treefind (&M->root, use) : NULL;
	if (!entry) { /* Not found, must insert new_entry entry at end */
		entry = gmt_treeinsert (M->root, use);
		entry->name = strdup (where);
		old = 0;
		M->n_ptr++;
		M->n_allocated++;
		kind = 0;
	}
	else {	/* Found existing pointer, get its previous size */
		old = entry->size;
		if (entry->ptr != ptr) {	/* Must delete and add back since the address changed */
			char *name = entry->name; /* remember pointer of name */
			entry->name = NULL; /* prevent pointer from being freed in gmt_treedelete */
			entry = gmt_treedelete (entry, entry->ptr);
			entry = gmt_treeinsert (entry, ptr);
			entry->name = name; /* put name back */
		}
		M->n_reallocated++;
		kind = 1;
	}
	if (M->find && M->find == M->n_ID) {	/* All code to stop here if set in ddd */
		/* The item you are looking for is being allocated now */
		int found = 1;	/* Add a debug stop point here and then examine where you are */
		found ++;	/* Just to keep compiler happy */
	}
	entry->ID = M->n_ID++;

	if (old > size) {	/* Reduction in memory */
		kind = 2;
		diff = old - size;	/* Change in memory */
		if (diff > M->current) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Memory tracker reports < 0 bytes allocated!\n");
			M->current = 0;	/* Cannot have negative in size_t */
		}
		else
			M->current -= diff;	/* Revised memory tally */
	}
	else {			/* Addition in memory */
		diff = size - old;	/* Change in memory */
		M->current += diff;	/* Revised memory tally */
	}

	entry->size = size;
	if (M->do_log)
		fprintf (M->fp, "%s: 0x%zx %10" PRIuS " %7.0lf %s %s\n", mode[kind], (size_t)entry->ptr, entry->size, M->current / 1024.0, GMT->init.module_name, entry->name);
	if (M->current > M->maximum) M->maximum = M->current;	/* Update total allocation */
	if (size > M->largest) M->largest = size;		/* Update largest single item */
	M->root = entry; /* Update root pointer */
}

static inline bool gmt_memtrack_sub (struct GMT_CTRL *GMT, const char *where, void *ptr) {
	/* Called from gmt_M_free to remove memory pointer */
	struct MEMORY_TRACKER *M = GMT->hidden.mem_keeper;
	struct MEMORY_ITEM *entry = gmt_treefind (&M->root, ptr);

	M->n_freed++; /* Increment first to also count multiple frees on same address */
	if (!entry) {
		/* Error, trying to free something not allocated by gmt_memory_func */
		gmtlib_report_func (GMT, GMT_MSG_WARNING, where, "Wrongly tries to free item\n");
		if (M->do_log)
			fprintf (M->fp, "!!!: 0x%zx ---------- %7.0lf %s @%s\n", (size_t)ptr, M->current / 1024.0, GMT->init.module_name, where);
		return false; /* Notify calling function that something went wrong */
	}
	if (entry->size > M->current) {
		gmtlib_report_func (GMT, GMT_MSG_WARNING, where, "Memory tracker reports < 0 bytes allocated!\n");
		M->current = 0;
	}
	else
		M->current -= entry->size;	/* "Free" the memory */
	if (M->do_log)
		fprintf (M->fp, "DEL: 0x%zx %10" PRIuS " %7.0lf %s %s\n", (size_t)entry->ptr, entry->size, M->current / 1024.0, GMT->init.module_name, entry->name);
	M->root = gmt_treedelete (entry, entry->ptr);
	M->n_ptr--;
	return true;
}

static inline void gmt_treereport (struct GMT_CTRL *GMT, struct MEMORY_ITEM *x) {
	unsigned int u;
	char *unit[3] = {"kb", "Mb", "Gb"};
	double size = gmt_memtrack_mem (x->size, &u);
	struct MEMORY_TRACKER *M = GMT->hidden.mem_keeper;
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "Memory not freed first allocated in %s (ID = %" PRIuS "): %.3f %s [%" PRIuS " bytes]\n", x->name, x->ID, size, unit[u], x->size);
	if (M->do_log)
		fprintf (M->fp, "# Memory not freed first allocated in %s (ID = %" PRIuS "): %.3f %s [%"
						 PRIuS " bytes]\n", x->name, x->ID, size, unit[u], x->size);
}

static inline void gmt_treeprint (struct GMT_CTRL *GMT, struct MEMORY_ITEM *t) {
	if (t != NULL) {
		gmt_treeprint (GMT, t->l);
		gmt_treereport (GMT, t);
		gmt_treeprint (GMT, t->r);
	}
}

void gmt_memtrack_report (struct GMT_CTRL *GMT) {
	/* Called at end of gmt_end() */
	unsigned int u, level;
	uint64_t excess = 0, n_multi_frees = 0;
	double size;
	char *unit[3] = {"kb", "Mb", "Gb"};
	struct MEMORY_TRACKER *M = GMT->hidden.mem_keeper;

	if (!M->active) return;	/* Not activated */
	if (M->n_allocated > M->n_freed)
		excess = M->n_allocated - M->n_freed;
	else if (M->n_freed > M->n_allocated)
		n_multi_frees = M->n_freed - M->n_allocated;
	/* Only insist on report if a leak or multi free, otherwise requires -Vd: */
	level = (excess || n_multi_frees) ? GMT_MSG_WARNING : GMT_MSG_DEBUG;
	size = gmt_memtrack_mem (M->maximum, &u);
	GMT_Report (GMT->parent, level, "Max total memory allocated was %.3f %s [%" PRIuS " bytes]\n",
							size, unit[u], M->maximum);
		if (M->do_log)
			fprintf (M->fp, "# Max total memory allocated was %.3f %s [%"
							 PRIuS " bytes]\n", size, unit[u], M->maximum);
	size = gmt_memtrack_mem (M->largest, &u);
	GMT_Report (GMT->parent, level, "Single largest allocation was %.3f %s [%" PRIuS " bytes]\n", size, unit[u], M->largest);
		if (M->do_log)
			fprintf (M->fp, "# Single largest allocation was %.3f %s [%"
							 PRIuS " bytes]\n", size, unit[u], M->largest);
	if (M->current) {
		size = gmt_memtrack_mem (M->current, &u);
		GMT_Report (GMT->parent, level, "MEMORY NOT FREED: %.3f %s [%" PRIuS " bytes]\n",
								size, unit[u], M->current);
		if (M->do_log)
			fprintf (M->fp, "# MEMORY NOT FREED: %.3f %s [%" PRIuS " bytes]\n",
							 size, unit[u], M->current);
	}
	GMT_Report (GMT->parent, level, "Items allocated: %" PRIu64 " reallocated: %" PRIu64 " freed: %" PRIu64 "\n", M->n_allocated, M->n_reallocated, M->n_freed);
	if (M->do_log)
		fprintf (M->fp, "# Items allocated: %" PRIu64 " reallocated: %" PRIu64 " freed: %"
						 PRIu64 "\n", M->n_allocated, M->n_reallocated, M->n_freed);
	if (M->n_freed > M->n_allocated) {
		uint64_t n_multi_frees = M->n_freed - M->n_allocated;
		GMT_Report (GMT->parent, level, "Items FREED MULTIPLE TIMES: %" PRIu64 "\n", n_multi_frees);
		if (M->do_log)
			fprintf (M->fp, "# Items FREED MULTIPLE TIMES: %" PRIu64 "\n", n_multi_frees);
	}
	if (excess) {
		GMT_Report (GMT->parent, level, "Items NOT PROPERLY FREED: %" PRIu64 "\n", excess);
		if (M->do_log)
			fprintf (M->fp, "# Items NOT PROPERLY FREED: %" PRIu64 "\n", excess);
	}
	gmt_treeprint (GMT, M->root);
	gmt_treedestroy (&M->root); /* Remove remaining items from tree if any */

	if (M->do_log) {
		time_t now = time (NULL);
		fprintf (M->fp, "# %s", ctime (&now));
		fclose (M->fp);
	}
}
#endif

void gmtlib_free_tmp_arrays (struct GMT_CTRL *GMT) {
	/* Free temporary coordinate memory used by this session */
	size_t col;

	if (GMT->hidden.mem_cols) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT memory: Free %" PRIuS " temporary column arrays, each of length : %" PRIuS "\n", GMT->hidden.mem_cols, GMT->hidden.mem_rows);
	for (col = 0; col < GMT->hidden.mem_cols; col++) {	/* For each column, free an array */
		gmt_M_free (GMT, GMT->hidden.mem_coord[col]);
	}
	gmt_M_free (GMT, GMT->hidden.mem_coord);
	if (GMT->hidden.mem_txt)
		gmt_M_free (GMT, GMT->hidden.mem_txt);
	GMT->hidden.mem_rows = GMT->hidden.mem_cols = 0;
	GMT->hidden.mem_set = false;	/* Back to where we started */
}

void gmt_prep_tmp_arrays (struct GMT_CTRL *GMT, int direction, size_t row, size_t n_cols) {
	size_t col;

	/* Check if this is the very first time, if so we initialize the arrays */
	if (!GMT->hidden.mem_set)
		memory_init_tmp_arrays (GMT, direction, n_cols);	/* First time we get here */

	/* Check if we are exceeding our column count so far, if so we must allocate more columns */
	else if (n_cols > GMT->hidden.mem_cols) {	/* Must allocate more columns, this is expected to happen rarely */
		GMT->hidden.mem_coord = gmt_M_memory (GMT, GMT->hidden.mem_coord, n_cols, double *);	/* New ones are NOT NULL */
		for (col = GMT->hidden.mem_cols; col < n_cols; col++)	/* Explicitly allocate the new additions */
			GMT->hidden.mem_coord[col] = gmt_M_memory (GMT, NULL, GMT->hidden.mem_rows, double);
		GMT->hidden.mem_cols = n_cols;		/* Updated column count */
	}

	/* Check if we are exceeding our allocated count for this column.  If so allocate more rows */

	if (row < GMT->hidden.mem_rows) return;	/* Nothing to do */

	/* Here we must allocate more rows, this is expected to happen rarely given the large initial allocation */

	while (row >= GMT->hidden.mem_rows) GMT->hidden.mem_rows = (size_t)lrint (1.5 * GMT->hidden.mem_rows);	/* Increase by 50% */
	for (col = 0; col < GMT->hidden.mem_cols; col++)	/* Add more memory via realloc */
		GMT->hidden.mem_coord[col] = gmt_M_memory (GMT, GMT->hidden.mem_coord[col], GMT->hidden.mem_rows, double);
	if (direction != GMT_NOTSET && GMT->current.io.record_type[direction] & GMT_READ_TEXT)
		GMT->hidden.mem_txt = gmt_M_memory (GMT, GMT->hidden.mem_txt, GMT->hidden.mem_rows, char *);

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT memory: Increase %" PRIuS " temporary column arrays to new length : %" PRIuS "\n", GMT->hidden.mem_cols, GMT->hidden.mem_rows);
	/* Note: Any additions to these arrays are not guaranteed to be set to zero */
}

void *gmt_memory_func (struct GMT_CTRL *GMT, void *prev_addr, size_t nelem, size_t size, bool align, const char *where) {
	/* Multi-functional memory allocation subroutine.
	   If prev_addr is NULL, allocate new memory of nelem elements of size bytes.
		Ignore when nelem == 0.
	   If prev_addr exists, reallocate the memory to a larger or smaller chunk of nelem elements of size bytes.
		When nelem = 0, free the memory.
	   If align is true we seek to get aligned memory.
	*/

	void *tmp = NULL;

	if (nelem == SIZE_MAX) {	/* Probably 32-bit overflow */
		gmtlib_report_func (GMT, GMT_MSG_WARNING, where, "Requesting SIZE_MAX number of items (%" PRIuS ") - exceeding 32-bit counting?\n", nelem);
#ifdef DEBUG
		gmtlib_report_func (GMT, GMT_MSG_WARNING, where, "gmt_M_memory called\n");
#endif
		GMT_exit (GMT, GMT_MEMORY_ERROR); return NULL;
	}

#if defined(WIN32) && !defined(USE_MEM_ALIGNED)
	align = false;	/* Turn off alignment for Windows if not specifically selected via USE_MEM_ALIGNED */
#endif

	if (prev_addr) {
		if (nelem == 0) { /* Take care of n == 0 */
			gmt_M_free (GMT, prev_addr);
			return (NULL);
		}
		if (align) {
#ifdef HAVE_FFTW3F
			tmp = NULL; /* currently defunct */
#elif defined(WIN32) || defined(USE_MEM_ALIGNED)
			tmp = _aligned_realloc ( prev_addr, nelem * size, 16U);
#elif defined(HAVE_POSIX_MEMALIGN)
			tmp = NULL; /* currently defunct */
#elif defined(HAVE_MEMALIGN)
			tmp = NULL; /* currently defunct */
#else
#			error "missing memalign"
#endif
		}
		else
			tmp = realloc ( prev_addr, nelem * size);
		if (tmp == NULL)
			memory_die_if_memfail (GMT, nelem, size, where);
	}
	else {
		if (nelem == 0) return (NULL); /* Take care of n == 0 */
		if (align) {
#ifdef HAVE_FFTW3F
			tmp = fftwf_malloc (nelem * size);
#elif defined(WIN32) || defined(USE_MEM_ALIGNED)
			tmp = _aligned_malloc (nelem * size, 16U);
#elif defined(HAVE_POSIX_MEMALIGN)
			(void)posix_memalign (&tmp, 16U, nelem * size);
#elif defined(HAVE_MEMALIGN)
			tmp = memalign (16U, nelem * size);
#else
#			error "missing memalign"
#endif
			if (tmp != NULL)
				tmp = memset (tmp, 0, nelem * size);
		}
		else
			tmp = calloc (nelem, size);
		if (tmp == NULL)
			memory_die_if_memfail (GMT, nelem, size, where);
	}

#ifdef MEMDEBUG
	if (GMT->hidden.mem_keeper->active)
		gmt_memtrack_add (GMT, where, tmp, prev_addr, nelem * size);
#endif
	return (tmp);
}

void gmt_free_func (struct GMT_CTRL *GMT, void *addr, bool align, const char *where) {
	if (addr == NULL) {
#ifndef DEBUG
		/* report freeing unallocated memory only in level GMT_MSG_DEBUG (-V4) */
		gmtlib_report_func (GMT, GMT_MSG_DEBUG, where,
				"tried to free unallocated memory\n");
#endif
		return; /* Do not free a NULL pointer, although allowed */
	}

#ifdef MEMDEBUG
	if (GMT->hidden.mem_keeper && GMT->hidden.mem_keeper->active) {
		bool is_safe_to_free = gmt_memtrack_sub (GMT, where, addr);
		if (is_safe_to_free == false)
			return; /* Address addr was not allocated by gmt_memory_func before */
	}
#endif

#if defined(WIN32) && !defined(USE_MEM_ALIGNED)
	align = false;	/* Turn off alignment for Windows if not specifically selected via USE_MEM_ALIGNED */
#endif

	if (align) {	/* Must free aligned memory */
#ifdef HAVE_FFTW3F
		fftwf_free (addr);
#elif defined(WIN32) && defined(USE_MEM_ALIGNED)
		_aligned_free (addr);
#else
		free (addr);
#endif
	}
	else
		free (addr);
	addr = NULL;
}

void * gmt_malloc_func (struct GMT_CTRL *GMT, void *ptr, size_t n, size_t *n_alloc, size_t element_size, const char *where) {
	/* gmt_M_malloc is used to initialize, grow, and finalize an array allocation in cases
	 * were more memory is needed as new data are read.  There are three different situations:
	 * A) Initial allocation of memory:
	 *	  Signaled by passing *n_alloc == 0 or n_alloc = NULL or ptr = NULL.
	 *    This will initialize the pointer to NULL first.
	 *	  Allocation size is controlled by GMT->session.min_meminc, unless n > 0 which then is used.
	 *	  If n_alloc == NULL then we also do not need to rreturn back the n_alloc value set herein.
	 * B) Incremental increase in memory:
	 *	  Signaled by passing n >= n_alloc.
	 *    The incremental memory is set to 50% of the
	 *	  previous size, but no more than GMT->session.max_meminc. Note, *ptr[n] is the location
	 *	  of where the next assignment will take place, hence n >= n_alloc is used.
	 * C) Finalize memory:
	 *	  Signaled by passing n == 0 and n_alloc > 0.
	 *    Unused memory beyond n_alloc is freed up.
	 * You can use gmt_set_meminc to temporarily change GMT_min_mininc and gmt_reset_meminc will
	 * reset this value to the compilation default.
	 * For 32-bit systems there are safety-values to avoid 32-bit overflow.
	 * Note that n_alloc refers to the number of items to allocate, not the total memory taken
	 * up by the allocated items (which is n_alloc * element_size).
	 * module is the name of the module requesting the memory (main program or library function).
	 * Note: This memory, used for all kinds of things, is not requested to be aligned (align = false),
	 */
	size_t in_n_alloc = (n_alloc) ? *n_alloc : 0U;	/* If NULL it means init, i.e. 0, and we don't pass n_alloc back out */
	if (in_n_alloc == 0 || !ptr) {	/* A) First time allocation, use default minimum size, unless n > 0 is given */
		in_n_alloc = (n == 0) ? GMT->session.min_meminc : n;
		ptr = NULL;	/* Initialize a new pointer to NULL before calling gmt_M_memory with it */
	}
	else if (n == 0 && in_n_alloc > 0)	/* C) Final allocation, set to actual final size */
		n = in_n_alloc;		/* Keep the given n_alloc */
	else if (n < in_n_alloc)	/* Nothing to do, already has enough memory.  This is a safety valve. */
		return (ptr);
	else {		/* B) n >= n_alloc: Compute an increment, but make sure not to exceed int limit under 32-bit systems */
		size_t add;	/* The increment of memory (in items) */
		add = MAX (GMT->session.min_meminc, MIN (*n_alloc/2, GMT->session.max_meminc));	/* Suggested increment from 50% rule, but no less than GMT->session.min_meminc */
		if (add < SIZE_MAX - in_n_alloc) /* test if addition of add and in_n_alloc is safe */
			/* add + in_n_alloc will not overflow */
			in_n_alloc = add + in_n_alloc;
		if (n >= in_n_alloc) in_n_alloc = n + 1;	/* If still not big enough, set n_alloc to n + 1 */
	}

	/* Here n_alloc is set one way or another.  Do the actual [re]allocation for non-aligned memory */

	ptr = gmt_memory_func (GMT, ptr, in_n_alloc, element_size, false, where);
	if (n_alloc) *n_alloc = in_n_alloc;	/* Pass allocated count back out unless given NULL */

	return (ptr);
}

bool gmt_this_alloc_level (struct GMT_CTRL *GMT, unsigned int alloc_level) {
	/* Returns true if the tested alloc_level matches the current function level */
	return (alloc_level == GMT->hidden.func_level);
}

#ifdef FISH_STRDUP_LEAKS
char *gmt_strdup(struct GMT_CTRL *GMT, const char *s) {
	char *p = gmt_M_memory(GMT, NULL, strlen(s) + 1, unsigned char);
	if (p) { strcpy(p, s); }
	return p;
}
#endif

void gmt_set_meminc (struct GMT_CTRL *GMT, size_t increment) {
	/* Temporarily set the GMT_min_memic to this value; restore with gmt_reset_meminc */
	GMT->session.min_meminc = increment;
}

void gmt_reset_meminc (struct GMT_CTRL *GMT) {
	/* Temporarily set the GMT_min_memic to this value; restore with gmt_reset_meminc */
	GMT->session.min_meminc = GMT_MIN_MEMINC;
}
