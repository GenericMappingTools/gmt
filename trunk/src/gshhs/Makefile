#	$Id: Makefile,v 1.2 2001-03-05 15:50:44 pwessel Exp $
#
#	makefile for gshhs directory
#
#----------------- stop here - nothing more to edit--------------------

-include ../makegmt.macros

CFLAGS		= $(CC_OPT) $(FLIP) $(WIN32)


all:		gshhs gshhs_dp

install:	all
		$(INSTALL) gshhs$(EXE) $(bindir)
		$(INSTALL) gshhs_dp$(EXE) $(bindir)

clean:	
		\rm -f *.o *% gshhs$(EXE) gshhs_dp$(EXE)

spotless:	clean

gshhs:		gshhs.o
		$(CC) $(CFLAGS) gshhs.o $(LIBS) $(LDFLAGS) -o gshhs

gshhs_dp:	gshhs_dp.o
		$(CC) $(CFLAGS) gshhs_dp.o $(LIBS) $(LDFLAGS) -o gshhs_dp

.c.o:
		$(CC) -c $(CFLAGS) $<
