# $Id: Makefile,v 1.17 2010-07-18 03:16:01 guru Exp $ # # makefile for gshhs directory # Does not depend on GMT src or libraries #
#------------------------------------------------------------------------------- # !! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

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
		$(CC) $(LDFLAGS) $(@:.exe=).o $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@
