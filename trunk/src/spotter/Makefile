#	$Id$
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
PROGS_O		= backtracker.o grdpmodeler.o grdrotater.o grdspotter.o \
		  hotspotter.o originator.o rotconverter.o
API_H		= gmt_spotter.h
MAN1		= $(PROGS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! .. -ef $(includedir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(API_H) $(includedir)/gmt; \
		fi
		if [ ! .. -ef $(libdir) ]; then \
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
		if [ ! .. -ef $(includedir) ]; then \
			cd $(includedir)/gmt; \rm -f $(API_H);\
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); rm -f $(LIBAPI:.$(LIBEXT)=).{a,$(SL),$(SL_VERSION)}; \
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
$(PROGS_O):	$(GMT_H) $(API_H)

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_spotter.$(LIBEXT)

libs:		libgmt_spotter.a

libgmt_spotter.a:	$(PROGS_O) $(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_spotter.$(SL):	libgmt_spotter.a
		$(LD) $(LD_OPT) $(PROGS_O) $(LIB_O) $(GMT_LIB) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

backtracker$(EXE):	backtracker.o
grdpmodeler$(EXE):	grdpmodeler.o
grdrotater$(EXE):	grdrotater.o
grdspotter$(EXE):	grdspotter.o
hotspotter$(EXE):	hotspotter.o
originator$(EXE):	originator.o
rotconverter$(EXE):	rotconverter.o

$(PROGS):	$(LIBGMT) $(LIBAPI) ../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$@ ../gmtprogram.c \
		$(LDFLAGS) $(LIBAPI) $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(PCRE_LIB) $(LIBS) -o $@
