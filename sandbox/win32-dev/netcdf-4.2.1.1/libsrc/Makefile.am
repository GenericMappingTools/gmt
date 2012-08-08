## This is an automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file is in charge of building the libsrc directory,
# which contains the classic library code.

include $(top_srcdir)/lib_flags.am

libnetcdf3_la_CPPFLAGS = ${AM_CPPFLAGS}

# Turn on a pre-processor flag when building a DLL for windows.
if BUILD_DLL
libnetcdf3_la_CPPFLAGS += -DDLL_EXPORT
endif # BUILD_DLL

# These files comprise the netCDF-3 classic library code.
libnetcdf3_la_SOURCES = v1hpg.c onstack.h                              \
nclistmgr.c putget.c attr.c nc3dispatch.c nc.c var.c dim.c ncx.c	\
ncx.h lookup3.c pstdint.h ncio.c ncio.h

if BUILD_DISKLESS
  libnetcdf3_la_SOURCES += memio.c
if BUILD_MMAP
  libnetcdf3_la_SOURCES += mmapio.c
endif BUILD_MMAP
endif BUILD_DISKLESS

# Does the user want to use ffio, a replacement for posixio for Cray
# computers?
if USE_FFIO
libnetcdf3_la_SOURCES += ffio.c
else !USE_FFIO
libnetcdf3_la_SOURCES += posixio.c
endif !USE_FFIO

noinst_LTLIBRARIES = libnetcdf3.la

# These files are cleaned on developer workstations (and then rebuilt
# with m4), but they are included in the distribution so that the user
# does not have to have m4.
MAINTAINERCLEANFILES = attr.c ncx.c putget.c $(man_MANS) attrx.c putgetx.c
EXTRA_DIST = attr.m4 ncx.m4 putget.m4 $(man_MANS) 

# This tells make how to turn .m4 files into .c files.
.m4.c:
	m4 $(AM_M4FLAGS) $(M4FLAGS) $< >$@

# The C API man page.
man_MANS = netcdf.3 

# Decide what goes in the man page, based on user configure options.
ARGS_MANPAGE = -DAPI=C
if USE_NETCDF4
ARGS_MANPAGE += -DNETCDF4=TRUE
endif
if BUILD_DAP
ARGS_MANPAGE += -DDAP=TRUE
endif
if BUILD_PARALLEL
ARGS_MANPAGE += -DPARALLEL_IO=TRUE
endif

# This rule generates the C manpage.
netcdf.3: $(top_srcdir)/man4/netcdf.m4 
	m4 $(M4FLAGS) $(ARGS_MANPAGE) $? >$@  || rm $@


