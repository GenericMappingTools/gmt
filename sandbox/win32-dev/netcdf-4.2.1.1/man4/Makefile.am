## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005-2006, see the COPYRIGHT file for more information.
# This file builds the netcdf documentation.

# The "all" target does nothing in this directory. The "dist" target
# *does* build the documentation, because the distribution contains
# all of the built documentation.

# The "install" target installs the documentation in a directory
# called $(prefix)/doc/$(PACKAGE)-$(VERSION).

# $Id$

# These are the source files for all the netcdf manuals.
info_TEXINFOS = netcdf.texi netcdf-install.texi netcdf-c.texi		\
netcdf-f77.texi netcdf-f90.texi netcdf-cxx.texi netcdf-tutorial.texi	
#netcdf-cxx4.texi

# Get lists of the coresponding ps, info, and pdf files.
html_docs = ${info_TEXINFOS:.texi=.html} 
html_dirs = ${info_TEXINFOS:.texi=} 
ps_docs = ${info_TEXINFOS:.texi=.ps} 
dvi_docs = ${info_TEXINFOS:.texi=.dvi} 
pdf_docs = ${info_TEXINFOS:.texi=.pdf}
info_docs= ${info_TEXINFOS:.texi=.info}
txt_docs= ${info_TEXINFOS:.texi=.txt}

# These are html versions of the man pages.
html_mans = ncdump-man-1.html ncgen-man-1.html netcdf_f77-man.html	\
netcdf-man-3.html netcdf_f90-man.html ncgen3-man-1.html \
nccopy-man-1.html

# These files will be included with the dist.
EXTRA_DIST = netcdf.m4 $(pdf_docs) $(html_mans) $(txt_docs) $(dvi_docs)	\
$(ps_docs) $(info_docs) $(html_docs) netcdf_f77.3

#doc_DATA = $(pdf_docs) $(html_mans) $(txt_docs) $(ps_docs)	\
#$(info_docs) $(html_docs)

# These are lists of the source code which will be inserted into the
# netCDF Tutorial. Note that the tutorial always contains info about
# all 4 APIs, not just the ones that are installed.
c_examples = simple_xy_wr.c simple_xy_rd.c sfc_pres_temp_wr.c		\
sfc_pres_temp_rd.c pres_temp_4D_wr.c pres_temp_4D_rd.c simple_nc4_wr.c	\
simple_nc4_rd.c simple_xy_nc4_wr.c simple_xy_nc4_rd.c
f77_examples = simple_xy_wr.f simple_xy_rd.f sfc_pres_temp_wr.f	\
sfc_pres_temp_rd.f pres_temp_4D_wr.f pres_temp_4D_rd.f		\
simple_xy_nc4_wr.f simple_xy_nc4_rd.f
f90_examples = simple_xy_wr.f90 simple_xy_rd.f90 sfc_pres_temp_wr.f90	\
sfc_pres_temp_rd.f90 pres_temp_4D_wr.f90 pres_temp_4D_rd.f90		\
simple_xy_nc4_wr.f90 simple_xy_nc4_rd.f90 simple_xy_par_wr.f90		\
simple_xy_par_rd.f90
cxx_examples = simple_xy_wr.cpp simple_xy_rd.cpp sfc_pres_temp_wr.cpp	\
sfc_pres_temp_rd.cpp pres_temp_4D_wr.cpp pres_temp_4D_rd.cpp		
cdl_examples = simple_xy.cdl sfc_pres_temp.cdl pres_temp_4D.cdl

# All the documents depend on defines.texi for some common
# definitions.
netcdf_TEXINFOS = defines.texi
netcdf_install_TEXINFOS = defines.texi
netcdf_c_TEXINFOS = defines.texi
netcdf_f77_TEXINFOS = defines.texi
netcdf_f90_TEXINFOS = defines.texi
netcdf_cxx_TEXINFOS = defines.texi
#netcdf_cxx4_TEXINFOS = defines.texi
netcdf_tutorial_TEXINFOS = defines.texi ${c_examples} ${f77_examples}	\
${f90_examples} ${cxx_examples} ${cdl_examples}

# Get texi2dvi to shut the hell up, and work better for parallel
# builds. The --tidy option may be useful getting parallel builds to
# work someday.
TEXI2DVI = texi2dvi -s 

# Turn off parallel builds in this directory.
.NOTPARALLEL:

# To build documentation you must have M4 in their path.
M4=m4

# Rule to generate the text copies of the documentation.
.texi.txt: 
	makeinfo -o $*.txt --no-headers $(srcdir)/$<

# Generate the HTML version of the netcdf-3 C man page.
netcdf-man-3.html: $(top_srcdir)/libsrc/netcdf.3
	groff -man -Thtml $? > $@

# Generate the HTML version of the netcdf-3 F77 man page.
netcdf_f77.3: netcdf.m4
	m4 $(M4FLAGS) $(ARGS_MANPAGE) $? >$@ || rm $@

