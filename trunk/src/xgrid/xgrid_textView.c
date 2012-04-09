
/*
	Hugh Fisher
	Feb 92
	
	View widget for displaying the values
	stored in a grid structure.
	
	Also see textInput.c which handles
	value editing.
					*/

#include <gmt.h>
#include <strings.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "xgrid_messages.h"
#include "xgrid_utility.h"
#include "xgrid_Xutility.h"
#include "xgrid_Canvas.h"
#include "xgrid_Panner.h"
#include "xgrid_GMTgrid.h"
#include "xgrid_textInput.h"

#include "xgrid_textView.h"

/****	Local variables for event handling ****/

/* Structure to keep screen, array, logical info together */
typedef struct {
	XRectangle	bounds;	/* Area in window */
	GridPoint	coord;
	XPoint		index;
	} CellInfo;
	
static CellInfo cellUnderMouse = {
	{ 0, 0, 1, 1 }, /* If this is all zeroes, entire window erased! */
	{ 0.0, 0.0 },
	{ 0, 0 }
	};

/****	Local procedures	****/

static void handleRealize (Widget w, XtPointer, XtPointer);
static void handleExpose (Widget w, XtPointer, XtPointer);
static void createContexts (Widget w, TextViewData * data);

static void scrollHorizontal (Widget w, XtPointer, XtPointer);
static void scrollVertical (Widget w, XtPointer, XtPointer);

static void redrawValues (Widget w, Position left, Position top, Dimension width, Dimension height);

static void highlightCell (Widget w, TextViewData * data, CellInfo * cell);
static void lowlightCell (Widget w, CellInfo * cell);

static void CursorMovement (Widget w, XEvent * event, String * params, Cardinal * nparams);
static void ClearMessage (Widget w, XEvent * event, String * params, Cardinal * nparams);
static void SelectValue (Widget w, XEvent * event, String * params, Cardinal * nparams);

static XtResource ViewResourceList[] = {
  { XtNgrid, XtCGrid, XtRPointer, sizeof(XtPointer),
    XtOffsetOf(TextViewData, grid), XtRPointer, NULL },
  { XtNvalueColor, XtCValueColor, XtRPixel, sizeof(Pixel),
    XtOffsetOf(TextViewData, valueColor), XtRString, XtDefaultForeground },
  { XtNgridColor, XtCGridColor, XtRPixel, sizeof(Pixel),
    XtOffsetOf(TextViewData, gridColor), XtRString, "Light blue" },
  { XtNshowGrid, XtCShowGrid, XtRBoolean, sizeof(Boolean),
    XtOffsetOf(TextViewData, showGrid), XtRImmediate, (XtPointer)True },
  { XtNmargin, XtCMargin, XtRInt, sizeof(int),
    XtOffsetOf(TextViewData, margin), XtRImmediate, (XtPointer)4 },
  { XtNprecision, XtCPrecision, XtRString, sizeof(String),
    XtOffsetOf(TextViewData, precision), XtRString, "5.2" }
};

static String ViewTranslations = "\
	<MotionNotify>: CursorMovement()\n\
	<LeaveNotify>:  ClearMessage()\n\
	<BtnDown>:	SelectValue()\n\
	";

static XtActionsRec ViewActions[] = {
  { "CursorMovement", CursorMovement },
  { "ClearMessage", ClearMessage },
  { "SelectValue", SelectValue }
};

/****	Initialization	****/

Widget createTextView (String	name,
		       Widget	parent,
		       Arg	args[],
		       int	nargs)
{
  Widget result;
  TextViewData * data;

  /* We are fussy about our parent: should be a panner */
  if (! XtIsSubclass(parent, pannerWidgetClass))
    XtError("Parent of text view widget must be a panner");
    
  result = XtCreateManagedWidget(name, canvasWidgetClass, parent, args, nargs);
  
  /* Allocate data block for widget and cross-ref data and canvas */
  data = (TextViewData *)XtMalloc(sizeof(TextViewData));
  XtVaSetValues(result, XtNcanvasData, data, NULL);
  data->canvas = result;
  
  XtGetApplicationResources(result, data, ViewResourceList,
  		XtNumber(ViewResourceList), args, nargs);
  /* Clear the non-resource fields */
  data->valueGC  = None;
  data->gridGC   = None;
  data->scrollGC = None;
  data->xOffset  = 0;
  data->yOffset  = 0;
  /* Set up event handlers for widget */
  XtAddCallback(result, XtNrealizeCallback, handleRealize, data);
  XtAddCallback(result, XtNexposeCallback, handleExpose, NULL);

  /* Set up scroll handlers for parent */
  XtAddCallback(parent, XtNxCallback, scrollHorizontal, data);
  XtAddCallback(parent, XtNyCallback, scrollVertical, data);

  /* Register actions */
  XtAppAddActions(XtWidgetToApplicationContext(result),
		ViewActions, XtNumber(ViewActions));
  XtAugmentTranslations(result,
  		XtParseTranslationTable(ViewTranslations));
  
  return result;
}

