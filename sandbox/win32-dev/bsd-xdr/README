This package contains a port of Sun's XDR library. It was
derived from the implementation in the libtirpc package
(version 0.1.10-7) from Fedora 11. That version was relicensed
with explicit permission from the copyright holder (Sun
Microsystems) to a BSD license.  See the LICENSE file for
more information.

The goals of this port
=========================================
  Maintain the BSD license rather than copylefting it, as
    in portableXDR and various other versions.
  Avoid autotools.
  Avoid "config.h" pollution (or similar) in public header files.
  Compile successfully at highest reasonable warning level,
    with warnings treated as errors.
  Compile library as both static and shared.
  Provide thorough tests of all libxdr facilities.
  Where feasible, link test programs against both static and shared
    libraries.
  Verify identical output on a variety of platforms, especially 32-
    and 64-bit hosts.

Tested platforms
=========================================
  cygwin (cygwin 1.5.25, gcc-3.4.5)
  cygwin (cygwin 1.7.0-48, gcc-4.3.2beta2)
  mingw  (mingw-runtime 3.11 or later, gcc-3.4.5)
  msvc   (Visual Studio C++ 2005)
  linux  (32bit, gcc-4.2.2, glibc-2.6.1)
  linux  (64bit, gcc-4.1, glibc-2.4)


Build instructions
=====================================================
This distribution does NOT build "outside the source tree".
If you must do so, on hosts that support symbolic links,
create a symlink tree (using lndir or similar). Below, it
is assumed that you are building "inside" the source tree.

1) unpack the distribution
2) cd to the top level of the distribution
3) make using the appropriate makefile:
    cygwin, mingw:  Makefile         (thus, 'make' will work).
    msvc:           Makefile.msvc80  (see platform-specific notes)
    linux:          Makefile.unix    (really linux-specific)

The compiled libraries and test code will be placed in
   ./<PLATFORM>

Build instructions: cygwin
=====================================================
From a cygwin bash shell, simply unpack the source distribution,
cd to the top level, and type 'make'. The compiled libraries and
test programs will be placed in
   ./cygwin/

The shared library will be named cygxdr-0.dll
The import library will be named libxdr.dll.a
The static library will be named libxdr.a

There are three test programs (all of them linked only
against the shared library):
  xdrmem_test.exe
  xdrstdio_test.exe
  xdrsizeof_test.exe
Each has help (-h) and several other options.

To use the library with your own projects, "install" the import and
static libraries somewhere convenient (/usr/local/lib?).  Also,
install the DLL somewhere in your PATH (/usr/local/bin?).  Finally,
install the following two header files -- somewhere OTHER than
/usr/include, such as /usr/local/include:
   ./rpc/types.h           -->    /usr/local/include/rpc/types.h
   ./rpc/xdr.h             -->    /usr/local/include/rpc/xdr.h
