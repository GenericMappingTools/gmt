#ifndef _CanvasP_h
#define _CanvasP_h

#include "xgrid_Canvas.h"
/* include superclass private header file */
#include <X11/CoreP.h>

/*	Class record	*/

typedef struct {
	int empty;
	} CanvasClassPart;

typedef struct _CanvasClassRec {
	CoreClassPart	core_class;
	CanvasClassPart	canvas_class;
	} CanvasClassRec;

extern CanvasClassRec canvasClassRec;

/*	Instance record	*/

typedef struct {
	void *		data;
	XFontStruct *	font;
	XtCallbackList	expose_callback;
	XtCallbackList	resize_callback;
	XtCallbackList	realize_callback;
	XtGeometryHandler query_geometry;
	} CanvasPart;

typedef struct _CanvasRec {
	CorePart	core;
	CanvasPart	canvas;
	} CanvasRec;

#endif /* _CanvasP_h */
