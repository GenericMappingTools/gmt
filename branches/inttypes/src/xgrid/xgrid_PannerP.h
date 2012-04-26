#ifndef _PannerP_h
#define _PannerP_h

#include "xgrid_Panner.h"
/* include superclass private header file */
#include <X11/CompositeP.h>
#include <X11/Xmu/Converters.h>

/*	Data for single scrollbar	*/

typedef struct {
	Widget		scrollbar;
	XtOrientation	orientation;
	int		range;
	int		shown; /* Equal to canvas width/height */
	int		position;
	XtCallbackList	callback;
	Dimension	scroll_step;
	} PanScrollRec;

/*	Class record	*/

typedef struct {
	int empty;
	} PannerClassPart;

typedef struct _PannerClassRecCustom {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	PannerClassPart		panner_class;
	} PannerClassRecCustom;

extern PannerClassRecCustom pannerClassRecCustom;

/*	Instance record	*/

typedef struct {
	PanScrollRec	horizontal;
	PanScrollRec	vertical;
	Widget		canvas;
	} PannerPart;

typedef struct _PannerRec {
	CorePart	core;
	CompositePart	composite;
	PannerPart	panner;
	} PannerRec;

#endif /* _PannerP_h */
