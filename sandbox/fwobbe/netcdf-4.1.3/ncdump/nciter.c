/*********************************************************************
 *   Copyright 2009, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   "$Id$"
 *********************************************************************/

#include "config.h"		/* for USE_NETCDF4 macro */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "nciter.h"

/* Initialize block iteration for variables, including those that
 * won't fit in the copy buffer all at once.  Returns error if
 * variable is chunked but size of chunks is too big to fit in bufsize
 * bytes. */
static int
nc_blkio_init(size_t bufsize, 	/* size in bytes of in-memory copy buffer */
	      size_t value_size, /* size in bytes of each variable element */
	      int rank,		 /* number of dimensions for variable */
	      int chunked,	  /* 1 if variable is chunked, 0 otherwise */
	      nciter_t *iter /* returned iteration state, don't mess with it */
    ) {
    int stat = NC_NOERR;
    int i;
    long long prod;
    size_t *dims = iter->dimsizes;

    iter->rank = rank;
    iter->first = 1;
    iter->more = 1;
    iter->chunked = chunked;
    prod = value_size;
    if(iter->chunked == 0) {	/* contiguous */
	iter->right_dim = rank - 1;
	for(i = rank; i > 0; i--) {
	    if(prod*dims[i-1] <= bufsize) {
		prod *= dims[i-1];
		iter->right_dim--;
	    } else {
		break;
	    }
	}
	if (i > 0) {	     /* variable won't fit in bufsize bytes */
	    iter->rows = bufsize/prod;
	    iter->numrows = dims[iter->right_dim] / iter->rows;
	    iter->leftover = dims[iter->right_dim] - iter->numrows * iter->rows;
	    iter->cur = 1;
	    iter->inc = iter->rows;
	    return stat;
	}
	/* else, variable will fit in bufsize bytes of memory. */
	iter->right_dim = 0;
	iter->rows = dims[0];
	iter->inc = 0;
	return stat;
    } 
    /* else, handle chunked case */
    for(i = 0; i < rank; i++) {
	prod *= iter->chunksizes[i];
    }
    if(prod > bufsize) {
	stat = NC_ENOMEM;
	fprintf(stderr, "chunksize (= %ld) > copy_buffer size (= %ld)\n",
			(long)prod, (long)bufsize);
    }
    return stat;
}

/* From netCDF type in group igrp, get size in memory needed for each
 * value.  Wouldn't be needed if nc_inq_type() was a netCDF-3 function
 * too. */
static int
inq_value_size(int igrp, nc_type vartype, size_t *value_sizep) {
    int stat = NC_NOERR;
    
#ifdef USE_NETCDF4
    NC_CHECK(nc_inq_type(igrp, vartype, NULL, value_sizep));
#else
    switch(vartype) {
    case NC_BYTE:
	*value_sizep = sizeof(signed char);
	break;
    case NC_CHAR:
	*value_sizep = sizeof(char);
	break;
    case NC_SHORT:
	*value_sizep = sizeof(short);
	break;
    case NC_INT:
	*value_sizep = sizeof(int);
	break;
    case NC_FLOAT:
	*value_sizep = sizeof(float);
	break;
    case NC_DOUBLE:
	*value_sizep = sizeof(double);
	break;
    default:
	NC_CHECK(NC_EBADTYPE);
	break;
    }
#endif	/* USE_NETCDF4 */
    return stat;
}

/*
 * Updates a vector of size_t, odometer style.  Returns 0 if odometer
 * overflowed, else 1.
 */
static int
up_start(
     int ndims,		 /* Number of dimensions */
     const size_t *dims, /* The "odometer" limits for each dimension */
     int incdim,	 /* the odmometer increment dimension */
     size_t inc,	 /* the odometer increment for that dimension */
     size_t* odom	 /* The "odometer" vector to be updated */
     )
{
    int id;
    int ret = 1;

    if(inc == 0) {
	return 0;
    }
    odom[incdim] += inc;
    for (id = incdim; id > 0; id--) {
	if(odom[id] >= dims[id]) {
	    odom[id-1]++;
	    odom[id] -= dims[id];
	}
    }
    if (odom[0] >= dims[0])
      ret = 0;
    return ret;
}