You must use an include directory other than /usr/include, because
cygwin provides the rpc/* headers -- but no implementation. And
cygwin's headers don't exactly match these.

Compile and link your project with appropriate CFLAGS and LDFLAGS
(such as -I/usr/local/include  and  -L/usr/local/lib).

To build the library (and test programs) in debug mode, set CFLAGS
to "-g -O0" before running make.

Build instructions: mingw
=====================================================
From an msys bash shell, simply unpack the source distribution,
cd to the top level, and type 'make'. The compiled libraries and
test programs will be placed in
   ./mingw/

The shared library will be named mgwxdr-0.dll
The import library will be named libxdr.dll.a
The static library will be named libxdr.a

There are three test programs (all of them linked only
against the shared library):
  xdrmem_test.exe
  xdrstdio_test.exe
  xdrsizeof_test.exe
Each has help (-h) and several other options.

To use the library with your own projects, "install" the import and
static libraries somewhere convenient (/mingw/lib?).  Also,
install the DLL somewhere in your PATH (/mingw/bin?).  Finally,
install the following two header files:
   ./rpc/types.h           -->    /mingw/include/rpc/types.h
   ./rpc/xdr.h             -->    /mingw/include/rpc/xdr.h
Compile and link your project with appropriate CFLAGS and LDFLAGS
(if using /mingw/include and /mingw/lib, then the default CFLAGS
and LDFLAGS are sufficient).

To build the library (and test programs) in debug mode, set CFLAGS
to "-g -O0" before running make.

Build instructions: msvc
=====================================================
Building this package with Visual Studio requires some unix-like
tools and a unix-like shell. cygwin, mingw's "msys", or interix
will all work. You will also need GNU make (NOT nmake) and sed.
All three of the specified environments provide those tools.

After launching the (unix-like) shell of your choice, ensure that
you have configured your environment so that the Visual Studio
compiler cl.exe, linker link.exe, and library tool lib.exe are
present in your PATH (and precede your unix environment tools;
otherwise the Visual Studio linker may be hidden by some other
link.exe tool).  Typically this means setting:
  PATH
  INCLUDE
  LIB
  LIBPATH
  VSINSTALLDIR
  VCINSTALLDIR
See 'C:\Program Files\Microsoft Visual Studio *\Common7\tools\vsvars32.bat'
for the proper settings.

Then, after unpacking the distribution and changing directories
to the top level of it, build using
  make -f Makefile.msvc80

The compiled libraries and test programs will be placed in
  ./msvc80/

The shared library will be named xdr-0.dll
The import library will be named xdr.lib
The static library will be named xdr_s.lib
There are six test programs:
   xdrmem_test.exe
   xdrmem_test_static.exe
   xdrstdio_test.exe
   xdrstdio_test_static.exe
   xdrsizeof_test.exe
   xdrsizeof_test_static.exe
Each has help (-h) and several other options.

To use the library with your own projects, "install" the import or
static library in some convenient location so that the Visual Studio
linker can find it. Similarly "install" the following header files
in a convenient location:
   ./src/msvc/inttypes.h   -->    <some_dir>/inttypes.h  [o]
   ./src/msvc/stdint.h     -->    <some_dir>/stdint.h    [o]
   ./rpc/types.h           -->    <some_dir>/rpc/types.h
   ./rpc/xdr.h             -->    <some_dir>/rpc/xdr.h
Finally, "install" the DLL to some convenient location (preferably
in your PATH).

[o] it is possible that versions of Visual Studio newer than 2005
    may provide these files; if so, then the copies in this package
    are not necessary.

Then, compile and link your client application with appropriate
build settings so that the "installed" headers and libraries can
be found by the appropriate tools.

To build the library (and test programs) in debug mode, add
ENABLE_DEBUG=1 to the make invocation. That is,
   make -f Makefile.msvc80 ENABLE_DEBUG=1

Build instructions: linux
=====================================================
From a bash shell, simply unpack the source distribution,
cd to the top level, and type
   make -f Makefile.unix
The compiled libraries and test programs will be placed in
   ./linux/

The shared library will be named libxdr.so.0.0.0
The static library will be named libxdr.a

There are six test programs
   xdrmem_test.exe
   xdrmem_test_static.exe
   xdrstdio_test.exe
   xdrstdio_test_static.exe
   xdrsizeof_test.exe
   xdrsizeof_test_static.exe
Each has help (-h) and several other options. Note that on
64-bit platforms, some tests are known to fail and are skipped.
See below.

To use the library with your own projects, "install" the shared
and static libraries somewhere convenient (/usr/local/lib?).  Also,
install the following two header files:
   ./rpc/types.h           -->    <some_dir>/include/rpc/types.h
   ./rpc/xdr.h             -->    <some_dir>/include/rpc/xdr.h
It is recommended that <some_dir> NOT be /usr, because most
linux systems already provide /usr/include/rpc/*.

You may also need to create a 'so-name' link:
   cd <some_dir>/lib
   ln -fs libxdr.so.0.0.0 libxdr.so.0
You may also need to edit /etc/ld.so.conf and/or re-run
ldconfig.  See `man ld.so.conf' and `man ldconfig'.

Compile and link your project with appropriate CFLAGS and LDFLAGS.
Be careful to specify these correctly, and link against -lxdr;
otherwise you may not actually use libxdr as most linux systems
provide core xdr functionality via glibc.  So, for instance:
  CFLAGS=-I/usr/local/include
  LDFLAGS=-L/usr/local/lib
  LIBS=-lxdr

To build the library (and test programs) in debug mode, add
ENABLE_DEBUG=1 to the make invocation. That is,
   make -f Makefile.unix ENABLE_DEBUG=1

NOTES
=====================================================
Many xdr implementations are not 64bit clean. The original
SunRPC implementation relied heavily on core transport-specific
methods xdr_putlong and xdr_getlong -- and assumed that "longs"
were, in fact, exactly 32 bits.  For 64bit platforms this fails
in many wonderful ways.

The implementation here addresses this difficulty in two
ways:
  (1) the original integer primitives whose sizes can by
      problematic (xdr_[u_]int, xdr_[u_]long) take special
      care to do the right thing -- so long as the value
      can be represented in 32 bits. If the native type
      ([unsigned] int or [unsigned] long) is bigger than
      32 bits, BUT the value contained can be represented
      using just those 32 bits, then just those 32bits
      are XDRed. Otherwise, attempts to XDR the value
      return FALSE (instead of blindly XDRing the lower
      32 bits to represent the value).
  (2) New size-specific primitives (xdr_intN, xdr_uintN,
      xdr_u_intN, where N is 8, 16, 32, or 64) are provided.
      These use transport-specific implementation methods
      xdr_putint32 and xdr_getint32 that are insensitive
      to variations of the width of int/long/short on
      various platforms. New code should use these primitives.
      Most modern implementations of xdr support them.

The supplied tests are very thorough with regards to:
  memory buffer transport
  stdio FILE* transport
in that every core primitive is tested on both input and output.
Try '-v' (or '-v -v -v -v') when running the test programs.  All
of the tested platforms produce identical output for all cases
(except for 64bit linux; but only because it skips two
known-to-fail tests: xdr_long and xdr_u_long with MAX_LONG and
MAX_U_LONG. On 64bit platforms, MAX_LONG and MAX_U_LONG cannot
be represented with just 32 bits, so xdr_[u_]long returns FALSE
and the test fails -- but in fact, this actually demonstrates
that the "protect against assuming longs are 32 bits" code
is working as desired.

The sizeof test measures the size of a structure that includes
a primitive of every type, and also the size of a populated
linked list. (Yes, xdr can serialize data structures that have
pointers, under certain conditions: (a) there are no circular
references -- so doubly-linked lists are out, and (b) every
object to be serialized is pointed-to by exactly one pointer --
so no 'sentinel nodes' or 'every element has a reference to
HEAD/TAIL' schemes.  See comments in src/test/test_data.c
concerning xdr_pgn_node_t_RECURSIVE, xdr_pgn_list_t_RECURSIVE,
and xdr_pgn_list_t for more information).

The xdr_rec ("Record Marking") transport -- heavily used by the
RPC protocol -- is not tested here. However, by explicitly
compiling and linking a network protocol analyzer [-] against
this implementation on one platform, and against the system
RPC/XDR implementation on another platform, it is possible to
demonstrate that the xdr_rec transport works as desired.

[-] the talknet program
    http://www.geonius.com/software/tools/talknet.html
    with various combinations of the -xdr -hex -udp
    options, running as -server on one host and as a
    client on the other. It does take a little effort
    to compile this software and ensure that it links
    to our desired libxdr, rather than its own xdr
    implementation or to the system xdr, but it can
    be done.

