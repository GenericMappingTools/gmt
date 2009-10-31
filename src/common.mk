#	$Id: common.mk,v 1.3 2009-10-31 16:00:23 remko Exp $
#
#	Copyright (c) 1991-2009 by P. Wessel and W. H. F. Smith
#	See LICENSE.TXT file for copying and redistribution conditions.
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
#		Dependencies for GMT Supplements
#
#	This file must be included from all GMT src and suppl makefiles.
#	It specifies how any program file (main GMT or supplement) depends
#	on include files and libraries.
#	Note: Include files for the library source code is not used here
#	as those dependencies only go in the main GMT makefile
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:		01-NOV-2006
#-------------------------------------------------------------------------------
#	GMTSRCDIR must be initialized before including this file
#	Set GMTSRCDIR to blank in main and ../ in supplements
#-------------------------------------------------------------------------------

# Source include files:

GMT_H	= $(GMTSRCDIR)gmt.h \
	  $(GMTSRCDIR)gmt_bcr.h \
	  $(GMTSRCDIR)gmt_boundcond.h \
	  $(GMTSRCDIR)gmt_calclock.h \
	  $(GMTSRCDIR)gmt_colors.h \
	  $(GMTSRCDIR)gmt_common.h \
	  $(GMTSRCDIR)gmt_contour.h \
	  $(GMTSRCDIR)gmt_customio.h \
	  $(GMTSRCDIR)gmt_dimensions.h \
	  $(GMTSRCDIR)gmt_error.h \
	  $(GMTSRCDIR)gmt_grd.h \
	  $(GMTSRCDIR)gmt_grdio.h \
	  $(GMTSRCDIR)gmt_init.h \
	  $(GMTSRCDIR)gmt_io.h \
	  $(GMTSRCDIR)gmt_map.h \
	  $(GMTSRCDIR)gmt_math.h \
	  $(GMTSRCDIR)gmt_mgg_header2.h \
	  $(GMTSRCDIR)gmt_nan.h \
	  $(GMTSRCDIR)gmt_notposix.h \
	  $(GMTSRCDIR)gmt_notunix.h \
	  $(GMTSRCDIR)gmt_plot.h \
	  $(GMTSRCDIR)gmt_project.h \
	  $(GMTSRCDIR)gmt_shore.h \
	  $(GMTSRCDIR)gmt_stat.h \
	  $(GMTSRCDIR)gmt_support.h \
	  $(GMTSRCDIR)gmt_symbol.h \
	  $(GMTSRCDIR)gmt_synopsis.h \
	  $(GMTSRCDIR)gmt_vector.h \
	  $(GMTSRCDIR)gmt_version.h

PS_H	= $(GMTSRCDIR)pslib.h

LIBGMT	= $(GMTSRCDIR)libgmt.$(LIBEXT)
LIBGMTPS= $(LIBGMT) $(GMTSRCDIR)libgmtps.$(LIBEXT) $(GMTSRCDIR)libpsl.$(LIBEXT)

GMT_LIB	= -L$(GMTSRCDIR). -lgmt
GMTPS_LIB	= -L$(GMTSRCDIR). -lgmtps -lgmt -lpsl
