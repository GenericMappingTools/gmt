/***********************************************************************
                      tool_util.h
    This header file provide some utilities routine that will be used in many tools.
    
    contact: Zhi.Liang@noaa.gov
***********************************************************************/
#ifndef TOOL_UTIL_H_
#define TOOL_UTIL_H_
void get_file_path(const char *file, char *dir);
int get_int_entry(char *line, int *value); 
int get_double_entry(char *line, double *value);
double spherical_dist(double x1, double y1, double x2, double y2);
double spherical_area(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4 ); 
double bipolar_dist(double x1, double y1, double x2, double y2, double bpeq, double bpsp, double bpnp, double rp );
double bipolar_area(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4 );
void tp_trans(double *lon, double *lat, double lon_ref, double lon_start, 
              double lam0, double bpeq, double bpsp, double bpnp, double rp );
double* compute_grid_bound(int nb, const double *bnds, const int *npts, int *grid_size, const char *center);

typedef struct {
  int nx;
  int ny;
  double *xt;
  double *yt;
  double *xb;
  double *yb;
  double *xb_r;
  double *yb_r;
  double *area;
  double *landfrac;
  double *subA;
  double *cellarea;
  double *celllength;
  int    *tocell;
  int    *travel;
  int    *basin;
  int    *dir;
  int    *last_point;
  double subA_missing;
  double cellarea_missing;
  double celllength_missing;
  int    tocell_missing;
  int    travel_missing;
  int    basin_missing;
  char   filename[128];
} river_type;

//inline void swap(void *x, void *y, size_t l);
void qsort_index(double array[], int start, int end, int rank[]);
void get_source_data(const char *src_file, river_type *river_data);
void get_mosaic_grid(const char *coupler_mosaic, const char *land_mosaic,
int ntiles, river_type *river_data, unsigned int *opcode);
void init_river_data(int ntiles, river_type *river_out, const river_type * const river_in);
void calc_max_subA(const river_type *river_in, river_type *river_out,
int ntiles, unsigned int opcode);
void update_halo_double(int ntiles, double **data, int nx, int ny, unsigned int opcode);
void update_halo_int(int ntiles, int **data, int nx, int ny, unsigned int opcode);
int adjust_lon(double x[], double tlon);
void calc_tocell(int ntiles, river_type *river_out, unsigned int opcode );
void calc_river_data(int ntiles, river_type* river_data, unsigned int opcode  );
void sort_basin(int ntiles, river_type* river_data);

void check_river_data( );
void write_river_data(const char *river_src_file, const char *output_file,
river_type* river_out, const char *history, int ntiles);
double distance(double lon1, double lat1, double lon2, double lat2);


#endif
