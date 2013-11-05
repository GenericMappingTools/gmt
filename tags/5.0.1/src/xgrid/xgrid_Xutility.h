
#ifndef _Xutility_
#define _Xutility_

#define XSetPoint(pt, xVal, yVal) (pt).x = (xVal); (pt).y = (yVal)

extern Dimension XtWidth ();
		/* Widget w */

extern Dimension XtHeight ();
		/* Widget w */

extern Widget XtShell ();
		/* Widget w */
		
extern void   WaitNoExposes ();
		/* Widget w */

extern void InvertWidget ();
		/* Widget w */

#endif
