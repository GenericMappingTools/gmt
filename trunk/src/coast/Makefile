#	$Id: Makefile,v 1.81 2011-04-14 22:18:30 guru Exp $
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
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)
DEFINES		= -DDVER=\"$(GSHHS_DATA_VERSION)\" -DYEAR=`date +%Y`
PREV_VERSION	= 1.10
# These are the values to use for the next GMT version 4.5.1
VERSION			= 4.4
GSHHS_DATA_VERSION	= 2.0.1
GSHHS_PROG_VERSION	= 1.12

# Douglas-Peucker parameters:
DP_h 		= 0.2
DP_i		= 1
DP_l		= 5
DP_c		= 25
# Bin size in degrees:
BIN_f		= 1
BIN_h		= 2
BIN_i		= 5
BIN_l		= 10
BIN_c		= 20
# Get dependencies for data files:
include coast.dep

PROGS_H	= shore.h wvs.h

SUBS_O	= poly_check_subs.o string_check_subs.o douglas_peucker.o coast_io.o poly_misc_subs.o
PROGS_O	= border_to_bins.o ciaman_fix.o line_shrink.o linemaker.o swap.o polygon_close.o  \
	  lines_to_bins.o man_fix.o man_fix2.o polygon_bincount.o gshhs_to_polygon.o polygon_to_shape.o \
	  polygon_checkarea.o polygon_consistency.o polygon_deldups.o polygon_compare.o \
	  polygon_dump.o polygon_extract.o polygon_extract2.o polygon_fixnegwesn.o polygon_setarea.o \
	  polygon_extract_all.o polygon_final_info.o polygon_findarea.o polygon_findlevel.o \
	  polygon_fix.o polygon_fixlevel.o polygon_get.o polygon_rect.o polygon_sync.o \
	  polygon_id.o polygon_merge.o polygon_report.o polygon_restore.o  segment_restore.o \
	  polygon_set.o polygon_setnodes.o polygon_setwesn.o polygon_shrink.o polygon_match.o\
	  polygon_sort.o polygon_stats.o polygon_to_bins.o polygon_to_gshhs.o polygon_hierarchy.o \
	  polygon_update.o polygon_xover.o read_wvs.o river_fix.o segment_clean.o \
	  segment_connect.o segment_dump.o segment_final_dump.o segment_report.o \
	  shoremaker.o wvs_crosscheck.o wvs_segment_dump.o wvs_segment_restore2.o

LIB	= libcoast.a

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS)

clean:
		\rm -f *.o *% $(PROGS)

spotless:	clean
		\rm -f $(LIB)

# Data targets:

data:		full high int low crude

info:		info_f info_h info_i info_l info_c

set_new:
		cp -f res_?/binned_*_?.cdf $(rootdir)/share/coast
		
set_old:
		cd $(rootdir); tar xvjf ftp/GSHHS$(PREV_VERSION)_coast.tar.bz2
		cd $(rootdir); tar xvjf ftp/GSHHS$(PREV_VERSION)_high.tar.bz2
		cd $(rootdir); tar xvjf ftp/GSHHS$(PREV_VERSION)_full.tar.bz2
		
delete:		
		\rm res_?/binned_*_?.bin res_?/binned_*_?.seg res_?/binned_*_?.pt

