#	$Id: Makefile,v 1.26 2007-08-27 20:50:57 guru Exp $
#
#		Makefile for GMT coast suite
#
#	The coast suite are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.  This suite is not
#	part of GMT but is used to prepare coastline data.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#	Note that to work with GSHHS Master Files (which are in CVS) you
#	must also check out the GSHHS_master supplement from the coast dir.
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, NOAA
#
#	Date:	27-AUG-2007
#
#	Note: Most of these tools were last used in 1995/96 and only
#	minor changes to make them compile have been performed.
#	Several are probably obsolete.
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf
VERSION = 4.2
GSHHS_DATA_VERSION	= 1.5
GSHHS_PROG_VERSION	= 1.8

# Douglas-Peucker parameters:
DP_h 	= 0.2
DP_i		= 1
DP_l		= 5
DP_c	= 25
# Bin size in degrees:
BIN_f		= 1
BIN_h		= 2
BIN_i		= 5
BIN_l		= 10
BIN_c		= 20
# Get dependencies for data files:
include coast.dep

COAST_H	= shore.h wvs.h

COAST_C	= border_to_bins.c ciaman_fix.c line_shrink.c linemaker.c swap.c \
	  lines_to_bins.c man_fix.c man_fix2.c polygon_bincount.c gshhs_to_polygon.c \
	  polygon_checkarea.c polygon_consistency.c polygon_deldups.c polygon_compare.c \
	  polygon_dump.c polygon_extract.c polygon_extract2.c polygon_fixnegwesn.c \
	  polygon_extract_all.c polygon_final_info.c polygon_findarea.c polygon_findlevel.c \
	  polygon_findlevel2.c polygon_fix.c polygon_fixlevel.c polygon_get.c \
	  polygon_id.c polygon_merge.c polygon_report.c polygon_restore.c  segment_restore.c \
	  polygon_set.c polygon_setnodes.c polygon_setwesn.c polygon_shrink.c \
	  polygon_sort.c polygon_stats.c polygon_to_bins.c polygon_to_gshhs.c \
	  polygon_update.c polygon_xover.c read_wvs.c river_fix.c segment_clean.c \
	  segment_connect.c segment_dump.c segment_final_dump.c segment_report.c \
	  shoremaker.c wvs_crosscheck.c wvs_segment_dump.c wvs_segment_restore2.c

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

COAST_O	= ${COAST_C:.c=.o}

COAST	= ${COAST_C:.c=}

all:		libcoast.a $(COAST)

$(COAST_O):	$(GMT_H) $(COAST_H)

#-------------------------------------------------------------------------------
# Software targets:

install:	all
		for i in $(COAST); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

uninstall:
		for i in $(COAST); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done

clean:
		rm -f *.o
		for i in $(COAST); do \
			rm -f $$i$(EXE); \
		done

spotless:	clean
		rm -f *.a

# Data targets:

data:		full high int low crude

set_new:
		cp -f res_?/binned_GSHHS_?.cdf $(rootdir)/share/coast
		cp -f res_?/binned_border_?.cdf $(rootdir)/share/coast
		
set_old:
		cd $(rootdir); tar xvjf ftp/GMT$(VERSION)_coast.tar.bz2
		cd $(rootdir); tar xvjf ftp/GMT$(VERSION)_high.tar.bz2
		cd $(rootdir); tar xvjf ftp/GMT$(VERSION)_full.tar.bz2
		
delete:		
		rm res_?/binned_*_?.bin res_?/binned_*_?.seg res_?/binned_*_?.pt

gshhs:
		polygon_to_gshhs res_f/v${VERSION}_final_f_dbase.b > gshhs_f.b
		polygon_to_gshhs res_h/v${VERSION}_final_h_dbase.b > gshhs_h.b
		polygon_to_gshhs res_i/v${VERSION}_final_i_dbase.b > gshhs_i.b
		polygon_to_gshhs res_l/v${VERSION}_final_l_dbase.b > gshhs_l.b
		polygon_to_gshhs res_c/v${VERSION}_final_c_dbase.b > gshhs_c.b

# Make archive targets:

tar-gshhs:
		tar cvjf gshhs_$(GSHHS_DATA_VERSION).tbz gshhs_[clihf].b
		cd $(rootdir)/src; \
		tar cvjf gshhs_$(GSHHS_PROG_VERSION)_src.tbz gshhs/{gshhs.c,gshhs.h,gshhs_dp.c,gshhstograss.c,makefile,README.gshhs}

zip-gshhs:
		zip -r -9 -q gshhs_$(GSHHS_DATA_VERSION).zip gshhs_[clihf].b
		cd $(rootdir)/src; \
		zip -r -9 -q -l gshhs_$(GSHHS_PROG_VERSION)_src.zip gshhs/{gshhs.c,gshhs.h,gshhs_dp.c,gshhstograss.c,makefile,README.gshhs}

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

lib:		libcoast.a

libcoast.a:	poly_check_subs.o string_check_subs.o douglas_peucker.o coast_io.o poly_misc_subs.o
		$(AR) cvr libcoast.a $?
		$(RANLIB) libcoast.a

coast.o:	$(COAST_H)

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(COAST):	libcoast.a $(COAST_O) $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L. -lcoast -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) -DDVER=\"$(GSHHS_DATA_VERSION)\" -DYEAR=`date +%Y` $<
