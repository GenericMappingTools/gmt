
/*
	Hugh Fisher
	Dec 1991
	
	Status and message widget, just a line
	of text at the bottom of the main window.
					*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Label.h>

#include "xgrid_messages.h"

Widget	messageWidget = NULL;

Widget createMessageLine (String name,
			  Widget parent,
			  Arg    args[],
			  int    nargs)
{
  Widget result;
  Arg    list[32];
  int    nlist, loop;

  /* Copy resources supplied by caller. */  
  for (loop = 0; loop < nargs; loop ++)
    list[loop] = args[loop];
  nlist = nargs;
  
  XtSetArg(list[nlist], XtNjustify, XtJustifyCenter); nlist ++;
  XtSetArg(list[nlist], XtNlabel, ""); nlist ++;
  
  result = XtCreateManagedWidget(name, labelWidgetClass, parent,
  				list, nlist);
  messageWidget = result;
  return result;
}

void setMessageLine (String newMessage)
{
  if (messageWidget != NULL)
    XtVaSetValues(messageWidget, XtNlabel, newMessage, NULL);
}

