#	$Id: Makefile,v 1.19 2007-10-03 14:24:17 remko Exp $
#
#	makefile for dbase directory
#
# Uncomment SWAP to build in byte-order swapping of all data
#SWAP	= -DSWAP_ME
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) $(WIN32) $(SWAP) -I$(srcdir) -I$(NETCDF)/include

PROGS_O		= grdraster.o
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

$(PROGS_O):	$(GMT_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

grdraster$(EXE):	grdraster.o

$(PROGS):
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
