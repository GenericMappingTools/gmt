#	$Id: GNUmakefile,v 1.49 2011-03-25 16:08:03 remko Exp $
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
#		GNU Makefile for GMT Documentation
#
#	The following commands are available to create all the GMT documentation
#
#	make all	: same as : make pdf html
#	make pdf	: Create GMT_Docs.pdf, GMT_Tutorial.pdf and GMT_API.pdf
#	make html	: Create GMT_Docs.html, GMT_Tutorial.html and GMT_API.html
#	make install	: make all; and then install in place
#
#	All the above commands prepare all the necessary graphs and text to be included.
#	Make sure, however, that the examples are already made by running "make examples"
#	
#	make prepro	: Only create the prerequisites.
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, Lab for Satellite Altimetry, NOAA
#
#	Date:		08-Feb-2007
#-------------------------------------------------------------------------------

include ../src/config.mk

.SUFFIXES:	.eps .pdf .tex .ps .sh

# Papersize
PAPER=a4

# LaTeX commands
PDFLATEX=pdflatex --interaction=batchmode
HTLATEX=htlatex
HTLATEX_OPT="GMT_tex4ht_macros,xhtml,mathml,fn-in"
SEDIMG=sed -E 's:src="(scripts|fig|ppt)/:src="images/:g'

# PNG resolution
PNGRES=100

# PS and TEX files used by all documents
ALLPS=	fig/GMT_covertext.eps scripts/GMT_coverlogo.ps
ALLTEX=	GMT_version.tex GMT_macros.tex GMT_Cover.tex GMT_tex4ht_macros.cfg

# PS, PDF, PNG and TEX files used to make Tutorial
TUTPS=	fig/GMT_Environment.eps scripts/GMT_pstext_clearance.ps scripts/GMT_pstext_justify.ps \
	scripts/GMT_nearneighbor.ps scripts/GMT_atan.ps $(ALLPS)
TUTPDF=$(TUTPS:.ps=.pdf)
TUTPNG=$(TUTPS:.ps=.png)
TUTTEX=GMT_Tutorial.tex $(ALLTEX)

# PS, PDF, PNG and TEX files used to make API documentation
APIPS=
APIPDF=$(APIPS:.ps=.pdf)
APIPNG=$(APIPS:.ps=.png)
APITEX=GMT_API.tex $(ALLTEX)

# PS files to be converted from example PS files
EX_PS=	examples/example_01.ps examples/example_02.ps examples/example_03.ps examples/example_04.ps \
	examples/example_04c.ps examples/example_05.ps examples/example_06.ps examples/example_07.ps \
	examples/example_08.ps examples/example_09.ps examples/example_10.ps examples/example_11.ps \
	examples/example_12.ps examples/example_13.ps examples/example_14.ps examples/example_15.ps \
	examples/example_16.ps examples/example_17.ps examples/example_18.ps examples/example_19.ps \
	examples/example_20.ps examples/example_21.ps examples/example_22.ps examples/example_23.ps \
	examples/example_24.ps examples/example_25.ps examples/example_26.ps examples/example_27.ps \
	examples/example_28.ps examples/example_29.ps examples/example_30.ps examples/example_31.ps

# Ghostscript font lookup path
FONTPATH=examples/ex31/fonts

# PS files to be converted from animation PS files
AN_PS=	examples/anim_01.ps examples/anim_02.ps examples/anim_03.ps examples/anim_04.ps

# TEX files of .def files used in the Cookbook that are to retain the comments
DEF_TEX=	scripts/volcano.tex scripts/bullseye.tex scripts/functions.tex

# PS files that need special attention
APP_F_PS=scripts/GMT_App_F_dingbats.ps scripts/GMT_App_F_iso+.ps scripts/GMT_App_F_stand+.ps \
	scripts/GMT_App_F_symbol.ps
APP_J_PS=scripts/GMT_App_J_1.ps scripts/GMT_App_J_2.ps scripts/GMT_App_J_3.ps
APP_N_PS=scripts/GMT_App_N_1.ps
APP_O_PS= \
	scripts/GMT_App_O_1.ps scripts/GMT_App_O_2.ps scripts/GMT_App_O_3.ps \
	scripts/GMT_App_O_4.ps scripts/GMT_App_O_5.ps scripts/GMT_App_O_6.ps scripts/GMT_App_O_7.ps \
	scripts/GMT_App_O_8.ps scripts/GMT_App_O_9.ps
SQRT_PS=scripts/GMT_linear.ps scripts/GMT_log.ps scripts/GMT_pow.ps
IS_EPS=	fig/GMT_covertext.eps fig/GMT_Environment.eps
RGBPS=	scripts/GMT_RGBchart_a4.ps scripts/GMT_RGBchart_letter.ps scripts/GMT_RGBchart_tabloid.ps

