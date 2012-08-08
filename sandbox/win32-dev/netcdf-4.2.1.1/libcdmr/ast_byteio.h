/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef AST_BYTEIO_H
#define AST_BYTEIO_H

extern ast_err ast_byteio_new(ast_iomode, void* buf, size_t len, ast_runtime**);

/* Non-destructive read of the current position */
extern ast_err ast_byteio_count(ast_runtime*, size_t*);
/* Non-destructive read of the current byte buffer */
extern ast_err ast_byteio_content(ast_runtime*, bytes_t*);

/* Reset */ 
extern ast_err ast_byteio_reset(ast_runtime*);

extern unsigned char* xbytes(ast_runtime*);
extern size_t xpos(ast_runtime* rt);

#endif /*AST_BYTEIO_H*/
