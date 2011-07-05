#	$Id: Makefile,v 1.75 2011-07-05 19:50:19 guru Exp $
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
#		Makefile for GMT Version 5 src directory
#		GNU Compatible
#
#	Normally, this makefile is activated by the main Makefile in
#	the GMT main directory.  However, you can also issue commands
#	directly to this makefile from within the src directory.
#	First, run configure in the main GMT directory.
#	To compile/link them, try "make init", "make all", and "make install".
#	When done, clean out directory with "make clean".
#	To start installation from scratch, first do "make spotless"
#	To wipe out installed modules, do "make uninstall"
#
#	Authors:	Paul Wessel and Remko Scharroo
#
#	Date:		1-JAN-2011

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

include config.mk	# GMT-specific settings determined by user & configure
include common.mk	# Dependencies

DEFINES		= -DGMT_SHARE_PATH=\"$(GMT_SHARE_PATH)\" $(TRIANGLE_D)
INCLUDES	= $(NETCDF_INC) $(GDAL_INC)

DEMOS_O		+= gmt.o
DEMOS		= $(DEMOS_O:.o=$(EXE))

PSLDEMO_O	= psldemo.o
PSLDEMO		= $(PSLDEMO_O:.o=$(EXE))

FUNCS_O		+= blockmean_func.o blockmedian_func.o blockmode_func.o colmath_func.o filter1d_func.o fitcircle_func.o gmt2kml_func.o \
		  gmtconvert_func.o gmtdp_func.o gmtmath_func.o gmtselect_func.o gmtspatial_func.o gmtstitch_func.o gmtvector_func.o\
		  gmtwhich_func.o grd2cpt_func.o grd2rgb_func.o grd2xyz_func.o grdblend_func.o grdclip_func.o grdcut_func.o grdedit_func.o grdfft_func.o \
		  grdfilter_func.o grdgradient_func.o grdhisteq_func.o grdinfo_func.o grdlandmask_func.o grdmask_func.o grdmath_func.o \
		  grdpaste_func.o grdproject_func.o grdsample_func.o grdreformat_func.o grdtrack_func.o grdtrend_func.o grdvolume_func.o \
		  greenspline_func.o kml2gmt_func.o makecpt_func.o mapproject_func.o minmax_func.o nearneighbor_func.o project_func.o \
		  ps2raster_func.o sample1d_func.o spectrum1d_func.o splitxyz_func.o surface_func.o trend1d_func.o \
		  trend2d_func.o triangulate_func.o xyz2grd_func.o

FUNCSPS_O	+= gmtdefaults_func.o gmtget_func.o gmtset_func.o grdcontour_func.o grdimage_func.o grdvector_func.o \
		  grdview_func.o psbasemap_func.o psclip_func.o pscoast_func.o pscontour_func.o pshistogram_func.o psimage_func.o \
		  psmask_func.o psrose_func.o psscale_func.o pstext_func.o pswiggle_func.o psxy_func.o psxyz_func.o pslegend_func.o

API_O		= gmt_modules.o

PROGS_O		= $(FUNCS_O:_func.o=.o)

PROGSPS_O	= $(FUNCSPS_O:_func.o=.o)

LIBPS_O		= pslib.o
LIBGMT_O	= gmt_bcr.o gmt_calclock.o gmt_cdf.o gmt_customio.o gmt_fft.o gmt_grdio.o \
		  gmt_io.o  gmt_nc.o gmt_notposix.o gmt_proj.o gmt_shore.o gmt_stat.o gmt_support.o \
		  gmt_vector.o $(TRIANGLE_O) $(ALPHA_SINCOS_O) gmtapi_parse.o $(FUNCS_O)
LIBGMTPS_O	= gmt_init.o gmt_map.o gmt_plot.o gmtapi_util.o $(FUNCSPS_O) $(API_O)

SCRIPTS		= gmt_shell_functions.sh gmtlogo isogmt ../gmtswitch
ALLPROGS	= $(SCRIPTS) $(PROGS) $(PROGSPS) $(DEMOS) $(GMT_COMPAT_PROGS)