# PS files for GMT_Docs to be created from scripts. Scripts have to be converted to TEX (with comments)
DOCPS0=scripts/GMT_App_P_1.ps scripts/GMT_App_P_2.ps scripts/GMT_App_M_2.ps

# PS files for GMT_Docs to be created from scripts. Scripts have to be converted to TEX (without comments)
DOCPS1=scripts/GMT_App_K_1.ps scripts/GMT_App_K_2.ps scripts/GMT_App_K_3.ps scripts/GMT_App_K_4.ps \
	scripts/GMT_App_K_5.ps $(APP_O_PS) scripts/GMT_-B_custom.ps \
	scripts/GMT_-B_time1.ps scripts/GMT_-B_time2.ps scripts/GMT_-B_time3.ps \
	scripts/GMT_-B_time4.ps scripts/GMT_-B_time5.ps scripts/GMT_-B_time6.ps scripts/GMT_-B_time7.ps \
	$(SQRT_PS) scripts/GMT_linear_cal.ps scripts/GMT_linear_d.ps scripts/GMT_polar.ps \
	scripts/GMT_TM.ps scripts/GMT_albers.ps scripts/GMT_az_equidistant.ps \
	scripts/GMT_cassini.ps scripts/GMT_eckert4.ps scripts/GMT_eckert6.ps scripts/GMT_equi_cyl.ps \
	scripts/GMT_equidistant_conic.ps scripts/GMT_gall_stereo.ps scripts/GMT_general_cyl.ps \
	scripts/GMT_gnomonic.ps scripts/GMT_grinten.ps scripts/GMT_hammer.ps \
	scripts/GMT_lambert_az_hemi.ps scripts/GMT_lambert_az_rect.ps scripts/GMT_lambert_conic.ps \
	scripts/GMT_mercator.ps scripts/GMT_miller.ps scripts/GMT_mollweide.ps \
	scripts/GMT_obl_merc.ps scripts/GMT_orthographic.ps scripts/GMT_perspective.ps scripts/GMT_polyconic.ps \
	scripts/GMT_robinson.ps scripts/GMT_sinus_int.ps scripts/GMT_sinusoidal.ps \
	scripts/GMT_stereographic_general.ps scripts/GMT_stereographic_polar.ps \
	scripts/GMT_stereographic_rect.ps scripts/GMT_stereonets.ps scripts/GMT_transverse_merc.ps \
	scripts/GMT_winkel.ps

# PS files for GMT_Docs to be created from scripts. Scripts do not need to be converted to TEX
DOCPS2=scripts/GMT_-B_geo_1.ps scripts/GMT_-B_geo_2.ps scripts/GMT_-B_linear.ps scripts/GMT_-B_log.ps \
	scripts/GMT_-B_pow.ps scripts/GMT_utm_zones.ps \
	scripts/GMT_-J.ps scripts/GMT_-OK.ps scripts/GMT_-P.ps scripts/GMT_-R.ps scripts/GMT_-U.ps \
	scripts/GMT_-XY.ps scripts/GMT_registration.ps scripts/GMT_App_E.ps \
	$(APP_F_PS) scripts/GMT_App_G.ps $(APP_J_PS) scripts/GMT_App_M_1.ps $(APP_N_PS) \
	scripts/GMT_Defaults_1a.ps scripts/GMT_Defaults_1b.ps scripts/GMT_Defaults_1c.ps scripts/GMT_volcano.ps \
	scripts/GMT_color_interpolate.ps

# Prepared images for GMT_Docs
DOCIMG=	fig/JLuis.png fig/Scharroo.png fig/Smith.png fig/Wessel.png \
	ppt/formatpicture.png ppt/rendering.png \
	fig/gimp-panel.png fig/gimp-sliders.png fig/hsv-cone.png

# All PS, EPS, PDF and PNG files for GMT_Docs
DOCPS=	$(DOCPS0) $(DOCPS1) $(DOCPS2) $(ALLPS) $(RGBPS)
DOCEPS= $(IS_EPS)
DOCPDF= $(DOCPS:.ps=.pdf) $(DOCEPS:.eps=.pdf) $(EX_PS:.ps=.pdf) $(AN_PS:.ps=.pdf)
DOCPNG=	$(DOCPDF:.pdf=.png)

