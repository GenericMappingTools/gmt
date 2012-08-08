#ifndef BILINEAR_INTERP_H_
#define BILINEAR_INTERP_H_
void setup_bilinear_interp(int ntiles_in, const Grid_config *grid_in, int ntiles_out, const Grid_config *grid_out, 
                      Interp_config *interp, unsigned int opcode);
void do_scalar_bilinear_interp(const Interp_config *interp, int vid, int ntiles_in, const Grid_config *grid_in, const Grid_config *grid_out,
                          const Field_config *field_in, Field_config *field_out, int finer_step, int fill_missing);
void do_vector_bilinear_interp(Interp_config *interp, int vid, int ntiles_in, const Grid_config *grid_in, int ntiles_out, 
			  const Grid_config *grid_out, const Field_config *u_in,  const Field_config *v_in,
			  Field_config *u_out, Field_config *v_out, int finer_step, int fill_missing);
#endif
