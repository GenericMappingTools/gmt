#	$Id: Makefile,v 1.4 2001-04-11 19:58:09 pwessel Exp $
#
# makefile for x2sys directory
#
include ../makegmt.macros

#------------------------- stop here --------------------------

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I../mgg -I$(NETCDF)/include
ALLLIB	= -L.. -L$(libdir) -lgmt -L../mgg -lgmt_mgg -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

XSYSTEM_H	= x2sys.h
XSYSTEM_C	= x2sys_datalist.c x2sys_cross.c

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