# All TEX files needed for GMT_Docs
DOCTEX= \
	GMT_Docs.tex GMT_Appendix_A.tex GMT_Appendix_B.tex GMT_Appendix_C.tex GMT_Appendix_D.tex \
	GMT_Appendix_E.tex GMT_Appendix_F.tex GMT_Appendix_G.tex GMT_Appendix_H.tex GMT_Appendix_I.tex \
	GMT_Appendix_J.tex GMT_Appendix_K.tex GMT_Appendix_L.tex GMT_Appendix_M.tex GMT_Appendix_N.tex \
	GMT_Appendix_N_inc.tex GMT_Appendix_O.tex GMT_Appendix_P.tex \
	GMT_Chapter_1.tex GMT_Chapter_2.tex GMT_Chapter_3.tex GMT_Chapter_4.tex GMT_Chapter_5.tex \
	GMT_Chapter_6.tex GMT_Chapter_7.tex GMT_Chapter_8.tex GMT_Chapter_9.tex \
	GMT_Frontmatter.tex $(DOCPS0:.ps=.tex) $(DOCPS1:.ps=.tex) $(EX_PS:.ps=.tex) $(AN_PS:.ps=.tex) $(DEF_TEX) $(ALLTEX)

all:	pdf html
install::	all

# Shortcut, just to preprocess all required files
prep prepro:	prep-ps prep-pdf prep-png prep-tex $(DOCIMG)
prep-ps:	$(DOCPS) $(TUTPS) $(APIPS)
prep-pdf:	$(DOCPDF) $(TUTPDF) $(APIPDF)
prep-png:	$(DOCPNG) $(TUTPNG) $(APIPNG)
prep-tex:	$(DOCTEX) $(TUTTEX) $(APITEX)

# Test if plots are OK
tests doctests:	ps
	@cd scripts ; bash run_doc_tests.sh

# Cleanup, before or after
clean:
	\rm -f *% .gmt* gmt.conf
	\rm -f *.aux *.lot *.lof *.toc *.pdf *.out *.dvi *.ind *.ilg *.idx *.log *.ps \
		*.4ct *.4tc *.css *.html *.idv *.lg *.tmp *.xref *-js.tex
	\rm -f GMT_Appendix_N_inc.tex
	\rm -f scripts/*.{tex,eps,pdf,csh,ps,png} scripts/sqrt.d* scripts/.gmt* scrips/gmt.conf
	\rm -f examples/*.{tex,ps,pdf,png}
	\rm -f fig/*.pdf fig/GMT_*.png
	\rm -rf GMT_Docs GMT_Tutorial GMT_API

clean-png:
	\rm -f fig/GMT_*.png scripts/*.png

spotless:	clean
	\rm -f GMT_version.tex

# Shortcuts, just to do a couple of things
pdf:	docs tutorial api man
docs:	pdf/GMT_Docs.pdf
tutorial: pdf/GMT_Tutorial.pdf
api:	pdf/GMT_API.pdf
man:	pdf/GMT_Manpages.pdf

html:	htdocs httutorial htapi htman
htdocs:	html/GMT_Docs.html
httutorial: html/GMT_Tutorial.html
htapi:	html/GMT_API.html
htman:	html/man/blockmean.html

# Convert the PS files of the examples and animations to PS and PDF, while stripping off the time tag and
# rotating landscape images back to portrait.
# The extra FONTPATH options is only needed for example 31, but we do it here for all.
$(EX_PS:.ps=.pdf) $(AN_PS:.ps=.pdf):	%.pdf: %.ps
	ps2raster -Au -P -Tf -C-sFONTPATH=$(FONTPATH) $*.ps
$(EX_PS:.ps=.png) $(AN_PS:.ps=.png):    %.png: %.ps
	ps2raster -Au -P -Tg -E$(PNGRES) -C-sFONTPATH=$(FONTPATH) $*.ps

%.pdf: %.ps
	ps2raster -A -P -Tf $*.ps

%.png: %.ps
	ps2raster -A -P -Tg -E$(PNGRES) $*.ps

# To convert EPS to PDF
%.pdf:	%.eps
	ps2raster -P -Tf $*.eps

%.png:  %.eps
	ps2raster -P -Tg -E$(PNGRES) $*.eps

# Graphics of Appendices F, J, N, O need special attention. One script makes several PS and PDF files.
$(APP_F_PS):	scripts/GMT_App_F.sh
	$(MAKE) APP=F run_app

$(APP_J_PS):	scripts/GMT_App_J.sh
	$(MAKE) APP=J run_app

$(APP_N_PS) GMT_Appendix_N_inc.tex:	scripts/GMT_App_N.sh
	$(MAKE) APP=N run_app

$(APP_O_PS):	scripts/GMT_App_O.sh $(APP_O_PS:.ps=.sh)
	$(MAKE) APP=O run_app

run_app:
	cd scripts ; bash GMT_App_$(APP).sh

# The RGB prints should not be cropped.
$(RGBPS):	scripts/GMT_RGBchart.sh
	cd scripts ; bash GMT_RGBchart.sh $(word 3,$(subst _, ,$*))

# How to create the examples and animations:
examples/%.ps:
	cd examples ; $(MAKE) examples animations

# How to run the scripts in the scripts directory to make PS files
scripts/%.ps:	scripts/%.sh scripts/functions.sh
	cd scripts ; bash $*.sh

# Turn scripts into TeX files for inclusion in the documentation. Remove comments in these.
$(DOCPS1:.ps=.tex):	%.tex:	%.sh
	pr -t -e $*.sh | egrep -v '^#|^$$|. functions.sh' > $*.tex

# Turn scripts into TeX files for inclusion in the documentation. Keep comments in these.
%.tex:	%.sh
	pr -t -e $*.sh | egrep -v 'Id:|. functions.sh' > $*.tex

# Keep the comments in the examples, but remove CVS tag.
$(EX_PS:.ps=.tex):	%.tex:	%.ps
	pr -t -e $(subst example_,*/job,$*).sh | egrep -v 'Id:|. ../functions.sh' > $*.tex
