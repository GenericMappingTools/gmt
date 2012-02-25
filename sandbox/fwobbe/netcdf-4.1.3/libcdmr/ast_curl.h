#ifndef AST_HTTP_H
#define AST_HTTP_H 1

extern ast_err ast_curlopen(CURL** curlp);
extern ast_err ast_curlclose(CURL*);

extern ast_err ast_fetchurl(CURL*, char*, bytes_t*, long*);

extern long ast_fetchhttpcode(CURL* curl);

#endif /*AST_HTTP_H*/
