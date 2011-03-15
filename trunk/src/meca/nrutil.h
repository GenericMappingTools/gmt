/*	$Id: nrutil.h,v 1.8 2011-03-15 02:06:37 guru Exp $
 *    Public Domain NR stuff.
 */
#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#ifndef _WIN64
typedef unsigned long GMT_ULONG;		/* A signed 4 (or 8-byte for 64-bit) integer */
#else
typedef unsigned __int64 GMT_ULONG;		/* A signed 4 (or 8-byte for 64-bit) integer */
#endif

void nrerror(char error_text[]);
float *vector(GMT_LONG nl, GMT_LONG nh);
GMT_LONG *ivector(GMT_LONG nl, GMT_LONG nh);
unsigned char *cvector(GMT_LONG nl, GMT_LONG nh);
size_t *lvector(GMT_LONG nl, GMT_LONG nh);
double *dvector(GMT_LONG nl, GMT_LONG nh);
float **matrix(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
double **dmatrix(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
GMT_LONG **imatrix(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
float **submatrix(float **a, GMT_LONG oldrl, GMT_LONG oldrh, GMT_LONG oldcl,
	GMT_LONG newrl, GMT_LONG newcl);
float **convert_matrix(float *a, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
float ***f3tensor(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch, GMT_LONG ndl, GMT_LONG ndh);
void free_vector(float *v, GMT_LONG nl);
void free_ivector(GMT_LONG *v, GMT_LONG nl);
void free_cvector(unsigned char *v, GMT_LONG nl);
void free_lvector(size_t *v, GMT_LONG nl);
void free_dvector(double *v, GMT_LONG nl);
void free_matrix(float **m, GMT_LONG nrl, GMT_LONG ncl);
void free_dmatrix(double **m, GMT_LONG nrl, GMT_LONG ncl);
void free_imatrix(GMT_LONG **m, GMT_LONG nrl, GMT_LONG ncl);
void free_submatrix(float **b, GMT_LONG nrl);
void free_convert_matrix(float **b, GMT_LONG nrl);
void free_f3tensor(float ***t, GMT_LONG nrl, GMT_LONG ncl, GMT_LONG ndl);

#endif /* _NR_UTILS_H_ */
