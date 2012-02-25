#ifndef AST_DEBUG_H
#define AST_DEBUG_H

#define ASTDEBUG

#ifdef ASTDEBUG
extern int ast_catch(int);
#endif

/* Provide an error catcher */
#ifdef ASTDEBUG
#define ACATCH(status) ((status)?ast_catch(status):(status))
#define ATHROW(status,go) {ast_catch(status);goto go;}
#define AERR(status,err,go) {status=err;ast_catch(status);goto go;}
#else
#define ACATCH(status,status,go) (status)
#define ATHROW(status,go) {goto go;}
#define AERR(status,err,go) {goto go;}
#endif

#endif /*AST_DEBUG_H*/
