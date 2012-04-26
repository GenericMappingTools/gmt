
#ifndef _controls_
#define _controls_

extern Widget	createControlBar (String name, Widget parent, Arg args[], int nargs);
		/* String name, Widget parent, Arg args[], int nargs */

extern void addQuitCallback (XtCallbackProc callback, void * clientData);
		/* XtCallbackProc callback, void * clientData */

#endif

