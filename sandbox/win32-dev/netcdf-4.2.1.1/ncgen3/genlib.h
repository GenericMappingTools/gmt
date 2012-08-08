#ifndef NC_GENLIB_H
#define NC_GENLIB_H
/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen3/genlib.h,v 1.15 2009/12/29 18:42:35 dmh Exp $
 *********************************************************************/
#include <stdlib.h>
#include <limits.h>

extern const char *progname;	/* for error messages */
extern const char *cdlname;	/* for error messages */

#define FORT_MAX_LINES	20	/* max lines in FORTRAN statement */
#define	FORT_MAX_STMNT	66*FORT_MAX_LINES /* max chars in FORTRAN statement */
#define C_MAX_STMNT	FORT_MAX_STMNT /* until we fix to break up C lines */

#ifdef __cplusplus
extern "C" {
#endif

extern void cline ( const char* stmnt );
extern void fline ( const char* stmnt );
extern const char* nctype ( nc_type  type );
extern const char* ncctype ( nc_type  type );
extern const char* ncstype ( nc_type  type );
extern const char* ncatype ( nc_type  type );
extern const char* nfstype ( nc_type  type );
extern const char* nfftype ( nc_type  type );
extern char* fstring ( nc_type  type, void* valp, int num );
extern char* cstrstr ( const char* valp, size_t len );
extern char* fstrstr ( const char* str, size_t ilen );
extern size_t nctypesize( nc_type type );

extern void	derror ( const char *fmt, ... )
#ifdef _GNUC_
       __attribute__ ((format (printf, 1, 2)))
#endif
;
extern void	check_err ( int status );
extern void	*emalloc ( size_t size );
extern void	*ecalloc ( size_t size );
extern void	*erealloc ( void *ptr, size_t size );
extern void	expe2d ( char *ptr );
extern void	grow_iarray ( int narray, int **array );
extern void	grow_varray ( int narray, struct vars **array );
extern void	grow_darray ( int narray, struct dims **array );
extern void	grow_aarray ( int narray, struct atts **array );
extern char*	decodify (const char *name);
extern void	deescapify (char *name);

extern int put_variable ( void* rec_start );

/* initializes netcdf counts (e.g. nvars), defined in init.c */
extern void init_netcdf ( void );

/* generates all define mode stuff, defined in genlib.c */
extern void define_netcdf(const char *netcdfname);

/* generates variable puts, defined in load.c */
extern void load_netcdf ( void* rec_start );

/* generates close, defined in close.c */
extern void close_netcdf ( void );

/* defined in escapes.c */
extern void expand_escapes ( char* termstring, char* yytext, int yyleng );

/* to get fill value for various types, defined in getfill.c */
extern void nc_getfill ( nc_type  type, union generic* gval );

/* to put fill value for various types, defined in getfill.c */
extern void nc_putfill ( nc_type  type, void* val, union generic* gval );

/* fills a generic array with a value, defined in getfill.c */
extern void nc_fill ( nc_type  type, size_t num, void* datp,
	union generic fill_val );

/* reset symbol table to empty, defined in ncgen.y */
extern void clearout(void);

/* In case we are missing strlcat */
#ifndef HAVE_STRLCAT
extern size_t strlcat(char *dst, const char *src, size_t siz);
#endif

#ifdef __cplusplus
}
#endif
#endif /*!NC_GENLIB_H*/
