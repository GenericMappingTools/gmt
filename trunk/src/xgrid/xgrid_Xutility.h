
#ifndef _Xutility_
#define _Xutility_

#define XSetPoint(pt, xVal, yVal) (pt).x = (xVal); (pt).y = (yVal)

extern Dimension XtWidth (Widget);
		/* Widget w */

extern Dimension XtHeight (Widget);
		/* Widget w */

extern Widget XtShell (Widget);
		/* Widget w */
		
extern void   WaitNoExposes (Widget);
		/* Widget w */

extern void InvertWidget (Widget);
		/* Widget w */

#endif