void setTextViewGrid (Widget	view,
		      Grid *	grid)
{
  TextViewData * data;
  
  data = XtCanvasData(view);
  if (data->grid != NULL)
    DisposeGrid(data->grid);
  data->grid = grid;
  recalculateDisplayParameters(view);
  if (XtIsRealized(view))
    XClearArea(XtDisplay(view), XtWindow(view), 0, 0, 0, 0, True);    
}

void recalculateDisplayParameters (Widget view)
{
  TextViewData * data;
  XFontStruct *  font;
  int	    	 digits;
  Widget	 parent;

  data = XtCanvasData(view);
  font = XtCanvasFont(view);

  /* Keep these for convenience */
  data->height    = font->ascent + font->descent;
  data->ascent    = font->ascent;
  data->charWidth = font->max_bounds.width;

  /* Make sure left margin is big enough */
  data->margin    = Max(data->margin, font->max_bounds.lbearing);
  /* Cell contains however many digits precision allows + margins */
  data->cellHeight = data->height + 2 * data->margin;
  sscanf(data->precision, "%d", &digits);
  digits += 2; /* Allow for sign and decimal point */
  data->cellWidth = digits * data->charWidth + 2 * data->margin;

  /* Notify parent widget of new grid dimensions */
  if (data->grid != NULL) {
    parent = XtParent(view);
    XtVaSetValues(parent,
    	XtNxRange, data->grid->width * data->cellWidth,
	XtNyRange, data->grid->height * data->cellHeight,
	NULL);
  }
}

void redrawGridValue (Widget	view,
		      int	col,
		      int	row)
{
  TextViewData * data;
  int x, y;
  
  data = XtCanvasData(view);
  x = (col * data->cellWidth) - data->xOffset;
  y = (row * data->cellHeight) - data->yOffset;
  XClearArea(XtDisplay(view), XtWindow(view),
  	x, y, data->cellWidth, data->cellHeight, True);
}

/****	We can't calculate colors, etc until the widget
	window has been created, so do that when we
	are realized.		****/

static void createContexts (Widget view, TextViewData *data)
{
  XFontStruct *  font;
  XGCValues 	 gcv;
  Pixel	    	 foreground, background;

  font = XtCanvasFont(view);
  XtVaGetValues(view,
  	XtNforeground, &foreground,
	XtNbackground, &background,
	NULL);

  gcv.background = background;
  gcv.foreground = data->gridColor;
  gcv.line_width = 0; /* Draw as fast as possible */
  data->gridGC = XCreateGC(XtDisplay(view), XtWindow(view),
  			GCForeground | GCLineWidth, &gcv);

  gcv.foreground = foreground;
  gcv.background = background;
  gcv.function = GXcopy;
  data->scrollGC = XCreateGC(XtDisplay(view), XtWindow(view),
  			GCForeground | GCBackground | GCFunction, &gcv);

  gcv.foreground = data->valueColor;
  gcv.background = background;
  gcv.font	 = font->fid;
  data->valueGC = XCreateGC(XtDisplay(view), XtWindow(view),
  			GCForeground | GCBackground | GCFont, &gcv);
}

static void handleRealize (Widget w,
			   XtPointer client_data,
			   XtPointer call_data __attribute__((unused)))
{
  createContexts(w, client_data);
  recalculateDisplayParameters(w);
  setViewForInput(client_data);
}

/****	Expose event handling	****/

static void handleExpose (Widget	w,
			  XtPointer  client_data __attribute__((unused)),
			  XtPointer call_data)
{
  XRectangle bbox;
  Region  area = call_data;

  XClipBox(area, &bbox);
  redrawValues(w, bbox.x, bbox.y, bbox.width, bbox.height);
}

