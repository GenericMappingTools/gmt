#	$Id: Makefile,v 1.26 2007-09-04 15:22:27 remko Exp $
#
# makefile for x2sys directory
#

#------------------------- stop here --------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I../mgg -I../mgd77 -I$(NETCDF)/include
ALLLIB	= -L../mgd77 -lmgd77 -L../mgg -lgmt_mgg -L.. -L$(libdir) -lgmt -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

XSYSTEM_H	= x2sys.h
XSYSTEM_C	= x2sys_init.c x2sys_put.c x2sys_get.c x2sys_binlist.c x2sys_datalist.c x2sys_cross.c
XSYSTEM_O	= $(XSYSTEM_C:.c=.o)
XSYSTEM		= $(XSYSTEM_C:.c=$(EXE))

#---------------------------------------------------------------

all:			$(XSYSTEM) libx2sys.a

$(XSYSTEM_O):		$(GMT_H) $(XSYSTEM_H)

libx2sys.a:		x2sys.o
			$(AR) cvr libx2sys.a $?
			$(RANLIB) libx2sys.a

install:		all
			$(INSTALL) $(XSYSTEM) $(bindir); \
			if [ ! $(libdir) = $(srcdir) ]; then \
				$(INSTALL) -m 444 libx2sys.a $(libdir); \
				if [ -f libx2sys.$(SL) ]; then \
					$(INSTALL) -m 444 libx2sys.$(SL) $(libdir); \
				fi; \
			fi
			$(INSTALL) -d $(datadir)/x2sys
			for i in *.def; do \
				cp -f -p $$i $(datadir)/x2sys; \
			done

uninstall:	
			for i in $(XSYSTEM); do \
				\rm -f $(bindir)/$$i; \
			done
			if [ ! $(libdir) = $(srcdir) ]; then \
				\rm -f $(libdir)/libx2sys.a $(libdir)/libx2sys.$(SL); \
			fi
			rm -rf $(datadir)/x2sys

clean:
			\rm -f *.o $(XSYSTEM)

spotless:		clean
			\rm -f *.a

$(XSYSTEM):		$(XSYSTEM_O) libx2sys.a $(GMTLIB)
			$(CC) $(LDFLAGS) $(@:.exe=).o -o $@ -L. -lx2sys $(ALLLIB)
