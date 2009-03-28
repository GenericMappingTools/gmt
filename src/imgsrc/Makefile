#	$Id: Makefile,v 1.22 2009-03-28 22:01:49 guru Exp $
#
#
#	makefile for imgsrc directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

INCLUDES	= -I$(srcdir) -I$(NETCDF_INC)

PROGS_O		= img2mercgrd.o
MAN1		= $(PROGS_O:.o=.1) img2grd.1 img2google.1

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) img2grd img2google $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS) img2grd img2google

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

$(PROGS_O) gmt_imgsubs.o:	$(GMT_H) gmt_imgsubs.h

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

img2mercgrd$(EXE):	img2mercgrd.o gmt_imgsubs.o $(GMTLIB)
		$(CC) $(LDFLAGS) img2mercgrd.o gmt_imgsubs.o -L.. -lgmt $(NC_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@
