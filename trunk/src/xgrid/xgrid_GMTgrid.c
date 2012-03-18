
/*
	Hugh Fisher
	Dec 91
	
	Operations for GMT grids. The I/O code
	has a remarkable similarity to that
	in gmt_grdio.c, but uses different
	parameters and error handling.
					*/

#include "gmt.h"
#include <stdio.h>
#include <errno.h>
#include <X11/Intrinsic.h>

#include "xgrid_utility.h"
#include "xgrid_GMTgrid.h"

static void clear (grid)
	GMTGrid * grid;
{
  grid->methods.width     = 0;
  grid->methods.height    = 0;
  grid->methods.coordType = GridCoordNodes;
  
  grid->value = NULL;
  
  grid->header.wesn[XLO] = 0.0;
  grid->header.wesn[XHI] = 0.0;
  grid->header.wesn[YLO] = 0.0;
  grid->header.wesn[YHI] = 0.0;
  grid->header.z_min = 0.0;
  grid->header.z_max = 0.0;
  grid->header.inc[GMT_X] = 0.0;
  grid->header.inc[GMT_Y] = 0.0;
  grid->header.z_scale_factor	= 1.0;
  grid->header.z_add_offset	= 0.0;
  grid->header.nx = 0;
  grid->header.ny = 0;
  grid->header.registration = GridCoordNodes;

  strcpy(grid->header.x_units, "x");
  strcpy(grid->header.y_units, "y");
  strcpy(grid->header.z_units, "z");
  strcpy(grid->header.title, "Untitled");
  memset(grid->header.command, 0, sizeof(grid->header.command));
  memset(grid->header.remark, 0, sizeof(grid->header.remark));
}

static void readFromFile (GMTGrid *grid, String fileName, int *status)
{
    if ((*status = GMT_read_grd_info (grid->GMT, fileName, &grid->header)))
      return;

    grid->value = calloc (grid->header.nm, sizeof (float));
    GMT_memset (grid->GMT->current.io.pad, 4, GMT_LONG);
    *status = GMT_read_grd (grid->GMT, fileName, &grid->header, grid->value, NULL,
	grid->GMT->current.io.pad, FALSE);

    /* Update generic grid values */    
    grid->methods.width  = grid->header.nx;
    grid->methods.height = grid->header.ny;
    grid->methods.coordType = grid->header.registration; 

  return;
}

static void writeToFile (grid, fileName, status)
	GMTGrid * grid;
	String    fileName;
	int *	  status;
{
	double wesn[4];
	GMT_memset (wesn, 4, double);
	GMT_memset (grid->GMT->current.io.pad, 4, GMT_LONG);
	*status = GMT_write_grd (grid->GMT, fileName, &grid->header, grid->value, wesn, grid->GMT->current.io.pad, FALSE);
}

static void dispose (grid)
	GMTGrid * grid;
{
  free(grid->value);
  grid->value = NULL;
}

#if 0
static void dumpGrid (grid)
	GMTGrid * grid;
{
  int outer, inner;
  GridPoint  gPt;
  XPoint     xPt;
  
  printf("Grid: %s\n", grid->header.title);
  printf("Width, height = %d x %d\n", grid->header.nx, grid->header.ny);
  printf("X Coordinates: %f to %f by %f\n",
  	grid->header.wesn[XLO], grid->header.wesn[XHI], grid->header.inc[GMT_X]);
  printf("Y Coordinates: %f to %f by %f\n",
  	grid->header.wesn[YLO], grid->header.wesn[YHI], grid->header.inc[GMT_Y]);
  if (grid->header.registration == GridCoordNodes)
    printf("Data points centred on coordinates\n");
  else if (grid->header.registration == GridCoordPixels)
    printf("Data points between coordinates\n");  
  printf("Z range = %f to %f, scale by %f and add %f\n",
  	grid->header.z_min, grid->header.z_max,
	grid->header.z_scale_factor, grid->header.z_add_offset);
  /* Write out the values in same format as grd2xyz uses */	
  for (outer = 0; outer < grid->header.ny; outer ++) {
    for (inner = 0; inner < grid->header.nx; inner ++) {
      xPt.x = inner; xPt.y = outer;
      GetGridCoords(grid, &xPt, &gPt);
      printf("%g\t%g\t%g\n", gPt.x, gPt.y, GetGrid(grid, inner, outer));
    }
    printf("\n");
  }
}
#endif

static void getIndexes (grid, coord, index)
	GMTGrid *   grid;
	GridPoint * coord;
	XPoint *    index;
{
  int column, row;
  
  column = (int)((coord->x - grid->header.wesn[XLO]) / grid->header.inc[GMT_X]);
  row    = (int)((coord->y - grid->header.wesn[YLO]) / grid->header.inc[GMT_Y]);
  index->x = column;
  /* Allow for origin being at the bottom rather than the top */
  index->y = grid->header.ny - row;
}

static void getCoords (grid, index, coord)
	GMTGrid *   grid;
	XPoint *    index;
	GridPoint * coord;
{
  coord->x = grid->header.wesn[XLO] + index->x * grid->header.inc[GMT_X];
  coord->y = grid->header.wesn[YHI] - index->y * grid->header.inc[GMT_Y];
}

static void set (grid, xIndex, yIndex, value)
	GMTGrid * grid;
	int	  xIndex;
	int	  yIndex;
	GridValue value;
{
	int k = yIndex * grid->header.nx + xIndex;
	fprintf (stderr, "Before: %g\n", grid->value[k]);
	fprintf (stderr, "Using: %g\n", value);
  grid->value[k] = value;
	fprintf (stderr, "After: %g\n", grid->value[k]);
}

static GridValue get (grid, xIndex, yIndex)
	GMTGrid * grid;
	int	  xIndex;
	int	  yIndex;
{

/* Original Version had "grid->header.ny": should be "grid->header.nx */
/* Fixed 8/4/93 by J. Lillibrige @ NOAA/NOS to eliminate core dumps!  */

  return grid->value[yIndex * grid->header.nx + xIndex];
}

GMTGrid * CreateGMTGrid (struct GMT_CTRL *GMT)
{
  GMTGrid * result;
  
  result = (GMTGrid *)XtMalloc(sizeof(GMTGrid));
  clear(result);
  /* Assign methods */
  result->methods.dispose	= dispose;
  result->methods.readFromFile	= readFromFile;
  result->methods.writeToFile	= writeToFile;
  result->methods.getIndexes	= getIndexes;
  result->methods.getCoords	= getCoords;
  result->methods.set		= set;
  result->methods.get		= get;
  result->GMT		= GMT;	/* Pass CTRL struct pointer */
  
  return result;
}
