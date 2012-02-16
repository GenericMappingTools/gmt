////
# $Id$
#
# Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
////

CMake User Guide
================

Information for building GMT 5 with CMake
-----------------------------------------

Quick start:
~~~~~~~~~~~~

'svn checkout' GMT5 and cd into tree. Then:

  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_INSTALL_PREFIX=gmt5 -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
  $ make -j8

will build out-of-source in the the directory _build_. 'CMAKE_BUILD_TYPE' can
be one of: empty, Debug, Release, RelWithDebInfo or MinSizeRel

  $ make -j8 install

installs a basic gmt in build/gmt5


In more detail:
~~~~~~~~~~~~~~~

The CMake manual is available online:
http://www.cmake.org/cmake/help/cmake-2-8-docs.html

To avoid CMake's lengthy command line options you should create your custom
ConfigUser.cmake file in the cmake directory. Simply copy
cmake/ConfigUserTemplate.cmake to cmake/ConfigUser.cmake and then make changes
in cmake/ConfigUser.cmake. See additional comments in that file.

There are two configuration files:

1. "ConfigDefault.cmake" -- is version controlled and used to add new default
    variables and set defaults for everyone.
2. "ConfigUser.cmake" -- is not version controlled (currently listed in svn:ignore
    property) and used to override defaults on a per-user basis. There is a
    template file, ConfigUserTemplate.cmake, that you should copy to
    ConfigUser.cmake and make your changes therein.

NOTE: If you want to change CMake behaviour for your build then only modify the
      "ConfigUser.cmake" file (not "ConfigDefault.cmake").

Finding GSHHS:
~~~~~~~~~~~~~~

GSHHS shorelines are searched in FindGSHHS.cmake and a little helper programm
tests the version (gshhs_version.c).

IMPORTANT: add getenv("GSHHS_PATH") to gmt_init.c and do min version checks.

Documentation:
~~~~~~~~~~~~~~

Manpages are available in different formats and can be generated with:

  $ make -j8 manpages_all

To generate the PDF documentation you need LaTeX:

  $ make -j8 gmt_doc

Installing:
~~~~~~~~~~~

  $ make -j8 install

will compile and install libps, libgmt, and all GMT modules. Optionally it
will install the GSHHS shorelines (if found), manpages, and PDF documentation.
You have to explicitly make the latter two beforehand or they will be omitted
during installation. You have the choice between a monolithic installation
(_-DGMT_INSTALL_MONOLITHIC=ON_) where everything goes into a common
subdirectory or a distribution-like installation (PREFIX/bin/gmt-VERSION,
PREFIX/lib/gmt-VERSION/, PREFIX/share/gmt-VERSION/,
PREFIX/share/doc/gmt-VERSION/).

Packaging:
~~~~~~~~~~

Currently, packaging with CPack works on MacOSX (Bundle, TGZ, TBZ2),
Windows (ZIP, NSIS), and UNIX (TGZ, TBZ2). On Windows you need to install NSIS
(http://nsis.sourceforge.net/). After building GMT run either one of these:

  $ [n]make package
  $ cpack -G <TGZ|TBZ2|Bundle|ZIP|NSIS>

You probably also want to run cmake with _-DGMT_INSTALL_MONOLITHIC=ON_.

In-build-dir tests:
~~~~~~~~~~~~~~~~~~~

Examples and tests can be build inside GMT_BINARY_DIR *without* installing.
This is very convenient for testing. Just _enable_testing()_, set _DO_EXAMPLES_
and/or _DO_TESTS_ in ConfigUser.cmake and do:

  $ make -j6
  $ make check

Optionally set _N_TEST_JOBS_ to the number of ctest jobs to run
simultaneously. You can also select individual tests using regexp with ctest:

  $ ctest -R job1[3-6]


TODO list
---------
* Turn into GMT module: potential/gravfft.c

* Fix segfault in xgridedit: tries to access grid outside bounds in
   gmt_nc.c:536  grid[kk] = tmp[k[i]];

* Replace GMT_(malloc|memory): perl -i -pe 's/(GMT_(malloc|memory)[^_].+, )([^,)]+)/$1sizeof($3)/g' files ...

* Remove (void *) casts: perl -i -pe 's/(free.*[(])[(]void[ ]*[*][)][ ]*/$1/g' *.c

* Remove macros GMT_fopen et al. and implement LFS support

* Clean up config.h.cmake gmt_version.h.cmake gmt_notposix.h.cmake.
  Right now this is a mixture of partly redundant defines that can be
  simplified. (almost done)

* Make exported cmake variables consistent GMT_..., prefix temporary
  variables with "_"

* Man Pages
  - Editing troff is difficult at least for people who are not familiar with
    the macros. Why not use AsciiDoc (http://www.methods.co.nz/asciidoc/)
    instead?
  - AsciiDoc is human readable and it is easy to copy especially for those who
    know wiki (http://powerman.name/doc/asciidoc). This will encourage people
    to fix mistakes.
  - You can define convenience macros with AsciiDoc and including files is
    also possibile. Abusing the C-preprocessor -- which is not designed for
    this task --  unnecesary. And it can create links to other manpages, which
    is convenient for webpage/pdf creation. That is the reason why git's
    documentation is written in AsciiDoc. Git like GMT is a compilation of
    many small progams.
  - Any ideas how to remove blank lines from CPP output without using grep and
    cmake "file(read ..."? Btw: clang -E does not output blank lines

* Add further tests/cmake-find-modules:
  - fftw/Accelerate Framework
  - xgrid
  - WIN32: test for USE_MEM_ALIGNED, defined in src/gmt_config.h.cmake

* Generate macros
  - to simplify copying files (http://www.vtk.org/Wiki/CMakeCopyIfDifferent)
  - to build a release (including precompiled manpages, pdf docu, ...)

////
vim: ft=asciidoc tw=78 et ts=2 sts=2 sw=2 autoindent smartindent
////
