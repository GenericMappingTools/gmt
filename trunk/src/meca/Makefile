#       $Id: Makefile,v 1.20 2007-09-25 01:50:16 remko Exp $
#
#	makefile for meca directory
#	meca must be installed in the gmt/src directory or
#	you must fix the -L -I for the gmt-dependencies.
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF		= -L$(NETCDF)/lib -lnetcdf
ALLLIB		= -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) $(LDFLAGS)

PROGS_C		= psmeca.c pscoupe.c pspolar.c psvelo.c
PROGS_O		= $(PROGS_C:.c=.o)
PROGS		= $(PROGS_C:.c=$(EXE))
MAN1		= $(PROGS_C:.c=.1)

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

$(PROGS_O):	$(GMT_H) $(PS_H) meca.h nrutil.h submeca.h utilmeca.h utilstrain.h utilvelo.h

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

psmeca$(EXE): 	psmeca.o utilmeca.o nrutil.o $(GMTLIB)
		$(CC) $(CFLAGS) psmeca.o utilmeca.o nrutil.o $(ALLLIB) -o $@
		$(COMPRESS) $@

pscoupe$(EXE):	pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o $(GMTLIB)
		$(CC) $(CFLAGS) pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o $(ALLLIB) -o $@
		$(COMPRESS) $@

pspolar$(EXE): 	pspolar.o $(GMTLIB)
		$(CC) $(CFLAGS) pspolar.o $(ALLLIB) -o $@
		$(COMPRESS) $@

psvelo$(EXE):	psvelo.o utilvelo.o utilstrain.o $(GMTLIB)
		$(CC) $(CFLAGS) psvelo.o utilvelo.o utilstrain.o $(ALLLIB) -o $@
		$(COMPRESS) $@
