
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

typedef void      (* GridProc) (); /* Grid * grid, ... */
typedef GridValue (* GridFunc) (); /* Grid * grid, ... */

/****	Grid data structure	****/

typedef struct {
	int	 width;
	int	 height;
	int	 coordType; /* Nodes or pixels */
	/****	Methods	****/
	/* dispose (Grid * grid ) */
	GridProc dispose;
	/* readFromFile (Grid * grid, String fileName, int * status) */
	GridProc readFromFile;
	/* writeToFile (Grid * grid, String fileName, int * status) */
	GridProc writeToFile;
	/* getIndexes (Grid * grid, GridPoint * coords, XPoint * indexes) */
	GridProc getIndexes;
	/* getCoords (Grid * grid, XPoint * indexes, GridPoint * coords) */
	GridProc getCoords;
	/* set (Grid * grid, int xIndex, int yIndex, GridValue value) */
	GridProc set;
	/* get (Grid * grid, int xIndex, int yIndex) */
	GridFunc get;
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


