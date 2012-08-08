## This is a automake file, part of Unidata's NetCDF CF Library package.
# Copyright 2006, see the COPYRIGHT file for more information.

# This is the automake file for the NetCDF CF Library documentation.

# $Id$

info_TEXINFOS = libcf.texi

# Get texi2dvi to shut the hell up!
TEXI2DVI = texi2dvi -s

# Get lists of the coresponding ps, info, and pdf files.
html_docs = ${info_TEXINFOS:.texi=.html} 
ps_docs = ${info_TEXINFOS:.texi=.ps} 
pdf_docs = ${info_TEXINFOS:.texi=.pdf}
info_docs= ${info_TEXINFOS:.texi=.info}

EXTRA_DIST = defines.texi

# Distributes the ps, info, and pdf files with the distribution.
dist_pkgdata_DATA = $(ps_docs) $(info_docs) $(pdf_docs) 

# Automake seems to be broken with respect to generating html
# output. Also it steals the *.html targets, no matter what I do. So
# this cheesy phoney target does the html builds.
libcf_html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) libcf.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) libcf.texi

# This isn't made by all, but tars up everything needed for the website
web-tarball: all libcf_html
	tar cf libcf_info.tar *.info*
	gzip -f libcf_info.tar
	zip libcf_info *.info &> /dev/null
	tar cf libcf_pdf.tar *.pdf 
	gzip -f libcf_pdf.tar
	zip libcf_pdf *.pdf &> /dev/null
	tar cf libcf_html.tar libcf libcf.html
	gzip -f libcf_html.tar
	zip libcf_html *.pdf &> /dev/null
	tar cf libcf_docs.tar $(nmans) *.pdf *.html *.tar.gz *.zip libcf
	gzip -f libcf_docs.tar
