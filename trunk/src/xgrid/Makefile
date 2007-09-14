#		$Id: Makefile,v 1.1 2007-09-14 23:25:23 remko Exp $
#
#	Makefile for xgrid directory
#	X Windows application xgridedit
#	Contributed by Hugh Fisher, 1992
#
#	Brought under autoconf by Lloyd Parkes, May-2000.

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR	= ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

OBJS= 	xgrid_utility.o xgrid_Xutility.o xgrid_Canvas.o \
	xgrid_Panner.o xgrid_GMTgrid.o xgrid_controls.o \
	xgrid_messages.o

EDITOBJS= xGridEdit.o xgrid_view.o xgrid_textView.o xgrid_textInput.o

#	Location of GMT and CDF header files and all libraries

CFLAGS=	$(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include $(XCFLAGS)
ALL_LIBS=$(X_LIBS) -L.. -L$(libdir) -lgmt -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

all:		xgridedit$(EXE)

install:	all
		$(INSTALL) xgridedit$(EXE) $(bindir)

uninstall:
		\rm -f $(bindir)/xgridedit$(EXE)

spotless:	clean
		\rm -f makefile config.cache config.log config.status

clean:
		rm -f *.o xgridedit$(EXE) .gmt*

xgridedit$(EXE):	$(EDITOBJS) $(OBJS) $(GMTLIB)
		$(CC) $(LDFLAGS) -o $@ $(EDITOBJS) $(OBJS) $(ALL_LIBS)

xgrid_utility.o:	xgrid_utility.h

xgrid_Xutility.o:	xgrid_utility.h xgrid_Xutility.h

xgrid_Canvas.o:		xgrid_Canvas.h xgrid_CanvasP.h

xgrid_Panner.o:		xgrid_Canvas.h xgrid_Panner.h xgrid_PannerP.h

xgrid_GMTgrid.o:	xgrid_utility.h xgrid_GMTgrid.h $(GMT_H)

xgrid_controls.o:	xgrid_utility.h xgrid_Xutility.h xgrid_controls.h

xgrid_messages.o:	xgrid_utility.h xgrid_Xutility.h xgrid_messages.h

#	xGridEdit

xGridEdit.o:		xGridEdit.c xgrid_utility.h xgrid_controls.h \
			xgrid_messages.h xgrid_view.h xgrid_textInput.h $(GMT_H)

xgrid_textView.o:	xgrid_utility.h xgrid_Xutility.h xgrid_GMTgrid.h \
			xgrid_Canvas.h xgrid_Panner.h xgrid_textView.h $(GMT_H)

xgrid_textInput.o:	xgrid_utility.h xgrid_Xutility.h xgrid_GMTgrid.h \
			xgrid_textInput.h $(GMT_H)

xgrid_view.o:		xgrid_utility.h xgrid_Xutility.h xgrid_GMTgrid.h \
			xgrid_view.h $(GMT_H)
