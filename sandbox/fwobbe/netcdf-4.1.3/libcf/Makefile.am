## This is a automake file, part of Unidata's NetCDF CF Library package.
# Copyright 2006, see the COPYRIGHT file for more information.

# This is the main automake file for the NetCDF CF Library. 

# $Id$

# These files get added to the distribution.
EXTRA_DIST = COPYRIGHT

ACLOCAL_AMFLAGS = -I m4

# Does the user want to build cfcheck?
if BUILD_CFCHECK
CFCHECK = cfcheck
endif

# Does the user want to build the docs?
if BUILD_DOCS
DOC = doc
endif

# Does the user want to build the gridspec?
if BUILD_GRIDSPEC
#GRIDSPEC = gridspec
endif

SUBDIRS = $(GRIDSPEC) src $(CFCHECK) $(DOC) 

check_install: check install

# What needs to go in the binrary dist?
BINFILES = README_BINARIES.txt include/libcf.h lib/libcf.a

# At Unidata, package up binaries.
ftpbin: install
	echo "Getting binaries from ${prefix}"
	ls ${prefix}
	echo "These are the binaries for libCF @PACKAGE_VERSION@." > ${prefix}/README_BINARIES.txt
	echo "For this build: CC=$(CC) CXX=$(CXX) FC=$(FC) F77=$(F77) $F90=$(F90)" >> ${prefix}/README_BINARIES.txt
	echo "CFLAGS=$(CFLAGS) CXXFLAGS=$(CXXFLAGS)" >> ${prefix}/README_BINARIES.txt
	echo "FCFLAGS=$(FCFLAGS) F77FLAGS=$(F77FLAGS) $F90FLAGS=$(F90FLAGS)" >> ${prefix}/README_BINARIES.txt
	tar cf binary-libcf-@PACKAGE_VERSION@.tar -C ${prefix} ${BINFILES}
	gzip -f binary-libcf-@PACKAGE_VERSION@.tar

