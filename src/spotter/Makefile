#	$Id: Makefile,v 1.35 2011-03-25 14:36:18 remko Exp $
#
#		Makefile for GMT 5 spotter supplements
#
#	The spotter supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	6-AUG-2010
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

LIB_H		= spotter.h
LIB_O		= libspotter.o
FUNCS_O		= backtracker_func.o grdpmodeler_func.o grdrotater_func.o grdspotter_func.o hotspotter_func.o \
		  originator_func.o rotator_func.o rotconverter_func.o rotextractor_func.o rotsmoother_func.o
API_O		= gmt_spotter.o
API_H		= $(API_O:.o=.h)
PROGS_O		= $(FUNCS_O:_func.o=.o)
MAN1		= $(PROGS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(API_H) $(includedir)/gmt; \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL_DIR) $(libdir); \
			$(INSTALL_DATA) $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBAPI) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f $(LIBAPI:.$(LIBEXT)=).{$(SL),$(SL_VERSION)} ; \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).{$(SL_VERSION),$(SL)} ; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS)
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir)/gmt; \rm -f $(API_H);\
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f libgmt_spotter.{a,$(SL),$(SL_VERSION)}; \
		fi

install-man:	$(MAN1)
		$(INSTALL_DATA) $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:
		\rm -f *.o *.a *.$(SL) *% $(PROGS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(LIB_O):	$(GMT_H) $(LIB_H)
$(FUNCS_O) $(PROGS_O):	$(GMT_H) $(API_H)

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_spotter.$(LIBEXT)

libs:		libgmt_spotter.a

libgmt_spotter.a:	$(FUNCS_O) $(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_spotter.$(SL):	libgmt_spotter.a
		$(LD) $(LD_OPT) $(FUNCS_O) $(LIB_O) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

backtracker$(EXE):	backtracker.o
grdpmodeler$(EXE):	grdpmodeler.o
grdrotater$(EXE):	grdrotater.o
grdspotter$(EXE):	grdspotter.o
hotspotter$(EXE):	hotspotter.o
originator$(EXE):	originator.o
rotator$(EXE):		rotator.o
rotconverter$(EXE):	rotconverter.o
rotsmoother$(EXE):	rotsmoother.o
rotextractor$(EXE):	rotextractor.o

$(PROGS):	$(LIBGMT) $(LIBAPI) $(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(LIBAPI) $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGS_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c
