/* 
This is part of the libcf package from Unidata. Copyright UCAR
2006.

This is an internal header file for library-wide stuff that I don't
want in the libcf.h file.

Ed Hartnett, 10/1/06

$Id$
*/

#ifndef _LIBCF_INT_
#define _LIBCF_INT_

/* To log based on error code, and set retval. */
#define BAIL(e) do { \
ret = e; \
printf("file %s, line %d.\n%s", __FILE__, __LINE__, nc_strerror(e)); \
goto exit; \
} while (0) 

#endif /* _LIBCF_INT_ */
