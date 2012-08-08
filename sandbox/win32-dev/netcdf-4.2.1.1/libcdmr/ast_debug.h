#ifndef AST_DEBUG_H
#define AST_DEBUG_H

#define ASTDEBUG

#ifdef ASTDEBUG
extern int ast_catch(int);
extern void ast_breakpoint(int code);
#endif

/* Provide an error catcher */
#ifdef ASTDEBUG
#define ACATCH(status) (ast_catch(status))
#define ATHROW(status,go) {ast_catch(status);goto go;}
#define AERR(status,err,go) {status=err;ast_catch(status);goto go;}
#else
#define ACATCH(status) (status);
#define ATHROWCHK(status) ACATCH(status)
#define ATHROW(status,go) {goto go;}
#define AERR(status,err,go) {goto go;}
#endif

extern unsigned char* xbytes(ast_runtime*);
extern size_t xpos(ast_runtime* rt);


#endif /*AST_DEBUG_H*/
