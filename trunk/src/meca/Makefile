#       $Id: Makefile,v 1.3 2001-03-02 05:54:44 pwessel Exp $
#
#	makefile for meca directory
#	meca must be installed in the gmt/src directory or
#	you must fix the -L -I for the gmt-dependencies.
#
#-----------------stop here-------------------------------
include ../makegmt.macros

NETCDFLIB	= $(NETCDF)/lib
NETCDFINC	= $(NETCDF)/include
CDF		= -L$(NETCDFLIB) -lnetcdf
CFLAGS		= $(CC_OPT) -I.. -I$(NETCDFINC)
ALLLIB		= -L.. -lpsl -lgmt $(CDF) $(LIBS) $(LDFLAGS)
#
#

MECA_C		= psmeca.c pscoupe.c pspolar.c psvelo.c
MECA_O		= ${MECA_C:.c=.o}
MECA		= ${MECA_C:.c=}

all:		$(MECA)

install:	all
		for i in $(MECA); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

uninstall:
		for i in $(MECA); do \
        		\rm -f $(bindir)/$$i$(EXE); \
		done

spotless:	clean

psmeca: 	psmeca.o utilmeca.o nrutil.o
		$(CC) $(CFLAGS) psmeca.o utilmeca.o nrutil.o $(ALLLIB) -o ./$@

pscoupe:	pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o
		$(CC) $(CFLAGS) pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o $(ALLLIB) -o ./$@

pspolar: 	pspolar.o
		$(CC) $(CFLAGS) pspolar.o $(ALLLIB) -o ./$@

psvelo:		psvelo.o utilvelo.o utilstrain.o 
		$(CC) $(CFLAGS) psvelo.o utilvelo.o utilstrain.o $(ALLLIB) -o ./$@

.c.o:
		$(CC) -c $(CFLAGS) $<

clean:
		rm -f *.o
		for i in $(MECA); do \
			rm -f $$i$(EXE); \
		done
