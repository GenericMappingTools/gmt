#		$Id: Makefile,v 1.17 2011-03-25 14:36:18 remko Exp $
#
#	Makefile for gmt <--> octave/matlab interface programs
#	Use with GMT 5
#
# Note that you must have write permission to mkdir and write to $MEX_XDIR
# 
#----------------------------------------------------------------------------
#	The rest should should be ok. NO EDITING BEYOND HERE, unless your
#	Octave/Matlab setup was not auto-detected by the configure file
#	If so, please let us know what you needed to change.
#----------------------------------------------------------------------------

GMTSRCDIR	= ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

.SUFFIXES:	.$(MEX_EXT)

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(MEX_INC) $(GDAL_INC)
ALLLIB     	= $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(MEX_LIB)
FLAGS		= -I$(srcdir) $(MEX_INC) $(NETCDF_INC) $(GDAL_INC)
# PROGS_C are source files provided by us
PROGS_C		= blockmean.c grdinfo.c grdread.c grdwrite.c surface.c nearneighbor.c psxy.c pscoast.c \
		  grdclip.c grdcut.c grdedit.c grdfft.c grdfilter.c grdmath.c grdpaste.c grdproject.c grdsample.c grdtrend.c
PROGS_MEX	= $(PROGS_C:.c=.$(MEX_EXT))

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS_MEX)

install:	all
		$(INSTALL_DIR) $(MEX_XDIR)
		if [ -w $(MEX_XDIR) ] ; then \
			$(INSTALL) $(PROGS_MEX) $(MEX_XDIR); \
		else \
			$(INSTALL) $(PROGS_MEX) $(bindir); \
		fi

uninstall:
		if [ -w $(MEX_XDIR) ] ; then \
			cd $(MEX_XDIR); \
		else \
			cd $(bindir); \
		fi; rm -f $(PROGS_MEX)

install-man uninstall-man manpages:	;

spotless::	clean

clean:
		rm -f .gmt* *% *.o .mexrc.sh $(PROGS_MEX)

gmt_mex.o:	gmt_mex.c $(GMT_H)
	$(MEX_BLD) $(FLAGS) -c gmt_mex.c

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

# Special way to create some mex program objects
$(PROGS_MEX):	$(PROGS_C) gmt_mex.o $(GMT_H) $(LIBGMT)
	$(MEX_BLD) $(FLAGS) $(@:.$(MEX_EXT)=).c gmt_mex.o $(ALLLIB)
