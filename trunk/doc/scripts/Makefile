#	$Id: Makefile,v 1.1 2011-04-18 19:31:06 remko Exp $
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
#		GNU Makefile for GMT Documentation by gurus only
#
#	The following commands are available to create all the GMT documentation
#
#	make all	: same as : make pdf html
#	make pdf	: Create PDF versions of documentation and man pages
#	make html	: Create HTML version of man pages
#	make install	: make all; and then install in place
#
#	All the above commands prepare all the necessary graphs and text to be included.
#	Make sure, however, that the examples are already made by running "make examples"
#	
#	make prep	: Only prepare the prerequisites.
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, Lab for Satellite Altimetry, NOAA
#
#	Date:		08-Feb-2007
#-------------------------------------------------------------------------------

include ../../src/config.mk

.SUFFIXES:	.eps .pdf .tex .ps .sh
.PHONY:	pdf html all install

# PS files used by all documents
ALLPS=	GMT_covertext.eps GMT_coverlogo.ps

# PS files used to make Tutorial
TUTPS=	GMT_Environment.eps GMT_pstext_clearance.ps GMT_pstext_justify.ps \
	GMT_nearneighbor.ps GMT_atan.ps

# PS files user to make the API documentation
APIPS=

# PS files that need special attention
APP_F_PS=GMT_App_F_dingbats.ps GMT_App_F_iso+.ps GMT_App_F_stand+.ps \
	GMT_App_F_symbol.ps
APP_J_PS=GMT_App_J_1.ps GMT_App_J_2.ps GMT_App_J_3.ps
APP_N_PS=GMT_App_N_1.ps
APP_O_PS= \
	GMT_App_O_1.ps GMT_App_O_2.ps GMT_App_O_3.ps \
	GMT_App_O_4.ps GMT_App_O_5.ps GMT_App_O_6.ps GMT_App_O_7.ps \
	GMT_App_O_8.ps GMT_App_O_9.ps
SQRT_PS=GMT_linear.ps GMT_log.ps GMT_pow.ps
RGBPS=	GMT_RGBchart_a4.ps GMT_RGBchart_letter.ps GMT_RGBchart_tabloid.ps

# PS files for GMT_Docs to be created from scripts.
DOCPS0=GMT_App_P_1.ps GMT_App_P_2.ps GMT_App_M_2.ps

# PS files for GMT_Docs to be created from scripts.
DOCPS1=GMT_App_K_1.ps GMT_App_K_2.ps GMT_App_K_3.ps GMT_App_K_4.ps \
	GMT_App_K_5.ps $(APP_O_PS) GMT_-B_custom.ps \
	GMT_-B_time1.ps GMT_-B_time2.ps GMT_-B_time3.ps \
	GMT_-B_time4.ps GMT_-B_time5.ps GMT_-B_time6.ps GMT_-B_time7.ps \
	$(SQRT_PS) GMT_linear_cal.ps ar_d.ps GMT_polar.ps \
	GMT_TM.ps GMT_albers.ps GMT_az_equidistant.ps \
	GMT_cassini.ps GMT_eckert4.ps GMT_eckert6.ps GMT_equi_cyl.ps \
	GMT_equidistant_conic.ps GMT_gall_stereo.ps GMT_general_cyl.ps \
	GMT_gnomonic.ps GMT_grinten.ps GMT_hammer.ps \
	GMT_lambert_az_hemi.ps GMT_lambert_az_rect.ps GMT_lambert_conic.ps \
	GMT_mercator.ps GMT_miller.ps GMT_mollweide.ps \
	GMT_obl_merc.ps GMT_orthographic.ps GMT_perspective.ps GMT_polyconic.ps \
	GMT_robinson.ps GMT_sinus_int.ps GMT_sinusoidal.ps \
	GMT_stereographic_general.ps GMT_stereographic_polar.ps \
	GMT_stereographic_rect.ps GMT_stereonets.ps GMT_transverse_merc.ps \
	GMT_winkel.ps

# PS files for GMT_Docs to be created from scripts.
DOCPS2=GMT_-B_geo_1.ps GMT_-B_geo_2.ps GMT_-B_linear.ps GMT_-B_log.ps \
	GMT_-B_pow.ps GMT_utm_zones.ps \
	GMT_-J.ps GMT_-OK.ps GMT_-P.ps GMT_-R.ps GMT_-U.ps \
	GMT_-XY.ps GMT_registration.ps GMT_App_E.ps \
	$(APP_F_PS) GMT_App_G.ps $(APP_J_PS) GMT_App_M_1.ps $(APP_N_PS) \
	GMT_Defaults_1a.ps GMT_Defaults_1b.ps GMT_Defaults_1c.ps GMT_volcano.ps \
	GMT_color_interpolate.ps

# All PS, EPS and PDF files for GMT_Docs
DOCPS=	$(DOCPS0) $(DOCPS1) $(DOCPS2) $(ALLPS) $(RGBPS)

all:	$(DOCPS) $(TUTPS) $(APIPS)

# Test if plots are OK
tests doctests:	all
	bash run_doc_tests.sh

# Cleanup, before or after
clean:
	\rm -f *% .gmt* gmt.conf
	\rm -f *.{tex,eps,pdf,csh,ps,png} sqrt.d*

# Graphics of Appendices F, J, N, O need special attention. One script makes several PS and PDF files.
$(APP_F_PS):	GMT_App_F.sh
	$(MAKE) APP=F run_app

$(APP_J_PS):	GMT_App_J.sh
	$(MAKE) APP=J run_app

$(APP_N_PS) GMT_Appendix_N_inc.tex:	GMT_App_N.sh
	$(MAKE) APP=N run_app

$(APP_O_PS):	GMT_App_O.sh $(APP_O_PS:.ps=.sh)
	$(MAKE) APP=O run_app

run_app:
	bash GMT_App_$(APP).sh

# The RGB prints should not be cropped.
$(RGBPS):	GMT_RGBchart.sh
	bash GMT_RGBchart.sh $(word 3,$(subst _, ,$*))

# All PS depend on functions.sh
$(DOCPS) $(TUTPS) $(APIPS):	functions.sh

# How to run the scripts to make PS files
.sh.ps:
	bash $*.sh

# Additional dependency for some graphs
$(SQRT_PS):	sqrt.d sqrt.d10
sqrt.d sqrt.d10:	GMT_dummydata.sh
	bash GMT_dummydata.sh
