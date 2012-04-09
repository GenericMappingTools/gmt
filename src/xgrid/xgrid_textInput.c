
/*
	Hugh Fisher
	Feb 92
	
	Widgets and functions for entering new
	values in a text view.
	
	This is derived from Macintosh spreadsheets
	such as Excel: clicking on a cell activates
	an entry field in the control bar. The new
	value replaces the old when Tab or Enter
	is pressed, and the most recent change can
	be Undone.
	
	We also set the global flag gridHasChanged
	so that the main control code knows that
	there is something to save.
					*/

#include "gmt.h"
#include <strings.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>

#include "xgrid_messages.h"
#include "xgrid_utility.h"
#include "xgrid_Xutility.h"
#include "xgrid_GMTgrid.h"
#include "xgrid_view.h"
#include "xgrid_textView.h"

#include "xgrid_textInput.h"

/****	Local procedures	****/

static void clearUndo (); /* void */

static void Beep (); /* Widget w, XEvent * event, String * params, Cardinal * nparams */
static void EnterNewValue (); /* Widget w, XEvent * event, String * params, Cardinal * nparams */
static void undoNewValue (); /* Widget w, void * clientData, void * callData */

static XtActionsRec inputActions [] = {
  { "Beep", Beep },
  { "EnterNewValue", EnterNewValue }
};

static char inputTranslations[] = "\
	Ctrl<Key>J:   Beep()\n\
	Ctrl<Key>O:   Beep()\n\
	Ctrl<Key>M:   Beep()\n\
	<Key>Return:  EnterNewValue()\n\
	<Key>Tab:     EnterNewValue()\n\
	";

/****	State variables for managing selection	****/

typedef struct {
	int	col;
	int	row;
	GridValue value;
	} CellInfo;

static TextViewData * theView;

static CellInfo	selection;
static CellInfo	undoValue;

static Widget	inputArea;
static Widget	inputField;
static Widget	undoButton;

Widget createTextInputArea (Widget parent)
{
  Widget label;
  
  undoButton = XtVaCreateManagedWidget("undo", commandWidgetClass, parent,
  		XtNlabel, "Undo",
		XtNsensitive, False,
		NULL);
  XtAddCallback(undoButton, XtNcallback, undoNewValue, NULL);
  
  inputArea = XtVaCreateManagedWidget("input", boxWidgetClass, parent,
  		XtNsensitive, False,
  		XtNorientation, XtorientHorizontal,
		NULL);

  label = XtVaCreateManagedWidget("prompt", labelWidgetClass, inputArea,
  		XtNjustify, XtJustifyRight,
		XtNlabel, "New value:",
		NULL);

  inputField = XtVaCreateManagedWidget("field", asciiTextWidgetClass, inputArea,
  		XtNeditType, XawtextEdit,
		XtNstring, "        ",
		NULL);
  XtAppAddActions(XtWidgetToApplicationContext(inputField),
  		inputActions, XtNumber(inputActions));
  XtOverrideTranslations(inputField,
  		XtParseTranslationTable(inputTranslations));
 
  clearInputSelection();
  clearUndo();
   
  return inputArea;
  
}

void setViewForInput (TextViewData *data)
{
  theView = data;
  clearInputSelection();
  clearUndo();
  gridHasChanged = False;
}

/****	Managing the selection	****/

void clearInputSelection ()
{
  XtVaSetValues(inputField,
  	XtNstring, "",
	NULL);
  XtSetSensitive(inputArea, False);
  XtSetKeyboardFocus(XtShell(inputArea), None);
}

static void clearUndo ()
{
  XtSetSensitive(undoButton, False);
}

void setInputSelection (int	col,
			int	row)
{
  char str[64];

  selection.col = col;
  selection.row = row;
  selection.value = GetGrid(theView->grid, col, row);
  /* Grab all keyboard events */
  XtSetSensitive(inputArea, True);
  XtSetKeyboardFocus(XtShell(inputArea), inputField);
  /* Set up initial field */
  sprintf(str, "%f", selection.value);
  XtVaSetValues(inputField,
    	XtNstring, str,
	NULL);
  XawTextSetInsertionPoint(inputField, strlen(str));
}

static void Beep (Widget	w,
		  XEvent *	event __attribute__((unused)),
		  String *	params __attribute__((unused)),
		  Cardinal *	nparams __attribute__((unused)))
{
  XBell(XtDisplay(w), 100);
}

static void EnterNewValue (Widget	w __attribute__((unused)),
			   XEvent *	event __attribute__((unused)),
			   String *	params __attribute__((unused)),
			   Cardinal *	nparams __attribute__((unused)))
{
  GridValue newValue;
  String    field;
  XPoint    index;
  GridPoint coord;
  char	    msg[256];

  /* Record old value */
  undoValue = selection;
  XtSetSensitive(undoButton, True);
  /* Update grid */
  XtVaGetValues(inputField, XtNstring, &field, NULL);
  sscanf(field, "%f", &newValue);
  SetGrid(theView->grid, selection.col, selection.row, newValue);
  redrawGridValue(theView->canvas, selection.col, selection.row);
  /* Tell user */
  index.x = selection.col;
  index.y = selection.row;
  GetGridCoords(theView->grid, &index, &coord);
  sprintf(msg, "%f, %f changed to %f", coord.x, coord.y, newValue);
  setMessageLine(msg);

  clearInputSelection();
  gridHasChanged = True;
}

static void undoNewValue (Widget	w __attribute__((unused)),
			  void *	clientData __attribute__((unused)),
			  void *	callData __attribute__((unused)))
{
  XPoint    index;
  GridPoint coord;
  char	    msg[256];

  SetGrid(theView->grid, undoValue.col, undoValue.row, undoValue.value);
  redrawGridValue(theView->canvas, undoValue.col, undoValue.row);

  /* Tell user */
  index.x = undoValue.col;
  index.y = undoValue.row;
  GetGridCoords(theView->grid, &index, &coord);
  sprintf(msg, "Restored value at %f, %f to %f", coord.x, coord.y, undoValue.value);
  setMessageLine(msg);

  clearUndo();
}

