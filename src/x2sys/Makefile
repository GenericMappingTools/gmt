#	$Id$
#
#		Makefile for GMT x2sys supplements
#
#	The x2sys supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) -I. -I../mgd77 $(NETCDF_INC) $(GDAL_INC)

PROGS_O		= x2sys_init.o x2sys_put.o x2sys_get.o x2sys_binlist.o x2sys_datalist.o \
		  x2sys_cross.o x2sys_list.o x2sys_report.o x2sys_solve.o x2sys_merge.o
MAN1		= $(PROGS_O:.o=.1)

API_H		= gmt_x2sys.h
LIB_O		= x2sys.o
LIB_H		= x2sys.h

#---------------------------------------------------------------
#	software targets
#---------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! .. -ef $(includedir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(LIB_H) $(API_H) $(includedir)/gmt; \
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
			cd $(includedir)/gmt; \rm -f $(LIB_H) $(API_H);\
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
		\rm -f *% *.o *.a *.$(SL) $(PROGS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(LIB_H) $(GMT_H) $(API_H)
x2sys.o:	$(LIB_H) $(GMT_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_x2sys.$(LIBEXT)

libs:		libgmt_x2sys.a

libgmt_x2sys.a:	$(PROGS_O) $(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_x2sys.$(SL):	libgmt_x2sys.a
		$(LD) $(LD_OPT) $(PROGS_O) $(LIB_O) $(GMT_LIB) -L../mgd77 -lgmt_mgd77 $(LIBS) -o $@


#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

x2sys_init$(EXE):	x2sys_init.o
x2sys_put$(EXE):	x2sys_put.o
x2sys_get$(EXE):	x2sys_get.o
x2sys_binlist$(EXE):	x2sys_binlist.o
x2sys_datalist$(EXE):	x2sys_datalist.o
x2sys_cross$(EXE):	x2sys_cross.o
x2sys_list$(EXE):	x2sys_list.o
x2sys_report$(EXE):	x2sys_report.o
x2sys_solve$(EXE):	x2sys_solve.o
x2sys_merge$(EXE):	x2sys_merge.o

$(PROGS):	$(LIBGMT) $(LIBAPI) ../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$@ ../gmtprogram.c \
		$(LDFLAGS) $(LIBAPI) -L../mgd77 -lgmt_mgd77 $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(PCRE_LIB) $(LIBS) -o $@
