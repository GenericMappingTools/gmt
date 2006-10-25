#	$Id: Makefile,v 1.5 2006-10-25 01:35:06 pwessel Exp $
#
#	makefile for imgsrc directory
#
#----------------- stop here - nothing more to edit -------------------

include ../makegmt.macros
GMTSRCDIR	= ../
GMTLIBDIR	= ../
include ../gmtalldeps.macros

CDF		= -L$(NETCDF)/lib -lnetcdf
GMT		= -L.. -L$(libdir) -lgmt -lpsl
CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include

all:		img2mercgrd

install:	all
		$(INSTALL) img2mercgrd$(EXE) $(bindir)
		$(INSTALL) img2grd $(bindir)

uninstall:
		\rm -f $(bindir)/img2mercgrd$(EXE) $(bindir)/img2grd

img2mercgrd:	img2mercgrd.o gmt_imgsubs.o $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) img2mercgrd.o gmt_imgsubs.o $(GMT) $(CDF) $(LIBS) -o img2mercgrd

spotless:	clean

clean:	
		rm -f *.o img2mercgrd$(EXE)

.c.o:		gmt_imgsubs.h $(GMT_H)
		$(CC) -c $(CFLAGS) $<
