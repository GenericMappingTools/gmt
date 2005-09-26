#	$Id: Makefile,v 1.9 2005-09-26 03:18:03 pwessel Exp $
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
include $(GMTHOME)/makegmt.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf
VERSION = v4.0

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

set_new:
		cp -f res_?/binned_GSHHS_?.cdf $(GMTHOME)/share
		
set_old:
		cd $(GMTHOME); tar xvjf  $(GMTHOME)/ftp/GMT_share.tar.bz2
		cd $(GMTHOME); tar xvjf  $(GMTHOME)/ftp/GMT_high.tar.bz2
		cd $(GMTHOME); tar xvjf  $(GMTHOME)/ftp/GMT_full.tar.bz2
		
shrinkb: 	shrinkb_h shrinkb_i shrinkb_l shrinkb_c

shrinkl: 	shrinkl_h shrinkl_i shrinkl_l shrinkl_c

shrinkr: 	shrinkr_h shrinkr_i shrinkr_l shrinkr_c

bin:		bin_f bin_h bin_i bin_l bin_c

nodes:		nodes_f nodes_h nodes_i nodes_l nodes_c

lines:		lines_f lines_h lines_i lines_l lines_c
		rm -f res_?/binned_border_?.bin
		rm -f res_?/binned_border_?.seg
		rm -f res_?/binned_border_?.pt

rivers: 	river_f river_h river_i river_l river_c
		rm -f res_?/binned_river_?.bin
		rm res_?/binned_river_?.seg
		rm res_?/binned_river_?.pt

shrinkb_h:	
		polygon_shrink res_f/${VERSION}_final_dbase.b 0.2 res_h/${VERSION}_final_dbase_0.2km.b
		
shrinkb_i:	
		polygon_shrink res_f/${VERSION}_final_dbase.b 1 res_i/${VERSION}_final_dbase_1km.b
		
shrinkb_l:	
		polygon_shrink res_f/${VERSION}_final_dbase.b 5 res_l/${VERSION}_final_dbase_5km.b
		
shrinkb_c:	
		polygon_shrink res_f/${VERSION}_final_dbase.b 25 res_c/${VERSION}_final_dbase_25km.b

shrinkl_h:	
		line_shrink wdb/wdb_borders.b 0.2 wdb/wdb_borders_0.2km.b
		
shrinkl_i:	
		line_shrink wdb/wdb_borders.b 1 wdb/wdb_borders_1km.b
		
shrinkl_l:	
		line_shrink wdb/wdb_borders.b 5 wdb/wdb_borders_5km.b
		
shrinkl_c:	
		line_shrink wdb/wdb_borders.b 25 wdb/wdb_borders_25km.b

shrinkr_h:	
		line_shrink res_f/new_cia_rivers.b 0.2 res_h/new_cia_rivers_0.2km.b
		
shrinkr_i:	
		line_shrink res_f/new_cia_rivers.b 1 res_i/new_cia_rivers_1km.b
		
shrinkr_l:	
		line_shrink res_f/new_cia_rivers.b 5 res_l/new_cia_rivers_5km.b
		
shrinkr_c:	
		line_shrink res_f/new_cia_rivers.b 25 res_c/new_cia_rivers_25km.b

bin_f:
		polygon_to_bins res_f/${VERSION}_final_dbase.b 1 res_f/${VERSION}_final_nodes.grd res_f/binned_GSHHS_f
		shoremaker res_f/binned_GSHHS_f
		rm res_f/binned_GSHHS_f.bin res_f/binned_GSHHS_f.seg res_f/binned_GSHHS_f.pt

bin_h:
		polygon_to_bins res_h/${VERSION}_final_dbase_0.2km.b 2 res_h/${VERSION}_final_nodes_0.2km.grd res_h/binned_GSHHS_h
		shoremaker res_h/binned_GSHHS_h
		rm res_h/binned_GSHHS_h.bin res_h/binned_GSHHS_h.seg res_h/binned_GSHHS_h.pt

bin_i:
		polygon_to_bins res_i/${VERSION}_final_dbase_1km.b 5 res_i/${VERSION}_final_nodes_1km.grd res_i/binned_GSHHS_i
		shoremaker res_i/binned_GSHHS_i
		rm res_i/binned_GSHHS_i.bin res_i/binned_GSHHS_i.seg res_i/binned_GSHHS_i.pt

