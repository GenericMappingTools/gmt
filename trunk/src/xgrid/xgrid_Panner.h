/* $XConsortium: Template.h,v 1.4 89/07/21 01:41:49 kit Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987, 1988 */

#ifndef _Panner_h
#define _Panner_h

/****	General purpose panning widget for scrolling windows.

	This widget is used for a window which pans around a
	larger view. The Athena widget set already includes
	such a ViewPort widget, but it operates differently
	and cannot handle views larger than 65,000 pixels.
	Some of the datasets here are bigger than that!
	
	The panner widget has three children, the canvas
	for display and two scrollbars. It automatically
	manages the position and size of the scrollbars,
	although client programs can change the values if
	they want by using convenience routines.
	
	The coordinate system is in pixels, from zero
	to 'range' max, 'shown' visible at a time.
	Current position is returned by two convenience
	functions. IMPORTANT: needs 32 bit int.
	
	Based on Athena Template.
	Hugh Fisher, CRES, February 1992	****/

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
 x		     Cardinal		Cardinal	0
 y		     Cardinal		Cardinal	0

 canvas		     Widget		Widget		NULL
 xRange		     int		int		0
 xPosition	     int		int		0
 xCallback	     Callback		Pointer		NULL
 xScrollStep	     Dimension		Dimension	0
 
 yRange		     int		int		0
 yPosition	     int		int		0
 yCallback	     Callback		Pointer		NULL
 yScrollStep	     Dimension		Dimension	0
*/

/* define any special resource names here that are not in <X11/StringDefs.h> */

#define XtNcanvas		"canvas"
#define XtNxRange		"xRange"
#define XtNxPosition		"xPosition"
#define XtNxCallback		"xCallback"
#define XtNxScrollStep		"xScrollStep"
#define XtNyRange		"yRange"
#define XtNyPosition		"yPosition"
#define XtNyCallback		"yCallback"
#define XtNyScrollStep		"yScrollStep"

#define XtCCanvas		"Canvas"
#define XtCXRange		"XRange"
#define XtCXPosition		"XPosition"
#define XtCXCallback		"XCallback"
#define XtCXScrollStep		"XScrollStep"
#define XtCYRange		"YRange"
#define XtCYPosition		"YPosition"
#define XtCYCallback		"YCallback"
#define XtCYScrollStep		"YScrollStep"

typedef struct _PannerClassRec*	PannerWidgetClass;
typedef struct _PannerRec*	PannerWidget;

/* Declare the class constant */

extern WidgetClass pannerWidgetClass;

/* Convenience functions */

extern void	XtSetPannerCanvas (Widget, Widget canvas); /*  */

extern int	XtPannerXPosition (Widget); /*  */
extern int	XtPannerYPosition (Widget); /* Widget w */

extern void     XtSetPannerXPosition (Widget, int); /* Widget w, int x */
extern void	XtSetPannerYPosition (Widget, int); /* Widget w, int y */
extern void	XtSetPannerXRange (Widget, int);    /* Widget w, int x */
extern void	XtSetPannerYRange (Widget, int);    /* Widget w, int y */

#endif /* _Panner_h */
