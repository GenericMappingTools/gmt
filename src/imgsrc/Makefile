
#
#	makefile for imgsrc directory
#
#----------------- stop here - nothing more to edit -------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CDF		= -L$(NETCDF)/lib -lnetcdf
GMT		= -L.. -L$(libdir) -lgmt
CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include

PROGS_C		= img2mercgrd.c
PROGS_O		= $(PROGS_C:.c=.o)
PROGS		= $(PROGS_C:.c=$(EXE))

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

all:		$(PROGS)

$(PROGS_O):	$(GMT_H) gmt_imgsubs.h

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

install:	all
		$(INSTALL) $(PROGS) img2grd $(bindir)

uninstall:
		\rm -f $(bindir)/$(PROGS) $(bindir)/img2grd

img2mercgrd:	img2mercgrd.o gmt_imgsubs.o $(GMTLIB)
		$(CC) $(LDFLAGS) img2mercgrd.o gmt_imgsubs.o $(GMT) $(CDF) $(LIBS) -o $@

spotless:	clean

clean:	
		rm -f *.o $(PROGS)

.c.o:		gmt_imgsubs.h $(GMT_H)
		$(CC) -c $(CFLAGS) $<
