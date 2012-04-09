
#ifndef _VIEW_
#define _VIEW_

#include "xgrid_GMTgrid.h"

typedef struct {
	char	  fileName[1024];
	GMTGrid * grid;
	Widget	  canvas;
	} ViewData;

extern Widget	createView (struct GMT_CTRL *, String fileName, Widget parent, Arg args[], int nargs);

extern void	createFileCommands (Widget parent);

/* Anything which changes the grid must set this flag */

extern Boolean	gridHasChanged;

#endif


