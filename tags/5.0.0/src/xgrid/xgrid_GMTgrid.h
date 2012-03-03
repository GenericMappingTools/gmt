
#ifndef _GMTGRID_
#define _GMTGRID_

/* This is based on the generic grid structure */
#include "xgrid_grid.h"

typedef struct {
	/* Required fields for all grids */
	Grid		  methods;
	/* GMT specific fields */
	struct GMT_CTRL *GMT;
	struct GRD_HEADER header;
	GridValue *	  value;
	} GMTGrid;

extern GMTGrid * CreateGMTGrid (struct GMT_CTRL *GMT);	/* No arguments */
		
#endif
