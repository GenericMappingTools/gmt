#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>		/* for sysconf */
#include <sys/resource.h>
#include <nc_tests.h>		/* The ERR macro is here... */
#include <netcdf.h>

#define FILENAME "tst_chunks3.nc"

/*
 * The following timing macros can be used by including the necessary
 * declarations with
 *
 *     TIMING_DECLS(seconds)
 *
 * and surrounding sections of code to be timed with the "statements"
 *
 *     TIMING_START
 *     [code to be timed goes here]
 *     TIMING_END(seconds)
 *
 * The macros assume the user has stored a description of what is
 * being timed in a 100-char string time_mess, and has included
 * <sys/times.h> and <sys/resource.h>.  The timing message printed by
 * TIMING_END is not terminated by a new-line, to permit appending
 * additional text to that line, so user must at least printf("\n")
 * after that.
 */

#define TIMING_DECLS(seconds)						       \
	long TMreps;		/* counts repetitions of timed code */ \
	long TMrepeats;		/* repetitions needed to exceed 0.1 second */ \
	long emic ; 		/* elapsed time in microseconds */ \
	struct rusage ru; \
	long inb, oub; \
        char time_mess[100]; \
        float seconds;
      
#define TIMING_START \
	TMrepeats = 1; \
	do { \
	    if(getrusage(RUSAGE_SELF, &ru)) {			\
		printf("getrusage failed, returned %d\n", errno);}	\
	    emic = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) \
		     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec);     \
	    inb = ru.ru_inblock; \
            oub = ru.ru_oublock; \
	    for(TMreps=0; TMreps < TMrepeats; TMreps++) {
	
#define TIMING_END(seconds)				\
            } \
	    if(getrusage(RUSAGE_SELF, &ru)) {			\
		printf("getrusage failed, returned %d\n", errno);}	\
	    emic = (1000000*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) \
		     + ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) - emic; \
	    inb = ru.ru_inblock - inb; \
            oub = ru.ru_oublock - oub; \
	    TMrepeats *= 2; \
	} while (emic < 100000.0 ); \
	seconds = emic / (1000000.0 * TMreps); \
	printf("%-45.45s %7.2g sec", \
	       time_mess, seconds);

/* This macro prints an error message with line number and name of
 * test program. */
#define ERR1(n) do {						  \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d - %s\n", \
	__FILE__, __LINE__, nc_strerror(n));			 \
return n; \
} while (0)

#define NC_COMPRESSED 1

void
parse_args(int argc, char *argv[], /* from command-line invocation */
	   int *deflate_levelp,	   /* returned: 0 uncompressed, 
				      1-9 compression level */
	   int *shufflep,	   /* returned: 1 if shuffle, otherwise 0 */
	   size_t *dims,	   /* returned: dimension sizes */
	   size_t *chunks,	   /* returned: chunk sizes */
	   size_t *cache_sizep,	   /* returned: cache size (bytes) */
	   size_t *cache_nelemsp,  /* returned: cache capacity (chunks) */
	   float *cache_prep)	   /* returned: cache preemption policy (0-1) */
{

    if(argc > 1) {
	*deflate_levelp = atol(argv[1]);
	if (*deflate_levelp < 0) {
	    *deflate_levelp = -*deflate_levelp;
	    *shufflep = NC_SHUFFLE;
	}
    }
    if(argc > 2)
	dims[0] = atol(argv[2]);
    if(argc > 3)
	chunks[0] = atol(argv[3]);
    else
	chunks[0] = (dims[0]+7)/8;
    if(argc > 4)
	dims[1] = atol(argv[4]);
    else
	dims[1] = dims[0];
    if(argc > 5)
	chunks[1] = atol(argv[5]);
    else 
	chunks[1] = chunks[0];
    if(argc > 6)
	dims[2] = atol(argv[6]);
    else
	dims[2] = dims[1];
    if(argc > 7)
	chunks[2] = atol(argv[7]);
    else
	chunks[2] = chunks[1];
    if(argc > 8)
	*cache_sizep = atol(argv[8]);
    if(argc > 9)
	*cache_nelemsp = atol(argv[9]);
    if(argc > 10)
	*cache_prep = atof(argv[10]);
    if(argc > 11) {
	printf("Usage: %s [def_level] [dim1] [chunk1] [dim2] [chunk2] [dim3] [chunk3] [cache_size] [cache_nelems] [cache_pre]\n", 
	       argv[0]);
	exit(1);
    } 
    return;
}

