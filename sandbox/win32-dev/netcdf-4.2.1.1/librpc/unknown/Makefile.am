
## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This builds the netcdf-4 version of the opendap client.

# $Id$

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

DISTCLEANFILES =
CLEANFILES =
EXTRA_DIST =
LDADD=

ASTJAR=ast.jar

# RPC Sources

HDRS=\
protorpc.h \
nc_path.h \
rpc_includes.h \
rpc.h \
rpccurl.h \
rpcdebug.h \
rpcdispatch.h \
rpcutil.h

SRC=\
protorpc.c \
nc_path.c \
rpccurl.c \
rpcdebug.c \
rpcdispatch.c \
rpcserialize.c \
rpcutil.c

if BUILD_RPC

noinst_LTLIBRARIES = librpcclient.la librcpserver.la

# Build convenience library
librpcclient_la_SOURCES = $(SRC) $(HDRS)
#librpcserver_la_SOURCES = $(SRC) $(HDRS)

AM_CPPFLAGS += -I$(top_srcdir)/include \
               -I$(top_srcdir)/libdispatch \
               -I$(top_srcdir)/libsrc 

# Define the load libraries 
LDADD += \
         ${top_builddir}/libsrc/libnetcdf3.la \
         ${top_builddir}/libdispatch/libdispatch.la

if USE_NETCDF4
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4

# Define the load libraries 
LDADD += ${top_builddir}/libsrc4/libnetcdf4.la

endif #USE_NETCDF4

if BUILD_V2
LDADD += ${top_builddir}/libdispatch/libnetcdf2.la 
endif # BUILD_V2

##################################################
# Generate the augmented ncstream structures

EXTRA_DIST += ast.jar protorpc.proto

protorpc.c protorpc.h: protorpc.proto ast.jar
	java -jar ast.jar -WdT -I. protorpc.proto

##################################################

endif # BUILD_RPC
