#	$Id: GNUmakefile,v 1.34 2009-02-13 19:47:58 remko Exp $
#
#	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
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
#		Makefile for GMT Documentation
#		GNU, Sys V, and BSD Compatible
#
#	The following commands are available to create all the GMT documentation
#
#	make all	: same as : make pdf html
#	make pdf	: Create GMT_Docs.pdf and GMT_Tutorial.pdf
#	make html	: Create the HTML versions of GMT_Docs and GMT_Tutorial
#	make install	: same as : make all
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
# THINGS TO REMEMBER:
# 1. If we ever get strange messages like 
#    pstoimg: Error: "/sw/bin/pnmcrop -verbose  < /tmp/l2h29411/p29461.pnm | /sw/bin/ppmquant -floyd 256 | \
#    /sw/bin/pnmtopng -interlace -trans 'gray85'  > img294.png" failed: Invalid argument
#    it means that we must set export RGBDEF=/usr/X11/share/X11/rgb.txt
#    as for some reason it cannot be found where it is expected (/usr/X11R6/lib/X11)
# 2. If latex complains that it cannot find html.sty you need to run mktexlsr (as admin)
#

include ../src/makegmt.macros

.SUFFIXES:	.eps .pdf .tex .ps .sh

DOC=../share/doc/gmt
PAPER=a4
L2H=latex2html -local_icons -white -show_section_numbers -image_type png -bottom_navigation -notransparent
# NOTICE: If inline graphics shows up with underlines then you might need to edit
# your l2hconf.pm file and remove the -Ppdf option from DVIPSOPT.
LATEX=latex --interaction=batchmode
PDFLATEX=pdflatex --interaction=batchmode

# EPS and TEX files used by all documents
ALLEPS=	fig/GMT_covertext.eps scripts/GMT_coverlogo.eps
ALLTEX=	GMT_version.tex GMT_macros.tex GMT_Cover.tex

# EPS, PDF and TEX files used to make Tutorial
TUTEPS=	fig/GMT_Environment.eps scripts/GMT_pstext_clearance.eps scripts/GMT_pstext_justify.eps \
	scripts/GMT_nearneighbor.eps scripts/GMT_atan.eps $(ALLEPS)
TUTPDF=$(TUTEPS:.eps=.pdf)
TUTTEX=GMT_Tutorial.tex $(ALLTEX)

# EPS files to be converted from example PS files
EX_EPS=	scripts/example_01.eps scripts/example_02.eps scripts/example_03.eps scripts/example_04.eps \
	scripts/example_04c.eps scripts/example_05.eps scripts/example_06.eps scripts/example_07.eps \
	scripts/example_08.eps scripts/example_09.eps scripts/example_10.eps scripts/example_11.eps \
	scripts/example_12.eps scripts/example_13.eps scripts/example_14.eps scripts/example_15.eps \
	scripts/example_16.eps scripts/example_17.eps scripts/example_18.eps scripts/example_19.eps \
	scripts/example_20.eps scripts/example_21.eps scripts/example_22.eps scripts/example_23.eps \
	scripts/example_24.eps scripts/example_25.eps scripts/example_26.eps scripts/example_27.eps \
	scripts/example_28.eps scripts/example_29.eps

# EPS files to be converted from animation PS files
AN_EPS=	scripts/anim_01.eps scripts/anim_02.eps scripts/anim_03.eps scripts/anim_04.eps

# TEX files of examples used in the Cookbook that are to retain the comments
EX_TEX=	scripts/volcano.tex scripts/bullseye.tex scripts/GMT_App_M_2.tex \
	scripts/GMT_App_P_1.tex scripts/GMT_App_P_2.tex $(EX_EPS:.eps=.tex)

AN_TEX=	$(AN_EPS:.eps=.tex)

# EPS files that need special attention
APP_F_EPS=scripts/GMT_App_F_dingbats.eps scripts/GMT_App_F_iso+.eps scripts/GMT_App_F_stand+.eps \
	scripts/GMT_App_F_symbol.eps
