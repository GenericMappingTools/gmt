ifeq (0,${MAKELEVEL})
  top_srcdir:=$(shell pwd)
  uname:=$(shell uname -s)
  MAKE := ${MAKE} top_srcdir=$(top_srcdir) uname=$(uname)
  TOP_MAKEFILE := $(lastword $(MAKEFILE_LIST))
endif

top_builddir:=$(shell pwd)

ifeq ($(findstring CYGWIN,$(uname)),CYGWIN)
  PLATFORM:=cygwin
else
  ifeq ($(findstring MINGW,$(uname)),MINGW)
  PLATFORM:=mingw
  else
  PLATFORM:=unknown
  endif
endif
STAMP=stamp-$(PLATFORM)

ifeq ($(PLATFORM), cygwin)
  CC=gcc
  O=o
  EXEEXT=.exe
  DLLVER=0
  SHRNAME=cygxdr-$(DLLVER).dll
  DEFFILE=$(PLATFORM)/xdr.def
  IMPNAME=libxdr.dll.a
  LIBNAME=libxdr.a
  LIB_LDEXTRA=-Wl,--out-implib=$(PLATFORM)/$(IMPNAME) -Wl,--enable-auto-image-base
  LIB_DEPS =
  GETOPT_SRCS =
  GETOPT_HDRS =
  MKDTEMP_SRCS =
  # Some early versions of cygwin-1.7 included a broken
  # inttypes.h, which caused warnings. May need -Wno-format.
  CFLAGS+=-Wall -Werror # -Wno-format
else
  ifeq ($(PLATFORM), mingw)
  CC=gcc
  O=o
  EXEEXT=.exe
  DLLVER=0
  SHRNAME=mgwxdr-$(DLLVER).dll
  DEFFILE=$(PLATFORM)/xdr.def
  IMPNAME=libxdr.dll.a
  LIBNAME=libxdr.a
  LIB_LDEXTRA=-Wl,--out-implib=$(PLATFORM)/$(IMPNAME) -Wl,--enable-auto-image-base
  LIB_DEPS = -lws2_32
  GETOPT_SRCS = src/getopt_long.c
  GETOPT_HDRS = src/getopt.h
  MKDTEMP_SRCS = src/mkdtemp.c
  CFLAGS+=-Wall -Werror
  else
  CC=gcc
  O=o
  EXEEXT=
  SOMAJOR=0
  SOMINOR=0
  SOMICRO=0
  SHRNAME=libxdr.so.$(SOMAJOR).$(SOMINOR).$(SOMICRO)
  LIBNAME=libxdr.a
  LIB_DEPS =
  GETOPT_SRCS =
  GETOPT_HDRS =
  MKDTEMP_SRCS =
  endif
endif

CFLAGS+=-fno-strict-aliasing

INCLUDES = -I$(top_srcdir) -I$(top_builddir)

LIB_HDRS = rpc/xdr.h rpc/types.h
LIB_HDRS_PRIVATE = lib/xdr_private.h
LIB_SRCS = lib/xdr.c lib/xdr_array.c lib/xdr_float.c lib/xdr_mem.c \
	   lib/xdr_rec.c lib/xdr_reference.c lib/xdr_sizeof.c lib/xdr_stdio.c \
	   lib/xdr_private.c
LIB_OBJS = $(LIB_SRCS:%.c=$(PLATFORM)/%.$(O))
XDR_LIBRARIES = $(PLATFORM)/$(SHRNAME) $(PLATFORM)/$(LIBNAME)

TEST_XDR_LIBS = -lxdr $(LIB_DEPS)
TEST_XDR_LDFLAGS  = -L$(top_builddir)/$(PLATFORM)
TEST_HDRS = src/test/test_common.h src/test/test_data.h src/test/test_xdrs.h

TEST_XDRMEM_HDRS = $(TEST_HDRS) $(LIB_HDRS) $(GETOPT_HDRS)
TEST_XDRMEM_SRCS = src/test/xdrmem_test.c \
	src/test/test_common.c \
	src/test/test_data.c \
	src/test/test_xdrs.c \
	$(GETOPT_SRCS) $(MKDTEMP_SRCS)
TEST_XDRMEM_OBJS = $(TEST_XDRMEM_SRCS:%.c=$(PLATFORM)/%.$(O))
TEST_XDRMEM_LIBS = $(TEST_XDR_LIBS)
TEST_XDRMEM_LDFLAGS = $(TEST_XDR_LDFLAGS)

TEST_XDRSTDIO_HDRS = $(TEST_HDRS) $(LIB_HDRS) $(GETOPT_HDRS)
TEST_XDRSTDIO_SRCS = src/test/xdrstdio_test.c \
	src/test/test_common.c \
	src/test/test_data.c \
	src/test/test_xdrs.c \
	$(GETOPT_SRCS) $(MKDTEMP_SRCS)
TEST_XDRSTDIO_OBJS = $(TEST_XDRSTDIO_SRCS:%.c=$(PLATFORM)/%.$(O))
TEST_XDRSTDIO_LIBS = $(TEST_XDR_LIBS)
TEST_XDRSTDIO_LDFLAGS = $(TEST_XDR_LDFLAGS)

TEST_XDRSIZEOF_HDRS = $(TEST_HDRS) $(LIB_HDRS) $(GETOPT_HDRS)
TEST_XDRSIZEOF_SRCS = src/test/xdrsizeof_test.c \
	src/test/test_common.c \
	src/test/test_data.c \
	src/test/test_xdrs.c \
	$(GETOPT_SRCS) $(MKDTEMP_SRCS)
TEST_XDRSIZEOF_OBJS = $(TEST_XDRSIZEOF_SRCS:%.c=$(PLATFORM)/%.$(O))
TEST_XDRSIZEOF_LIBS = $(TEST_XDR_LIBS)
TEST_XDRSIZEOF_LDFLAGS = $(TEST_XDR_LDFLAGS)

