
/*
	Hugh Fisher
	Dec 1991
	
	Control bar: menus, coordinate display, etc.
	Always has a Quit button, to which callbacks
	can be added.
					*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xmu/Converters.h>

#include "xgrid_utility.h"
#include "xgrid_Xutility.h"

#include "xgrid_controls.h"

static Widget quitButton;

Widget createControlBar (name, parent, args, nargs)
	String name;
	Widget parent;
	Arg    args[];
	int    nargs;
{
  Widget result;
  Arg    list[32];
  int    loop, nlist;
  
  /* Copy arguments from creator */
  for (loop = 0; loop < nargs; loop ++)
    list[loop] = args[loop];
  nlist = nargs;
  
  XtSetArg(list[nlist], XtNheight, 32); nlist ++;
  XtSetArg(list[nlist], XtNorientation, (XtArgVal)XtorientHorizontal); nlist ++;
  
  result = XtCreateManagedWidget(name, boxWidgetClass, parent,
  			list, nlist);

  quitButton = XtVaCreateManagedWidget(
  		"quit", commandWidgetClass, result,
  		XtNlabel, "Quit",
		NULL);
    
  return result;
}

/* Use this routine to register callbacks for when the user
   chooses to quit. Such callbacks must NOT call exit because
   the order in which callbacks are called is undefined.  */
   
void addQuitCallback (callback, clientData)
	XtCallbackProc callback;
	void *	       clientData;
{
  XtAddCallback(quitButton, XtNcallback, callback, clientData);
}

