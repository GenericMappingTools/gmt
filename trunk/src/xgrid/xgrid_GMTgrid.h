
#ifndef _GMTGRID_
#define _GMTGRID_

#ifdef sgi
#define GMT_is_fnan(x) ((x) != (x))
#endif

#if !defined(GMT_is_fnan)
#define GMT_is_fnan(x) isnan((float)x)
#endif

/* This is based on the generic grid structure */
#include "xgrid_grid.h"

typedef struct {
	/* Required fields for all grids */
	Grid		  methods;
	/* GMT specific fields */
	struct GRD_HEADER header;
	GridValue *	  value;
	} GMTGrid;

extern GMTGrid * CreateGMTGrid ();	/* No arguments */
		
#endif