void *
emalloc(size_t bytes) {
    size_t *memory;
    memory = malloc(bytes);
    if(memory == 0) {
	printf("malloc failed\n");
	exit(2);
    }
    return memory;
}


/* compare contiguous, chunked, and compressed performance */
int
main(int argc, char *argv[]) {

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */
    int i, j, k;
    int dim1id, dim2id, dim3id;
    int varid_g;		  /* varid for contiguous */
    int varid_k;		  /* varid for chunked */
    int varid_x;		  /* varid for compressed */

    float *varxy, *varxz, *varyz;    /* 2D memory slabs used for I/O */
    int mm;
    size_t dims[] = {256, 256, 256}; /* default dim lengths */
    size_t chunks[] = {32, 32, 32}; /* default chunk sizes */
    size_t start[3], count[3];
    float contig_time, chunked_time, compressed_time, ratio;
    int deflate_level = 1;	/* default compression level, 9 is
				 * better and slower.  If negative,
				 * turn on shuffle filter also. */
    int shuffle = NC_NOSHUFFLE;
    size_t cache_size_def;
    size_t cache_hash_def;
    float cache_pre_def;
    size_t cache_size = 0;	    /* use library default */
    size_t cache_hash = 0;	    /* use library default */
    float cache_pre = -1.0f;	    /* use library default */

    /* rank (number of dimensions) for each variable */
#   define RANK_var1 3

    /* variable shapes */
    int var_dims[RANK_var1];

    TIMING_DECLS(TMsec) ;

    /* From args, get parameters for timing, including variable and
       chunk sizes.  Negative deflate level means also use shuffle
       filter. */
    parse_args(argc, argv, &deflate_level, &shuffle, dims, 
	       chunks, &cache_size, &cache_hash, &cache_pre);

    /* get cache defaults, then set cache parameters that are not default */
    if((stat = nc_get_chunk_cache(&cache_size_def, &cache_hash_def, 
				   &cache_pre_def)))
	ERR1(stat);
    if(cache_size == 0)
	cache_size = cache_size_def;
    if(cache_hash == 0)
	cache_hash = cache_hash_def;
    if(cache_pre == -1.0f)
	cache_pre = cache_pre_def;
    if((stat = nc_set_chunk_cache(cache_size, cache_hash, cache_pre)))
	ERR1(stat);
    printf("cache: %3.2f MBytes  %ld objs  %3.2f preempt, ", 
	   cache_size/1.e6, cache_hash, cache_pre);

    if(deflate_level == 0) {
	printf("uncompressed        ");
    } else {
	printf("compression level %d", deflate_level);
    }
    if(shuffle == 1) {
	printf(", shuffled");
    }
    printf("\n\n");

    /* initialize 2D slabs for writing along each axis with phony data */
    varyz = (float *) emalloc(sizeof(float) * 1 * dims[1] * dims[2]);
    varxz = (float *) emalloc(sizeof(float) * dims[0] * 1 * dims[2]);
    varxy = (float *) emalloc(sizeof(float) * dims[0] * dims[1] * 1);
    mm = 0;
    for(j = 0; j < dims[1]; j++) {
	for(k = 0; k < dims[2]; k++) {
	    varyz[mm++] = k + dims[2]*j;
	}
    }
    mm = 0;
    for(i = 0; i < dims[0]; i++) {
	for(k = 0; k < dims[2]; k++) {
	    varxz[mm++] = k + dims[2]*i;
	}
    }
    mm = 0;
    for(i = 0; i < dims[0]; i++) {
	for(j = 0; j < dims[1]; j++) {
	    varxy[mm++] = j + dims[1]*i;
	}
    }

    if((stat = nc_create(FILENAME, NC_NETCDF4 | NC_CLASSIC_MODEL, &ncid)))
	ERR1(stat);
    
    /* define dimensions */
    if((stat = nc_def_dim(ncid, "dim1", dims[0], &dim1id)))
	ERR1(stat);
    if((stat = nc_def_dim(ncid, "dim2", dims[1], &dim2id)))
	ERR1(stat);
    if((stat = nc_def_dim(ncid, "dim3", dims[2], &dim3id)))
	ERR1(stat);
    
    /* define variables */
    var_dims[0] = dim1id;
    var_dims[1] = dim2id;
    var_dims[2] = dim3id;
    if((stat = nc_def_var(ncid, "var_contiguous", NC_FLOAT, RANK_var1, 
			   var_dims, &varid_g)))
	ERR1(stat);
    if((stat = nc_def_var(ncid, "var_chunked", NC_FLOAT, RANK_var1, 
			   var_dims, &varid_k)))
	ERR1(stat);
    if((stat = nc_def_var(ncid, "var_compressed", NC_FLOAT, RANK_var1, 
			   var_dims, &varid_x)))
	ERR1(stat);

    if((stat = nc_def_var_chunking(ncid, varid_g, NC_CONTIGUOUS, 0)))
	ERR1(stat);

    if((stat = nc_def_var_chunking(ncid, varid_k, NC_CHUNKED, chunks)))
	ERR1(stat);

    if((stat = nc_def_var_chunking(ncid, varid_x, NC_CHUNKED, chunks)))
	ERR1(stat);

    if (deflate_level != 0) {
	if((stat = nc_def_var_deflate(ncid, varid_x, shuffle, 
				       NC_COMPRESSED, deflate_level)))
	    ERR1(stat);
    }

    /* leave define mode */
    if((stat = nc_enddef (ncid)))
	ERR1(stat);
    
    /* write each variable one yz slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = 1;
    count[1] = dims[1];
    count[2] = dims[2];

    sprintf(time_mess,"  contiguous write %3ld %3ld %3ld", 
	    1, dims[1], dims[2]);
    TIMING_START ;
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_put_vara(ncid, varid_g, start, count, &varyz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    printf("\n");
    contig_time = TMsec;

    sprintf(time_mess,"  chunked    write %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    1, dims[1], dims[2], chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_put_vara(ncid, varid_k, start, count, &varyz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    chunked_time = TMsec;
    ratio = contig_time/chunked_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);

    sprintf(time_mess,"  compressed write %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    1, dims[1], dims[2], chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_put_vara(ncid, varid_x, start, count, &varyz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    compressed_time = TMsec;
    ratio = contig_time/compressed_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);
    printf("\n");

    /* write each variable one xz slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = dims[0];
    count[1] = 1;
    count[2] = dims[2];

    sprintf(time_mess,"  contiguous write %3ld %3ld %3ld", 
	    dims[0], 1, dims[2]);
    TIMING_START ;
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_put_vara(ncid, varid_g, start, count, &varxz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    printf("\n");
    contig_time = TMsec;

    sprintf(time_mess,"  chunked    write %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], 1, dims[2], chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_put_vara(ncid, varid_k, start, count, &varxz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    chunked_time = TMsec;
    ratio = contig_time/chunked_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);

    sprintf(time_mess,"  compressed write %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], 1, dims[2], chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_put_vara(ncid, varid_x, start, count, &varxz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    compressed_time = TMsec;
    ratio = contig_time/compressed_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);
    printf("\n");
    
    /* write each variable one xy slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = dims[0];
    count[1] = dims[1];
    count[2] = 1;

    sprintf(time_mess,"  contiguous write %3ld %3ld %3ld", 
	    dims[0], dims[1], 1);
    TIMING_START ;
    for(i = 0; i < dims[2]; i++) {
	start[2] = i;
	if((stat = nc_put_vara(ncid, varid_g, start, count, &varxy[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    printf("\n");
    contig_time = TMsec;
    
    sprintf(time_mess,"  chunked    write %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], dims[1], 1, chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[2]; i++) {
	start[2] = i;
	if((stat = nc_put_vara(ncid, varid_k, start, count, &varxy[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    chunked_time = TMsec;
    ratio = contig_time/chunked_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);

    sprintf(time_mess,"  compressed write %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], dims[1], 1, chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[2]; i++) {
	start[2] = i;
	if((stat = nc_put_vara(ncid, varid_x, start, count, &varxy[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    compressed_time = TMsec;
    ratio = contig_time/compressed_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);
    printf("\n");

    /* read each variable one yz slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = 1;
    count[1] = dims[1];
    count[2] = dims[2];

    sprintf(time_mess,"  contiguous  read %3ld %3ld %3ld", 
	    1, dims[1], dims[2]);
    TIMING_START ;
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_get_vara(ncid, varid_g, start, count, &varyz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    printf("\n");
    contig_time = TMsec;
    
    sprintf(time_mess,"  chunked     read %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    1, dims[1], dims[2] , chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_get_vara(ncid, varid_k, start, count, &varyz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    chunked_time = TMsec;
    ratio = contig_time/chunked_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);
    
    sprintf(time_mess,"  compressed  read %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    1, dims[1], dims[2] , chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_get_vara(ncid, varid_x, start, count, &varyz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    compressed_time = TMsec;
    ratio = contig_time/compressed_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);
    printf("\n");

    /* read each variable one xz slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = dims[0];
    count[1] = 1;
    count[2] = dims[2];

    sprintf(time_mess,"  contiguous  read %3ld %3ld %3ld", 
	    dims[0], 1, dims[2]);
    TIMING_START ;
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_get_vara(ncid, varid_g, start, count, &varxz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    printf("\n");
    contig_time = TMsec;
    
    sprintf(time_mess,"  chunked     read %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], 1, dims[2], chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_get_vara(ncid, varid_k, start, count, &varxz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    chunked_time = TMsec;
    ratio = contig_time/chunked_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);
    
    sprintf(time_mess,"  compressed  read %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], 1, dims[2], chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_get_vara(ncid, varid_x, start, count, &varxz[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    compressed_time = TMsec;
    ratio = contig_time/compressed_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);
    printf("\n");

    /* read variable one xy slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = dims[0];
    count[1] = dims[1];
    count[2] = 1;

    sprintf(time_mess,"  contiguous  read %3ld %3ld %3ld", 
	    dims[0], dims[1], 1);
    TIMING_START ;
    for(i = 0; i < dims[2]; i++) {
	start[2] = i;
	if((stat = nc_get_vara(ncid, varid_g, start, count, &varxy[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    printf("\n");
    contig_time = TMsec;

    sprintf(time_mess,"  chunked     read %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], dims[1], 1, chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[2]; i++) {
	start[2] = i;
	if((stat = nc_get_vara(ncid, varid_k, start, count, &varxy[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    chunked_time = TMsec;
    ratio = contig_time/chunked_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);

    sprintf(time_mess,"  compressed  read %3ld %3ld %3ld  %3ld %3ld %3ld", 
	    dims[0], dims[1], 1, chunks[0], chunks[1], chunks[2]);
    TIMING_START ;
    for(i = 0; i < dims[2]; i++) {
	start[2] = i;
	if((stat = nc_get_vara(ncid, varid_x, start, count, &varxy[0])))
	    ERR1(stat);
    }
    TIMING_END(TMsec) ;
    compressed_time = TMsec;
    ratio = contig_time/compressed_time;
    if(ratio >= 1.0)
	printf(" %5.2g x faster\n", ratio);
    else
	printf(" %5.2g x slower\n", 1.0/ratio);

    if((stat = nc_close(ncid)))
	ERR1(stat);

    return 0;
}
