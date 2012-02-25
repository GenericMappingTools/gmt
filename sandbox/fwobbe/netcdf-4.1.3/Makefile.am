## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This is the main automake file for netCDF. It builds the different
# netcdf directories. Not all directories are built, depending on the
# options selected during configure.

# This directory stores libtool macros, put there by aclocal.
ACLOCAL_AMFLAGS = -I m4

# These files get added to the distribution.
EXTRA_DIST = README COPYRIGHT RELEASE_NOTES INSTALL test_prog.c	\
test_prog.F test_prog.f90 lib_flags.am

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA = netcdf.pc

# This will go to the man directory and build, if necessary, the
# netcdf-install.txt file, which will be copied into the main
# directory, renamed "INSTALL".
INSTALL:
	cd man4; make netcdf-install.info netcdf-install.txt
	cp man4/netcdf-install.txt INSTALL

# If the user wants the F77 API this will cause it to be built and
# tested.
if BUILD_F77
F77_DIR = fortran
F77_TEST = nf_test
endif

LIBSRC_DIR = libsrc
NC_TEST_DIR = nc_test

# Does the user want to build the V2 API?
if BUILD_V2
V2_TEST = nctest
endif

# Does the user want C++ API?
if BUILD_CXX
CXX_DIR = cxx
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

DISPATCHDIR = libdispatch
ASSEMBLEDIR = liblib

# Does the user want F90 API?
if BUILD_F90
F90_DIR = f90
endif

# Build the new netCDF-4 C++ API?
if BUILD_CXX4
CXX4 = cxx4
endif

# Build the opendap client?
if BUILD_DAP
OCLIB = oc
NCDAP3 = libncdap3
NCDAP4 = libncdap4
DAP2 = libdap2
if BUILD_UTILITIES
NCDAPTESTDIR = ncdap_test
endif # BUILD_UTILITIES
endif

# Build UDUNITS?
if BUILD_UDUNITS
UDUNITS = udunits
endif

# Build libcf?
if BUILD_LIBCF
LIBCF = libcf
endif

# Build libcdmr
if BUILD_CDMREMOTE
LIBCDMR = libcdmr
endif

# This is the list of subdirs for which Makefiles will be constructed
# and run. ncgen must come before ncdump and cxx, because their tests
# depend on it. nf_test depends upon ncgen as well. If not using
# dispatch then NC3DAPDIR must be built right before libsrc and
# NC4DAPDIR must be built right before LIBSRC4_DIR.
SUBDIRS = include $(UDUNITS) $(H5_TEST_DIR) $(DISPATCHDIR) $(OCLIB)	\
libsrc $(LIBSRC4_DIR) $(DAP2) $(LIBCDMR) ${ASSEMBLEDIR} $(V2_TEST)	\
nc_test $(F90_DIR) $(F77_DIR) $(NCGEN3) $(NCGEN) $(NCDUMP) $(F77_TEST)	\
${CXX_DIR} $(CXX4) $(NC_TEST4) $(NCDAPTESTDIR) man4 $(EXAMPLES)		\
$(LIBCF)

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

if BUILD_F77
BINFILES += include/netcdf.inc share/man/man3/netcdf_f77.3	\
lib/libnetcdff.a
ZIPBINFILES += ${prefix}/include/netcdf.inc ${prefix}/share/man/man3/netcdf_f77.3	\
${prefix}/lib/libnetcdff.a
endif

if BUILD_F90
if UPPER_CASE_MOD
BINFILES += include/NETCDF.mod include/TYPESIZES.mod
ZIPBINFILES += ${prefix}/include/NETCDF.mod ${prefix}/include/TYPESIZES.mod
else
BINFILES += include/netcdf.mod include/typesizes.mod
ZIPBINFILES += ${prefix}/include/netcdf.mod ${prefix}/include/typesizes.mod
endif
BINFILES += share/man/man3/netcdf_f90.3
ZIPBINFILES += ${prefix}/share/man/man3/netcdf_f90.3
endif

if BUILD_CXX
BINFILES += include/netcdf.hh include/ncvalues.h include/netcdfcpp.h	\
lib/libnetcdf_c++.a
ZIPBINFILES += ${prefix}/include/netcdf.hh			\
${prefix}/include/ncvalues.h ${prefix}/include/netcdfcpp.h	\
${prefix}/lib/libnetcdf_c++.a
endif

if BUILD_DLL
BINFILES += bin/libnetcdf-7.dll lib/libnetcdf.dll.a lib/libnetcdf.a	\
lib/libnetcdf.la lib/netcdfdll.def
ZIPBINFILES += ${prefix}/bin/libnetcdf-7.dll ${prefix}/lib/libnetcdf.dll.a ${prefix}/lib/libnetcdf.a	\
${prefix}/lib/libnetcdf.la ${prefix}/lib/netcdfdll.def
if BUILD_F77
BINFILES += bin/libnetcdff-5.dll lib/libnetcdff.dll.a	\
lib/libnetcdff.la lib/netcdffdll.def
ZIPBINFILES += ${prefix}/bin/libnetcdff-5.dll			\
${prefix}/lib/libnetcdff.dll.a ${prefix}/lib/libnetcdff.la	\
${prefix}/lib/netcdffdll.def
endif # BUILD_F77
endif # BUILD_DLL

# At Unidata, package up binaries.
ftpbin: install
	echo "Getting binaries from ${prefix}"
	ls -l ${prefix}
	echo "These are the binaries for netCDF @PACKAGE_VERSION@." > ${prefix}/README_BINARIES.txt
	echo "For this build: CC=$(CC) CXX=$(CXX) FC=$(FC) F77=$(F77) $F90=$(F90)" >> ${prefix}/README_BINARIES.txt
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
	$(F77) `./nc-config --fflags` test_prog.F -o test_prog `./nc-config --libs`
	./test_prog
	$(FC) `./nc-config --fflags` test_prog.f90 -o test_prog `./nc-config --libs`
	./test_prog

install-data-hook:
if BUILD_DLL
	cp liblib/netcdfdll.def ${prefix}/lib
if BUILD_F77
	cp fortran/netcdffdll.def ${prefix}/lib
endif # BUILD_F77
if BUILD_CXX
	cp cxx/netcdf_c++dll.def ${prefix}/lib
endif # BUILD_CXX
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
