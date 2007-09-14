#       $Id: Makefile,v 1.18 2007-09-14 12:31:17 remko Exp $
#
#	makefile for meca directory
#	meca must be installed in the gmt/src directory or
#	you must fix the -L -I for the gmt-dependencies.
#
#-----------------stop here-------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF		= -L$(NETCDF)/lib -lnetcdf
ALLLIB		= -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) $(LDFLAGS)

MECA_C		= psmeca.c pscoupe.c pspolar.c psvelo.c
MECA_O		= $(MECA_C:.c=.o)
MECA		= $(MECA_C:.c=$(EXE))

all:		$(MECA)

$(MECA_O):	$(GMT_H) $(PS_H) meca.h nrutil.h submeca.h utilmeca.h utilstrain.h utilvelo.h

install:	all
		$(INSTALL) $(MECA) $(bindir)

uninstall:
		for i in $(MECA); do \
        		\rm -f $(bindir)/$$i; \
		done

spotless:	clean

psmeca$(EXE): 	psmeca.o utilmeca.o nrutil.o $(GMTLIB)
		$(CC) $(CFLAGS) psmeca.o utilmeca.o nrutil.o $(ALLLIB) -o $@

pscoupe$(EXE):	pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o $(GMTLIB)
		$(CC) $(CFLAGS) pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o $(ALLLIB) -o $@

pspolar$(EXE): 	pspolar.o $(GMTLIB)
		$(CC) $(CFLAGS) pspolar.o $(ALLLIB) -o $@

psvelo$(EXE):	psvelo.o utilvelo.o utilstrain.o $(GMTLIB)
		$(CC) $(CFLAGS) psvelo.o utilvelo.o utilstrain.o $(ALLLIB) -o $@

clean:
		rm -f *.o $(MECA)
