#	$Id: Makefile,v 1.14 2007-09-24 00:38:01 remko Exp $
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
CDF		= -L$(NETCDF)/lib -lnetcdf

PS	= -lpsl
GMT	= -lgmt
GMTPS	= -lgmtps

LIB_C	= gmt_bcr.c gmt_calclock.c gmt_cdf.c gmt_customio.c gmt_grdio.c gmt_init.c gmt_io.c \
	  gmt_map.c gmt_nc.c gmt_proj.c gmt_shore.c gmt_stat.c gmt_support.c gmt_vector.c

LIBPS_C	= gmt_plot.c

GMT_I	= gmt_globals.h gmt_defaults.h gmt_media_name.h gmt_media_size.h gmt_colornames.h \
	  gmt_color_rgb.h gmt_pennames.h

PS_I	= pslib_inc.h

PROGS_C	= blockmean.c blockmedian.c blockmode.c filter1d.c fitcircle.c grdfilter.c gmtconvert.c \
	  gmtdefaults.c gmtmath.c gmtselect.c gmtset.c grd2cpt.c grd2xyz.c grdblend.c grdcut.c \
	  grdclip.c grdedit.c grdfft.c grdgradient.c grdhisteq.c grdinfo.c grdlandmask.c grdmask.c \
	  grdtrack.c grdreformat.c grdmath.c grdpaste.c grdproject.c grdsample.c grdtrend.c grdvolume.c \
	  makecpt.c mapproject.c minmax.c nearneighbor.c project.c ps2raster.c sample1d.c spectrum1d.c \
	  splitxyz.c surface.c trend1d.c trend2d.c triangulate.c xyz2grd.c

PROGSPS_C	= gmt2rgb.c grdcontour.c grdimage.c grdvector.c grdview.c psbasemap.c psclip.c \
	  pscoast.c pshistogram.c psimage.c pslegend.c psmask.c psrose.c psscale.c pstext.c pscontour.c \
	  pswiggle.c psxy.c psxyz.c

LIB_O	= $(LIB_C:.c=.o)
LIBPS_O	= $(LIBPS_C:.c=.o)
PROGS_O	= $(PROGS_C:.c=.o)
PROGSPS_O= $(PROGSPS_C:.c=.o)

PROGS	= $(PROGS_C:.c=$(EXE))
PROGSPS	= $(PROGSPS_C:.c=$(EXE))
ALLPROGS= GMT $(PROGS) $(PROGSPS)

MAN1	= GMT.1 $(PROGS_C:.c=.1) $(PROGSPS_C:.c=.1)
MAN3	= pslib.3

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		init libs $(ALLPROGS)

init:		gmtmacros gmt_notposix.h GMT
		
gmtmacros:
		@if [ ! -s makegmt.macros ]; then \
			echo "src/makegmt.macros is empty - you must rerun configure in the main GMT directory"; \
			exit; \
		fi

gmt_notposix.h:	gmt_notposix.h.in
		echo "Warning: You probably should rerun configure manually instead"
		cd ..; ./configure

GMT:		GMT.in
		echo "Warning: You probably should rerun configure manually instead"
		cd ..; ./configure

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
			\ln -s libpsl{.4,}.dylib ; \
			\mv -f libgmt{,.4}.dylib ; \
			\ln -s libgmt{.4,}.dylib ; \
			\mv -f libgmtps{,.4}.dylib ; \
			\ln -s libgmtps{.4,}.dylib ; \
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
		\rm -f *.o *.a *.$(SL) $(ALLPROGS)

spotless::	clean
		\rm -f gmt_notposix.h makegmt.macros

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
gmtmath.o:	gmtmath.h
grdmath.o:	grdmath.h

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
