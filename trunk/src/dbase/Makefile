#	$Id: Makefile,v 1.6 2006-10-25 02:25:10 pwessel Exp $
#
#	makefile for dbase directory
#
# Uncomment SWAP to build in byte-order swapping of all data
#SWAP	= -DSWAP_ME
#----------------- stop here - nothing to edit ------------------

include ../makegmt.macros
GMTSRCDIR	= ../
include ../gmtalldeps.macros

CDF		= -L$(NETCDF)/lib -lnetcdf
CFLAGS		= $(CC_OPT) $(WIN32) $(SWAP) -I$(srcdir) -I$(NETCDF)/include

all:		grdraster

install:	all
		$(INSTALL) grdraster$(EXE) $(bindir)

uninstall:
		\rm -f $(bindir)/grdraster$(EXE)

grdraster:	grdraster.o $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) grdraster.o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o grdraster

spotless:	clean

clean:	
		rm -f *.o grdraster$(EXE)

.c.o:		$(GMT_H)
		$(CC) -c $(CFLAGS) $<
