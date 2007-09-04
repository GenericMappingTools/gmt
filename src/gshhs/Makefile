#	$Id: Makefile,v 1.7 2007-09-04 15:22:27 remko Exp $
#
#	makefile for gshhs directory
#	Does not depend on GMT src or libraries
#
#----------------- stop here - nothing more to edit--------------------

include ../makegmt.macros

CFLAGS		= $(CC_OPT) $(WIN32)

PROGS_C		= gshhs.c gshhs_dp.c gshhstograss.c
PROGS_O		= $(PROGS_C:.c=.o)
PROGS		= $(PROGS_C:.c=$(EXE))

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

all:		$(PROGS)

$(PROGS_O):	$(GMT_H) gshhs.h

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		for i in $(PROGS); do \
			\rm -f $(bindir)/$$i; \
		done

clean:	
		\rm -f *.o *% $(PROGS)

spotless:	clean

gshhs:		gshhs.o
		$(CC) $(LDFLAGS) gshhs.o $(LIBS) -o $@

gshhs_dp:	gshhs_dp.o
		$(CC) $(LDFLAGS) gshhs_dp.o $(LIBS) -o $@

gshhstograss:	gshhstograss.o
		$(CC) $(LDFLAGS) gshhstograss.o $(LIBS) -o $@
