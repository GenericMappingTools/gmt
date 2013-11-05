
#ifndef _TEXTVIEW_
#define _TEXTVIEW_

/****	A text view is a canvas widget which shows the
	numeric values stored in a GMT grid.	****/

typedef struct {
	Grid *		grid;
	Widget		canvas;
	/* Extra resources for display */
	Pixel		valueColor;
	Pixel		gridColor;
	Boolean		showGrid;
	String		precision;
	/* Position of scrolling text view within grid */
	int		xOffset;
	int		yOffset;
	/* Size of each value cell */
	int		height;
	int		ascent;
	int		charWidth;
	int		margin;
	int		cellWidth;
	int		cellHeight;
	/* Graphic contexts */
	GC		valueGC;
	GC		gridGC;
	GC		scrollGC;
} TextViewData;

#define XtNgrid		"grid"
#define XtNvalueColor	"valueColor"
#define XtNgridColor	"gridColor"
#define XtNshowGrid	"showGrid"
#define XtNmargin	"margin"
#define XtNprecision	"precision"

#define XtCGrid		"Grid"
#define XtCValueColor	"ValueColor"
#define XtCGridColor	"GridColor"
#define XtCShowGrid	"ShowGrid"
#ifndef XtCMargin     	/* Many versions of X define this already. */
#define XtCMargin	"Margin"
#endif
#define XtCPrecision	"Precision"

extern Widget createTextView ();
		/* String name, Widget parent, Arg args[], int nargs */

extern void setTextViewGrid ();
		/* Widget view, Grid * grid */

extern void recalculateDisplayParameters ();
		/* Widget view */

extern void redrawGridValue ();
		/* Widget view, int col, int row */
#endif

