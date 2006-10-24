
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
  
  grid->header.x_min = 0.0;
  grid->header.x_max = 0.0;
  grid->header.y_min = 0.0;
  grid->header.y_max = 0.0;
  grid->header.z_min = 0.0;
  grid->header.z_max = 0.0;
  grid->header.x_inc = 0.0;
  grid->header.y_inc = 0.0;
  grid->header.z_scale_factor	= 1.0;
  grid->header.z_add_offset	= 0.0;
  grid->header.nx = 0;
  grid->header.ny = 0;
  grid->header.node_offset = GridCoordNodes;

  strcpy(grid->header.x_units, "x");
  strcpy(grid->header.y_units, "y");
  strcpy(grid->header.z_units, "z");
  strcpy(grid->header.title, "Untitled");
  memset(grid->header.command, 0, sizeof(grid->header.command));
  memset(grid->header.remark, 0, sizeof(grid->header.remark));
}

static void readFromFile (GMTGrid *grid, String fileName, int *status)
{
    if ((*status = GMT_read_grd_info (fileName, &grid->header)))
      return;

    grid->value = calloc (grid->header.nx * grid->header.ny, sizeof (float));

    *status = GMT_read_grd (fileName, &grid->header, grid->value, 0.0, 0.0,
			 0.0, 0.0, GMT_pad, FALSE);

    /* Update generic grid values */    
    grid->methods.width  = grid->header.nx;
    grid->methods.height = grid->header.ny;
    grid->methods.coordType = grid->header.node_offset; 

  return;
}

static void writeToFile (grid, fileName, status)
	GMTGrid * grid;
	String    fileName;
	int *	  status;
{
  *status = GMT_write_grd (fileName, &grid->header, grid->value, 0.0, 0.0, 0.0,
			   0.0, GMT_pad, FALSE);
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
  	grid->header.x_min, grid->header.x_max, grid->header.x_inc);
  printf("Y Coordinates: %f to %f by %f\n",
  	grid->header.y_min, grid->header.y_max, grid->header.y_inc);
  if (grid->header.node_offset == GridCoordNodes)
    printf("Data points centred on coordinates\n");
  else if (grid->header.node_offset == GridCoordPixels)
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
  
  column = (int)((coord->x - grid->header.x_min) / grid->header.x_inc);
  row    = (int)((coord->y - grid->header.y_min) / grid->header.y_inc);
  index->x = column;
  /* Allow for origin being at the bottom rather than the top */
  index->y = grid->header.ny - row;
}

static void getCoords (grid, index, coord)
	GMTGrid *   grid;
	XPoint *    index;
	GridPoint * coord;
{
  coord->x = grid->header.x_min + index->x * grid->header.x_inc;
  coord->y = grid->header.y_max - index->y * grid->header.y_inc;
}

static void set (grid, xIndex, yIndex, value)
	GMTGrid * grid;
	int	  xIndex;
	int	  yIndex;
	GridValue value;
{
  grid->value[yIndex * grid->header.nx + xIndex] = value;
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

GMTGrid * CreateGMTGrid ()
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
  
  return result;
}
