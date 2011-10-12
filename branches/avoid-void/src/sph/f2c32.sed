# $Id$
# Used to modify the makefile.u to use CFLAGS everywhere.
18c\
CFLAGS = -O -fPIC
93,97c\
f77vers.o: f77vers.c\
	$(CC) -c $(CFLAGS) f77vers.c\
\
i77vers.o: i77vers.c\
	$(CC) -c $(CFLAGS) i77vers.c
