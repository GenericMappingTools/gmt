#	$Id: GNUmakefile,v 1.61 2011-07-16 02:20:55 guru Exp $
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
#	See COPYING file for copying and redistribution conditions.
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
#		GNU Makefile for GMT Documentation by gurus only
#
#	The following commands are available to create all the GMT documentation
#
#	make all	: same as : make pdf html
#	make pdf	: Creƒwebmanate PDF versions of documentation and man pages
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

include ../src/config.mk

.SUFFIXES:	.eps .pdf .tex .ps .sh
.PHONY:	pdf html all install

# Papersize
PAPER=a4

# LaTeX command
PDFLATEX=pdflatex --interaction=batchmode

# PS and TEX files used by all documents
ALLPS=	fig/GMT_covertext.eps fig/GMT_coverlogo.ps
ALLTEX=	GMT_version.tex GMT_macros.tex GMT_Cover.tex

# PS, PDF and TEX files used to make Tutorial
TUTPS=	fig/GMT_Environment.eps fig/GMT_pstext_clearance.ps fig/GMT_pstext_justify.ps \
	fig/GMT_nearneighbor.ps fig/GMT_atan.ps $(ALLPS)
TUTPDF=$(TUTPS:.ps=.pdf)
TUTTEX=GMT_Tutorial.tex $(ALLTEX)

# PS, PDF and TEX files used to make API documentation
APIPS=
APIPDF=	$(APIPS:.ps=.pdf)
APITEX=	GMT_API.tex $(ALLTEX)
APIIMG=	fig/GMT4_mode.png fig/GMT5_mode.png

# PS files created from example scripts
EX_PS=	fig/example_01.ps fig/example_02.ps fig/example_03.ps fig/example_04.ps \
	fig/example_04c.ps fig/example_05.ps fig/example_06.ps fig/example_07.ps \
	fig/example_08.ps fig/example_09.ps fig/example_10.ps fig/example_11.ps \
	fig/example_12.ps fig/example_13.ps fig/example_14.ps fig/example_15.ps \
	fig/example_16.ps fig/example_17.ps fig/example_18.ps fig/example_19.ps \
	fig/example_20.ps fig/example_21.ps fig/example_22.ps fig/example_23.ps \
	fig/example_24.ps fig/example_25.ps fig/example_26.ps fig/example_27.ps \
	fig/example_28.ps fig/example_29.ps fig/example_30.ps fig/example_31.ps \
	fig/example_32.ps

# Ghostscript font lookup path
FONTPATH=examples/ex31/fonts

# PS files created from animation scripts
AN_PS=	fig/anim_01.ps fig/anim_02.ps fig/anim_03.ps fig/anim_04.ps

# TEX files of .def files used in the Cookbook that are to retain the comments
DEF_TEX=	fig/volcano.tex fig/bullseye.tex fig/functions.tex

# PS files that need special attention
IS_EPS=	fig/GMT_covertext.eps fig/GMT_Environment.eps
RGBPS=	fig/GMT_RGBchart_a4.ps fig/GMT_RGBchart_letter.ps fig/GMT_RGBchart_tabloid.ps

# PS files for GMT_Docs created from scripts. Scripts have to be converted to TeX (with comments)
DOCPS0=	fig/GMT_App_P_1.ps fig/GMT_App_P_2.ps fig/GMT_App_M_2.ps

# PS files for GMT_Docs created from scripts. Scripts have to be converted to TeX (without comments)
DOCPS1=	fig/GMT_App_K_1.ps fig/GMT_App_K_2.ps fig/GMT_App_K_3.ps fig/GMT_App_K_4.ps \
	fig/GMT_App_K_5.ps \
	fig/GMT_App_O_1.ps fig/GMT_App_O_2.ps fig/GMT_App_O_3.ps \
	fig/GMT_App_O_4.ps fig/GMT_App_O_5.ps fig/GMT_App_O_6.ps fig/GMT_App_O_7.ps \
	fig/GMT_App_O_8.ps fig/GMT_App_O_9.ps \
	fig/GMT_-B_custom.ps fig/GMT_-B_time1.ps fig/GMT_-B_time2.ps fig/GMT_-B_time3.ps \
	fig/GMT_-B_time4.ps fig/GMT_-B_time5.ps fig/GMT_-B_time6.ps fig/GMT_-B_time7.ps \
	fig/GMT_linear.ps fig/GMT_log.ps fig/GMT_pow.ps \
	fig/GMT_linear_cal.ps fig/GMT_linear_d.ps fig/GMT_polar.ps \
	fig/GMT_TM.ps fig/GMT_albers.ps fig/GMT_az_equidistant.ps \
	fig/GMT_cassini.ps fig/GMT_eckert4.ps fig/GMT_eckert6.ps fig/GMT_equi_cyl.ps \
	fig/GMT_equidistant_conic.ps fig/GMT_gall_stereo.ps fig/GMT_general_cyl.ps \
	fig/GMT_gnomonic.ps fig/GMT_grinten.ps fig/GMT_hammer.ps \
	fig/GMT_lambert_az_hemi.ps fig/GMT_lambert_az_rect.ps fig/GMT_lambert_conic.ps \
	fig/GMT_mercator.ps fig/GMT_miller.ps fig/GMT_mollweide.ps \
	fig/GMT_obl_merc.ps fig/GMT_orthographic.ps fig/GMT_perspective.ps fig/GMT_polyconic.ps \
	fig/GMT_robinson.ps fig/GMT_sinus_int.ps fig/GMT_sinusoidal.ps \
	fig/GMT_stereographic_general.ps fig/GMT_stereographic_polar.ps \
	fig/GMT_stereographic_rect.ps fig/GMT_stereonets.ps fig/GMT_transverse_merc.ps \
	fig/GMT_winkel.ps