APP_J_EPS=scripts/GMT_App_J_1.eps scripts/GMT_App_J_2.eps scripts/GMT_App_J_3.eps
APP_N_EPS=scripts/GMT_App_N_1.eps
APP_O_EPS= \
	scripts/GMT_App_O_1.eps scripts/GMT_App_O_2.eps scripts/GMT_App_O_3.eps \
	scripts/GMT_App_O_4.eps scripts/GMT_App_O_5.eps scripts/GMT_App_O_6.eps scripts/GMT_App_O_7.eps \
	scripts/GMT_App_O_8.eps scripts/GMT_App_O_9.eps
SQRT_EPS=scripts/GMT_linear.eps scripts/GMT_log.eps scripts/GMT_pow.eps
IS_EPS=	fig/GMT_covertext.eps fig/GMT_Environment.eps
RGBEPS=	scripts/GMT_RGBchart_a4.eps scripts/GMT_RGBchart_letter.eps scripts/GMT_RGBchart_tabloid.eps

# EPS files for GMT_Docs to be created from scripts. Scripts have to be converted to TEX (without comments)
DOCEPS1=scripts/GMT_App_K_1.eps scripts/GMT_App_K_2.eps scripts/GMT_App_K_3.eps scripts/GMT_App_K_4.eps \
	scripts/GMT_App_K_5.eps $(APP_O_EPS) \
	scripts/GMT_-B_time1.eps scripts/GMT_-B_time2.eps scripts/GMT_-B_time3.eps \
	scripts/GMT_-B_time4.eps scripts/GMT_-B_time5.eps scripts/GMT_-B_time6.eps scripts/GMT_-B_time7.eps \
	$(SQRT_EPS) scripts/GMT_linear_cal.eps scripts/GMT_linear_d.eps scripts/GMT_polar.eps \
	scripts/GMT_TM.eps scripts/GMT_albers.eps scripts/GMT_az_equidistant.eps \
	scripts/GMT_cassini.eps scripts/GMT_eckert4.eps scripts/GMT_eckert6.eps scripts/GMT_equi_cyl.eps \
	scripts/GMT_equidistant_conic.eps scripts/GMT_gall_stereo.eps scripts/GMT_general_cyl.eps \
	scripts/GMT_gnomonic.eps scripts/GMT_grinten.eps scripts/GMT_hammer.eps \
	scripts/GMT_lambert_az_hemi.eps scripts/GMT_lambert_az_rect.eps scripts/GMT_lambert_conic.eps \
	scripts/GMT_mercator.eps scripts/GMT_miller.eps scripts/GMT_mollweide.eps \
	scripts/GMT_perspective.eps scripts/GMT_obl_merc.eps scripts/GMT_orthographic.eps \
	scripts/GMT_robinson.eps scripts/GMT_sinus_int.eps scripts/GMT_sinusoidal.eps \
	scripts/GMT_stereographic_general.eps scripts/GMT_stereographic_polar.eps \
	scripts/GMT_stereographic_rect.eps scripts/GMT_stereonets.eps scripts/GMT_transverse_merc.eps \
	scripts/GMT_winkel.eps

# EPS files for GMT_Docs to be created from scripts. Scripts do not need to be converted to TEX
DOCEPS2=scripts/GMT_-B_geo_1.eps scripts/GMT_-B_geo_2.eps scripts/GMT_-B_linear.eps scripts/GMT_-B_log.eps \
	scripts/GMT_-B_pow.eps scripts/GMT_utm_zones.eps \
	scripts/GMT_-J.eps scripts/GMT_-OK.eps scripts/GMT_-P.eps scripts/GMT_-R.eps scripts/GMT_-U.eps \
	scripts/GMT_-XY.eps scripts/GMT_App_B_1.eps scripts/GMT_App_B_2.eps scripts/GMT_App_E.eps \
	$(APP_F_EPS) scripts/GMT_App_G.eps $(APP_J_EPS) scripts/GMT_App_M_1.eps scripts/GMT_App_M_2.eps $(APP_N_EPS) \
	scripts/GMT_Defaults_1a.eps scripts/GMT_Defaults_1b.eps scripts/GMT_Defaults_1c.eps scripts/GMT_volcano.eps \
	scripts/GMT_App_P_2.eps scripts/GMT_color_interpolate.eps

