
#ifndef _GRID_
#define _GRID_

/****	This is a generic grid data structure for
	use by X Windows programs. Anything you
	can force onto this can be displayed and
	edited by xgridedit, etc.
	
	The data structure is just a record with
	a number of function pointers as "methods"
	See GMTgrid.c for an example of how to use
	it.
					****/

typedef float GridValue;	/* Things you store in grids */

typedef double GridCoord;	/* How you reference them */

#define GridCoordNodes	0	/* Values ON grid coordinates */
#define GridCoordPixels	1	/* Values BETWEEN grid coordinates */

typedef struct {
	GridCoord x;
	GridCoord y;
	} GridPoint;

/****	Grid methods ****/

/****	Grid data structure	****/

typedef struct _Grid_s {
	int	 width;
	int	 height;
	int	 coordType; /* Nodes or pixels */
	/****	Methods	****/
	void (*dispose) (struct _Grid_s * grid );
	void (*readFromFile) (struct _Grid_s *grid, String fileName, int * status);
	void (*writeToFile) (struct _Grid_s * grid, String fileName, int * status);
	void (*getIndexes) (struct _Grid_s * grid, GridPoint * coords, XPoint * indexes);
	void (*getCoords) (struct _Grid_s * grid, XPoint * indexes, GridPoint * coords);
	void (*set) (struct _Grid_s *grid, int xIndex, int yIndex, GridValue value);
	GridValue (*get) (struct _Grid_s * grid, int xIndex, int yIndex);
	} Grid;

/****	Convenience macros to make code cleaner	****/

#define GridWidth(grid)	\
		(((Grid *)grid)->width)

#define GridHeight(grid)\
		(((Grid *)grid)->height)

#define DisposeGrid(grid)	\
		((*(((Grid *)grid)->dispose))(grid))

#define ReadGridFromFile(grid,fileName,status)	\
		((*(((Grid *)grid)->readFromFile))(grid,fileName,status))

#define WriteGridToFile(grid,fileName,status)	\
		((*(((Grid *)grid)->writeToFile))(grid,fileName,status))

#define GetGridIndexes(grid,coords,indexes) \
		((*(((Grid *)grid)->getIndexes))(grid,coords,indexes))

#define GetGridCoords(grid,indexes,coords) \
		((*(((Grid *)grid)->getCoords))(grid,indexes,coords))


#define SetGrid(grid,xIndex,yIndex,value) \
		((*(((Grid *)grid)->set))(grid,xIndex,yIndex,value))

#define GetGrid(grid,xIndex,yIndex) \
		((*(((Grid *)grid)->get))(grid,xIndex,yIndex))
		
#endif


