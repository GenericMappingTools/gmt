#	$Id: Makefile,v 1.3 2004-09-06 02:37:43 pwessel Exp $
#
#		Makefile for GMT coast suite
#
#	The coast suite are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.  This suite is not
#	part of GMT but is used to prepare coastline data.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, NOAA
#
#	Date:	04-SEPT-2004
#
#	Note: Most of these tools were last used in 1995/96 and only
#	minor changes to make them compile have been performed.
#	Several are probably obsolete.
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
include ../makegmt.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf
DEGREES = 1

COAST_H	= shore.h wvs.h

COAST_C	= border_to_bins.c ciaman_fix.c line_shrink.c linemaker.c \
	  lines_to_bins.c man_fix.c man_fix2.c polygon_bincount.c \
	  polygon_checkarea.c polygon_consistency.c polygon_deldups.c \
	  polygon_dump.c polygon_extract.c polygon_extract2.c \
	  polygon_extract_all.c polygon_final_info.c polygon_findlevel.c \
	  polygon_findlevel2.c polygon_fix.c polygon_fixlevel.c polygon_get.c \
	  polygon_id.c polygon_merge.c polygon_report.c polygon_restore.c \
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

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

lib:		libcoast.a

libcoast.a:	poly_check_subs.o string_check_subs.o douglas_peucker.o coast_io.o
		$(AR) cvr libcoast.a $?
		$(RANLIB) libcoast.a

coast.o:	$(COAST_H)

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(COAST):	libcoast.a $(COAST_O)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L. -lcoast -L.. -L$(libdir) -lgmt -lpsl $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) -DDEGREES=${DEGREES} $<
