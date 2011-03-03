#	$Id: Makefile,v 1.59 2011-03-03 21:02:50 guru Exp $
#
#	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 or any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#		Makefile for GMT Version 4 src directory
#		GNU, Sys V, and BSD Compatible
#
#	Normally, this makefile is activated by the main Makefile in
#	the GMT main directory.  However, you can also issue commands
#	directy to this makefile from within the src directory.
#	First, run configure in the main GMT directory.
#	To compile/link them, try "make init", "make all", and "make install".
#	When done, clean out directory with "make clean".
#	To start installation from scratch, first do "make spotless"
#	To wipe out installed modules, do "make uninstall"
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, Lab for Satellite Altimetry, NOAA
#
#	Date:		20-SEPT-2009

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

include config.mk	# GMT-specific settings determined by user & configure
include common.mk	# Dependencies

DEFINES		= -DGMT_SHARE_PATH=\"$(GMT_SHARE_PATH)\" $(TRIANGLE_D)
INCLUDES	= $(NETCDF_INC) $(GDAL_INC)

LIB_O		= gmt_bcr.o gmt_calclock.o gmt_cdf.o gmt_customio.o gmt_grdio.o gmt_init.o gmt_io.o \
		gmt_map.o gmt_nc.o gmt_proj.o gmt_shore.o gmt_stat.o gmt_support.o gmt_vector.o

LIBPS_O		= gmt_plot.o

PROGS_O		= blockmean.o blockmedian.o blockmode.o filter1d.o fitcircle.o gmtconvert.o \
		gmtdefaults.o gmtmath.o gmtselect.o gmtset.o grd2cpt.o grd2xyz.o grdblend.o grdcut.o \
		grdclip.o grdedit.o grdfft.o grdfilter.o grdgradient.o grdhisteq.o grdinfo.o grdlandmask.o \
		grdmask.o grdtrack.o grdreformat.o grdmath.o grdpaste.o grdproject.o grdsample.o grdtrend.o \
		grdvolume.o greenspline.o makecpt.o mapproject.o minmax.o nearneighbor.o project.o ps2raster.o \
		sample1d.o spectrum1d.o splitxyz.o surface.o trend1d.o trend2d.o triangulate.o xyz2grd.o

PROGSPS_O	= gmt2rgb.o grdcontour.o grdimage.o grdvector.o grdview.o psbasemap.o psclip.o \
	  	pscoast.o pscontour.o pshistogram.o psimage.o pslegend.o psmask.o psrose.o \
		psscale.o pstext.o pswiggle.o psxy.o psxyz.o

SCRIPTS		= GMT gmt_shell_functions.sh gmtget gmtlogo isogmt psbbox.sh ../gmtswitch
ALLPROGS	= $(SCRIPTS) $(PROGS) $(PROGSPS)

CONFIGS		= config.mk gmt_version.h gmt_notposix.h GMT

MAN1		= GMT.1 gmt_shell_functions.1 gmtget.1 gmtlogo.1 isogmt.1 psbbox.1 \
		$(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)
MAN3		= pslib.3
MAN5		= gmtcolors.5

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		init $(ALLPROGS)

init:		gmtmacros gmt_version.h gmt_notposix.h GMT
		
gmtmacros:
		@if [ ! -s config.mk ]; then \
			echo "src/config.mk is empty - you must rerun configure in the main GMT directory"; \
			exit 1; \
		fi

gmt_version.h:	gmt_version.h.in
		@echo "You must first run configure in the main GMT directory"; exit 1

gmt_notposix.h:	gmt_notposix.h.in
		@echo "You must first run configure in the main GMT directory"; exit 1

GMT:		GMT.in
		@echo "You must first run configure in the main GMT directory"; exit 1

install:	all
		$(INSTALL) -d $(bindir)
		$(INSTALL) $(ALLPROGS) $(bindir)
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -d $(libdir) ; \
			$(INSTALL) -m 644 $(LIBGMTPS:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) -m 755 $(LIBGMTPS) $(libdir); fi \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL) -d $(includedir) ; \
			$(INSTALL) -m 644 $(GMT_H) $(PS_H) $(PS_I) $(includedir); \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			for lib in $(LIBGMTPS:.$(LIBEXT)=) ; do\
				\mv -f $$lib.$(SL) $$lib.$(SL_VERSION); \
				$(LN_S) $$lib.$(SL_VERSION) $$lib.$(SL); \
			done; \
		fi

uninstall:
		cd $(bindir); \rm -f $(ALLPROGS)
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f $(LIBGMTPS) $(LIBGMTPS:.$(LIBEXT)=.a); \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir); \rm -f $(GMT_H) $(PS_H) $(PS_I); \
		fi

install-man:	$(MAN1) $(MAN3) $(MAN5)
		$(INSTALL) -d $(mandir)/man1 $(mandir)/man3 $(mandir)/man5
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1
		$(INSTALL) -m 644 $(MAN3) $(mandir)/man3
		$(INSTALL) -m 644 $(MAN5) $(mandir)/man5

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)
		cd $(mandir)/man3; \rm -f $(MAN3)
		cd $(mandir)/man5; \rm -f $(MAN5)

clean:
		\rm -f *% *.o *.a *.$(SL) $(PROGS) $(PROGSPS)

spotless::	clean
		\rm -f $(CONFIGS)

distclean:	spotless

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(LIB_O) $(PROGS_O):		$(GMT_H)
$(LIBPS_O) $(PROGSPS_O):	$(GMT_H) $(PS_H)
pslib.o:	$(PS_H) gmt_notunix.h gmt_notposix.h gmt_math.h

