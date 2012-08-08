/****************************************************************
                        mpp_domain.h 
   This headers define interface to define domain layout, 
   define domain decomposition and global field to root pe, 
   some utilities routine to return domain decomposition.
   Currently it only used in tools and assume only one domain is created. 
   If more domains are needed, we may define a struct to hold domain informaiton.
   contact: Zhi.Liang@noaa.gov

****************************************************************/
#ifndef MPP_DOMAIN_H_
#define MPP_DOMAIN_H_

typedef struct{
  int start, end;           /* starting and ending index of compute domain */
  int size;                 /* compute domain size */
  int sizeg;                /* global domain size */
  int *beglist, *endlist; /* list of starting and ending index of compute domain */
} domain1D;

typedef struct {
  int isc, iec, jsc, jec;   /* compute domain decomposition */
  int isd, ied, jsd, jed;   /* data    domain decomposition */
  int nxc, nyc;             /* compute domain size */
  int nxd, nyd;             /* data    domain size */
  int nxg, nyg;             /* global  domain size */
  int *isclist, *ieclist;   /* list of i-index of compute domain */
  int *jsclist, *jeclist;   /* list of j-index of compute domain */
  int xhalo, yhalo;         /* halo size */
} domain2D;

void mpp_domain_init();
void mpp_domain_end();
void mpp_define_layout(int ni, int nj, int ndivs, int layout[]);
void mpp_define_domain1d(int npts, int ndvis, domain1D *domain );
void mpp_define_domain2d(int ni, int nj, int layout[], int xhalo, int yhalo, domain2D *domain );
void mpp_delete_domain1d(domain1D *domain);
void mpp_delete_domain2d(domain2D *domain);
void mpp_get_compute_domain2d(domain2D domain, int *is, int *ie, int *js, int *je);
void mpp_get_data_domain2d(domain2D domain, int *is, int *ie, int *js, int *je);
void mpp_get_global_domain2d(domain2D domain,  int *nx, int *ny);
void mpp_get_compute_domains2d(domain2D domain, int *is, int *ie, int *js, int *je);
void mpp_get_shift(domain2D domain, int sizex, int sizey, int *ishift, int *jshift);
void mpp_global_field_double(domain2D domain, int sizex, int sizey, const double* ldata, double* gdata);
void mpp_global_field_double_3D(domain2D domain, int sizex, int sizey, int sizez,
				const double* ldata, double* gdata);
void mpp_gather_field_int(int lsize, int *ldata, int *gdata);
void mpp_gather_field_double(int lsize, double *ldata, double *gdata);

#endif
