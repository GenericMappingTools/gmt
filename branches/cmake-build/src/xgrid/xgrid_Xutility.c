
/*  Hugh Fisher Oct 90.
    Taken from DA Young, X window system
    programming and applications with Xt;
    Asente & Swick, X Window System
    Toolkit; and my own imagination.
    
    Common functions */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/Shell.h>

#include "xgrid_Xutility.h"

Dimension XtWidth (w)
	Widget w;
{
  return w->core.width;
}

Dimension XtHeight (w)
	Widget w;
{
  return w->core.height;
}

Widget XtShell (w)
	Widget w;
{
  while (w != NULL && !XtIsSubclass(w, shellWidgetClass))
    w = XtParent(w);
  return w;
}

/****	Wait until all windows redrawn	****/

void WaitNoExposes (w)
	Widget w;
{
  XEvent	event;
  Display *	display;
  XtAppContext	app;
  Window	win;

  win	  = XtWindow(w);
  display = XtDisplay(w);
  app     = XtWidgetToApplicationContext(w);
  XFlush(display);
  while (XCheckTypedWindowEvent(display, win, ExposureMask, &event)) {
    XtAppNextEvent(app, &event);
    XtDispatchEvent(&event);
  }
}

/****	Highlight things	****/

void InvertWidget (w)
  Widget w;
{
  Pixel foreground, background;

  XtVaGetValues(w,
	XtNforeground, &foreground,
	XtNbackground, &background,
	NULL);
  XtVaSetValues(w,
	XtNforeground, background,
	XtNbackground, foreground,
	NULL);
  XClearArea(XtDisplay(w), XtWindow(w),
  		0, 0, 0, 0, True);
}

