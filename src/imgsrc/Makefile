#	$Id: Makefile,v 1.1 2001-03-02 05:54:44 pwessel Exp $
#
#	makefile for imgsrc directory
#
#----------------- stop here - nothing more to edit -------------------

include ../makegmt.macros

CDF		= -L$(NETCDF)/lib -lnetcdf
GMT		= -L.. -lgmt -lpsl
CFLAGS		= $(CC_OPT) $(WIN32) -I.. -I$(NETCDF)/include

all:		img2mercgrd

install:	all
		$(INSTALL) img2mercgrd$(EXE) $(bindir)
		$(INSTALL) img2grd $(bindir)

uninstall:
		\rm -f $(bindir)/img2mercgrd$(EXE) $(bindir)/img2grd

img2mercgrd:	img2mercgrd.o gmt_imgsubs.o
		$(CC) $(CFLAGS) $(LDFLAGS) img2mercgrd.o gmt_imgsubs.o $(GMT) $(CDF) $(LIBS) -o img2mercgrd

spotless:	clean

clean:	
		rm -f *.o img2mercgrd$(EXE)
.c.o:
		$(CC) -c $(CFLAGS) $<
