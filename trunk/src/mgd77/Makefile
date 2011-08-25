#	$Id$
#
#		Makefile for GMT mgd77 supplements
#
#	The mgd77 supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Authors:	Paul Wessel & Mike Chandler, SOEST, U. of Hawaii
#
#	Date:	20-JUN-2008
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

API_O		= gmt_mgd77.o
API_H		= $(API_O:.o=.h)
FUNCS_O		= mgd77convert_func.o mgd77info_func.o mgd77list_func.o mgd77path_func.o \
		  mgd77manage_func.o mgd77magref_func.o mgd77sniffer_func.o
FUNCSPS_O	= mgd77track_func.o
LIBMGD77_API_H	= gmt_mgd77.h
LIB_H		= mgd77.h mgd77_IGF_coeffs.h mgd77_init.h mgd77_recalc.h mgd77magref.h mgd77defaults.h mgd77_functions.h cm4_functions.h
LIB_O		= mgd77.o

PROGS_H		= mgd77_IGF_coeffs.h mgd77sniffer.h mgd77snifferdefaults.h mgd77magref.h mgd77_codes.h mgd77_e77.h
PROGS_O		= $(FUNCS_O:_func.o=.o)
PROGSPS_O	= $(FUNCSPS_O:_func.o=.o)
MAN1		= $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS) $(PROGSPS)

install:	all
		$(INSTALL) $(PROGS) $(PROGSPS) $(bindir)
		if [ ! . -ef $(includedir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(LIB_H) $(API_H) $(includedir)/gmt; \
		fi
		if [ ! . -ef $(libdir) ]; then \
			$(INSTALL_DIR) $(libdir); \
			$(INSTALL_DATA) $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBAPI) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			mv -f $(LIBAPI:.$(LIBEXT)=).{$(SL),$(SL_VERSION)} ; \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).{$(SL_VERSION),$(SL)} ; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)
		if [ ! . -ef $(includedir) ]; then \
			cd $(includedir)/gmt; \rm -f $(LIB_H) $(API_H);\
		fi
		if [ ! . -ef $(libdir) ]; then \
			cd $(libdir); \rm -f $(LIBAPI) $(LIBAPI:.$(LIBEXT)=.a); \
		fi

install-man:	$(MAN1)
		$(INSTALL_DATA) $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:
		rm -f *% *.o *.a *.$(SL) $(PROGS) $(PROGSPS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(FUNCS_O)  $(FUNCSPS_O) $(PROGS_O) $(PROGSPS_O):	$(LIB_H) $(PROGS_H) $(GMT_H) $(PS_H) $(API_H)
$(LIB_O):	$(LIB_H) $(GMT_H) cm4_functions.c

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_mgd77.$(LIBEXT)

libs:		libgmt_mgd77.a

libgmt_mgd77.a:	$(FUNCS_O) $(FUNCSPS_O) $(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_mgd77.$(SL):	libgmt_mgd77.a
		$(LD) $(LD_OPT) $(FUNCS_O) $(FUNCSPS_O) $(LIB_O) $(GMT_LIB) $(NETCDF_LIB) $(LIBS) -o $@


#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------
mgd77convert$(EXE):	mgd77convert.o
mgd77info$(EXE):	mgd77info.o
mgd77list$(EXE):	mgd77list.o
mgd77path$(EXE):	mgd77path.o
mgd77manage$(EXE):	mgd77manage.o
mgd77sniffer$(EXE):	mgd77sniffer.o
mgd77magref$(EXE):	mgd77magref.o
mgd77track$(EXE):	mgd77track.o

$(PROGS):	$(LIBGMT) $(LIBAPI) $(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgd77 $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(PCRE_LIB) $(LIBS) $(LIBMGD77_API) -o $@

$(PROGSPS):	$(LIBGMT) $(LIBAPI) $(PROGSPS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgd77 $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(PCRE_LIB) $(LIBS) $(LIBMGD77_API) -o $@

$(PROGS_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c

$(PROGSPS_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMTPSL -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c
