#ifndef _TEXTINPUT_
#define _TEXTINPUT_

extern Widget createTextInputArea (); /* Widget parent */

extern void setViewForInput (); /* TextViewData * data */

extern void clearInputSelection (); /* void */
extern void setInputSelection (); /* int col, int row */

#endif

