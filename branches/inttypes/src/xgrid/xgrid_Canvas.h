/* $XConsortium: Template.h,v 1.4 89/07/21 01:41:49 kit Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987, 1988 */

#ifndef _Canvas_h
#define _Canvas_h

/****	General purpose canvas widget.

	This widget is a generic 'workspace' or 'canvas' for
	writing applications. Each instance has:
	- a pointer for user data
	- a callback list for when realised
	- a callback list for handling expose events
	- a callback list for if/when the canvas is resized
	- a geometry function that returns the preferred size
	- a font resource as well as the usual colors

	It does not have generic input callbacks because
	action lists and translations can be specified for
	individual widgets already.
	
	The realize callback is handy for creating GCs
	and other values that depend on a window being
	available.
	
	The expose callback is used for GraphicsExpose
	events as well.
	
	Based on Athena Template.
	Hugh Fisher, CRES, December 1991	****/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

 canvasData	     Pointer		Pointer		NULL
 exposeCallback	     Callback	        Pointer		NULL
 resizeCallback      Callback		Pointer		NULL
 realizeCallback     Callback		Pointer		NULL
 queryGeometry	     Pointer		Pointer		NULL
 font		     Font		XFontStruct *	XtDefaultFont
 
*/

/**** define any special resource names here that are not in <X11/StringDefs.h> ****/

#define XtNcanvasData		"canvasData"
#define XtNexposeCallback	"exposeCallback"
#define XtNresizeCallback	"resizeCallback"
#define XtNrealizeCallback	"realizeCallback"
#define XtNqueryGeometry	"queryGeometry"

#define XtCCanvasData		"CanvasData"
#define XtCExposeCallback	"ExposeCallback"
#define XtCResizeCallback	"ResizeCallback"
#define XtCRealizeCallback	"RealizeCallback"
#define XtCQueryGeometry	"QueryGeometry"

typedef struct _CanvasClassRec*	CanvasWidgetClass;
typedef struct _CanvasRec*	CanvasWidget;

/**** declare the class constant ****/

extern WidgetClass canvasWidgetClass;

/**** Convenience functions ****/

extern XFontStruct * XtCanvasFont (Widget w);
extern void * XtCanvasData (Widget w);

#endif /* _Canvas_h */
