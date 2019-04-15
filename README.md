# Generic Mapping Tools

[![TravisCI](http://img.shields.io/travis/GenericMappingTools/gmt/master.svg?label=TravisCI)](https://travis-ci.org/GenericMappingTools/gmt)
[![Azure](https://dev.azure.com/GenericMappingTools/GMT/_apis/build/status/GenericMappingTools.gmt?branchName=master)](https://dev.azure.com/GenericMappingTools/GMT/_build/latest?definitionId=2&branchName=master)
[![CodeCov](https://img.shields.io/codecov/c/github/GenericMappingTools/gmt.svg)](https://codecov.io/gh/GenericMappingTools/gmt/)
[![Coverity](https://scan.coverity.com/projects/7153/badge.svg)](https://scan.coverity.com/projects/gmt)
[![Documentation (development version)](https://img.shields.io/badge/docs-development-green.svg)](https://genericmappingtools.github.io/gmt/dev/)

## What is GMT?

GMT is an open source collection of about 80 command-line tools for manipulating
geographic and Cartesian data sets (including filtering, trend fitting, gridding,
projecting, etc.) and producing PostScript illustrations ranging from simple x–y
plots via contour maps to artificially illuminated surfaces and 3D perspective
views. The GMT supplements add another 40 more specialized and discipline-specific
tools. GMT supports over 30 map projections and transformations and requires
support data such as [GSHHG](http://www.soest.hawaii.edu/pwessel/gshhg/)
coastlines, rivers, and political boundaries and optionally
[DCW](http://www.soest.hawaii.edu/pwessel/dcw) country polygons.

GMT is developed and maintained by Paul Wessel, Walter H. F. Smith, Remko Scharroo,
Joaquim Luis and Florian Wobbe, with help from a global set of
[contributors](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Volunteers) and
support by the [National Science Foundation](http://www.nsf.gov/).
It is released under the
[GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)
version 3 or any later version.

## The GMT World Domination

Considering its flexibility at no charge, people worldwide are using GMT in their
work and at home. Most users of GMT are Earth, ocean or planetary scientists, but
there are apparently no limits to the kind of applications that may benefit from
GMT. We know GMT is used in medical research, engineering, physics, mathematics,
social and biological sciences, and by geographers, fisheries institutes, oil
companies, a wide range of government agencies, and last but not least innumerable
hobbyists.

![Map of GMT downloads](http://gmt.soest.hawaii.edu/gmt/map_geoip_all.png)

The map above illustrates the spreading of the current GMT release around the world
based on web traffic. Each colored circle in the map above represents a 15x15 arc
minute block with one or more users who downloaded GMT since the last release. Download geolocation is based
on [MaxMind's](http://www.maxmind.com/) freely available GeoLite data.

## A reminder

If you think it is appropriate, you may consider paying us back by including
our latest EOS article in the reference list of your future publications that
will benefit from the availability of GMT:

> Wessel, P., W. H. F. Smith, R. Scharroo, J. F. Luis, and F. Wobbe (2013),
> Generic Mapping Tools: Improved version released, Eos Trans. AGU, 94(45),
> 409-410, doi:[10.1002/2013EO450001](https://doi.org/10.1002/2013EO450001)

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

## Note to package maintainers

Package maintainers note packaging recommendations at
http://gmt.soest.hawaii.edu/projects/gmt/wiki/PackagingGMT

## Build and runtime prerequisites

- Software:
  You need Ghostscript, CMake (>=2.8.5), netCDF (>=4.0, netCDF-4/HDF5
  support mandatory) and Curl.  Optionally install Sphinx, PCRE1 or PCRE2, GDAL, LAPACK,
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

The development sources are available from GitHub at
https://github.com/GenericMappingTools/gmt.

Extract the files and put them in a separate directory (need not be
where you eventually want to install GMT).

## Building GMT (quick start)

This is just a quick start description. For a more thorough description read more on [Building GMT](BUILDING.md)

Checkout GMT from its GitHub repository:

```
git clone https://github.com/GenericMappingTools/gmt
cd gmt
cp cmake/ConfigUserTemplate.cmake cmake/ConfigUser.cmake
```

Edit *cmake/ConfigUser.cmake* [see comments in the file]. Then:

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j
```
You can add an argument *x* to *-j* (e.g. *-j4*) which means make will use *x*
cores in the build; this depends on the number of cores in your CPU and if
hyperthreading is available or not. By using *-j* without any argument, *make*
will not limit the number of jobs that can run simultaneously.
cmake will build out-of-source in the the directory _build_. 'CMAKE_BUILD_TYPE'
can be one of: empty, Debug, Release, RelWithDebInfo or MinSizeRel

```
make -j install
```

installs a basic gmt in _build/gmt_.

NOTE: All cmake command line options such as _-DCMAKE\_INSTALL\_PREFIX_ can be
configured in *cmake/ConfigUser.cmake*.

### Set path

Make sure users set their PATH to include the directory containing
the GMT executables (BINDIR) if this is not a standard directory
like /usr/local/bin.  You should now be able to run GMT programs.

## GMT supplemental Code

GMT users elsewhere have developed programs that utilize the GMT libraries and
produce PostScript code compatible with the rest of GMT or simply perform data
manipulation. Currently, the supplemental archive include the directories:

  gshhg     - Data extractor for GSHHG shoreline polygons and rivers, borders.
  img       - Data extractor for Smith/Sandwell altimetry grids.
  meca      - Plotting of focal mechanisms, velocity arrows,
              and error ellipses on maps.
  mgd77     - Programs for handling of native MGD77 files.
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