# PS files for GMT_Docs created from scripts. Scripts do not need to be converted to TeX.
DOCPS2=	fig/GMT_-B_geo_1.ps fig/GMT_-B_geo_2.ps fig/GMT_-B_linear.ps fig/GMT_-B_log.ps \
	fig/GMT_-B_pow.ps fig/GMT_utm_zones.ps  fig/GMT_linecap.ps \
	fig/GMT_-J.ps fig/GMT_-OK.ps fig/GMT_-P.ps fig/GMT_-R.ps fig/GMT_-U.ps \
	fig/GMT_-XY.ps fig/GMT_registration.ps fig/GMT_App_E.ps \
	fig/GMT_App_F_dingbats.ps fig/GMT_App_F_iso+.ps fig/GMT_App_F_stand+.ps \
	fig/GMT_App_F_symbol.ps fig/GMT_App_G.ps fig/GMT_App_J_1.ps fig/GMT_App_J_2.ps fig/GMT_App_J_3.ps \
	fig/GMT_App_M_1.ps fig/GMT_App_N_1.ps \
	fig/GMT_Defaults_1a.ps fig/GMT_Defaults_1b.ps fig/GMT_Defaults_1c.ps fig/GMT_volcano.ps \
	fig/GMT_color_interpolate.ps

# Prepared images for GMT_Docs
DOCIMG=	fig/GMT5_Summit_2011.png fig/gimp-panel.png fig/gimp-sliders.png fig/hsv-cone.png \
	ppt/formatpicture.png ppt/rendering.png

# All PS, EPS and PDF files for GMT_Docs
DOCPS=	$(DOCPS0) $(DOCPS1) $(DOCPS2) $(ALLPS) $(RGBPS)
DOCPDF= $(DOCPS:.ps=.pdf) $(IS_EPS:.eps=.pdf) $(EX_PS:.ps=.pdf) $(AN_PS:.ps=.pdf)

# All TEX files needed for GMT_Docs
DOCTEX= GMT_Docs.tex GMT_Appendix_A.tex GMT_Appendix_B.tex GMT_Appendix_C.tex GMT_Appendix_D.tex \
	GMT_Appendix_E.tex GMT_Appendix_F.tex GMT_Appendix_G.tex GMT_Appendix_H.tex GMT_Appendix_I.tex \
	GMT_Appendix_J.tex GMT_Appendix_K.tex GMT_Appendix_L.tex GMT_Appendix_M.tex GMT_Appendix_N.tex \
	GMT_Appendix_O.tex GMT_Appendix_P.tex \
	GMT_Chapter_1.tex GMT_Chapter_2.tex GMT_Chapter_3.tex GMT_Chapter_4.tex GMT_Chapter_5.tex \
	GMT_Chapter_6.tex GMT_Chapter_7.tex GMT_Chapter_8.tex GMT_Chapter_9.tex \
	GMT_Frontmatter.tex $(DOCPS0:.ps=.tex) $(DOCPS1:.ps=.tex) $(EX_PS:.ps=.tex) $(AN_PS:.ps=.tex) $(DEF_TEX) $(ALLTEX)

all:	pdf html
install::	all

# Shortcut, just to preprocess all required files
prep prepro:	prep-ps prep-pdf prep-tex $(DOCIMG)
prep-ps:	$(DOCPS) $(TUTPS) $(APIPS)
prep-pdf:	$(DOCPDF) $(TUTPDF) $(APIPDF)
prep-tex:	$(DOCTEX) $(TUTTEX) $(APITEX)

