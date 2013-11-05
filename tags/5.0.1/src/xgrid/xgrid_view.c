
/*
	Hugh Fisher
	Dec 1991
	
	Top level code for grid display. The code
	here creates a new text view, and takes
	care of saving changes when the application
	quits.
					*/

#include <gmt.h>
#include <strings.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>

#include "xgrid_messages.h"
#include "xgrid_controls.h"
#include "xgrid_utility.h"
#include "xgrid_Xutility.h"
#include "xgrid_Canvas.h"
#include "xgrid_GMTgrid.h"
#include "xgrid_Panner.h"
#include "xgrid_textView.h"
#include "xgrid_textInput.h"

#include "xgrid_view.h"

Boolean	gridHasChanged;

static ViewData theView;
static Widget   saveChangesPopup;

/****	Local procedures	****/

static void getBaseName (); /* String fileName, String baseName */
static void saveChanges (); /* Widget w, ViewData * view, void * callData */
static void doSave (); /* Widget w, ViewData * view, void * callData */
static void doDiscard (); /* Widget w, ViewData * view, void * callData */
	  	
static void getBaseName(fileName, baseName)
	String fileName;
	String baseName;
{
  String slash, stop;
  
  /* Strip off directory path */
  slash = strrchr(fileName, '/');
  if (slash != NULL)
    strcpy(baseName, slash + 1);
  else
    strcpy(baseName, fileName);
  /* Strip off any . extensions, otherwise likely
     to confuse X resource manager */
  stop = strchr(baseName, '.');
  if (stop != NULL)
    *stop = '\0';
}

Widget createView (GMT,fileName, parent, args, nargs)
	struct GMT_CTRL *GMT;
	String fileName;
	Widget parent;
	Arg    args[];
	int    nargs;
{
  Widget result;
  char   widgetName[255];
  Arg    list[32];
  int    err, loop, nlist;
  char   msg[256];

  /* Read in grid to be viewed */
  strcpy(theView.fileName, fileName);
  theView.grid = CreateGMTGrid(GMT);
  Trace("Reading grid file...");
  ReadGridFromFile(theView.grid, fileName, &err);
  if (err) {
    sprintf(msg, "Unable to read file %s: C error code %d", fileName, err);
    XtError(msg);
  }
  
  /* Create scrolling view widget */
  /* Copy supplied resources */
  for (loop = 0; loop < nargs; loop ++)
    list[loop] = args[loop];
  nlist = nargs;
  XtSetArg(list[nlist], XtNwidth, 640); nlist ++;
  XtSetArg(list[nlist], XtNheight, 480); nlist ++;
  result = XtCreateManagedWidget("view", pannerWidgetClass, parent, list, nlist);

  /* Derive widget name from grid file */
  getBaseName(fileName, widgetName);
  nlist = 0;
  XtSetArg(list[nlist], XtNgrid, theView.grid); nlist ++;
  /* Purists avert eyes...hardcoded dimensions follow */
  XtSetArg(list[nlist], XtNwidth, 640); nlist ++;
  XtSetArg(list[nlist], XtNheight, 480); nlist ++;
  theView.canvas = createTextView(widgetName, result, list, nlist);
  XtSetPannerCanvas(result, theView.canvas);

  /* Register exit action */
  addQuitCallback(saveChanges, &theView);
  
  return result;
}

void createFileCommands (parent)
	Widget	parent;
{
  Widget save;
  
  save = XtVaCreateManagedWidget(
  		"save", commandWidgetClass, parent,
  		XtNlabel, "Save",
		NULL);
  XtAddCallback(save, XtNcallback, saveChanges, &theView);
}

/****	When the user quits, be nice and offer to save
	any changes made in a popup dialog	****/
	
static void saveChanges (w, view, callData)
	Widget	   w;
	ViewData * view;
	void *	   callData;
{
  Widget	dialog;
  XtAppContext	app;
  XEvent	event;
  
  if (! gridHasChanged)
    return;
    
  saveChangesPopup = XtVaCreatePopupShell(
  	"popup", transientShellWidgetClass,
	XtShell(theView.canvas),
	XtNinput, True,
	NULL);
  dialog = XtVaCreateManagedWidget(
  		"save", dialogWidgetClass, saveChangesPopup,
  		XtNlabel, "Save changes to grid?",
		XtNvalue, view->fileName,
		NULL);
  XawDialogAddButton(dialog, "Save", doSave, view);
  XawDialogAddButton(dialog, "Discard", doDiscard, view);
  
  /* This gets called outside the main application loop,
     so we need a mini-loop of our own. Callbacks on
     buttons destroy dialog, so that is the exit condition. */
  XtPopup(saveChangesPopup, XtGrabNonexclusive);

  app = XtWidgetToApplicationContext(w);
  while (saveChangesPopup != NULL)  {
    XtAppNextEvent(app, &event);
    XtDispatchEvent(&event);
  }
}

static void doSave (w, view, callData)
	Widget	   w;
	ViewData * view;
	void *	   callData;
{
  String saveName;
  int    err;
  char	 msg[256];
  
  saveName = XawDialogGetValueString(XtParent(w));
  strcpy(view->fileName, saveName);
  WriteGridToFile(view->grid, view->fileName, &err);
  if (err) {
    sprintf(msg, "Unable to save file %s: C error code %d", saveName, err);
    setMessageLine(msg);
    XtWarning(msg);
  } else {
    XtVaSetValues(XtShell(w), XtNtitle, view->fileName, NULL);
    gridHasChanged = False;
    XtPopdown(saveChangesPopup);
    XtDestroyWidget(saveChangesPopup);
    /* MUST do this, otherwise save dialog loops forever */
    saveChangesPopup = NULL;
  }
}

static void doDiscard (w, view, callData)
	Widget	   w;
	ViewData * view;
	void *	   callData;
{
  XtPopdown(saveChangesPopup);
  XtDestroyWidget(saveChangesPopup);
  /* MUST do this, otherwise save dialog loops forever */
  saveChangesPopup = NULL;
}


