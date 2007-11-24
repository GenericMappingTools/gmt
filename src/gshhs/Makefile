#	$Id: Makefile,v 1.13 2007-11-24 21:44:18 remko Exp $
#
#	makefile for gshhs directory
#	Does not depend on GMT src or libraries
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT)

PROGS_O		= gshhs.o gshhs_dp.o gshhstograss.o
MAN1		= $(PROGS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS)

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:	
		\rm -f *.o *% $(PROGS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	gshhs.h

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

gshhs$(EXE):		gshhs.o
gshhs_dp$(EXE):		gshhs_dp.o
gshhstograss$(EXE):	gshhstograss.o

$(PROGS):
		$(CC) $(LDFLAGS) $(@:.exe=).o $(LIBS) -o $@
		$(COMPRESS) $@