build-gshhs:
		mkdir -p gshhs
		polygon_to_gshhs res_f/v${VERSION}_final_f_dbase.b > gshhs/gshhs_f.b
		polygon_to_gshhs res_h/v${VERSION}_final_h_dbase.b > gshhs/gshhs_h.b
		polygon_to_gshhs res_i/v${VERSION}_final_i_dbase.b > gshhs/gshhs_i.b
		polygon_to_gshhs res_l/v${VERSION}_final_l_dbase.b > gshhs/gshhs_l.b
		polygon_to_gshhs res_c/v${VERSION}_final_c_dbase.b > gshhs/gshhs_c.b
		polygon_to_gshhs -l wdb/wdb_f_borders.b > gshhs/wdb_borders_f.b
		polygon_to_gshhs -l wdb/wdb_h_borders.b > gshhs/wdb_borders_h.b
		polygon_to_gshhs -l wdb/wdb_i_borders.b > gshhs/wdb_borders_i.b
		polygon_to_gshhs -l wdb/wdb_l_borders.b > gshhs/wdb_borders_l.b
		polygon_to_gshhs -l wdb/wdb_c_borders.b > gshhs/wdb_borders_c.b
		polygon_to_gshhs -l wdb/wdb_f_rivers.b > gshhs/wdb_rivers_f.b
		polygon_to_gshhs -l wdb/wdb_h_rivers.b > gshhs/wdb_rivers_h.b
		polygon_to_gshhs -l wdb/wdb_i_rivers.b > gshhs/wdb_rivers_i.b
		polygon_to_gshhs -l wdb/wdb_l_rivers.b > gshhs/wdb_rivers_l.b
		polygon_to_gshhs -l wdb/wdb_c_rivers.b > gshhs/wdb_rivers_c.b

shape-gshhs:
		mkdir -p GSHHS_shp GSHHS_shp/f GSHHS_shp/h GSHHS_shp/i GSHHS_shp/l GSHHS_shp/c
		polygon_to_shape res_f/v${VERSION}_final_f_dbase.b GSHHS_f
		polygon_to_shape res_h/v${VERSION}_final_h_dbase.b GSHHS_h
		polygon_to_shape res_i/v${VERSION}_final_i_dbase.b GSHHS_i
		polygon_to_shape res_l/v${VERSION}_final_l_dbase.b GSHHS_l
		polygon_to_shape res_c/v${VERSION}_final_c_dbase.b GSHHS_c
		mv GSHHS_f_L[1-4]/* GSHHS_shp/f
		mv GSHHS_h_L[1-4]/* GSHHS_shp/h
		mv GSHHS_i_L[1-4]/* GSHHS_shp/i
		mv GSHHS_l_L[1-4]/* GSHHS_shp/l
		mv GSHHS_c_L[1-4]/* GSHHS_shp/c
		cp GSHHS+WDBII/README.TXT GSHHS_shp
		zip -r -q -v -9 GSHHS_shp_$(GSHHS_DATA_VERSION).zip GSHHS_shp/*
		rm -rf GSHHS_[fhilc]_L[1-4] GSHHS_[fhilc]_L[1-4].gmt

# Make archive targets:

tar-gshhs:
		cp GSHHS+WDBII/README.TXT gshhs
		tar cvjf gshhs_$(GSHHS_DATA_VERSION).tbz gshhs/*_[clihf].b gshhs/README.TXT
		rm -f gshhs/README.TXT
		cd $(rootdir)/src; \
		tar cvjf gshhs_$(GSHHS_PROG_VERSION)_src.tbz gshhs/{gshhs.c,gshhs.h,gshhs_dp.c,gshhstograss.c,makefile,README.gshhs}

zip-gshhs:
		cp GSHHS+WDBII/README.TXT gshhs
		zip -r -9 -q gshhs_$(GSHHS_DATA_VERSION).zip gshhs/*_[clihf].b gshhs/README.TXT
		rm -f gshhs/README.TXT
		cd $(rootdir)/src; \
		zip -r -9 -q -l gshhs_$(GSHHS_PROG_VERSION)_src.zip gshhs/{gshhs.c,gshhs.h,gshhs_dp.c,gshhstograss.c,makefile,README.gshhs}
		mv $(rootdir)/src/gshhs_$(GSHHS_PROG_VERSION)_src.zip .

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PROGS_H)
coast.o:	$(PROGS_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

lib:		$(LIB)

$(LIB):		$(SUBS_O)
		$(AR) cvr $@ $?
		$(RANLIB) $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

$(PROGS):	$(LIB) $(PROGS_O) $(LIBGMT)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lcoast $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
