#ifndef GLOBALS_H_
#define GLOBALS_H_
#include "constant.h"
#include "mpp_domain.h"

#define MAXATT   4096
#define MAXSTRING 10240
#define MAXENTRY  512
#define NVAR     2048
#define NFILE    32
#define MAXDIM   10
/* constants for grid type and grid location */
#define ZERO   0
#define NINETY 1
#define MINUS_NINETY -1
#define ONE_HUNDRED_EIGHTY 2
#define CENTER 3
#define CORNER 4
#define EAST   4
#define NORTH  5
#define WEST   6
#define SOUTH  7



/* constants for option */
#define CONSERVE_ORDER1 1
#define CONSERVE_ORDER2 2
#define BILINEAR        4
#define VECTOR          8
#define TARGET          16
#define SYMMETRY        32
#define AGRID           64
#define BGRID           128
#define READ            256
#define WRITE           512
#define CHECK_CONSERVE  1024

typedef struct {
  char   name[STRING];   /* variable name */
  int    vid;
  int    type;
  int    ndim;
  int    index[4];
  int    nz;
  int    kstart;
  int    kend;
  int    lstart;
  int    lend;
  int    has_zaxis;
  int    has_taxis;
  double missing;
  int    has_missing;
} Var_config;

typedef struct {
  char       *file;
  int        *fid;
  int         nvar;
  double     *data;   /* array to store one variable data */
  double     *grad_x; /* array to store one variable data gradient in x-direction */
  double     *grad_y; /* array to store one variable data gradient in y-direction */
  int        *grad_mask; /* array to store the mask for gradient */
  Var_config *var;
} Field_config;

typedef struct {
  char name[STRING];
  char bndname[STRING];
  int  dimid;
  int  vid;
  int  bndid;
  int  size;
  int  type;
  char cart; 
  int  bndtype;
  double *bnddata;
  double *data; 
} Axis_config;

typedef struct {
  int  nt;
  char name[STRING];
  int fid;
  int ndim;
  Axis_config  *axis;
  int has_tavg_info;
  int id_t1, id_t2, id_dt;
  double *t1, *t2, *dt;
} File_config;

typedef struct {
  size_t nxgrid;
  int *i_in;
  int *j_in;
  int *i_out;
  int *j_out;
  int *t_in;
  double *di_in;
  double *dj_in;
  double *area;
  double *weight;
  int    *index;
  char   remap_file[STRING];
  int    file_exist;
} Interp_config;


typedef struct {
  int halo;
  int nx;
  int ny;
  int nx_fine;
  int ny_fine;
  int isc;
  int iec;
  int jsc;
  int jec;
  int nxc;
  int nyc;
  double *lonc;
  double *latc;
  double *lont;
  double *latt;
  double *xt;
  double *yt;
  double *xc;
  double *yc;
  double *zt;
  double *dx;
  double *dy;
  double *area;
  double *lonc1D;
  double *latc1D;
  double *lont1D;
  double *latt1D;
  double *latt1D_fine;
  double *en_e;
  double *en_n;
  double *edge_w;
  double *edge_e;
  double *edge_s;
  double *edge_n;
  double *vlon_t;
  double *vlat_t;
  double *cosrot;
  double *sinrot;
  int    rotate;
  domain2D domain;
} Grid_config;

typedef struct{
  int nbound;
  int *tile2;
  int *is1, *ie1, *js1, *je1;
  int *is2, *ie2, *js2, *je2;
  int *rotate;
} Bound_config;

typedef struct{
  double *data;
  int nx;
  int ny;
} Data_holder;

#endif
