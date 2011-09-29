////
#	$Id$
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 or any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
////

CMake User Guide
================

Information for building GMT 5 with CMake
-----------------------------------------

Quick start:
~~~~~~~~~~~~

'svn switch' to cmake-build and cd into tree:

  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_INSTALL_PREFIX=gmt5 -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
  $ make -j8

will build out-of-source in the the directory build.

  $ make install

installs a basic gmt in build/gmt5


In more detail:
~~~~~~~~~~~~~~~

See http://www.cmake.org/cmake/help/cmake-2-8-docs.html

'CMAKE_BUILD_TYPE' can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel

To store the command line options, are to configure other options, copy
cmake/ConfigUserTemplate.cmake to cmake/ConfigUser.cmake and then make changes
in cmake/ConfigUser.cmake. See comments in that file.

There are two configuration files:

1. "ConfigDefault.cmake" -- is version controlled and used to add new default
    variables and set defaults for everyone.
2. "ConfigUser.cmake" -- is not version controlled (currently listed in svn:ignore
    property) and used to override defaults on a per-user basis.

NOTE: If you want to change CMake behaviour just for yourself then modify the
      "ConfigUser.cmake" file (not "ConfigDefault.cmake").


TODO list
---------

* Replace shell scripts, grep, sed, awk, etc. by cmake macros (this
  is necessary for cross platform compatibility). Basically port
  cmake/generateExtraFilesNotPorted.sh to cmake. Sometimes it might be
  easier to create new cmake macros if the layout of the underlying
  source files would be altered a litte. If absolutely necessary write
  a little C program that is build and run during configure pahase to
  generate support files.

* Clean up config.h.cmake gmt_version.h.cmake gmt_notposix.h.cmake.
  Right now this is a mixture of partly redundant defines that can be
  simplified.

* Make exported cmake variables consistent GMT_..., prefix temporary
  variables with "_"

* Add further subtrees: follow example of CMakeLists.txt in src/meca, share
  and share/conf

* Build targets for examples, doc, tests

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
    cmake "file(read ..."?

* Add further tests/cmake-find-modules:
  - octave/Matlab
  - fftw/Accelerate Framework
  - xgrid
  - WIN32: test for USE_MEM_ALIGNED, defined in src/gmt_config.h.cmake

* Generate macros
  - to simplify copying files (http://www.vtk.org/Wiki/CMakeCopyIfDifferent)
  - to simplify manpage creation *(done)*
  - for LaTeX docu (http://www.vtk.org/Wiki/CMakeUserUseLATEX)
  - to build a release (including precompiled manpages, pdf docu, ...)

////
vim: ft=asciidoc tw=78 et ts=2 sts=2 sw=2 autoindent smartindent
////
