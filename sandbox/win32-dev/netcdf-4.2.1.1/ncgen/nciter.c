/*********************************************************************
 *   Copyright 2009, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   "$Id $"
 *********************************************************************/

#include "includes.h"
#include "nciter.h"

#undef DOCHUNK

#define CHECK(stat,f) if(stat != NC_NOERR) {check(stat,#f,__FILE__,__LINE__);} else {}

/* forward declarations */
static int nc_blkio_init(size_t bufsize, size_t value_size, int rank, 
			  const size_t *dims, int chunked, size_t *chunksizes, 
			  nciter_t *iter);
static int up_start(int ndims, const size_t *dims, int incdim, size_t inc, 
		    size_t* odom);
#ifdef DOCHUNK	
static int up_start_by_chunks(int ndims, const size_t *dims, 
			      const size_t *chunks, size_t* odom);
#endif

static int nciter_ndims(Symbol*,int*);
static int nciter_dimlens(Symbol*,size_t*);
static int nciter_vartype(Symbol*,Symbol**);
static int nciter_valuesize(Symbol*,size_t*);
#ifdef DOCHUNK
static int nciter_contiguous(Symbol*,int*); /* always returns contiguous */
static int nciter_chunking(Symbol*,int,size_t*); /* no-op */
#endif

static void
check(int err, const char* fcn, const char* file, const int line)
{
    fprintf(stderr,"%s\n",nc_strerror(err));
    fprintf(stderr,"Location: function %s; file %s; line %d\n",
	    fcn,file,line);
    fflush(stderr); fflush(stdout);
    exit(1);
}

/* Initialize iteration for a variable.  Just a wrapper for
 * nc_blkio_init() that makes the netCDF calls needed to initialize
 * lower-level iterator. */
int
nc_get_iter(Symbol* vsym,
	     size_t bufsize,   /* size in bytes of memory buffer */
	     nciter_t *iterp    /* returned opaque iteration state */) 
{
    int stat = NC_NOERR;
    Symbol* vartype;
    size_t value_size;      /* size in bytes of each variable element */
    int ndims;		    /* number of dimensions for variable */
    size_t dimsizes[NC_MAX_VAR_DIMS]; /* variable dimension sizes */
    size_t chunksizes[NC_MAX_VAR_DIMS]; /* corresponding chunk sizes */
    long long nvalues = 1;
    int dim;
    int chunked = 0;

    memset((void*)iterp,0,sizeof(nciter_t)); /* make sure it is initialized */

    stat = nciter_ndims(vsym, &ndims);
    CHECK(stat, nciter_ndims);
    stat = nciter_dimlens(vsym,dimsizes);
    /* compute total # elements */
    nvalues=1;
    for(dim = 0; dim < ndims; dim++) {
	nvalues *= dimsizes[dim];
    }
    stat = nciter_vartype(vsym, &vartype);
    CHECK(stat, nciter_vartype);
    stat = nciter_valuesize(vartype,&value_size);
    CHECK(stat, nciter_valuesize);
#ifdef USE_NETCDF4    
#ifdef DOCHUNK
    {
	int contig = 1;
	if(ndims > 0) {
	    stat = nc_inq_var_chunking(ncid, varid, &contig, NULL);
	    CHECK(stat, nc_inq_var_chunking);
	}
	if(contig == 0) {	/* chunked */
	    stat = nc_inq_var_chunking(ncid, varid, &contig, chunksizes);
	    CHECK(stat, nc_inq_var_chunking);
	    chunked = 1;
	}
    }
#else
    chunked = 0;
#endif
#endif	/* USE_NETCDF4 */
    stat = nc_blkio_init(bufsize, value_size, ndims, dimsizes, 
			 chunked, chunksizes, iterp);
    CHECK(stat, nc_blkio_init);
    iterp->to_get = 0;
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
#ifdef DOCHUNK
	    for(i = 0; i < iter->rank; i++) {
		start[i] = 0;
		count[i] = iter->chunksizes[i];
	    }
#endif
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
#ifdef DOCHUNK
	    iter->more = up_start_by_chunks(iter->rank, iter->dimsizes, 
					    iter->chunksizes, start);
	    /* adjust count to stay in range of dimsizes */
	    for(i = 0; i < iter->rank; i++) {
		int leftover = iter->dimsizes[i] - start[i];
		count[i] = iter->chunksizes[i];
		if(leftover < count[i]) 
		    count[i] = leftover;
	    }
#endif
	}
    }
    iter->to_get = 1;
    for(i = 0; i < iter->rank; i++) {
	iter->to_get *= count[i];
    }
    return iter->more == 0 ? 0 : iter->to_get ;
}


/* Initialize block iteration for variables, including those that
 * won't fit in the copy buffer all at once.  Returns error if
 * variable is chunked but size of chunks is too big to fit in bufsize
 * bytes. */
static int
nc_blkio_init(size_t bufsize, 	/* size in bytes of in-memory copy buffer */
              size_t value_size, /* size in bytes of each variable element */
	      int rank,		 /* number of dimensions for variable */
	      const size_t *dims, /* variable dimension sizes */
	      int chunked,	  /* 1 if variable is chunked, 0 otherwise */
	      size_t *chunksizes, /* if chunked, variable chunk sizes */
	      nciter_t *iter /* returned iteration state, don't mess with it */
    ) {
    int stat = NC_NOERR;
    int i;
    long long prod;

    iter->rank = rank;
    iter->first = 1;
    iter->more = 1;
    iter->chunked = chunked;
    for(i = 0; i < rank; i++)
	iter->dimsizes[i] = dims[i];
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
#ifdef DOCHUNK
    /* else, handle chunked case */
    for(i = 0; i < rank; i++) {
	iter->chunksizes[i] = chunksizes[i];
	prod *= chunksizes[i];
    }
    if(prod > bufsize) {
	stat = NC_ENOMEM;
	fprintf(stderr, "chunksize (= %ld) > copy_buffer size (= %ld)\n", prod, bufsize);
    }
#endif
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

#ifdef DOCHUNK
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
#endif

static int
nciter_ndims(Symbol* sym, int* ndimsp)
{
    if(ndimsp) *ndimsp = sym->typ.dimset.ndims;
    return NC_NOERR;
}

static int
nciter_dimlens(Symbol* sym, size_t* dimlens)
{
    int i;
    int ndims = sym->typ.dimset.ndims;
    for(i=0;i<ndims;i++) {
	Symbol* dim = sym->typ.dimset.dimsyms[i];
	dimlens[i] = MAX(dim->dim.declsize,dim->dim.unlimitedsize);
    }
    return NC_NOERR;
}

static int
nciter_vartype(Symbol* sym, Symbol** tsymp)
{
    if(tsymp)
	*tsymp = sym->typ.basetype;
    return NC_NOERR;
}

/* For ncgen, we want the iterator to return
   dimension indices, not offsets, so force size
   to be one.
*/
static int
nciter_valuesize(Symbol* tsym, size_t* valuesizep)
{
    if(valuesizep)
	*valuesizep = 1;
    return NC_NOERR;
}

#ifdef DOCHUNK
static int
nciter_contiguous(Symbol* sym, int* iscontiguous)
{
    if(iscontiguous != NULL)
        *iscontiguous = 1; /* always returns contiguous */
    return NC_NOERR;
}

static int
nciter_chunking(Symbol* sym, int contig, size_t* sizep)
{
    return NC_NOERR; /* no-op */
}
#endif

