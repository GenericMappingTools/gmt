# $Id$
# Used to modify the makefile.u to use 64-bit flags.
18c\
CFLAGS = -O -fPIC -m64
93,97c\
f77vers.o: f77vers.c\
	$(CC) -c $(CFLAGS) f77vers.c\
\
i77vers.o: i77vers.c\
	$(CC) -c $(CFLAGS) i77vers.c