/*
 * Updates a vector of size_t, odometer style, for chunk access.
 * Returns 0 if odometer overflowed, else 1.
 */
static int
up_start_by_chunks(
     int ndims,		 /* Number of dimensions */
     const size_t *dims, /* The "odometer" limits for each dimension */
     const size_t *chunks, /* the odometer increments for each dimension */
     size_t* odom	 /* The "odometer" vector to be updated */
     )
{
    int incdim = ndims - 1;
    int id;
    int ret = 1;

    odom[incdim] += chunks[incdim];
    for (id = incdim; id > 0; id--) {
	if(odom[id] >= dims[id]) {
	    odom[id-1] += chunks[id-1];
	    /* odom[id] -= dims[id]; */
	    odom[id] = 0;
	}
    }
    if (odom[0] >= dims[0])
      ret = 0;
    return ret;
}

/* initialize and return a new empty stack of grpids */
static ncgiter_t *
gs_init() {
    ncgiter_t *s = emalloc(sizeof(ncgiter_t));
    s->ngrps = 0;
    s->top = NULL;
    return s;
}

/* free a stack and all its nodes */
static void
gs_free(ncgiter_t *s) {
    grpnode_t *n0, *n1;
    n0 = s->top;
    while (n0) {
	n1 = n0->next;
	free(n0);
	n0 = n1;
    }
    free(s);
}

/* test if a stack is empty */
static int
gs_empty(ncgiter_t *s)
{
    return s->ngrps == 0;
}

/* push a grpid on stack */
static void
gs_push(ncgiter_t *s, int grpid)
{
    grpnode_t *node = emalloc(sizeof(grpnode_t));
 
    node->grpid = grpid;
    node->next = gs_empty(s) ? NULL : s->top;
    s->top = node;
    s->ngrps++;
}

/* pop value off stack and return */
static int 
gs_pop(ncgiter_t *s)
{
    if (gs_empty(s)) {
	return -1;		/* underflow, stack is empty */
    } else {			/* pop a node */
	grpnode_t *top = s->top;
	int value = top->grpid;
	s->top = top->next;
	/* TODO: first call to free gets seg fault with libumem */
	free(top);
	s->ngrps--;
	return value;
    }
}

/* Return top value on stack without popping stack.  Defined for
 * completeness but not used (here). */
static int 
gs_top(ncgiter_t *s)
{
    if (gs_empty(s)) {
	return -1;		/* underflow, stack is empty */
    } else {			/* get top value */
	grpnode_t *top = s->top;
	int value = top->grpid;
	return value;
    }
}

/* Like netCDF-4 function nc_inq_grps(), but can be called from
 * netCDF-3 only code as well.  Maybe this is what nc_inq_grps()
 * should do if built without netCDF-4 data model support. */
static int
nc_inq_grps2(int ncid, int *numgrps, int *grpids)
{
    int stat = NC_NOERR;

    /* just check if ncid is valid id of open netCDF file */
    NC_CHECK(nc_inq(ncid, NULL, NULL, NULL, NULL));

#ifdef USE_NETCDF4
    NC_CHECK(nc_inq_grps(ncid, numgrps, grpids));
#else
    *numgrps = 0;
#endif
    return stat;
}

/* Begin public interfaces */

/* Initialize iteration for a variable.  Just a wrapper for
 * nc_blkio_init() that makes the netCDF calls needed to initialize
 * lower-level iterator. */
