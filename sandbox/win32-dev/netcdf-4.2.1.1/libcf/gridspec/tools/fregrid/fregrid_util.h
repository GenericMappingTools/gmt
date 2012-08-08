#ifndef FREGRID_UTIL_H_
#define FREGRID_UTIL_H_
#include "globals.h"
void set_mosaic_data_file(int ntiles, const char *mosaic_file, const char *dir, File_config *file,
			  const char *filename);
void set_field_struct(int ntiles, Field_config *field, int nvar, char * varname, File_config *file);
void get_input_grid(int ntiles, Grid_config *grid, Bound_config *bound, const char *mosaic_file, unsigned int opcode);
void get_output_grid_from_mosaic(int ntiles, Grid_config *grid, const char *mosaic_file, unsigned int opcode);
void get_output_grid_by_size(int ntiles, Grid_config *grid, double lonbegin, double lonend, double latbegin, double latend, 
                             int nlon, int nlat, int finer_steps, int center_y, unsigned int opcode);
void get_input_metadata(int ntiles, int nfiles, File_config *file1, File_config *file2,
		        Field_config *scalar, Field_config *u_comp, Field_config *v_comp,
			const Grid_config *grid, int kbegin, int kend, int lbegin, int lend);
void set_output_metadata (int ntiles_in, int nfiles, const File_config *file1_in, const File_config *file2_in,
			  const Field_config *scalar_in, const Field_config *u_in, const Field_config *v_in,
			  int ntiles_out, File_config *file1_out, File_config *file2_out, Field_config *scalar_out,
			  Field_config *u_out, Field_config *v_out, const Grid_config *grid_out, const char *history, const char *tagname);
void get_field_missing( int ntiles, Field_config *field);
void set_remap_file( int ntiles, const char *mosaic_file, const char *remap_file, Interp_config *interp, unsigned int *opcode, int save_weight_only);
void write_output_time(int ntiles, File_config *output, int level);
void get_input_data(int ntiles, Field_config *field, Grid_config *grid, Bound_config *bound, 
                    int l, int level, unsigned int opcode);
void get_test_input_data(char *test_case, double test_param, int ntiles, Field_config *field,
			 Grid_config *grid, Bound_config *bound, unsigned int opcode);
void allocate_field_data(int ntiles, Field_config *field, Grid_config *grid, int l);
void write_field_data(int ntiles, Field_config *output, Grid_config *grid, int l, int level);

#endif
