
/*	Hugh Fisher Feb 92

	Program to display and edit the numerical
	values in a GMT binary grid file.	*/

#include "gmt.h"
EXTERN_MSC void GMT_grdio_init (void);       /* Defined in gmt_customio.c */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>

#include "xgrid_utility.h"
#include "xgrid_controls.h"
#include "xgrid_messages.h"
#include "xgrid_view.h"
#include "xgrid_textInput.h"

static Boolean done;

/****	Local procedures  ****/

static void usage (String argv[]);
static void quitGridEdit (Widget w, void * callData, void * clientData);

static void usage (String argv[])
{
  printf("Usage: %s <grid file name>\n", argv[0]);
}

static void quitGridEdit (Widget w __attribute__((unused)),
			  void *call __attribute__((unused)),
			  void *client __attribute__((unused)))
{
  /* There may be more than one callback on the Quit button, and
     the order of execution is undefined. We therefore can't
     destroy the application context, call exit, or similar
     destructive actions.					*/
  done = True;
}

int main (int argc, String argv[])
{
  XtAppContext self;
  String gridFileName;
  Widget shell;
  Widget frame;
  Widget control, view, message;
  Arg    args[8];
  int    nargs;
  XEvent event;
  struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
  struct GMT_CTRL *GMT = NULL;

  /* 1. Initializing new GMT session */
  if ((API = GMT_Create_Session (argv[0], GMTAPI_GMT)) == NULL) exit (EXIT_FAILURE);

  GMT = API->GMT;
  /* This is a pretty simple application, so don't bother
     with WM_TAKE_FOCUS messages.	*/
  nargs = 0;
  XtSetArg(args[nargs], XtNinput, True); nargs ++;  
  shell = XtAppInitialize(&self, "XGridEdit", NULL, 0, &argc, argv,
  		NULL, args, nargs);
  
  if (argc != 2) {
    usage(argv);
    XtDestroyApplicationContext(self);
    exit(1);
  } else {
    gridFileName = argv[1];
    /* Change app title to that of file. Should really
       have application name + file name */
    XtVaSetValues(shell, XtNtitle, gridFileName, NULL);
  }
      
  frame = XtCreateManagedWidget("frame", panedWidgetClass, shell, NULL, 0);

  /* Control bar: see controls.c */
  nargs = 0;
  XtSetArg(args[nargs], XtNshowGrip, False); nargs ++;
  XtSetArg(args[nargs], XtNskipAdjust, True); nargs ++;
  control = createControlBar("control", frame, args, nargs);

  addQuitCallback(quitGridEdit, NULL);
  createFileCommands(control);	/* view.c */
  createTextInputArea(control);	/* textInput.c */
  
  /* Display widget: see view.c */
  nargs = 0;
  XtSetArg(args[nargs], XtNshowGrip, False); nargs ++;
  XtSetArg(args[nargs], XtNskipAdjust, False); nargs ++;
  view = createView(GMT,gridFileName, frame, args, nargs);
	(void)view; /* fixme: -Wunused-but-set-variable */

  /* Status/info line: see messages.c */
  nargs = 0;
  XtSetArg(args[nargs], XtNshowGrip, False); nargs ++;
  XtSetArg(args[nargs], XtNskipAdjust, True); nargs ++;
  message = createMessageLine("message", frame, args, nargs);
	(void)message; /* fixme: -Wunused-but-set-variable */

  XtRealizeWidget(shell);

  /* Don't use XtAppMainLoop: see quit routine for reason */  
  done = False;
  while (! done) {
    XtAppNextEvent(self, &event);
    XtDispatchEvent(&event);
  }
  XtDestroyApplicationContext(self);
  GMT_Destroy_Session (&API);
  exit(0);
}