static void redrawValues (Widget	w,
			  Position	left,
			  Position	top,
			  Dimension	width,
			  Dimension	height)
{
  TextViewData * data;
  int	x, y;
  int	col, row;
  char	format[32];
  char	value[32];

  data = XtCanvasData(w);
  XClearArea(XtDisplay(w), XtWindow(w), left, top, width, height, False);

  /* Draw grid? */
  if (data->showGrid) {
    /* Verticals */
    x = RoundUp(left + data->xOffset, data->cellWidth) - data->xOffset;
    while (x <= left + width) {
      XDrawLine(XtDisplay(w), XtWindow(w), data->gridGC,
      		x, top, x, top + height);
      x += data->cellWidth;
    }
    /* Horizontals */
    y = RoundUp(top + data->yOffset, data->cellHeight) - data->yOffset;
    while (y <= top + height) {
      XDrawLine(XtDisplay(w), XtWindow(w), data->gridGC,
      		left, y, left + width, y);
      y += data->cellHeight;
    }
  } /* End if show grid */

  /* Just make sure we have a grid to draw. */
  if (data->grid == NULL)
    return;
    
  /* Turn precision spec of form "5.2" into C format "%5.2f" */
  sprintf(format, "%%%sf", data->precision);

  /* Find topmost row intersecting area */
  y   = RoundDown(top + data->yOffset, data->cellHeight);
  row = y / data->cellHeight;
  /* Scale y back to screen and to text origin within cell */
  y = y - data->yOffset + data->margin + data->height;
  while (y < (int)(top + height) && row < data->grid->height) {
    /* Calculate column */
    x   = RoundDown(left + data->xOffset, data->cellWidth);
    col = x / data->cellWidth;
    x = x - data->xOffset + data->margin;
    while (x < (int)(left + width) && col < data->grid->width) {
      sprintf(value, format, GetGrid(data->grid, col, row));
      XDrawImageString(XtDisplay(w), XtWindow(w), data->valueGC,
      			x, y, value, strlen(value));
      x += data->cellWidth;
      col ++;
    } /* End horizontal loop */
    y += data->cellHeight;
    row ++;
  } /* End vertical loop */
}

/****	Handle scrolling. Note that the widget passed to
	these callbacks is the parent of the actual canvas.	****/

#define XIndex(x) (((x) + data->xOffset) / data->cellWidth)
#define YIndex(y) (((y) + data->yOffset) / data->cellHeight)

static void scrollHorizontal (Widget		w __attribute__((unused)),
			      XtPointer 	client_data,
			      XtPointer		call_data)
{
  Widget    canvas;
  Dimension width, height;
  int       previous, distance;
  char	    msg[256];
  XPoint    index1, index2;
  GridPoint grid1, grid2;

  TextViewData *data = client_data;
  int		x = (int)call_data;


  canvas = data->canvas;
  width  = XtWidth(canvas);
  height = XtHeight(canvas);
  
  previous = data->xOffset;
  distance = abs(x - previous);
  data->xOffset = x;

  /* Adjust the display */
  if (distance >= width) {
    /* Moved more than a full window: redraw the lot */
    redrawValues(canvas, 0, 0, width, height);
  } else if (x < previous) {
    /* Data moves right */
    XCopyArea(XtDisplay(canvas),
    		XtWindow(canvas), XtWindow(canvas), data->scrollGC,
		0, 0, width - distance, height, distance, 0);
    redrawValues(canvas, 0, 0, distance, height);
  } else if (x > previous) {
    /* Data moves left */
    XCopyArea(XtDisplay(canvas),
    		XtWindow(canvas), XtWindow(canvas), data->scrollGC,
		distance, 0, width - distance, height, 0, 0);
    redrawValues(canvas, width - distance, 0, distance, height);
  }
  /* And show user where we are */
  index1.x = XIndex(0); index1.y = 0;
  GetGridCoords(data->grid, &index1, &grid1);
  index2.x = XIndex(width); index2.y = 0;
  GetGridCoords(data->grid, &index2, &grid2);
  sprintf(msg, "%7.3f to %7.3f", grid1.x, grid2.x);
  setMessageLine(msg);
}

