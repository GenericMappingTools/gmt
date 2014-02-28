/*	$Id$
 *    Public Domain NR stuff.
 */
#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

void nrerror(char error_text[]);
float *vector(int nl, int nh);
int *ivector(int nl, int nh);
unsigned char *cvector(int nl, int nh);
int64_t *lvector(int nl, int nh);
double *dvector(int nl, int nh);
float **matrix(int nrl, int nrh, int ncl, int nch);
double **dmatrix(int nrl, int nrh, int ncl, int nch);
int **imatrix(int nrl, int nrh, int ncl, int nch);
float **submatrix(float **a, int oldrl, int oldrh, int oldcl,
	int newrl, int newcl);
float **convert_matrix(float *a, int nrl, int nrh, int ncl, int nch);
float ***f3tensor(int nrl, int nrh, int ncl, int nch, int ndl, int ndh);
void free_vector(float *v, int nl);
void free_ivector(int *v, int nl);
void free_cvector(unsigned char *v, int nl);
void free_lvector(size_t *v, int nl);
void free_dvector(double *v, int nl);
void free_matrix(float **m, int nrl, int ncl);
void free_dmatrix(double **m, int nrl, int ncl);
void free_imatrix(int **m, int nrl, int ncl);
void free_submatrix(float **b, int nrl);
void free_convert_matrix(float **b, int nrl);
void free_f3tensor(float ***t, int nrl, int ncl, int ndl);

#endif /* _NR_UTILS_H_ */