int
nc_get_iter(int ncid,
	     int varid,
	     size_t bufsize,   /* size in bytes of memory buffer */
	     nciter_t **iterpp /* returned opaque iteration state */) 
{
    int stat = NC_NOERR;
    nciter_t *iterp;
    nc_type vartype;
    size_t value_size;      /* size in bytes of each variable element */
    int ndims;		    /* number of dimensions for variable */
    int *dimids;
    long long nvalues = 1;
    int dim;
    int chunked = 0;

    /* Caller should free this by calling nc_free_iter(iterp) */
    iterp = (nciter_t *) emalloc(sizeof(nciter_t));
    memset((void*)iterp,0,sizeof(nciter_t)); /* make sure it is initialized */

    NC_CHECK(nc_inq_varndims(ncid, varid, &ndims));
    dimids = (int *) emalloc((ndims + 1) * sizeof(size_t));
    iterp->dimsizes = (size_t *) emalloc((ndims + 1) * sizeof(size_t));
    iterp->chunksizes = (size_t *) emalloc((ndims + 1) * sizeof(size_t));

    NC_CHECK(nc_inq_vardimid (ncid, varid, dimids));
    for(dim = 0; dim < ndims; dim++) {
	size_t len;
	NC_CHECK(nc_inq_dimlen(ncid, dimids[dim], &len));
	nvalues *= len;
	iterp->dimsizes[dim] = len;
    }
    NC_CHECK(nc_inq_vartype(ncid, varid, &vartype));
    NC_CHECK(inq_value_size(ncid, vartype, &value_size));
#ifdef USE_NETCDF4    
    {
	int contig = 1;
	if(ndims > 0) {
	    NC_CHECK(nc_inq_var_chunking(ncid, varid, &contig, NULL));
	}
	if(contig == 0) {	/* chunked */
	    NC_CHECK(nc_inq_var_chunking(ncid, varid, &contig, iterp->chunksizes));
	    chunked = 1;
	}
    }
#endif	/* USE_NETCDF4 */
    NC_CHECK(nc_blkio_init(bufsize, value_size, ndims, chunked, iterp));
    iterp->to_get = 0;
    free(dimids);
    *iterpp = iterp;
    return stat;
}

/* Iterate on blocks for variables, by updating start and count vector
 * for next vara call.  Assumes nc_get_iter called first.  Returns
 * number of variable values to get, 0 if done, negative number if
 * error, so use like this: 
   size_t to_get;
   while((to_get = nc_next_iter(&iter, start, count)) > 0) { 
      ... iteration ... 
   } 
   if(to_get < 0) { ... handle error ... }
 */
size_t
nc_next_iter(nciter_t *iter,	/* returned opaque iteration state */
	     size_t *start, 	/* returned start vector for next vara call */
	     size_t *count	/* returned count vector for next vara call */
    ) {
    int i;
    /* Note: special case for chunked variables is just an
     * optimization, the contiguous code below is OK even
     * for chunked variables, but in general will do more I/O ... */
    if(iter->first) {
	if(!iter->chunked) { 	/* contiguous storage */
	    for(i = 0; i < iter->right_dim; i++) {
		start[i] = 0;
		count[i] = 1;
	    }
	    start[iter->right_dim] = 0;
	    count[iter->right_dim] = iter->rows;
	    for(i = iter->right_dim + 1; i < iter->rank; i++) {
		start[i] = 0;
		count[i] = iter->dimsizes[i];
	    }
	} else {		/* chunked storage */
	    for(i = 0; i < iter->rank; i++) {
		start[i] = 0;
		count[i] = iter->chunksizes[i];
	    }
	}
	iter->first = 0;
    } else {
	if(!iter->chunked) { 	/* contiguous storage */
	    iter->more = up_start(iter->rank, iter->dimsizes, iter->right_dim, 
				  iter->inc, start);
	    /* iterate on pieces of variable */
	    if(iter->cur < iter->numrows) {
		iter->inc = iter->rows;
		count[iter->right_dim] = iter->rows;
		iter->cur++;
	    } else {
		if(iter->leftover > 0) {
		    count[iter->right_dim] = iter->leftover;
		    iter->inc = iter->leftover;
		    iter->cur = 0;
		}
	    }
	} else {		/* chunked storage */
	    iter->more = up_start_by_chunks(iter->rank, iter->dimsizes, 
					    iter->chunksizes, start);
	    /* adjust count to stay in range of dimsizes */
	    for(i = 0; i < iter->rank; i++) {
		int leftover = iter->dimsizes[i] - start[i];
		count[i] = iter->chunksizes[i];
		if(leftover < count[i]) 
		    count[i] = leftover;
	    }
	}
    }
    iter->to_get = 1;
    for(i = 0; i < iter->rank; i++) {
	iter->to_get *= count[i];
    }
    return iter->more == 0 ? 0 : iter->to_get ;
}