CONFIGS		= config.mk gmt_version.h gmt_notposix.h

MAN1		= gmt.1 gmt_shell_functions.1 gmtlogo.1 isogmt.1 \
		  $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)
MAN3		= pslib.3
MAN5		= gmtcolors.5 gmt.conf.5

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		init $(ALLPROGS)

init:		gmtmacros gmt_version.h gmt_notposix.h

gmtmacros:
		@if [ ! -s config.mk ]; then \
			echo "src/config.mk is empty - you must rerun configure in the main GMT directory"; \
			exit 1; \
		fi

gmt_version.h:	gmt_version.h.in
		@echo "You must first run configure in the main GMT directory"; exit 1

gmt_notposix.h:	gmt_notposix.h.in
		@echo "You must first run configure in the main GMT directory"; exit 1

psldemo.ps:	$(PSLDEMO)
		$(PSLDEMO) > $@

install:	all
		$(INSTALL_DIR) $(bindir)
		$(INSTALL) $(ALLPROGS) $(bindir)
		if [ ! . -ef $(includedir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt ; \
			$(INSTALL_DATA) $(GMT_H) $(PS_H) $(PS_I) $(includedir)/gmt; \
		fi
		if [ ! . -ef $(libdir) ]; then \
			$(INSTALL_DIR) $(libdir) ; \
			$(INSTALL_DATA) $(LIBGMT:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBGMT) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			for lib in $(LIBGMT:.$(LIBEXT)=) ; do \
				\mv -f $$lib.$(SL) $$lib.$(SL_VERSION); \
				$(LN_S) $$lib.$(SL_VERSION) $$lib.$(SL); \
			done; \
		fi
		if [ "$(DSYMUTIL)" ]; then \
			cd $(bindir) ; \
			$(DSYMUTIL) $(PROGS) $(PROGSPS) $(DEMOS) $(GMT_COMPAT_PROGS); \
			cd $(libdir) ; \
			$(DSYMUTIL) $(LIBGMT:$(LIBEXT)=$(SL_VERSION)); \
		fi

uninstall:
		cd $(bindir); \rm -f $(ALLPROGS)
		if [ ! . -ef $(includedir) ]; then \
			cd $(includedir)/gmt; \rm -f $(GMT_H) $(PS_H) $(PS_I); \
		fi
		if [ ! . -ef $(libdir) ]; then \
			cd $(libdir); \rm -f $(LIBGMT) $(LIBGMT:.$(LIBEXT)=.a); \
		fi

install-man:	$(MAN1) $(MAN3) $(MAN5)
		$(INSTALL_DIR) $(mandir)/man1 $(mandir)/man3 $(mandir)/man5
		$(INSTALL_DATA) $(MAN1) $(mandir)/man1
		$(INSTALL_DATA) $(MAN3) $(mandir)/man3
		$(INSTALL_DATA) $(MAN5) $(mandir)/man5

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)
		cd $(mandir)/man3; \rm -f $(MAN3)
		cd $(mandir)/man5; \rm -f $(MAN5)

clean:
		\rm -f *% *.o *.a *.$(SL) $(PROGS) $(PROGSPS) $(DEMOS) $(PSLDEMO)

spotless::	clean
		\rm -f $(CONFIGS)

distclean:	spotless

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(LIBGMT_O) $(PROGS_O):		$(GMT_H)
$(LIBGMTPS_O) $(PROGSPS_O):	$(GMT_H) $(PS_H)
$(LIBPS_O) $(PSLDEMO_O):	$(PS_H)

# Additional unusual dependancies

blockmean.o blockmedian.o blockmode.o:	block_subs.h
gmt_customio.o: gmt_mgg_header2.c gmt_mgg_header2.h gmt_agc_io.c gmt_esri_io.c gmt_gdalread.c
gmt_init.o:	gmt_colornames.h gmt_datums.h gmt_ellipsoids.h gmt_unique.h \
		gmt_keycases.h gmt_keywords.h gmt_media_name.h gmt_media_size.h
gmt_support.o:	gmt_color_rgb.h gmt_pennames.h
gmtmath_func.o:	gmtmath_func.h gmtmath_op.h gmtmath_explain.h
grdmath_func.o:	grdmath_func.h grdmath_op.h grdmath_explain.h
gmt_bcr.o gmt_calclock.o gmt_cdf.o gmt_customio.o gmt_grdio.o gmt_init.o \
gmt_io.o gmt_map.o gmt_nc.o gmt_plot.o gmt_proj.o gmt_shore.o gmt_stat.o \
gmt_fft.o gmt_support.o gmt_vector.o gmtapi_util.o:	gmt_internals.h
gmtapi_util.o:	gmtapi_errstr.h
gmt.o:		gmt_prognames.h gmt_progcases.h gmt_progpurpose.h

# Jonathan Shewchuk's triangulation model (www.cs.cmu.edu/~quake)
# Only used if TRIANGLE_D and TRIANGLE_O are set in config.mk
# Please read README.TRIANGLE in this source directory.

triangle.o:	triangle.c triangle.h
		$(CC) -c $(CFLAGS) -DNO_TIMER -DTRILIBRARY -DREDUCED -DCDT_ONLY $<

# Special way to create the program objects
$(PROGS_O):	gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$* -c -o $*.o gmtprogram.c

$(PROGSPS_O):	gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMTPSL -DFUNC=GMT_$* -c -o $*.o gmtprogram.c

# Assembly wrapper for sincos on Dec Alphas
# Only used if ALPHA_SINCOS_O are set in config.mk

alpha-sincos.o:	alpha-sincos.s
		$(AS) -o alpha-sincos.o $<

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

libs:		$(LIBGMT)

libpsl.a:	$(LIBPS_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt.a:	$(LIBGMT_O) $(LIBGMTPS_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libpsl.$(SL):	libpsl.a
		$(LD) $(LD_OPT) $(LIBPS_O) $(LIBS) -o $@

libgmt.$(SL):	libgmt.a libpsl.$(SL)
		$(LD) $(LD_OPT) $(LIBGMT_O) $(LIBGMTPS_O) libpsl.$(SL) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------
gmt$(EXE):		gmt.o

blockmean$(EXE):	blockmean.o
blockmedian$(EXE):	blockmedian.o
blockmode$(EXE):	blockmode.o
colmath$(EXE):		colmath.o
filter1d$(EXE):		filter1d.o
fitcircle$(EXE):	fitcircle.o
gmt2kml$(EXE):		gmt2kml.o
gmtconvert$(EXE):	gmtconvert.o
gmtdefaults$(EXE):	gmtdefaults.o
gmtdp$(EXE):		gmtdp.o
gmtget$(EXE):		gmtget.o
gmtmath$(EXE):		gmtmath.o
gmtselect$(EXE):	gmtselect.o
gmtset$(EXE):		gmtset.o
gmtspatial$(EXE):	gmtspatial.o
gmtstitch$(EXE):	gmtstitch.o
gmtvector$(EXE):	gmtvector.o
gmtwhich$(EXE):		gmtwhich.o
grd2cpt$(EXE):		grd2cpt.o
grd2rgb$(EXE):		grd2rgb.o
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
kml2gmt$(EXE):		kml2gmt.o
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

gmt2rgb$(EXE):		grd2rgb$(EXE)
		$(RM) gmt2rgb$(EXE)
		$(LN_S) grd2rgb$(EXE) gmt2rgb$(EXE)

$(FUNCS) $(PROGS) $(PROGSPS):	$(LIBGMT)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@

$(DEMOS):	$(LIBGMT)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@

# To create to PSL demo program. Requires pslib only
$(PSLDEMO):	$(PSLDEMO_O) $(LIBPS_O)
		$(CC) $(LDFLAGS) $(PSLDEMO_O) $(LIBPS_O) $(LIBS) -o $@
