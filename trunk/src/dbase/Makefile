#	$Id: Makefile,v 1.27 2011-03-25 14:36:17 remko Exp $
#
#	makefile for dbase directory
#
# Uncomment SWAP to build in byte-order swapping of all data
#SWAP	= -DSWAP_ME
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

DEFINES		= $(SWAP)
INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

API_O		= gmt_dbase.o
API_H		= $(API_O:.o=.h)
FUNCS_O		= grdraster_func.o

PROGS_O		= $(FUNCS_O:_func.o=.o)
MAN1		= $(PROGS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL_DIR) $(libdir) ; \
			$(INSTALL_DATA) $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBAPI) $(libdir); fi \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(API_H) $(includedir)/gmt; \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f $(LIBAPI:.$(LIBEXT)=).$(SL) $(LIBAPI:.$(LIBEXT)=).$(SL_VERSION); \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).$(SL_VERSION) $(LIBAPI:.$(LIBEXT)=).$(SL); \
		fi

uninstall:
		cd $(bindir); rm -f $(PROGS)
		cd $(includedir)/gmt; rm -f $(API_H)

install-man:	$(MAN1)
		$(INSTALL_DATA) $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; rm -f $(MAN1)

spotless::	clean

clean:	
		\rm -f *.o *.a *.$(SL) *% $(PROGS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(FUNCS_O) $(PROGS_O):	$(GMT_H) $(API_H)

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_dbase.$(LIBEXT)

libs:		libgmt_dbase.a

libgmt_dbase.a:	$(FUNCS_O) $(API_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_dbase.$(SL):	libgmt_dbase.a
		$(LD) $(LD_OPT) $(FUNCS_O) $(API_O) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

grdraster$(EXE):	grdraster.o

$(PROGS):	$(LIBGMT) $(LIBAPI) $(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) $(LIBAPI) -o $@
		$(COMPRESS) $@

$(PROGS_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c