bin_l:
		polygon_to_bins res_l/${VERSION}_final_dbase_5km.b 10 res_l/${VERSION}_final_nodes_5km.grd res_l/binned_GSHHS_l
		shoremaker res_l/binned_GSHHS_l
		rm res_l/binned_GSHHS_l.bin res_l/binned_GSHHS_l.seg res_l/binned_GSHHS_l.pt

bin_c:
		polygon_to_bins res_c/${VERSION}_final_dbase_25km.b 20 res_c/${VERSION}_final_nodes_25km.grd res_c/binned_GSHHS_c
		shoremaker res_c/binned_GSHHS_c
		rm res_c/binned_GSHHS_c.bin res_c/binned_GSHHS_c.seg res_c/binned_GSHHS_c.pt

nodes_f:
		polygon_setnodes res_f/${VERSION}_final_dbase.b 1 res_f/${VERSION}_final_nodes.grd

nodes_h:
		polygon_setnodes res_h/${VERSION}_final_dbase_0.2km.b 2 res_h/${VERSION}_final_nodes_0.2km.grd

nodes_i:
		polygon_setnodes res_i/${VERSION}_final_dbase_1km.b 5 res_i/${VERSION}_final_nodes_1km.grd

nodes_l:
		polygon_setnodes res_l/${VERSION}_final_dbase_5km.b 10 res_l/${VERSION}_final_nodes_5km.grd

nodes_c:
		polygon_setnodes res_c/${VERSION}_final_dbase_25km.b 20 res_c/${VERSION}_final_nodes_25km.grd

lines_f:
		lines_to_bins wdb/wdb_borders.b 1 res_f/binned_border_f
		linemaker res_f/binned_border_f

lines_h:
		lines_to_bins wdb/wdb_borders_0.2km.b 2 res_h/binned_border_h
		linemaker res_h/binned_border_h

lines_i:
		lines_to_bins wdb/wdb_borders_1km.b 5 res_i/binned_border_i
		linemaker res_i/binned_border_i

lines_l:
		lines_to_bins wdb/wdb_borders_5km.b 10 res_l/binned_border_l
		linemaker res_l/binned_border_l

lines_c:
		lines_to_bins wdb/wdb_borders_25km.b 20 res_c/binned_border_c
		linemaker res_c/binned_border_c

river_f:
		lines_to_bins res_f/new_cia_rivers.b 1 res_f/binned_river_f
		linemaker res_f/binned_river_f

river_h:
		lines_to_bins res_h/new_cia_rivers_0.2km.b 2 res_h/binned_river_h
		linemaker res_h/binned_river_h

river_i:
		lines_to_bins res_i/new_cia_rivers_1km.b 5 res_i/binned_river_i
		linemaker res_i/binned_river_i

river_l:
		lines_to_bins res_l/new_cia_rivers_5km.b 10 res_l/binned_river_l
		linemaker res_l/binned_river_l

river_c:
		lines_to_bins res_c/new_cia_rivers_25km.b 20 res_c/binned_river_c
		linemaker res_c/binned_river_c

gshhs:
		polygon_to_gshhs res_f/${VERSION}_final_dbase.b > gshhs_f.b
		polygon_to_gshhs res_h/${VERSION}_final_dbase_0.2km.b > gshhs_h.b
		polygon_to_gshhs res_i/${VERSION}_final_dbase_1km.b > gshhs_i.b
		polygon_to_gshhs res_l/${VERSION}_final_dbase_5km.b > gshhs_l.b
		polygon_to_gshhs res_c/${VERSION}_final_dbase_25km.b > gshhs_c.b

tar-gshhs:
		tar cvjf gshhs_1.3.tbz gshhs_[clihf].b
		cd $(GMTHOME)/src; \
		tar cvjf gshhs_1.5_src.tbz gshhs/{gshhs.c,gshhs.h,gshhs_dp.c,gshhstograss.c,makefile,README.gshhs}

zip-gshhs:
		zip -r -9 -q gshhs_1.3.zip gshhs_[clihf].b
		cd $(GMTHOME)/src; \
		zip -r -9 -q -l gshhs_1.5_src.zip gshhs/{gshhs.c,gshhs.h,gshhs_dp.c,gshhstograss.c,makefile,README.gshhs}

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
		$(CC) -c $(CFLAGS) $<
