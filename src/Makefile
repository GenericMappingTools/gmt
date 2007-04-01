#	$Id: Makefile,v 1.6 2007-04-01 19:59:46 remko Exp $
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
include makegmt.macros		# GMT-specific settings determined by user & install_gmt
include gmtalldeps.macros	# Dependencies

CFLAGS		= $(CC_OPT) $(WIN32) -I$(NETCDF)/include -DGMT_SHARE_PATH=\"$(GMT_SHARE_PATH)\" $(TRIANGLE_D)
CDF		= -L$(NETCDF)/lib -lnetcdf

PS	= -lpsl
GMT	= -lgmt
GMTPS	= -lgmtps

LIB_C	= gmt_calclock.c \
	  gmt_cdf.c \
	  gmt_customio.c \
	  gmt_grdio.c \
	  gmt_init.c \
	  gmt_io.c \
	  gmt_map.c \
	  gmt_nc.c \
	  gmt_proj.c \
	  gmt_shore.c \
	  gmt_stat.c \
	  gmt_support.c \
	  gmt_vector.c

LIBPS_C	= gmt_plot.c

GMT_I	= gmt_globals.h \
	  gmt_defaults.h \
	  gmt_media_name.h \
	  gmt_media_size.h \
	  gmt_colornames.h \
	  gmt_color_rgb.h \
	  gmt_pennames.h

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

first:	all

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

LIB_O	= ${LIB_C:.c=.o}

LIBPS_O	= ${LIBPS_C:.c=.o}

PROGS_O	= ${PROGS_C:.c=.o}

PROGS	= ${PROGS_C:.c=}

PROGSPS_O	= ${PROGSPS_C:.c=.o}

PROGSPS	= ${PROGSPS_C:.c=}

ALLPROGS	= $(PROGS) $(PROGSPS)

$(LIB_O):	$(GMT_H) $(GMT_I)

$(LIBPS_O):	$(GMT_H) pslib.h $(GMT_I)

$(PROGS_O):	$(GMT_H)

$(PROGSPS_O):	$(GMT_H) pslib.h

# Jonathan Shewchuk's triangulation model (www.cs.cmu.edu/~quake)
# Only used if TRIANGLE_D and TRIANGLE_O are set in makegmt.macros
# Please read README.TRIANGLE in this source directory.

triangle.o:	triangle.c
		$(CC) -c $(CFLAGS) -DNO_TIMER -DTRILIBRARY -DREDUCED -DCDT_ONLY $<

gmt_customio.o: gmt_customio.c gmt_mgg_header2.c gmt_agc_io.c $(GMT_H)

# Assembly wrapper for sincos on Dec Alphas
# Only used if ALPHA_SINCOS_O are set in makegmt.macros

alpha-sincos.o:	alpha-sincos.s
		$(AS) -o alpha-sincos.o $<

#-------------------------------------------------------------------------------

all:		init libs $(ALLPROGS)

init:		gmtmacros gmt_notposix.h
		
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
		if [ ! -d $(bindir) ]; then \
			mkdir -p $(bindir); \
		fi
		for i in $(ALLPROGS); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done
		$(INSTALL) GMT $(bindir)
		if [ ! -d $(libdir) ]; then \
			mkdir -p $(libdir); \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			rm -f $(libdir)/libpsl.* $(libdir)/libgmt.* $(libdir)/libgmtps.*; \
			$(INSTALL) -m 444 libpsl.a $(libdir); \
			$(INSTALL) -m 444 libgmt.a $(libdir); \
			$(INSTALL) -m 444 libgmtps.a $(libdir); \
			if [ -f libpsl.$(SL) ]; then \
				$(INSTALL) -m 444 libpsl.$(SL) $(libdir); \
				$(INSTALL) -m 444 libgmt.$(SL) $(libdir); \
				$(INSTALL) -m 444 libgmtps.$(SL) $(libdir); \
			fi; \
		fi
		if [ ! -d $(includedir) ]; then \
			mkdir -p $(includedir); \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			for i in $(GMT_H); do \
				$(INSTALL) -m 444 $$i $(includedir); \
			done; \
			for i in $(PS_H) $(PS_I); do \
				$(INSTALL) -m 444 $$i $(includedir); \
			done; \
		fi

uninstall:
		for i in $(ALLPROGS); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done
		\rm -f $(bindir)/GMT
		if [ ! $(libdir) = $(srcdir) ]; then \
			\rm -f $(libdir)/libgmt.a $(libdir)/libpsl.a $(libdir)/libgmtps.a ; \
			if [ -f libpsl.$(SL) ]; then \
				\rm -f $(libdir)/libpsl.$(SL); \
				\rm -f $(libdir)/libgmt.$(SL); \
				\rm -f $(libdir)/libgmtps.$(SL); \
			fi; \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			for i in $(GMT_H); do \
				\rm -f $(includedir)/$$i; \
			done; \
			for i in $(PS_H) $(PS_I); do \
				\rm -f $(includedir)/$$i; \
			done; \
		fi

clean:
		rm -f *.o *.a *.$(SL)
		for i in $(ALLPROGS); do \
			rm -f $$i$(EXE); \
		done

spotless:	clean
		rm -f $(GMTLIB) gmt_notposix.h makegmt.macros *.man

distclean:	spotless

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

libs:		$(GMTLIB)

pslib.o:	pslib.c $(PS_H) $(PS_I) gmt_notunix.h gmt_notposix.h gmt_math.h
		$(CC) $(CFLAGS) -c pslib.c

libpsl.a:	pslib.o
		$(AR) cvur libpsl.a $?
		$(RANLIB) libpsl.a

libgmt.a:	$(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmtps.a:	$(LIBPS_O) 
		$(AR) cvur $@ $?
		$(RANLIB) $@

libpsl.$(SL):	pslib.o
		$(LD) $(LD_OPT) $? -o $@

libgmt.$(SL):	$(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O)
		$(LD) $(LD_OPT) $(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O) -o $@

libgmtps.$(SL):	$(LIBPS_O)
		$(LD) $(LD_OPT) $(LIBPS_O) -o $@

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(PROGS):	$(GMTLIB) $(PROGS_O)
		$(CC) $(CFLAGS) $@.o -L. $(GMT) $(CDF) $(LIBS) $(LDFLAGS) -o $@
		$(COMPRESS) $@$(EXE)

$(PROGSPS):	$(GMTLIB) $(PROGSPS_O)
		$(CC) $(CFLAGS) $@.o -L. $(GMT) $(GMTPS) $(PS) $(CDF) $(LIBS) $(LDFLAGS) -o $@
		$(COMPRESS) $@$(EXE)

.c.o:
		$(CC) -c $(CFLAGS) $<

gmtmath.o:	gmtmath.c gmtmath.h
grdmath.o:	grdmath.c grdmath.h

#---------------------------- CVS GURU EXTENSIONS-------------------------------
include $(MAKEGURU)
