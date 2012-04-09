
#ifndef _messages_
#define _messages_

extern Widget	messageWidget;

extern Widget	createMessageLine (String name, Widget parent, Arg args[], int nargs);

extern void	setMessageLine (String newMessage);
#endif