TEST_PROGS = $(PLATFORM)/xdrmem_test$(EXEEXT) \
	$(PLATFORM)/xdrstdio_test$(EXEEXT) \
	$(PLATFORM)/xdrsizeof_test$(EXEEXT)
TEST_OBJS = $(TEST_XDRMEM_OBJS) \
	$(TEST_XDRSTDIO_OBJS) \
	$(TEST_XDRSIZEOF_OBJS)

all:
	@if test $(MAKELEVEL) -eq 0 ; then \
	  if test "$(PLATFORM)" = "unknown" ; then \
	    echo "Can't build for $(uname) using this makefile" 1>&2 ;\
	    false ;\
	  else \
	    echo "Building for $(PLATFORM)" ;\
	    $(MAKE) -f $(TOP_MAKEFILE) recursive-all ;\
	  fi ;\
	fi

recursive-all: $(XDR_LIBRARIES) $(TEST_PROGS)


$(STAMP):
	@for d in $(PLATFORM)/lib $(PLATFORM)/src/test; do\
	  if ! test -d $$d ; then\
	    mkdir -p $$d ;\
	  fi;\
	done
	touch $(STAMP)

$(DEFFILE): lib/libxdr.def.in
	cat lib/libxdr.def.in | sed -e "s/@@LIBNAME@@/$(SHRNAME)/" > $@

$(PLATFORM)/$(SHRNAME): $(STAMP) $(DEFFILE) $(LIB_OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $(LIB_LDEXTRA) $(DEFFILE) $(LIB_OBJS) $(LIB_DEPS)

$(PLATFORM)/$(LIBNAME): $(STAMP) $(LIB_OBJS)
	$(AR) cr $@ $(LIB_OBJS)

$(PLATFORM)/xdrmem_test$(EXEEXT): $(TEST_XDRMEM_OBJS) $(XDR_LIBRARIES)
	$(CC) $(TEST_XDRMEM_LDFLAGS) $(LDFLAGS) -o $@ $(TEST_XDRMEM_OBJS) $(TEST_XDRMEM_LIBS)

$(PLATFORM)/xdrstdio_test$(EXEEXT): $(TEST_XDRSTDIO_OBJS) $(XDR_LIBRARIES)
	$(CC) $(TEST_XDRSTDIO_LDFLAGS) $(LDFLAGS) -o $@ $(TEST_XDRSTDIO_OBJS) $(TEST_XDRSTDIO_LIBS)

$(PLATFORM)/xdrsizeof_test$(EXEEXT): $(TEST_XDRSIZEOF_OBJS) $(XDR_LIBRARIES)
	$(CC) $(TEST_XDRSIZEOF_LDFLAGS) $(LDFLAGS) -o $@ $(TEST_XDRSIZEOF_OBJS) $(TEST_XDRSIZEOF_LIBS)

$(PLATFORM)/%.$(O) : %.c
	$(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -f $(LIB_OBJS) $(DEFFILE) $(TEST_OBJS) $(STAMP)

.PHONY: realclean
realclean: clean
	-rm -f $(PLATFORM)/$(SHRNAME) $(PLATFORM)/$(LIBNAME)
	@if test -n "$(IMPNAME)" ; then \
	  rm -f $(PLATFORM)/$(IMPNAME) ;\
	fi
	-rm -f $(TEST_PROGS)
	-rmdir $(PLATFORM)/src/test
	-rmdir $(PLATFORM)/src
	-rmdir $(PLATFORM)/lib
	-rmdir $(PLATFORM)

# dependencies
$(PLATFORM)/lib/xdr.$(O):           lib/xdr.c           $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_array.$(O):     lib/xdr_array.c     $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_float.$(O):     lib/xdr_float.c     $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_mem.$(O):       lib/xdr_mem.c       $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_rec.$(O):       lib/xdr_rec.c       $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_reference.$(O): lib/xdr_reference.c $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_sizeof.$(O):    lib/xdr_sizeof.c    $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_stdio.$(O):     lib/xdr_stdio.c     $(LIB_HDRS) $(LIB_HDRS_PRIVATE)
$(PLATFORM)/lib/xdr_private.$(O):   lib/xdr_private.c   $(LIB_HDRS) $(LIB_HDRS_PRIVATE)

$(PLATFORM)/src/test/test_common.$(O): src/test/test_common.c $(TEST_HDRS) $(LIB_HDRS) $(GETOPT_HDRS)
$(PLATFORM)/src/test/test_data.$(O): src/test/test_data.c $(TEST_HDRS) $(LIB_HDRS) $(GETOPT_HDRS)
$(PLATFORM)/src/test/test_xdrs.$(O): src/test/test_xdrs.c $(TEST_HDRS) $(LIB_HDRS) $(GETOPT_HDRS)
$(PLATFORM)/src/test/xdrmem_test.$(O): src/test/xdrmem_test.c $(TEST_XDRMEM_HDRS) $(GETOPT_HDRS)
$(PLATFORM)/src/test/xdrstdio_test.$(O): src/test/xdrstdio_test.c $(TEST_XDRSTDIO_HDRS) $(GETOPT_HDRS)
$(PLATFORM)/src/test/xdrsizeof_test.$(O): src/test/xdrsizeof_test.c $(TEST_XDRSTDIO_HDRS) $(GETOPT_HDRS)
$(PLATFORM)/src/getopt.$(O):         src/getopt.c src/getopt.h
$(PLATFORM)/src/getopt1.$(O):        src/getopt1.c src/getopt.h
$(PLATFORM)/src/mkdtemp.$(O):        src/mkdtemp.c


