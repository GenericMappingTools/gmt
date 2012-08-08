## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005-2011, see the COPYRIGHT file for more information.

# This is the main automake file for netCDF. It builds the different
# netcdf directories. Not all directories are built, depending on the
# options selected during configure.

# This directory stores libtool macros, put there by aclocal.
ACLOCAL_AMFLAGS = -I m4

# These files get added to the distribution.
EXTRA_DIST = README COPYRIGHT RELEASE_NOTES INSTALL test_prog.c lib_flags.am

# Doxygen doesn't build nicely in vpath builds.
# Don't do this; it wipes out any exported values
#DISTCHECK_CONFIGURE_FLAGS = --disable-doxygen

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA = netcdf.pc

# We need a way to automatically generate INSTALL from
# man4/install.doc, now that we've switched from texinfo to doxygen.
# INSTALL:
#	non-existent-doxygen-to-text-utility <  man4/install.doc > INSTALL

# Does the user want to build the V2 API?
if BUILD_V2
V2_TEST = nctest
endif

# Does the user want to build ncgen/ncdump?
if BUILD_UTILITIES
NCGEN3 = ncgen3
NCGEN = ncgen
NCDUMP = ncdump
endif

# Does the user want to build the examples?
if BUILD_EXAMPLES
EXAMPLES = examples
endif

# Is the user building netCDF-4?
if USE_NETCDF4
H5_TEST_DIR = h5_test
LIBSRC4_DIR = libsrc4
NC_TEST4 = nc_test4
endif

# Build the opendap client?
if BUILD_DAP
OCLIB = oc2
DAP2 = libdap2
#if BUILD_UTILITIES
NCDAPTESTDIR = ncdap_test
#endif # BUILD_UTILITIES
endif

# Build libcdmr
if BUILD_CDMREMOTE
LIBCDMR = libcdmr
endif

# Build librpc
if BUILD_RPC
LIBRPC = librpc
endif

# Build UDUNITS?
#if BUILD_UDUNITS
#UDUNITS = udunits
#endif

# Build libcf?
#if BUILD_LIBCF
#LIBCF = libcf
#endif

# Define Test directories
if BUILD_TESTSETS
TESTDIRS = $(V2_TEST) nc_test $(NC_TEST4) $(NCDAPTESTDIR)
endif

# This is the list of subdirs for which Makefiles will be constructed
# and run. ncgen must come before ncdump, because their tests
# depend on it. 
SUBDIRS = include $(OCLIB) $(H5_TEST_DIR) libdispatch libsrc		\
$(LIBSRC4_DIR) $(DAP2) $(LIBCDMR) $(LIBRPC) liblib  \
$(NCGEN3) $(NCGEN) $(NCDUMP) \
$(TESTDIRS) \
man4 $(EXAMPLES) \
$(UDUNITS) $(LIBCF)

# Remove these generated files, for a distclean.
DISTCLEANFILES = VERSION comps.txt test_prog

# The nc-config script helps the user build programs with netCDF.
bin_SCRIPTS = nc-config

# What needs to go in the binrary dist?
BINFILES = README_BINARIES.txt
BINFILES += include/netcdf.h share/man/man3/netcdf.3 lib/libnetcdf.a
ZIPBINFILES = ${prefix}/include/netcdf.h ${prefix}/share/man/man3/netcdf.3 ${prefix}/lib/libnetcdf.a

if BUILD_UTILITIES
BINFILES += bin/ncgen3$(EXEEXT) bin/ncgen$(EXEEXT) bin/ncdump$(EXEEXT)	\
share/man/man1/ncgen.1 share/man/man1/ncdump.1
ZIPBINFILES += ${prefix}/bin/ncgen3$(EXEEXT) ${prefix}/bin/ncgen$(EXEEXT) ${prefix}/bin/ncdump$(EXEEXT)	\
${prefix}/share/man/man1/ncgen.1 ${prefix}/share/man/man1/ncdump.1
endif

if BUILD_DLL
BINFILES += bin/libnetcdf-7.dll lib/libnetcdf.dll.a lib/libnetcdf.a	\
lib/libnetcdf.la lib/netcdfdll.def
ZIPBINFILES += ${prefix}/bin/libnetcdf-7.dll ${prefix}/lib/libnetcdf.dll.a ${prefix}/lib/libnetcdf.a	\
${prefix}/lib/libnetcdf.la ${prefix}/lib/netcdfdll.def
endif # BUILD_DLL

# At Unidata, package up binaries.
ftpbin: install
	echo "Getting binaries from ${prefix}"
	ls -l ${prefix}
	echo "These are the binaries for netCDF @PACKAGE_VERSION@." > ${prefix}/README_BINARIES.txt
	echo "For this build: CC=$(CC) CXX=$(CXX)" >> ${prefix}/README_BINARIES.txt
	echo "CFLAGS=$(CFLAGS) CXXFLAGS=$(CXXFLAGS)" >> ${prefix}/README_BINARIES.txt
	echo "FCFLAGS=$(FCFLAGS) F77FLAGS=$(F77FLAGS) $F90FLAGS=$(F90FLAGS)" >> ${prefix}/README_BINARIES.txt
	which tar
	echo "PATH: $(PATH)"
	tar cf @BINFILE_NAME@ -C ${prefix} ${BINFILES}
	gzip -f @BINFILE_NAME@
	zip -j netcdf_${VERSION}.zip ${ZIPBINFILES}

check_nc_config:
	$(CC) `./nc-config --cflags` test_prog.c -o test_prog `./nc-config --libs`
	./test_prog

install-data-hook:
if BUILD_DLL
	cp liblib/netcdfdll.def ${prefix}/lib
endif # BUILD_DLL
	@echo ''
	@echo '+-------------------------------------------------------------+'
	@echo '| Congratulations! You have successfully installed netCDF!    |'
	@echo '|                                                             |'
	@echo '| You can use script "nc-config" to find out the relevant     |'
	@echo '| compiler options to build your application. Enter           |'
	@echo '|                                                             |'
	@echo '|     nc-config --help                                        |'
	@echo '|                                                             |'
	@echo '| for additional information.                                 |'
	@echo '|                                                             |'
	@echo '| CAUTION:                                                    |'
	@echo '|                                                             |'
	@echo '| If you have not already run "make check", then we strongly  |'
	@echo '| recommend you do so. It does not take very long.            |'
	@echo '|                                                             |'
	@echo '| Before using netCDF to store important data, test your      |'
	@echo '| build with "make check".                                    |'
	@echo '|                                                             |'
	@echo '| NetCDF is tested nightly on many platforms at Unidata       |'
	@echo '| but your platform is probably different in some ways.       |'
	@echo '|                                                             |'
	@echo '| If any tests fail, please see the netCDF web site:          |'
	@echo '| http://www.unidata.ucar.edu/software/netcdf/                |'
	@echo '|                                                             |'
	@echo '| NetCDF is developed and maintained at the Unidata Program   |'
	@echo '| Center. Unidata provides a broad array of data and software |' 
	@echo '| tools for use in geoscience education and research.         |'
	@echo '| http://www.unidata.ucar.edu                                 |'
	@echo '+-------------------------------------------------------------+'
	@echo ''
