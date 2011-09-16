#	$Id$
#
#
#	makefile for imgsrc directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

API_H		= gmt_img.h
LIB_O		= gmt_imgsubs.o
PROGS_O		= img2grd.o
LIB_H		= gmt_imgsubs.h

MAN1		= $(PROGS_O:.o=.1) img2google.1

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) img2google $(bindir)
		if [ ! . -ef $(includedir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(API_H) $(includedir)/gmt; \
		fi
		if [ ! . -ef $(libdir) ]; then \
			$(INSTALL_DIR) $(libdir) ; \
			$(INSTALL_DATA) $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBAPI) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f $(LIBAPI:.$(LIBEXT)=).{$(SL),$(SL_VERSION)}; \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).{$(SL_VERSION),$(SL)}; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS) img2google
		if [ ! .. -ef $(includedir) ]; then \
			cd $(includedir)/gmt; \rm -f $(API_H);\
		fi
		if [ ! .. -ef $(libdir) ]; then \
			cd $(libdir); \rm -f $(LIBAPI:.$(LIBEXT)=).{a,$(SL),$(SL_VERSION)}; \
		fi

install-man:	$(MAN1)
		$(INSTALL_DATA) $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:	
		\rm -f *.o *.a *.$(SL) *% $(PROGS)

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_img.$(LIBEXT)

libs:		libgmt_img.a

libgmt_img.a:	$(PROGS_O) $(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_img.$(SL):	libgmt_img.a
		$(LD) $(LD_OPT) $(PROGS_O) $(LIB_O) $(GMT_LIB) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(API_H) $(LIB_H)
$(LIB_O):	$(LIB_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

img2grd$(EXE):	img2grd.o

$(PROGS):	$(LIBGMT) $(LIBAPI) ../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$(@:.exe=) ../gmtprogram.c \
		$(LDFLAGS) $(LIBAPI) $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(PCRE_LIB) $(LIBS) -o $@