# Additional unusual dependancies

blockmean.o blockmedian.o blockmode.o:	block_subs.c block_subs.h
gmt_customio.o: gmt_mgg_header2.c gmt_mgg_header2.h gmt_agc_io.c
gmt_grdio.o:	gmt_grdformats.h
gmt_init.o:	gmt_colornames.h gmt_datums.h gmt_ellipsoids.h gmt_globals.h \
		gmt_keycases.h gmt_keywords.h gmt_media_name.h gmt_media_size.h \
		gmt_unique.h
gmt_map.o gmt_proj.o:	gmt_proj.h
gmt_support.o:	gmt_color_rgb.h gmt_pennames.h
gmtmath.o:	gmtmath.h gmtmath_def.h gmtmath_explain.h
grdmath.o:	grdmath.h grdmath_def.h grdmath_explain.h
grd2xyz.o xyz2grd.o:	gmt_parse_z_io.h
grdreformat.o:	grdreformat.h

# Jonathan Shewchuk's triangulation model (www.cs.cmu.edu/~quake)
# Only used if TRIANGLE_D and TRIANGLE_O are set in config.mk
# Please read README.TRIANGLE in this source directory.

triangle.o:	triangle.c triangle.h
		$(CC) -c $(CFLAGS) -DNO_TIMER -DTRILIBRARY -DREDUCED -DCDT_ONLY $<

# Assembly wrapper for sincos on Dec Alphas
# Only used if ALPHA_SINCOS_O are set in config.mk

alpha-sincos.o:	alpha-sincos.s
		$(AS) -o alpha-sincos.o $<

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

libs:		$(LIBGMT)

libpsl.a:	pslib.o
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt.a:	$(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmtps.a:	$(LIBPS_O) 
		$(AR) cvur $@ $?
		$(RANLIB) $@

libpsl.$(SL):	libpsl.a
		$(LD) $(LD_OPT) pslib.o $(LIBS) -o $@

libgmt.$(SL):	libgmt.a
		$(LD) $(LD_OPT) $(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@

libgmtps.$(SL):	libgmtps.a libgmt.$(SL) libpsl.$(SL)
		$(LD) $(LD_OPT) $(LIBPS_O) libgmt.$(SL) libpsl.$(SL) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

blockmean$(EXE):	blockmean.o
blockmedian$(EXE):	blockmedian.o
blockmode$(EXE):	blockmode.o
filter1d$(EXE):		filter1d.o
fitcircle$(EXE):	fitcircle.o
gmt2rgb$(EXE):		gmt2rgb.o
gmtconvert$(EXE):	gmtconvert.o
gmtdefaults$(EXE):	gmtdefaults.o
gmtmath$(EXE):		gmtmath.o
gmtselect$(EXE):	gmtselect.o
gmtset$(EXE):		gmtset.o
grd2cpt$(EXE):		grd2cpt.o
grd2xyz$(EXE):		grd2xyz.o
grdblend$(EXE):		grdblend.o
grdclip$(EXE):		grdclip.o
grdcontour$(EXE):	grdcontour.o
grdcut$(EXE):		grdcut.o
grdedit$(EXE):		grdedit.o
grdfft$(EXE):		grdfft.o
grdfilter$(EXE):	grdfilter.o
grdgradient$(EXE):	grdgradient.o
grdhisteq$(EXE):	grdhisteq.o
grdimage$(EXE):		grdimage.o
grdinfo$(EXE):		grdinfo.o
grdlandmask$(EXE):	grdlandmask.o
grdmask$(EXE):		grdmask.o
grdmath$(EXE):		grdmath.o
grdpaste$(EXE):		grdpaste.o
grdproject$(EXE):	grdproject.o
grdreformat$(EXE):	grdreformat.o
grdsample$(EXE):	grdsample.o
grdtrack$(EXE):		grdtrack.o
grdtrend$(EXE):		grdtrend.o
grdvector$(EXE):	grdvector.o
grdview$(EXE):		grdview.o
grdvolume$(EXE):	grdvolume.o
greenspline$(EXE):	greenspline.o
makecpt$(EXE):		makecpt.o
mapproject$(EXE):	mapproject.o
minmax$(EXE):		minmax.o
nearneighbor$(EXE):	nearneighbor.o
project$(EXE):		project.o
ps2raster$(EXE):	ps2raster.o
psbasemap$(EXE):	psbasemap.o
psclip$(EXE):		psclip.o
pscoast$(EXE):		pscoast.o
pscontour$(EXE):	pscontour.o
pshistogram$(EXE):	pshistogram.o
psimage$(EXE):		psimage.o
pslegend$(EXE):		pslegend.o
psmask$(EXE):		psmask.o
psrose$(EXE):		psrose.o
psscale$(EXE):		psscale.o
pstext$(EXE):		pstext.o
pswiggle$(EXE):		pswiggle.o
psxy$(EXE):		psxy.o
psxyz$(EXE):		psxyz.o
sample1d$(EXE):		sample1d.o
spectrum1d$(EXE):	spectrum1d.o
splitxyz$(EXE):		splitxyz.o
surface$(EXE):		surface.o
trend1d$(EXE):		trend1d.o
trend2d$(EXE):		trend2d.o
triangulate$(EXE):	triangulate.o
xyz2grd$(EXE):		xyz2grd.o

$(PROGS):	$(LIBGMT)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(LIBGMTPS)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(GMTPS_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@