netcdf_f77-man.html: netcdf_f77.3
	groff -man -Thtml $? > $@

# Generate the HTML version of the netcdf-3 F90 man page.
netcdf_f90-man.html: $(top_srcdir)/f90/netcdf_f90.3
	groff -man -Thtml $? > $@

# Generate the HTML version of the ncgen man page.
ncgen-man-1.html: $(top_srcdir)/ncgen/ncgen.1
	groff -man -Thtml $? > $@

# Generate the HTML version of the ncgen3 man page.
ncgen3-man-1.html: $(top_srcdir)/ncgen3/ncgen3.1
	groff -man -Thtml $? > $@

# Generate the HTML version of the ncdump man page.
ncdump-man-1.html: $(top_srcdir)/ncdump/ncdump.1
	groff -man -Thtml $? > $@

# Generate the HTML version of the nccopy man page.
nccopy-man-1.html: $(top_srcdir)/ncdump/nccopy.1
	groff -man -Thtml $? > $@

# Copy C examples to this directory.
${c_examples}:
	sed -e 's/{/@{/' -e 's/}/@}/' $(top_srcdir)/examples/C/$@ > $@

# Copy F77 examples to this directory.
${f77_examples}:
	sed -e 's/{/@{/' -e 's/}/@}/' $(top_srcdir)/examples/F77/$@ > $@

# Copy F90 examples to this directory.
${f90_examples}:
	sed -e 's/{/@{/' -e 's/}/@}/' $(top_srcdir)/examples/F90/$@ > $@

# Copy CXX examples to this directory.
${cxx_examples}:
	sed -e 's/{/@{/' -e 's/}/@}/' $(top_srcdir)/examples/CXX/$@ > $@

# Copy CDL examples to this directory.
${cdl_examples}:
	sed -e 's/{/@{/' -e 's/}/@}/' $(top_srcdir)/examples/CDL/$@ > $@

netcdf.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf.texi

netcdf-install.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-install.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-install.texi

netcdf-c.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-c.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-c.texi

netcdf-f77.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f77.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f77.texi

netcdf-cxx.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-cxx.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-cxx.texi

# netcdf-cxx4.html:
# 	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-cxx4.texi
# 	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-cxx4.texi

netcdf-f90.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f90.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f90.texi

netcdf-tutorial.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-tutorial.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-tutorial.texi

# This builds the docs from source, if necessary, and tars up
# everything needed for the website. Run this and copy the resulting
# tarball to the /contents/netcdf/docs directory to update the on-line
# docs.
web-tarball: $(pdf_docs) $(html_mans) $(txt_docs) $(dvi_docs) $(ps_docs) $(info_docs) $(html_docs)
	tar cf netcdf_info.tar *.info*
	gzip -f netcdf_info.tar
	zip netcdf_info *.info
	tar cf netcdf_pdf.tar *.pdf
	gzip -f netcdf_pdf.tar
	zip netcdf_pdf *.pdf
	tar cf netcdf_html.tar *.html $(html_dirs)
	gzip -f netcdf_html.tar
	zip netcdf_html *.html $(html_dirs)
	tar cf netcdf_txt.tar *.txt
	gzip -f netcdf_txt.tar
	zip netcdf_txt *.txt
	cp $(top_srcdir)/ncdump/ncdump.1 .
	cp $(top_srcdir)/ncgen/ncgen.1 .
	cp $(top_srcdir)/ncdump/nccopy.1 .
	cp $(top_srcdir)/ncgen3/ncgen3.1 .
	cp $(top_srcdir)/libsrc/netcdf.3 .
	cp $(top_srcdir)/fortran/netcdf_f77.3 .
	cp $(top_srcdir)/f90/netcdf_f90.3 .
	tar cf netcdf_man.tar netcdf.3 netcdf_f77.3 netcdf_f90.3 ncdump.1 ncgen.1 ncgen3.1
	gzip -f netcdf_man.tar
	zip netcdf_man netcdf.3 netcdf_f77.3 netcdf_f90.3 ncdump.1 ncgen.1 ncgen3.1
	tar cf netcdf_docs.tar $(html_dirs) *.pdf *.txt *.html *.tar.gz *.zip 
	gzip -f netcdf_docs.tar

# Clean up all the junk we created!
MAINTAINERCLEANFILES = $(ps_docs) $(pdf_docs) $(txt_docs) $(info_docs)	\
$(html_docs) $(man_MANS) ${c_examples} ${f77_examples} ${f90_examples}	\
${cxx_examples} ${cdl_examples} *.1 *.3 ncdump-man-1.html		\
ncgen-man-1.html ncgen3-man-1.html netcdf_f77-man.html netcdf-man-3.html \
netcdf_f90-man.html stamp-* version* netcdf_f77.3 nccopy-man-1.html