$(AN_PS:.ps=.tex):	%.tex:	%.ps
	pr -t -e $(subst anim_,*/anim_,$*).sh | egrep -v 'Id:|. ../functions.sh' > $*.tex
scripts/volcano.tex:
	pr -t -e ../share/custom/volcano.def | egrep -v 'Id:' > $@
scripts/bullseye.tex:
	pr -t -e examples/ex20/bullseye.def | egrep -v 'Id:' > $@

# Additional dependency for some graphs
$(SQRT_PS) $(SQRT_PS:.ps=.pdf):	scripts/sqrt.d scripts/sqrt.d10
scripts/sqrt.d scripts/sqrt.d10:	scripts/GMT_dummydata.sh
	cd scripts ; bash GMT_dummydata.sh

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
	\mv GMT_Docs.pdf $@
	\ln -f scripts/GMT_RGBchart_*.pdf pdf
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}

pdf/GMT_Tutorial.pdf:	$(TUTPDF) $(TUTTEX)
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	$(PDFLATEX) GMT_Tutorial
	$(PDFLATEX) GMT_Tutorial
	makeindex -s GMT.ist GMT_Tutorial.idx
	$(PDFLATEX) GMT_Tutorial
	$(PDFLATEX) GMT_Tutorial
	\mv GMT_Tutorial.pdf $@
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}

pdf/GMT_API.pdf:	$(APIPDF) $(APITEX)
	\rm -f GMT_API.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	$(PDFLATEX) GMT_API
	$(PDFLATEX) GMT_API
	makeindex -s GMT.ist GMT_API.idx
	$(PDFLATEX) GMT_API
	$(PDFLATEX) GMT_API
	\mv GMT_API.pdf $@
	\rm -f GMT_API.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}

pdf/GMT_Manpages.pdf:	pdfman.sh ../src/blockmean.1
	bash pdfman.sh -s

html/GMT_Docs.html:	$(DOCPNG) $(DOCIMG) $(DOCTEX)
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi,4ct,4tc,css,html,idv,lg,tmp,xref} GMT_Docs{-js.tex,1.tmp}
	$(HTLATEX) GMT_Docs $(HTLATEX_OPT)
	\mkdir -p html/images
	$(SEDIMG) GMT_Docs.html > $@
	\mv GMT_Docs.css html
	\ln -f $(DOCPNG) $(DOCIMG) html/images
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi,4ct,4tc,css,html,idv,lg,tmp,xref} GMT_Docs{-js.tex,1.tmp}

html/GMT_Tutorial.html:	$(TUTPNG) $(TUTTEX)
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi,4ct,4tc,css,html,idv,lg,tmp,xref} GMT_Tutorial{-js.tex,1.tmp}
	$(HTLATEX) GMT_Tutorial $(HTLATEX_OPT)
	\mkdir -p html/images
	$(SEDIMG) GMT_Tutorial.html > $@
	\mv GMT_Tutorial.css html
	\ln -f $(TUTPNG) html/images
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi,4ct,4tc,css,html,idv,lg,tmp,xref} GMT_Tutorial{-js.tex,1.tmp}

html/GMT_API.html:	$(APIPNG) $(APITEX)
	\rm -f GMT_API.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi,4ct,4tc,css,html,idv,lg,tmp,xref} GMT_API{-js.tex,1.tmp}
	$(HTLATEX) GMT_API $(HTLATEX_OPT)
	\mkdir -p html/images
	$(SEDIMG) GMT_API.html > $@
	\mv GMT_API.css html
	\rm -f GMT_API.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi,4ct,4tc,css,html,idv,lg,tmp,xref} GMT_API{-js.tex,1.tmp}

html/man/blockmean.html:	webman.sh ../src/blockmean.1
	bash webman.sh -s

include Makefile