static void scrollVertical (Widget	w __attribute__((unused)),
			    XtPointer	client_data,
			    XtPointer	call_data)
{
  Widget    canvas;
  Dimension width, height;
  int       previous, distance;
  char	    msg[256];
  XPoint    index1, index2;
  GridPoint grid1, grid2;

  TextViewData *data = client_data;
  int		y = (int)call_data;

  canvas = data->canvas;
  width  = XtWidth(canvas);
  height = XtHeight(canvas);

  previous = data->yOffset;
  distance = abs(y - previous);
  data->yOffset = y;
  
  if (distance >= height) {
    /* Moved more than a full window: redraw the lot */
    XClearWindow(XtDisplay(canvas), XtWindow(canvas));
    redrawValues(canvas, 0, 0, width, height);
  } else if (y < previous) {
    /* Data moves down */
    distance = previous - y;
    XCopyArea(XtDisplay(canvas),
    		XtWindow(canvas), XtWindow(canvas), data->scrollGC,
		0, 0, width, height - distance, 0, distance);
    redrawValues(canvas, 0, 0, width, distance);
  } else if (y > previous) {
    /* Data moves up */
    distance = y - previous;
    XCopyArea(XtDisplay(canvas),
    		XtWindow(canvas), XtWindow(canvas), data->scrollGC,
		0, distance, width, height - distance, 0, 0);
    redrawValues(canvas, 0, height - distance, width, distance);
  }
  data->yOffset = y;

  /* And show user where we are */
  index1.y = YIndex(height); index1.x = 0;
  GetGridCoords(data->grid, &index1, &grid1);
  index2.y = YIndex(0); index2.x = 0;
  GetGridCoords(data->grid, &index2, &grid2);
  sprintf(msg, "%7.3f to %7.3f", grid1.y, grid2.y);
  setMessageLine(msg);
}

/****	Event handling	****/

static void highlightCell (Widget		w,
			   TextViewData *	data,
			   CellInfo *	cell)
{
  XDrawRectangle(XtDisplay(w), XtWindow(w), data->valueGC,
  	cell->bounds.x + 1, cell->bounds.y + 1,
	cell->bounds.width - 2, cell->bounds.height - 2);
}

static void lowlightCell (Widget		w,
			  CellInfo *	cell)
{
  redrawValues(w, cell->bounds.x, cell->bounds.y,
  		cell->bounds.width, cell->bounds.height);
}

static void CursorMovement (Widget	w,
			    XEvent *	event,
			    String *	params __attribute__((unused)),
			    Cardinal *	nparams __attribute__((unused)))
{
  TextViewData * data;
  char	    msg[256];
  XPoint    index;
  GridPoint grid;

  data = XtCanvasData(w);  
  /* Convert pixels to grid indexes and then coordinates */
  index.x = XIndex(event->xmotion.x);
  index.y = YIndex(event->xmotion.y);
  GetGridCoords(data->grid, &index, &grid);
  /* Check if moved. Might be easier just to always redraw the thing */
  if (grid.x != cellUnderMouse.coord.x || grid.y != cellUnderMouse.coord.y) {
    lowlightCell(w, &cellUnderMouse);
    /* Store new cell */
    cellUnderMouse.coord = grid;
    cellUnderMouse.index = index;
    cellUnderMouse.bounds.x = (index.x * data->cellWidth) - data->xOffset;
    cellUnderMouse.bounds.y = (index.y * data->cellHeight) - data->yOffset;
    cellUnderMouse.bounds.width = data->cellWidth;
    cellUnderMouse.bounds.height = data->cellHeight;
    highlightCell(w, data, &cellUnderMouse);
    sprintf(msg, "%7.3f, %7.3f", grid.x, grid.y);
    setMessageLine(msg);
  }
}


static void ClearMessage (Widget	w, 
			  XEvent *	event __attribute__((unused)),
			  String *	params __attribute__((unused)),
			  Cardinal *	nparams __attribute__((unused)))
{
  lowlightCell(w, &cellUnderMouse);
  setMessageLine("");
}

static void SelectValue (Widget		w __attribute__((unused)),
			 XEvent *	event __attribute__((unused)),
			 String *	params __attribute__((unused)),
			 Cardinal *	nparams __attribute__((unused)))
{
  /* All the actual input is handled in textInput.c */
  setInputSelection(cellUnderMouse.index.x, cellUnderMouse.index.y);
}