# All EPS and PDF files for GMT_Docs
DOCEPS=	$(DOCEPS1) $(DOCEPS2) $(ALLEPS) $(EX_EPS) $(AN_EPS) $(IS_EPS) $(RGBEPS)
DOCPDF= $(DOCEPS:.eps=.pdf)

# All PNG files for GMT_Docs
DOCPNG=	fig/Smith.png fig/Wessel.png ppt/formatpicture.png ppt/rendering.png fig/gimp-panel.png fig/gimp-sliders.png \
	fig/hsv-cone.png

# Additional TEX files to be converted from scripts
SCRTEX=	scripts/getbox.tex scripts/getrect.tex scripts/GMT_dummydata.tex

# All TEX files needed for GMT_Docs
DOCTEX= \
	GMT_Docs.tex GMT_Appendix_A.tex GMT_Appendix_B.tex GMT_Appendix_C.tex GMT_Appendix_D.tex \
	GMT_Appendix_E.tex GMT_Appendix_F.tex GMT_Appendix_G.tex GMT_Appendix_H.tex GMT_Appendix_I.tex \
	GMT_Appendix_J.tex GMT_Appendix_K.tex GMT_Appendix_L.tex GMT_Appendix_M.tex GMT_Appendix_N.tex \
	GMT_Appendix_N_inc.tex GMT_Appendix_O.tex GMT_Appendix_P.tex \
	GMT_Chapter_1.tex GMT_Chapter_2.tex GMT_Chapter_3.tex GMT_Chapter_4.tex GMT_Chapter_5.1.tex \
	GMT_Chapter_5.2.tex GMT_Chapter_5.tex GMT_Chapter_6.1.tex GMT_Chapter_6.2.tex GMT_Chapter_6.3.tex \
	GMT_Chapter_6.4.tex GMT_Chapter_6.tex GMT_Chapter_7.tex GMT_Chapter_8.tex \
	GMT_Frontmatter.tex $(DOCEPS1:.eps=.tex) $(SCRTEX) $(EX_TEX) $(AN_TEX) $(ALLTEX)

all install:	pdf html

# Shortcut, just to preprocess all required files
prepro:		$(DOCTEX) $(DOCEPS) $(DOCPDF) $(DOCPNG) $(TUTTEX) $(TUTEPS) $(TUTPDF) $(RGBEPS) $(RGBPDF)

# Test if plots are OK
tests doctests:	prepro
	@cd scripts ; sh run_doc_tests.sh

