#ifndef CONSERVE_INTERP_H_
#define CONSERVE_INTERP_H_
#include "globals.h"

void setup_conserve_interp(int ntiles_in, const Grid_config *grid_in, int ntiles_out,
			   Grid_config *grid_out, Interp_config *interp, unsigned int opcode);
void do_scalar_conserve_interp(Interp_config *interp, int varid, int ntiles_in, const Grid_config *grid_in,
			       int ntiles_out, const Grid_config *grid_out, const Field_config *field_in,
			       Field_config *field_out, unsigned int opcode);
void do_vector_conserve_interp(Interp_config *interp, int varid, int ntiles_in, const Grid_config *grid_in, int ntiles_out, 
                               const Grid_config *grid_out, const Field_config *u_in,  const Field_config *v_in,
                               Field_config *u_out, Field_config *v_out, unsigned int opcode);  
#endif
