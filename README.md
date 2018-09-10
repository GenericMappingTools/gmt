# Generic Mapping Tools

[![TravisCI](http://img.shields.io/travis/GenericMappingTools/gmt/master.svg?style=flat-square&label=TravisCI)](https://travis-ci.org/GenericMappingTools/gmt)

## Information for installing GMT 6

Note: The build system has recently been switched to CMake which is a
cross-platform system for managing the build process. If you are familiar
with the old GNU Build Tools (automake, autoconf, and configure) you can
probably skip over to the CMake quick start guide in README.CMake.


## Introduction

You do not need to read these instructions unless you plan to build and
install the programs manually.

GMT has been installed successfully under UNIX/Linux/OS X on workstations.  It
also installs under Windows and in UNIX emulators such as Cygwin or on virtual
machines.  We anticipate few problems if you are installing the package on
other platforms.

Note there are three GMT tar archives available (#3 is optional):

1. gmt-6.x.x.tar.bz2:          The GMT 6 distribution
2. gshhg-gmt-2.x.x.tar.gz:     All five resolutions of GSHHG coastline data
3. dcw-gmt-1.x.x.tar.bz2:      Digital Chart of the World polygon data

The archives are available in bzip2 (\*.bz2) and gzip (\*.gz) formats.
If you do not have bzip2 installed you can obtain source or executables
from http://www.bzip.org.

For Windows users there are separate Windows installers available; this
discussion only considers UNIX/Linux/OS X installations. Windows users who
which to build GMT from the sources refer to README.WIN32.

## A reminder

If you think it is appropriate, you may consider paying us back by including
our latest EOS article (Wessel, P., W. H. F. Smith, R. Scharroo, J. F. Luis,
and F. Wobbe (2013), Generic Mapping Tools: Improved version released, Eos
Trans. AGU, 94(45), 409-410, doi:10.1002/2013EO450001) in the reference list
of your future publications that will benefit from the availability of GMT.

## Note to package maintainers

Package maintainers note packaging recommendations at
http://gmt.soest.hawaii.edu/projects/gmt/wiki/PackagingGMT

## Build and runtime prerequisites

- Software:
  You need Ghostscript, CMake (>=2.8.5), netCDF (>=4.0, netCDF-4/HDF5
  support mandatory).  Optionally install Sphinx, PCRE1 or PCRE2, GDAL, LAPACK,
  BLAS and FFTW (single precision version).
- Data:
  You need gshhg (>=2.2.2); optionally install dcw-gmt (>=1.0.5)

### CMake

Install CMake (>=2.8.5) from http://www.cmake.org/cmake/resources/software.html


### Install netCDF library

For all major Linux distributions there are prepackaged development binaries
available. netCDF is also available on MacOSX trough the macports and fink
package managers.

Otherwise, get netCDF from http://www.unidata.ucar.edu/downloads/netcdf/.
You need at least version 4.0 with netCDF-4/HDF5 data model support (do not
disable HDF5/ZLIB in netCDF with --disable-netcdf-4).

### Install CURL library

To handle URLs we depend on libcurl so install via your favorite package
manager if it is not intrinsic to your Unix installation.  Otherwise, get
it from https://curl.haxx.se.

### GDAL (optional)

To use the GDAL interface (ability to provide grids or images to be imported
via gdal) you must have the GDAL library and include files installed.  Like
netCDF, GDAL is available through your favorite package manager on many *NIX
systems.


### PCRE (optional)

To use the PCRE interface (ability to specify regular expressions in some
search options, e.g., gmtconvert) you must have the PCRE library and include
files installed.  PCRE is available through your favorite package manager
on many *NIX systems.

Because GDAL already links with PCRE1 it is most practical to use that version.
But if you insist, GMT can also be compiled with PCRE2.


### LAPACK (optional)

To greatly speed up some linear algebra calculations (greenspline in
particular) you must have the LAPACK library and include files installed.
LAPACK is available through your favorite package manager on many *NIX
systems or in the case of OS X is built in.  Normally this also installs
BLAS but if not you need to do that separately as we are using some
cblas_* functions to do linear algebra calculations.


### Install support data

You can obtain GMT from http://gmt.soest.hawaii.edu/. Alternatively you may
get GMT from any of the following FTP sites. Try the site that is closest to
you to minimize transmission times:

| Site                                                        | FTP address             |
|:------------------------------------------------------------|:------------------------|
| SOEST, U. of Hawaii                                         | ftp.soest.hawaii.edu    |
| Lab for Satellite Altimetry, NOAA                           | ibis.grdl.noaa.gov      |
| IRIS, Washington, US                                        | ftp.iris.washington.edu |
| IAG-USP, U. of Sao Paulo, BRAZIL                            | ftp.iag.usp.br          |
| ISV, Hokkaido U, Sapporo, JAPAN                             | ftp.eos.hokudai.ac.jp   |
| GDS, Vienna U. of Technology, AUSTRIA                       | gd.tuwien.ac.at         |
| TENET, Tertiary Education & Research Networks, SOUTH AFRICA | gmt.mirror.ac.za        |

The development sources are available from the subversion repository at
svn://gmtserver.soest.hawaii.edu/gmt/trunk

Extract the files and put them in a separate directory (need not be
where you eventually want to install GMT).

## Information for building GMT with CMake

### Quick start

Checkout GMT5 from its GitHub repository:

  $ git clone https://github.com/GenericMappingTools/gmt
  $ cd gmt
  $ cp cmake/ConfigUserTemplate.cmake cmake/ConfigUser.cmake

Edit *cmake/ConfigUser.cmake* [see comments in the file]. Then:

  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
  $ make -jx

where _x_ is the number of threads you want to use and depends on the number
of cores in your CPU and if hyperthreading is available or not.
cmake will build out-of-source in the the directory _build_. 'CMAKE_BUILD_TYPE'
can be one of: empty, Debug, Release, RelWithDebInfo or MinSizeRel

  $ make -jx install

installs a basic gmt in _build/gmt_.

NOTE: All cmake command line options such as _-DCMAKE_INSTALL_PREFIX_ can be
configured in *cmake/ConfigUser.cmake*.

### In more detail

For CMake related questions refer to the CMake manual which is available
online: http://www.cmake.org/cmake/help/documentation.html

To avoid CMake's lengthy command line options you can create your custom
ConfigUser.cmake file in the cmake directory. Simply duplicate the
cmake/ConfigUserTemplate.cmake to cmake/ConfigUser.cmake and then make
changes in cmake/ConfigUser.cmake. See additional comments in that file.

There are two configuration files:

- "ConfigDefault.cmake" -- is version controlled and used to add new default
    variables and set defaults for everyone. You should not edit this file.
- "ConfigUser.cmake" -- is not version controlled and used to override defaults
    on a per-user basis.
    There is a template file, ConfigUserTemplate.cmake, that you should copy
    to ConfigUser.cmake and make your changes therein.

NOTE: If you want to change CMake behaviour for your build then only modify
      the "ConfigUser.cmake" file (not "ConfigDefault.cmake").


### Configuring

GMT can be build on any platform supported by CMake.  CMake is a
cross-platform, open-source system for managing the build process.
Refer to README.CMake for further details.  In the source tree copy
cmake/ConfigUserTemplate.cmake to cmake/ConfigUser.cmake and edit
the file according to your demands.

By default, GMT will use Dave Watson's Delaunay triangulation routine.
However, a much faster alternative is available from Jonathan Shewchuk, but
his routine is not distributed under the GNU Public License.  If you work for
a for-profit organization you should read Shewchuk's copyright statement (in
src/triangle.c) first.  If you agree with the license terms you can enable
Shewchuk's triangulation routine in cmake/ConfigUser.cmake.

At run-time, GMT will initialize all default variables. You can change
this by adding a gmt.conf file in your current or home directory
and edit those settings since GMT will check for that file before loading
system defaults (actually, it will first look in the current directory, then
the home directory, and then finally in share).  See the gmt.conf man page
for a description of all defaults.

To prevent two GMT processes writing to the same gmt.conf file simultaneously
(thereby corrupting it), GMT can implement the POSIX advisory file locking
scheme and sets and releases locks on these files.  This might not be reliable
when the files reside in directories on network filesystems, such as NFS.
Whether flock works on network filesystems is implementation dependent.  If
you want to activate file locking you may enable it in cmake/ConfigUser.cmake.

By default, both GMT and all its supplements are built.  You can turn
off all supplements via the BUILD_SUPPLEMENTS setting in ConfigUsers.cmake

The top-level installation directory is configured with the variable
CMAKE\_INSTALL\_PREFIX.

Now that you made your configuration choices it is time for invoking CMake.
Create a subdirectory where the build files will be generated, e.g., in the
source tree 'mkdir build'.

In the build subdirectory, type

  cmake [options] ..

Append any of the options explained above as you see fit.  If CMake cannot
figure out all the dependent libraries or required compiler and linker flags
it will give you a message and you will be asked to edit
cmake/ConfigUser.cmake.


### Finding GSHHG

GSHHG shorelines are searched in FindGSHHG.cmake and a little helper program
tests the version (gshhg_version.c). If CMake cannot find the shorelines you
have to configure _GSHHG\_ROOT_ in cmake/ConfigUser.cmake.

### Finding DCW

DCW (Digital Chart of the World) country polygons are searched at compile time.
The DCW data are optional; they are currently used in pscoast -E for painting
individual countries only.  If CMake cannot auto-find DCW for you then you can
configure _DCW_ROOT_ in cmake/ConfigUser.cmake.

## Build GMT

In the build directory, type

<pre>
make -j all
</pre>
which will compile all the programs.

### Build documentation

The GMT manual is available in different formats and can be generated with:

<pre>
make -j docs_man        # UNIX manual pages
make -j docs_html       # HTML manual, cookbook, and API reference
make -j docs_pdf        # PDF manual, cookbook, and API reference
make -j docs_pdf_shrink # Like docs_pdf but with reduced size
</pre>
To generate the documentation you need to install the Sphinx documentation
builder, and for PDFs you also need LaTeX.  You can choose to install the
documentation files from an external location instead of generating the
Manpages, PDF, and HTML files from the sources.  This is convenient if Sphinx
and/or LaTeX are not available.  Set GMT_INSTALL_EXTERNAL_DOC in
cmake/ConfigUser.cmake.

### Installing

<pre>
make -j install
</pre>
will install libps, libgmt, and the gmt executable. Optionally it
will install the GSHHG shorelines (if found), DCW (if found), UNIX manpages,
and HTML and PDF documentation.

You have to explicitly make the latter two beforehand or they will be omitted
during installation.  You have the choice between installing into a
traditional directory structure (_-DGMT\_INSTALL\_TRADITIONAL\_FOLDERNAMES=ON_)
where everything goes into a common subdirectory or a distribution-like
installation (PREFIX/bin/gmtSUFFIX, PREFIX/lib/gmtSUFFIX/,
PREFIX/include/gmtSUFFIX/, PREFIX/share/gmtSUFFIX/,
PREFIX/share/doc/gmtSUFFIX/).

GMT is shipped as a single executable.  By default we also set "classic mode"
by installing convenience links for all GMT modules. New GMT users should set
GMT\_INSTALL\_MODULE\_LINKS to FALSE in cmake/ConfigUser.cmake.

### Updating

Assuming you did not delete the build directory and that your current
working directory is the build directory this is just as simple as

<pre>
cd ..
git pull
cd -
make -j install
</pre>
CMake will detect any changes to the source files and will automatically
reconfigure. If you deleted all files inside the build directory you have to
run cmake again manually.

### Packaging

Currently, packaging with CPack works on MacOSX (Bundle, TGZ, TBZ2),
Windows (ZIP, NSIS), and UNIX (TGZ, TBZ2). On Windows you need to install NSIS
(http://nsis.sourceforge.net/). After building GMT and the documentation run
either one of these:

<pre>
make package
cpack -G <TGZ|TBZ2|Bundle|ZIP|NSIS>
</pre>

### Creating a source package

Set GMT\_RELEASE\_PREFIX in cmake/ConfigUser.cmake and run cmake. Then do

<pre>
make -j docs_depends # optional but increases speed (parallel build)
make gmt_release      # export the source tree and install doc
</pre>
You should then edit ${GMT_RELEASE_PREFIX}/cmake/ConfigDefault.cmake and
set GMT\_PACKAGE\_VERSION\_MAJOR, GMT\_PACKAGE\_VERSION\_MINOR, and
GMT\_PACKAGE\_VERSION\_PATCH. Also uncomment and set
GMT\_SOURCE\_CODE\_CONTROL\_VERSION\_STRING to the current git version. Then
create tarballs with:

<pre>
make -j gmt_release_tar
</pre>

### In-build-dir tests

A complete set of the example scripts used to create all the example plots,
including all necessary data files, are provided by the installation.
Examples and tests can be build inside GMT\_BINARY\_DIR *without* installing.
This is very convenient for testing. Just _enable\_testing()_, set
_DO\_EXAMPLES_ and/or _DO\_TESTS_ in ConfigUser.cmake and do:

<pre>
make -jx
make check
</pre>
Optionally set _N\_TEST\_JOBS_ to the number of ctest jobs to run
simultaneously. You can also select individual tests using regexp with ctest,
e.g.:

<pre>
ctest -R ex2[3-6]
</pre>
Tests are preferably run through valgrind and GMT's internal memory tracker.
Configure cmake with _add\_definitions(-DMEMDEBUG)_ and prior to invoking the
test, export the following variables:

<pre>
export GMT_TRACK_MEMORY=2
export VALGRIND_ARGS="--track-origins=yes --leak-check=full"
</pre>
The test script will keep gmt_memtrack_<pid>.log and valgrind\_<pid>.log files
in the test directories for later inspection, when memory related errors were
identified.


### Set path

Make sure users set their PATH to include the directory containing
the GMT executables (BINDIR) if this is not a standard directory
like /usr/local/bin.  You should now be able to run GMT programs.


## GMT supplemental Code

GMT users elsewhere have developed programs that utilize the GMT libraries and
produce PostScript code compatible with the rest of GMT or simply perform data
manipulation.  In addition, misc.  code developed by us depend on GMT
libraries.  Currently, the supplemental archive include the directories:

  gshhg     - Data extractor for GSHHG shoreline polygons and rivers, borders.
  img       - Data extractor for Smith/Sandwell altimetry grids.
  meca      - Plotting of focal mechanisms, velocity arrows,
              and error ellipses on maps.
  mgd77     - Programs for handling of native MGD77 files.
  misc      - dimfilter
  potential - geopotential manipulations
  segyprogs - Plotting SEGY seismic data sets.
  spotter   - Plate tectonic & kinematics applications.
  x2sys     - Track intersection (crossover) tools.


## Misc

Before running programs, there are a few things you should do/know:

  - Read carefully the documentation for the gmt system.  This can be
    found as both PDF and HTML files in the doc/pdf|html directories.
    The successful operation of gmt-programs depends directly on your
    understanding of how gmt "works", its option lists, I/O, and composite
    plot mechanisms.  Then, before running individual gmt programs, read
    the associated man page.


## Software support

You haven't bought anything so you cannot expect full service.  However, if
you find a bug in any of the programs, please report it to us
(http://gmt.soest.hawaii.edu/) rather than trying to fix it yourself so that
we, and through us, other users may benefit from your find.  Make sure you
provide us with enough information so that we can recreate the problem.

In addition to the bug tracking feature (New Issues) on the website, you
can also post general questions on the GMT user forum.  Note that registration
is required to post on the site.


## Ordering the GMT package on CD/DVD-Rs

Should you or someone you know without net-access need to obtain GMT:
Geoware makes and distributes CD/DVD-Rs with the GMT package and many
useful data sets.  For more details and a full description of the data
sets (up to 60 Gb of data!) visit http://www.geoware-online.com/.


Good luck!

The GMT Team.
