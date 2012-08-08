void create_rectangular_topog(int nx, int ny, double basin_depth, double *depth);
void create_bowl_topog(int nx, int ny, const double *x, const double *y, double bottom_depth,
		       double min_depth, double bowl_east, double bowl_south, double bowl_west,
		       double bowl_north, double *depth);
void create_gaussian_topog(int nx, int ny, const double *x, const double *y, double bottom_depth,
			   double min_depth, double gauss_amp, double gauss_scale, double slope_x,
			   double slope_y, double *depth);
void create_idealized_topog( int nx, int ny, const double *x, const double *y,
			     double bottom_depth, double min_depth, double *depth);
void create_realistic_topog(int nx_dst, int ny_dst, const double *x_dst, const double *y_dst,
			    const char* topog_file, const char* topog_field, double scale_factor,
			    double fill_first_row, int filter_topog, int num_filter_pass,
			    int smooth_topo_allow_deepening, int round_shallow, int fill_shallow,
			    int deepen_shallow, double min_depth, double *depth );
