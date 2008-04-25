/*	$Id: nrutil.h,v 1.6 2008-04-25 04:00:41 guru Exp $
 *    Public Domain NR stuff.
 */
#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

void nrerror(char error_text[]);
float *vector(GMT_LONG nl, GMT_LONG nh);
int *ivector(GMT_LONG nl, GMT_LONG nh);
unsigned char *cvector(GMT_LONG nl, GMT_LONG nh);
size_t *lvector(GMT_LONG nl, GMT_LONG nh);
double *dvector(GMT_LONG nl, GMT_LONG nh);
float **matrix(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
double **dmatrix(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
int **imatrix(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
float **submatrix(float **a, GMT_LONG oldrl, GMT_LONG oldrh, GMT_LONG oldcl, GMT_LONG oldch,
	GMT_LONG newrl, GMT_LONG newcl);
float **convert_matrix(float *a, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
float ***f3tensor(GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch, GMT_LONG ndl, GMT_LONG ndh);
void free_vector(float *v, GMT_LONG nl, GMT_LONG nh);
void free_ivector(int *v, GMT_LONG nl, GMT_LONG nh);
void free_cvector(unsigned char *v, GMT_LONG nl, GMT_LONG nh);
void free_lvector(size_t *v, GMT_LONG nl, GMT_LONG nh);
void free_dvector(double *v, GMT_LONG nl, GMT_LONG nh);
void free_matrix(float **m, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
void free_dmatrix(double **m, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
void free_imatrix(int **m, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
void free_submatrix(float **b, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
void free_convert_matrix(float **b, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch);
void free_f3tensor(float ***t, GMT_LONG nrl, GMT_LONG nrh, GMT_LONG ncl, GMT_LONG nch,
	GMT_LONG ndl, GMT_LONG ndh);

#endif /* _NR_UTILS_H_ */
