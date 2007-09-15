#		$Id: Makefile,v 1.2 2007-09-15 04:32:22 remko Exp $
#
#	Makefile for xgrid directory
#	X Windows application xgridedit
#	Contributed by Hugh Fisher, 1992
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR	= ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

OBJS1= 	xgrid_utility.o xgrid_Xutility.o xgrid_Canvas.o \
	xgrid_Panner.o xgrid_controls.o xgrid_messages.o
OBJS2=	xgrid_GMTgrid.o xgrid_view.o xgrid_textView.o xgrid_textInput.o

HDRS=	xgrid_Canvas.h xgrid_CanvasP.h xgrid_GMTgrid.h xgrid_Panner.h xgrid_PannerP.h \
	xgrid_Xutility.h xgrid_controls.h xgrid_grid.h xgrid_messages.h xgrid_textInput.h \
	xgrid_textView.h xgrid_utility.h xgrid_view.h

PROGS_C	= xGridEdit.c
PROGS_O	= $(PROGS_C:.c=.o)
PROGS	= xgridedit$(EXE)

#	Location of GMT and CDF header files and all libraries

CFLAGS=	$(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include $(XCFLAGS)
ALL_LIBS=$(X_LIBS) -L.. -L$(libdir) -lgmt -L$(NETCDF)/lib -lnetcdf $(LIBS)

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		\rm -f $(bindir)/$(PROGS)

spotless:	clean

clean:
		\rm -f *.o $(PROGS) .gmt*

#	Dependencies of objects on header files (xgrid and GMT)

$(PROGS_O) $(OBJS1) $(OBJS2):	$(HDRS)
$(PROGS_O) $(OBJS2):		$(GMT_H)

#	Building executables

$(PROGS):	$(PROGS_O) $(OBJS1) $(OBJS2) $(GMTLIB)
		$(CC) $(LDFLAGS) -o $@ $(PROGS_O) $(OBJS1) $(OBJS2) $(ALL_LIBS)
