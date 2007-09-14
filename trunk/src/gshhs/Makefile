#	$Id: Makefile,v 1.8 2007-09-14 12:31:17 remko Exp $
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

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		for i in $(PROGS); do \
			\rm -f $(bindir)/$$i; \
		done

spotless:	clean

clean:	
		\rm -f *.o *% $(PROGS)

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

gshhs$(EXE):		gshhs.o
gshhs_dp$(EXE):		gshhs_dp.o
gshhstograss$(EXE):	gshhstograss.o

$(PROGS):
		$(CC) $(LDFLAGS) $(@:.exe=).o -o $@
