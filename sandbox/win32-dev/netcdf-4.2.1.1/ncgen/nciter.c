/*********************************************************************
 *   Copyright 2009, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   "$Id $"
 *********************************************************************/

#include "includes.h"
#include "nciter.h"


#define CHECK(stat,f) if(stat != NC_NOERR) {check(stat,#f,__FILE__,__LINE__);} else {}

/* forward declarations */
static int nc_blkio_init(size_t bufsize, size_t value_size, int rank, 
			  const size_t *dims, nciter_t *iter);
static int up_start(int ndims, const size_t *dims, int incdim, size_t inc, 
		    size_t* odom);

static int nciter_ndims(Symbol*,int*);
static int nciter_dimlens(Symbol*,size_t*);
static int nciter_vartype(Symbol*,Symbol**);
static int nciter_valuesize(Symbol*,size_t*);

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
    long long nvalues = 1;
    int dim;

    memset((void*)iterp,0,sizeof(nciter_t)); /* make sure it is initialized */

    stat = nciter_ndims(vsym, &ndims);
    CHECK(stat, nciter_ndims);
    stat = nciter_dimlens(vsym,dimsizes);
    CHECK(stat, nciter_dimlens);
    /* compute total # elements */
    nvalues=1;
    for(dim = 0; dim < ndims; dim++) {
	nvalues *= dimsizes[dim];
    }
    stat = nciter_vartype(vsym, &vartype);
    CHECK(stat, nciter_vartype);
    stat = nciter_valuesize(vartype,&value_size);
    CHECK(stat, nciter_valuesize);
    stat = nc_blkio_init(bufsize, value_size, ndims, dimsizes, iterp);
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
    if(iter->first) {
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
        iter->first = 0;
    } else {
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
        
    }
    iter->to_get = 1;
    for(i = 0; i < iter->rank; i++) {
	iter->to_get *= count[i];
    }
    return iter->more == 0 ? 0 : iter->to_get ;
}


/* Initialize block iteration for variables, including those that
 * won't fit in the copy buffer all at once.
 */
static int
nc_blkio_init(size_t bufsize, 	/* size in bytes of in-memory copy buffer */
              size_t value_size, /* size in bytes of each variable element */
	      int rank,		 /* number of dimensions for variable */
	      const size_t *dims, /* variable dimension sizes */
	      nciter_t *iter /* returned iteration state, don't mess with it */
    ) {
    int stat = NC_NOERR;
    int i;
    long long prod;

    iter->rank = rank;
    iter->first = 1;
    iter->more = 1;
    for(i = 0; i < rank; i++)
	iter->dimsizes[i] = dims[i];
    prod = value_size;
    iter->right_dim = rank - 1;
    for(i = rank; i > 0; i--) {
        if(prod*dims[i-1] <= bufsize) {
            prod *= dims[i-1];
            iter->right_dim--;
        } else {
            break;
        }
    }
    if (i > 0) {         /* variable won't fit in bufsize bytes */
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
	dimlens[i] = dim->dim.declsize;
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
