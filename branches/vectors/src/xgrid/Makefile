#		$Id$
#
#	Makefile for xgrid directory
#	X Windows application xgridedit
#	Contributed by Hugh Fisher, 1992
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR	= ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(X_CFLAGS) $(NETCDF_INC) $(GDAL_INC)

OBJS1		= xgrid_utility.o xgrid_Xutility.o xgrid_Canvas.o \
		xgrid_Panner.o xgrid_controls.o xgrid_messages.o
OBJS2		= xgrid_GMTgrid.o xgrid_view.o xgrid_textView.o xgrid_textInput.o

PROGS_H		= xgrid_Canvas.h xgrid_CanvasP.h xgrid_GMTgrid.h xgrid_Panner.h xgrid_PannerP.h \
		xgrid_Xutility.h xgrid_controls.h xgrid_grid.h xgrid_messages.h xgrid_textInput.h \
		xgrid_textView.h xgrid_utility.h xgrid_view.h
PROGS_O		= xGridEdit.o
PROGS		= xgridedit$(EXE)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS)

install-man uninstall-man manpages:	;

spotless:	clean

clean:
		\rm -f *.o *% $(PROGS) .gmt*

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(OBJS1):		$(PROGS_H)
$(PROGS_O) $(OBJS2):	$(PROGS_H) $(GMT_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

xgridedit$(EXE):	xGridEdit.o

$(PROGS):	$(OBJS1) $(OBJS2) $(LIBGMT)
		$(CC) $(LDFLAGS) -o $@ $(PROGS_O) $(OBJS1) $(OBJS2) $(X_LIBS) $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(PCRE_LIB) $(LIBS)
