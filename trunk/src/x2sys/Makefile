#	$Id: Makefile,v 1.8 2004-06-10 02:57:44 pwessel Exp $
#
# makefile for x2sys directory
#
include ../makegmt.macros

#------------------------- stop here --------------------------

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I../mgg -I$(NETCDF)/include
ALLLIB	= -L../mgg -lgmt_mgg -L.. -L$(libdir) -lgmt -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

XSYSTEM_H	= x2sys.h
XSYSTEM_C	= x2sys_bin.c x2sys_binlist.c x2sys_datalist.c x2sys_cross.c

XSYSTEM_O	= ${XSYSTEM_C:.c=.o}

XSYSTEM		= ${XSYSTEM_C:.c=}

#---------------------------------------------------------------

libx2sys.a:		x2sys.o
			$(AR) cvr libx2sys.a $?
			$(RANLIB) libx2sys.a

all:			$(XSYSTEM) libx2sys.a

install:		all
			for i in $(XSYSTEM); do \
				$(INSTALL) $$i$(EXE) $(bindir); \
			done
			if [ ! -d $(libdir) ]; then \
				mkdir -p $(libdir); \
			fi
			if [ ! $(libdir) = $(srcdir) ]; then \
				$(INSTALL) -m 444 libx2sys.a $(libdir); \
				if [ -f libx2sys.$(SL) ]; then \
					$(INSTALL) -m 444 libx2sys.$(SL) $(libdir); \
				fi; \
			fi
			if [ ! -d $(datadir)/x2sys ]; then \
				mkdir -p $(datadir)/x2sys; \
			fi
			for i in *.def; do \
				cp -f -p $$i $(datadir)/x2sys; \
			done

uninstall:	
			for i in $(XSYSTEM); do \
				\rm -f $(bindir)/$$i$(EXE); \
			done
			if [ ! $(libdir) = $(srcdir) ]; then \
				\rm -f $(libdir)/libx2sys.a; \
				if [ -f libx2sys.$(SL) ]; then \
					\rm -f $(libdir)/libx2sys.$(SL); \
				fi; \
			fi
			rm -rf $(datadir)/x2sys

clean:
			\rm -f *.o
			for i in $(XSYSTEM); do \
				\rm -f $$i$(EXE); \
			done

spotless:		clean
			\rm -f *.a

$(XSYSTEM):		$(XSYSTEM_O) libx2sys.a
			$(CC) $(CFLAGS) $@.o -o $@ -L. -lx2sys $(ALLLIB) $(LDFLAGS)

.c.o:
			$(CC) -c $(CFLAGS) $<
