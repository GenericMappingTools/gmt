#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "xgrid_CanvasP.h"

static XtResource resources[] = {
#define offset(field) XtOffset(CanvasWidget, canvas.field)
  { XtNcanvasData, XtCCanvasData, XtRPointer, sizeof(XtPointer),
  	offset(data), XtRPointer, NULL },
  { XtNexposeCallback, XtCExposeCallback, XtRCallback, sizeof(XtCallbackList),
  	offset(expose_callback), XtRPointer, NULL },
  { XtNresizeCallback, XtCResizeCallback, XtRCallback, sizeof(XtCallbackList),
  	offset(resize_callback), XtRPointer, NULL },
  { XtNrealizeCallback, XtCRealizeCallback, XtRCallback, sizeof(XtCallbackList),
  	offset(realize_callback), XtRPointer, NULL },
  { XtNqueryGeometry, XtCQueryGeometry, XtRPointer, sizeof(XtPointer),
  	offset(query_geometry), XtRPointer, NULL },
  { XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
  	offset(font), XtRString, XtDefaultFont }
#undef offset
};

/****	Methods for Canvas.	****/

static void CanvasRealize (self, mask, attributes)
	Widget	self;
	XtValueMask * mask;
	XSetWindowAttributes * attributes;
{
  WidgetClass super;
  
  /* Call superclass method */
  super = canvasWidgetClass->core_class.superclass;
  (*(super->core_class.realize))(self, mask, attributes);
  /* And then client code */
  XtCallCallbackList(self, ((CanvasWidget)self)->canvas.realize_callback, NULL);
}

static void CanvasResize (self)
	Widget self;
{
  XRectangle	bounds;
  
  XtVaGetValues(self,
  	XtNx, &bounds.x, XtNy, &bounds.y,
	XtNwidth, &bounds.width,
	XtNheight, &bounds.height,
	NULL);
  XtCallCallbackList(self,
  	((CanvasWidget)self)->canvas.resize_callback, &bounds);
}

static void CanvasExpose (self, event, region)
	Widget	 self;
	XEvent * event;
	Region	 region;
{
  XtCallCallbackList(self,
  	((CanvasWidget)self)->canvas.expose_callback, region);
}

static XtGeometryResult CanvasQueryGeometry (self, request, reply)
	Widget		   self;
	XtWidgetGeometry * request;
	XtWidgetGeometry * reply;
{
  XtGeometryHandler instMethod;
  
  instMethod = ((CanvasWidget)self)->canvas.query_geometry;
  if (instMethod == NULL)
    return XtGeometryYes;
  else
    return (*instMethod)(self, request, reply);
}

CanvasClassRec canvasClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
    /* class_name		*/	"Canvas",
    /* widget_size		*/	sizeof(CanvasRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	NULL,
    /* initialize_hook		*/	NULL,
    /* realize			*/	CanvasRealize, /* XtInheritRealize, */
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	XtExposeCompressSeries | XtExposeGraphicsExpose,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	CanvasResize,
    /* expose			*/	CanvasExpose,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	CanvasQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* canvas fields */
    /* empty			*/	0
  }
};

WidgetClass canvasWidgetClass = (WidgetClass)&canvasClassRec;

/****	Convenience functions	****/

XFontStruct * XtCanvasFont (self)
	CanvasWidget self;
{
  XtCheckSubclass(self, canvasWidgetClass, "XtCanvasFont");
  return self->canvas.font;
}

void * XtCanvasData (self)
	CanvasWidget self;
{
  XtCheckSubclass(self, canvasWidgetClass, "XtCanvasData");
  return self->canvas.data;
}