# Cleanup, before or after
clean:
	\rm -f *% .gmt* gmt.conf
	\rm -f *.aux *.lot *.lof *.toc *.pdf *.out *.dvi *.ind *.ilg *.idx *.log *.ps \
		*.4ct *.4tc *.css *.html *.idv *.lg *.tmp *.xref *-js.tex
	\rm -f GMT_Appendix_N_inc.tex
	\rm -f fig/*.{tex,pdf}
	\rm -rf GMT_Docs GMT_Tutorial GMT_API

spotless:	clean
	\rm -f GMT_version.tex
	\rm -rf html/man html/index.html pdf

# Shortcuts, just to do a couple of things
pdf:	docs tutorial api man
docs:	pdf/GMT_Docs.pdf
tutorial: pdf/GMT_Tutorial.pdf
api:	pdf/GMT_API.pdf
man:	pdf/GMT_Manpages.pdf
html:	html/man/blockmean.html html/index.html

# Convert the PS files of the examples and animations to PS and PDF, while stripping off the time tag and
# rotating landscape images back to portrait.
# The extra FONTPATH options is only needed for example 31, but we do it here for all.
$(EX_PS:.ps=.pdf) $(AN_PS:.ps=.pdf):	%.pdf: %.ps
	ps2raster -Au -P -Tf -C-sFONTPATH=$(FONTPATH) $*.ps

%.pdf: %.ps
	ps2raster -A -P -Tf $*.ps

# To convert EPS to PDF
%.pdf:	%.eps
	ps2raster -P -Tf $*.eps

# Turn scripts into TeX files for inclusion in the documentation. Keep comments in these.
$(DOCPS0:.ps=.tex) fig/functions.tex:	fig/%.tex:	scripts/%.sh
	pr -t -e scripts/$*.sh | egrep -v 'Id:|functions.sh' > fig/$*.tex

# Turn scripts into TeX files for inclusion in the documentation. Remove comments in these.
$(DOCPS1:.ps=.tex):	fig/%.tex:	scripts/%.sh
	pr -t -e scripts/$*.sh | egrep -v '^#|^$$|functions.sh' > fig/$*.tex

# Keep the comments in the examples, but remove CVS tag.
$(EX_PS:.ps=.tex):	%.tex:	%.ps
	pr -t -e $(subst fig/example_,examples/*/job,$*).sh | egrep -v 'Id:|functions.sh' > $*.tex
$(AN_PS:.ps=.tex):	%.tex:	%.ps
	pr -t -e $(subst fig/anim_,examples/*/anim_,$*).sh | egrep -v 'Id:|functions.sh' > $*.tex
fig/volcano.tex:	../share/custom/volcano.def
	pr -t -e ../share/custom/volcano.def | egrep -v 'Id:' > $@
fig/bullseye.tex:	examples/ex20/bullseye.def
	pr -t -e examples/ex20/bullseye.def | egrep -v 'Id:' > $@

# How to create version info
GMT_version.tex:	GMT_version.tex.in
	@echo "You must first run configure in the main GMT directory"; exit 1

# Dependencies and rules to make PDF documentation
pdf/GMT_Docs.pdf:	$(DOCPDF) $(DOCIMG) $(DOCTEX)
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	$(PDFLATEX) GMT_Docs
	$(PDFLATEX) GMT_Docs
	makeindex -s GMT.ist GMT_Docs.idx
	$(PDFLATEX) GMT_Docs
	$(PDFLATEX) GMT_Docs
	mkdir -p pdf
	\mv GMT_Docs.pdf $@
	\ln -f fig/GMT_RGBchart_*.pdf pdf
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}

pdf/GMT_Tutorial.pdf:	$(TUTPDF) $(TUTIMG) $(TUTTEX)
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	$(PDFLATEX) GMT_Tutorial
	$(PDFLATEX) GMT_Tutorial
	makeindex -s GMT.ist GMT_Tutorial.idx
	$(PDFLATEX) GMT_Tutorial
	$(PDFLATEX) GMT_Tutorial
	mkdir -p pdf
	\mv GMT_Tutorial.pdf $@
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}

pdf/GMT_API.pdf:	$(APIPDF) $(APIIMG) $(APITEX)
	\rm -f GMT_API.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	$(PDFLATEX) GMT_API
	$(PDFLATEX) GMT_API
	makeindex -s GMT.ist GMT_API.idx
	$(PDFLATEX) GMT_API
	$(PDFLATEX) GMT_API
	mkdir -p pdf
	\mv GMT_API.pdf $@
	\rm -f GMT_API.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}

pdf/GMT_Manpages.pdf:	pdfman.sh ../src/blockmean.1
	bash pdfman.sh -s

html/man/blockmean.html:	webman.sh ../src/blockmean.1
	bash webman.sh -s
html/index.html:
	ln -f html/gmt_services.html $@

include Makefile