# Cleanup, before or after
clean:
	\rm -f *% .gmt*
	\rm -f *.aux *.lot *.lof *.toc *.pdf *.out *.dvi *.ind *.ilg *.idx *.log *.ps *.eps
	\rm -f GMT_Appendix_N_inc.tex
	\rm -f scripts/*.{tex,eps,pdf,csh,ps} scripts/sqrt.d* scripts/.gmtdefaults4* scripts/.gmtcommands4
	\rm -f scripts/anim_*.sh scripts/example_*.sh scripts/volcano.sh scripts/bullseye.sh
	\rm -f fig/*.pdf
	\rm -rf GMT_Docs GMT_Tutorial

spotless:	clean
	\rm -f GMT_version.tex

# Shortcuts, just to do a couple of things
pdf:	$(DOC)/pdf/GMT_Docs.pdf $(DOC)/pdf/GMT_Tutorial.pdf
html:	$(DOC)/html/GMT_Docs/GMT_Docs.html $(DOC)/html/GMT_Tutorial/GMT_Tutorial.html
docs:	$(DOC)/pdf/GMT_Docs.pdf $(DOC)/html/GMT_Docs/GMT_Docs.html
tutorial: $(DOC)/pdf/GMT_Tutorial.pdf $(DOC)/html/GMT_Tutorial/GMT_Tutorial.html

# Create links to script files elsewhere on the system
$(EX_TEX:.tex=.sh):
	$(LN_S) -f ../../share/custom/volcano.def scripts/volcano.sh
	$(LN_S) -f ../../examples/ex20/bullseye.def scripts/bullseye.sh
	for f in `ls ../examples/*/job*.sh` ; do $(LN_S) -f ../$$f scripts/example_`basename $$f|cut -c4-` ; done

$(AN_TEX:.tex=.sh):
	for f in `ls ../examples/anim*/anim*.sh` ; do $(LN_S) -f ../$$f scripts/anim_`basename $$f|cut -c6-` ; done

# Convert the PS files of the examples to EPS and PDF, while stripping off the time tag and
# rotating landscape images back to portrait. Leave intermediate EPS behind.
$(EX_EPS) $(EX_EPS:.eps=.pdf):	../examples/ex01/example_01.ps
	ps2raster -Au -P -Tef -Dscripts ../examples/ex*/$(*F).ps

# Convert the PS files of the animations to EPS and PDF, while stripping off any time tags and
# rotating landscape images back to portrait. Leave intermediate EPS behind.
$(AN_EPS) $(AN_EPS:.eps=.pdf):	../examples/anim01/anim_01.ps
	ps2raster -Au -P -Tef -Dscripts ../examples/anim*/$(*F).ps

# To convert EPS to PDF
$(IS_EPS:.eps=.pdf) $(RGBEPS:.eps=.pdf):	%.pdf:	%.eps
	ps2raster -P -Tf $*.eps

# Graphics of Appendices F, J, N, O need special attention. One script makes several EPS and PDF files.
$(APP_F_EPS) $(APP_F_EPS:.eps=.pdf):	scripts/GMT_App_F.sh
	$(MAKE) APP=F run_app

$(APP_J_EPS) $(APP_J_EPS:.eps=.pdf):	scripts/GMT_App_J.sh
	$(MAKE) APP=J run_app

$(APP_N_EPS) $(APP_N_EPS:.eps=.pdf) GMT_Appendix_N_inc.tex:	scripts/GMT_App_N.sh
	$(MAKE) APP=N run_app

$(APP_O_EPS) $(APP_O_EPS:.eps=.pdf):	scripts/GMT_App_O.sh $(APP_O_EPS:.eps=.sh)
	$(MAKE) APP=O run_app

run_app:
	cd scripts ; cp -f .gmtdefaults4.doc .gmtdefaults4 ; LANG=C sh GMT_App_$(APP).sh
	ps2raster -A -P -Tef scripts/GMT_App_$(APP)_*.ps
	\rm -f scripts/GMT_App_$(APP)_*.ps

# The RGB prints should not be cropped.
$(RGBEPS):	scripts/GMT_RGBchart.sh
	cd scripts ; cp -f .gmtdefaults4.doc .gmtdefaults4 ; LANG=C sh GMT_RGBchart.sh $(word 3,$(subst _, ,$*))

# How to run the scripts in the scripts directory to make EPS and PDF files
.sh.eps .sh.pdf:
	cd scripts ; cp -f .gmtdefaults4.doc .gmtdefaults4 ; LANG=C sh ../$*.sh
	ps2raster -A -P -Tef $*.ps
	\rm -f $*.ps

# Turn scripts into TEX files for inclusion in the documentation. Remove comments in these.
$(SCRTEX) $(DOCEPS1:.eps=.tex):	%.tex:	%.sh
	pr -t -e $*.sh | egrep -v '^#|^$$' > $*.tex

# Keep the comments in the examples, but remove CVS tag.
$(EX_TEX):	%.tex:	%.sh
	pr -t -e $*.sh | egrep -v 'Id:' > $*.tex

$(AN_TEX):	%.tex:	%.sh
	pr -t -e $*.sh | egrep -v 'Id:' > $*.tex

# Make sure gmtdefaults file is created afresh
$(DOCEPS) $(DOCPDF) $(TUTEPS) $(TUTPDF):	scripts/.gmtdefaults4.doc
scripts/.gmtdefaults4.doc:	../share/conf/gmtdefaults_US
	gmtdefaults -Du > $@
	gmtset -G$@ ANNOT_FONT_SIZE_PRIMARY 10p ANNOT_FONT_SIZE_SECONDARY 14p LABEL_FONT_SIZE 14p FRAME_WIDTH 0.05i CHAR_ENCODING ISOLatin1+

# Additional dependency for some graphs
$(SQRT_EPS) $(SQRT_EPS:.eps=.pdf):	scripts/sqrt.d scripts/sqrt.d10
scripts/sqrt.d scripts/sqrt.d10:	scripts/GMT_dummydata.sh
	cd scripts && sh GMT_dummydata.sh

# How to create version info
GMT_version.tex:	GMT_version.tex.in
	@echo "You must first run configure in the main GMT directory"; exit 1

# Dependencies and rules to make PDF documentation
$(DOC)/pdf/GMT_Docs.pdf:	$(DOCPDF) $(DOCPNG) $(DOCTEX)
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	$(PDFLATEX) GMT_Docs
	$(PDFLATEX) GMT_Docs
	makeindex -s GMT.ist GMT_Docs.idx
	$(PDFLATEX) GMT_Docs
	$(PDFLATEX) GMT_Docs
	\mv GMT_Docs.pdf $@
	\cp -p scripts/GMT_RGBchart_*.pdf $(DOC)/pdf
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
$(DOC)/pdf/GMT_Tutorial.pdf:	$(TUTPDF) $(TUTTEX)
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	$(PDFLATEX) GMT_Tutorial
	$(PDFLATEX) GMT_Tutorial
	makeindex -s GMT.ist GMT_Tutorial.idx
	$(PDFLATEX) GMT_Tutorial
	$(PDFLATEX) GMT_Tutorial
	\mv GMT_Tutorial.pdf $@
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}

# Dependencies and rules to make HTML documentation
$(DOC)/html/GMT_Docs/GMT_Docs.html:	$(DOCEPS) $(DOCPNG) $(DOCTEX)
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	\rm -rf GMT_Docs
	$(LATEX) GMT_Docs.tex
	$(LATEX) GMT_Docs.tex
	makeindex -s GMT.ist GMT_Docs.idx
	$(LATEX) GMT_Docs.tex
	$(L2H) -t "GMT -- Technical Reference and Cookbook" GMT_Docs
	\rm -rf GMT_Docs/{index.html,images.*,*.pl,WARNINGS} $(DOC)/html/GMT_Docs
	\mv GMT_Docs $(DOC)/html
	\rm -f GMT_{Docs,Frontmatter,Chapter*,Appendix*}.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
$(DOC)/html/GMT_Tutorial/GMT_Tutorial.html:	$(TUTEPS) $(TUTTEX)
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
	\rm -rf GMT_Tutorial
	$(LATEX) GMT_Tutorial.tex
	$(LATEX) GMT_Tutorial.tex
	makeindex -s GMT.ist GMT_Tutorial.idx
	$(LATEX) GMT_Tutorial.tex
	$(L2H) -t "GMT -- A Map-making Tutorial" GMT_Tutorial
	\rm -rf GMT_Tutorial/{index.html,images.*,*.pl,WARNINGS} $(DOC)/html/GMT_Tutorial
	\mv GMT_Tutorial $(DOC)/html
	\rm -f GMT_Tutorial.{aux,idx,ilg,ind,log,lof,lot,toc,out,dvi}