/* Free iterator and its internally allocated memory */
int
nc_free_iter(nciter_t *iterp) {
    if(iterp->dimsizes)
	free(iterp->dimsizes);
    if(iterp->chunksizes)
	free(iterp->chunksizes);
    if(iterp)
	free(iterp);
    return NC_NOERR;
}

/* Initialize group iterator for start group and all its descendant
 * groups. */
int
nc_get_giter(int grpid,	       /* start group id */
	    ncgiter_t **iterp  /* returned opaque iteration state */
    ) 
{
    int stat = NC_NOERR;

    stat = nc_inq(grpid, NULL, NULL, NULL, NULL); /* check if grpid is valid */
    if(stat != NC_EBADGRPID && stat != NC_EBADID) {
	*iterp = gs_init();
	gs_push(*iterp, grpid);
    }

    return stat;
}

/* 
 * Get group id of next group.  On first call gets start group id,
 * subsequently returns other subgroup ids in preorder.  Returns zero
 * when no more groups left.
 */
int
nc_next_giter(ncgiter_t *iterp, int *grpidp) {
    int stat = NC_NOERR;
    int numgrps;
    int *grpids;
    int i;

    if(gs_empty(iterp)) {
	*grpidp = 0;		/* not a group, signals iterator is done */
    } else {
	*grpidp = gs_pop(iterp);
	NC_CHECK(nc_inq_grps2(*grpidp, &numgrps, NULL));
	if(numgrps > 0) {
	    grpids = (int *)emalloc(sizeof(int) * numgrps);
	    NC_CHECK(nc_inq_grps2(*grpidp, &numgrps, grpids));
	    for(i = numgrps - 1; i >= 0; i--) { /* push ids on stack in reverse order */
		gs_push(iterp, grpids[i]);
	    }
	    free(grpids);
	}
    }
    return stat;
}

/*
 * Release group iter.
 */
void
nc_free_giter(ncgiter_t *iterp)
{
    gs_free(iterp);
}

/* 
 * Get total number of groups (including the top-level group and all
 * descendant groups, recursively) and all descendant subgroup ids
 * (including the input rootid of the start group) for a group and
 * all its descendants, in preorder.
 *
 * If grpids or numgrps is NULL, it will be ignored.  So typical use
 * is to call with grpids NULL to get numgrps, allocate enough space
 * for the group ids, then call again to get them.
 */
int
nc_inq_grps_full(int rootid, int *numgrps, int *grpids) 
{
    int stat = NC_NOERR;
    ncgiter_t *giter;		/* pointer to group iterator */
    int grpid;
    size_t count;

    NC_CHECK(nc_get_giter(rootid, &giter));
    
    count = 0;
    NC_CHECK(nc_next_giter(giter, &grpid));
    while(grpid != 0) {
	if(grpids)
	    grpids[count] = grpid;
	count++;
	NC_CHECK(nc_next_giter(giter, &grpid));
    }
    if(numgrps)
	*numgrps = count;
    nc_free_giter(giter);
    return stat;
}
