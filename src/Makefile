#	$Id: Makefile,v 1.20 2007-11-14 21:15:32 remko Exp $
#
#	Copyright (c) 1991-2007 by P. Wessel and W. H. F. Smith
#	See COPYING file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 of the License.
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
#	Date:		11-NOV-2006

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR	=# Keep this empty, use ../ in supplements
include makegmt.macros		# GMT-specific settings determined by user & configure
include gmtalldeps.macros	# Dependencies

CFLAGS		= $(CC_OPT) $(WIN32) -I$(NETCDF)/include -DGMT_SHARE_PATH=\"$(GMT_SHARE_PATH)\" $(TRIANGLE_D)

PS		= -lpsl
GMT		= -lgmt
GMTPS		= -lgmtps

LIB_O		= gmt_bcr.o gmt_calclock.o gmt_cdf.o gmt_customio.o gmt_grdio.o gmt_init.o gmt_io.o \
		gmt_map.o gmt_nc.o gmt_proj.o gmt_shore.o gmt_stat.o gmt_support.o gmt_vector.o

LIBPS_O		= gmt_plot.o

GMT_I		= gmt_globals.h gmt_defaults.h gmt_media_name.h gmt_media_size.h gmt_colornames.h \
		gmt_color_rgb.h gmt_pennames.h

PS_I		= pslib_inc.h

PROGS_O		= blockmean.o blockmedian.o blockmode.o filter1d.o fitcircle.o gmtconvert.o \
		gmtdefaults.o gmtmath.o gmtselect.o gmtset.o grd2cpt.o grd2xyz.o grdblend.o grdcut.o \
		grdclip.o grdedit.o grdfft.o grdfilter.o grdgradient.o grdhisteq.o grdinfo.o grdlandmask.o \
		grdmask.o grdtrack.o grdreformat.o grdmath.o grdpaste.o grdproject.o grdsample.o grdtrend.o \
		grdvolume.o makecpt.o mapproject.o minmax.o nearneighbor.o project.o ps2raster.o \
		sample1d.o spectrum1d.o splitxyz.o surface.o trend1d.o trend2d.o triangulate.o xyz2grd.o

PROGSPS_O	= gmt2rgb.o grdcontour.o grdimage.o grdvector.o grdview.o psbasemap.o psclip.o \
	  	pscoast.o pshistogram.o psimage.o pslegend.o psmask.o psrose.o psscale.o pstext.o \
		pscontour.o pswiggle.o psxy.o psxyz.o

ALLPROGS	= GMT $(PROGS) $(PROGSPS)

CONFIGS		= makegmt.macros gmt_version.h gmt_notposix.h GMT

MAN1		= GMT.1 $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)
MAN3		= pslib.3

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		init $(ALLPROGS)

init:		gmtmacros $(CONFIGS)
		
gmtmacros:
		@if [ ! -s makegmt.macros ]; then \
			echo "src/makegmt.macros is empty - you must rerun configure in the main GMT directory"; \
			exit 1; \
		fi

makegmt.macros:	makegmt.macros.in
		@echo "You must first run configure in the main GMT directory"; exit 1

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
			$(INSTALL) -m 444 $(GMTLIB) $(libdir); \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL) -d $(includedir) ; \
			$(INSTALL) -m 444 $(GMT_H) $(PS_H) $(PS_I) $(includedir); \
		fi
		if [ -f $(libdir)/libpsl.dylib ]; then \
			cd $(libdir) ; \
			\mv -f libpsl{,.4}.dylib ; \
			$(LN_S) libpsl{.4,}.dylib ; \
			\mv -f libgmt{,.4}.dylib ; \
			$(LN_S) libgmt{.4,}.dylib ; \
			\mv -f libgmtps{,.4}.dylib ; \
			$(LN_S) libgmtps{.4,}.dylib ; \
		fi

uninstall:
		cd $(bindir); \rm -f $(ALLPROGS)
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f $(GMTLIB); \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir); \rm -f  $(GMT_H) $(PS_H) $(PS_I); \
		fi

install-man:	$(MAN1) $(MAN3)
		$(INSTALL) -d $(mandir)/man1 $(mandir)/man3
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1
		$(INSTALL) -m 644 $(MAN3) $(mandir)/man3

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)
		cd $(mandir)/man3; \rm -f $(MAN3)

clean:
		\rm -f *.o *.a *.so *.sa *.sl *.dylib *% $(PROGS) $(PROGSPS)

spotless::	clean
		\rm -f $(CONFIGS)

distclean:	spotless

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(LIB_O):	$(GMT_H) $(GMT_I)
$(LIBPS_O):	$(GMT_H) $(GMT_I) pslib.h
$(PROGS_O):	$(GMT_H)
$(PROGSPS_O):	$(GMT_H) pslib.h
pslib.o:	$(PS_H) $(PS_I) gmt_notunix.h gmt_notposix.h gmt_math.h

# Additional unusual dependancies

blockmean.o blockmedian.o blockmode.o:	block_subs.c
gmt_customio.o: gmt_mgg_header2.c gmt_agc_io.c
gmtmath.o:	gmtmath.h gmtmath_def.h gmtmath_explain.h
grdmath.o:	grdmath.h grdmath_def.h grdmath_explain.h

# Jonathan Shewchuk's triangulation model (www.cs.cmu.edu/~quake)
# Only used if TRIANGLE_D and TRIANGLE_O are set in makegmt.macros
# Please read README.TRIANGLE in this source directory.

triangle.o:	triangle.c
		$(CC) -c $(CFLAGS) -DNO_TIMER -DTRILIBRARY -DREDUCED -DCDT_ONLY $<

# Assembly wrapper for sincos on Dec Alphas
# Only used if ALPHA_SINCOS_O are set in makegmt.macros

alpha-sincos.o:	alpha-sincos.s
		$(AS) -o alpha-sincos.o $<

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

libs:		$(GMTLIB)

libpsl.a:	pslib.o
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt.a:	$(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmtps.a:	$(LIBPS_O) 
		$(AR) cvur $@ $?
		$(RANLIB) $@

libpsl.$(SL):	pslib.o
		$(LD) $(LD_OPT) pslib.o -o $@

libgmt.$(SL):	$(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O)
		$(LD) $(LD_OPT) $(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O) -o $@

libgmtps.$(SL):	$(LIBPS_O)
		$(LD) $(LD_OPT) $(LIBPS_O) -o $@

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

$(PROGS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. $(GMT) $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. $(GMT) $(GMTPS) $(PS) $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
