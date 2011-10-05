#	$Id$
#
# makefile for segyprogs directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

API_H		= gmt_segy.h
PROGS_O		= pssegy.o pssegyz.o segy2grd.o
LIB_O		= segy_io.o
LIB_H		= segy_io.h segy.h segyreel.h

MAN1		= $(PROGS_O:.o=.1)

#---------------------------------------------------------------
#	software targets
#---------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! .. -ef $(includedir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(API_H) $(LIB_H) $(includedir)/gmt; \
		fi
		if [ ! .. -ef $(libdir) ]; then \
			$(INSTALL_DIR) $(libdir); \
			$(INSTALL_DATA) $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBAPI) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f $(LIBAPI:.$(LIBEXT)=).{$(SL),$(SL_VERSION)}; \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).{$(SL_VERSION),$(SL)}; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS)
		if [ ! .. -ef $(includedir) ]; then \
			cd $(includedir)/gmt; \rm -f $(API_H) $(LIB_H);\
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
		\rm -rf *.dSYM

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(LIB_H) $(GMT_H) $(PS_H) $(API_H)
$(LIB_O):	$(LIB_H) $(GMT_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_segy.$(LIBEXT)

libs:		libgmt_segy.a

libgmt_segy.a:	$(PROGS_O) $(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_segy.$(SL):	libgmt_segy.a
		$(LD) $(LD_OPT) $(PROGS_O) $(LIB_O) $(GMT_LIB) $(LIBS) -o $@


#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

pssegyz$(EXE):	pssegyz.o
pssegy$(EXE):	pssegy.o
segy2grd$(EXE):	segy2grd.o

$(PROGS):	$(LIBGMT) $(LIBAPI) ../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$(@:.exe=) ../gmtprogram.c \
		$(LDFLAGS) -L. -lgmt_segy $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(PCRE_LIB) $(LIBS) $(LIBAPI) -o $@
