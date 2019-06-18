# Generic Mapping Tools

[![TravisCI](http://img.shields.io/travis/GenericMappingTools/gmt/master.svg?label=TravisCI)](https://travis-ci.org/GenericMappingTools/gmt)
[![Azure](https://dev.azure.com/GenericMappingTools/GMT/_apis/build/status/GenericMappingTools.gmt?branchName=master)](https://dev.azure.com/GenericMappingTools/GMT/_build/latest?definitionId=2&branchName=master)
[![CodeCov](https://img.shields.io/codecov/c/github/GenericMappingTools/gmt.svg)](https://codecov.io/gh/GenericMappingTools/gmt/)
[![Coverity](https://scan.coverity.com/projects/7153/badge.svg)](https://scan.coverity.com/projects/gmt)
[![Documentation (development version)](https://img.shields.io/badge/docs-development-green.svg)](http://docs.generic-mapping-tools.org/dev/)

## What is GMT?

GMT is an open source collection of about 90 command-line tools for manipulating
geographic and Cartesian data sets (including filtering, trend fitting, gridding,
projecting, etc.) and producing PostScript illustrations ranging from simple x–y
plots via contour maps to artificially illuminated surfaces and 3D perspective
views. The GMT supplements add another 50 more specialized and discipline-specific
tools. GMT supports over 30 map projections and transformations and requires
support data such as [GSHHG](http://www.soest.hawaii.edu/pwessel/gshhg/)
coastlines, rivers, and political boundaries and optionally
[DCW](http://www.soest.hawaii.edu/pwessel/dcw) country polygons.

GMT is developed and maintained by [the GMT Team](AUTHORS.md),
with help from a global set of [contributors](AUTHORS.md)
and support by the [National Science Foundation](http://www.nsf.gov/).
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

## A reminder

If you think it is appropriate, you may consider paying us back by including
our latest EOS article in the reference list of your future publications that
will benefit from the availability of GMT:

> Wessel, P., W. H. F. Smith, R. Scharroo, J. F. Luis, and F. Wobbe (2013),
> Generic Mapping Tools: Improved version released, Eos Trans. AGU, 94(45),
> 409-410, doi:[10.1002/2013EO450001](https://doi.org/10.1002/2013EO450001)

## Install GMT

GMT has been installed successfully under UNIX/Linux/OS X on workstations.  It
also installs under Windows and in UNIX emulators such as Cygwin or on virtual
machines.  We anticipate few problems if you are installing the package on
other platforms.

Note there are three GMT tar archives available (#3 is optional):

1. gmt-6.x.x.tar.bz2:          The GMT 6 distribution
2. gshhg-gmt-2.x.x.tar.gz:     All five resolutions of GSHHG coastline data
3. dcw-gmt-1.x.x.tar.bz2:      Digital Chart of the World polygon data

For macOS and Windows users there are separate installers available.
You can obtain GMT and support data from the [GMT main site](https://www.generic-mapping-tools.org).
Alternatively you may get GMT from any of the [FTP sites](MIRRORS.md).
Try the site that is closest to you to minimize transmission times:

Refer to the [install instructions](INSTALL.md) to install GMT,
and [build instructions](BUILDING.md) to build GMT from the sources.

## GMT supplemental Code

GMT users elsewhere have developed programs that utilize the GMT libraries and
produce PostScript code compatible with the rest of GMT or simply perform data
manipulation. Currently, the supplemental archive include these directories:

-  geodesy: Velocity arrows and error ellipses, solid Earth tides, GPS gridding.
-  gshhg: Data extractor for GSHHG shoreline polygons and rivers, borders.
-  img: Data extractor for Smith/Sandwell altimetry grids.
-  mgd77: Programs for handling of native MGD77 files.
-  potential: Geopotential manipulations.
-  segy: Plotting SEGY seismic data sets.
-  seis: Plotting of focal mechanisms and SAC (seismic Analysis Code) data.
-  spotter: Plate tectonic & kinematics applications.
-  x2sys: Track intersection (crossover) tools.

## Misc

Before running programs, there are a few things you should do/know:

    Read carefully the documentation for the gmt system. This can be
    found as both PDF and HTML files in the doc/pdf|html directories.
    The successful operation of gmt-programs depends directly on your
    understanding of how gmt "works", its option lists, I/O, and composite
    plot mechanisms. Then, before running individual gmt programs, read
    the associated man page.

## Software support

You haven't bought anything so you cannot expect full service.  However, if
you find a bug in any of the programs, please report it to us
(https://github.com/GenericMappingTools/gmt) rather than trying to fix it yourself so that
we, and through us, other users may benefit from your find.  Make sure you
provide us with enough information so that we can recreate the problem.

In addition to the bug tracking feature (New Issues) on the website, you
can also post general questions.  Note that GitHub registration
is required to post on the site.

## Ordering the GMT package on CD/DVD-Rs

Should you or someone you know without net-access need to obtain GMT:
Geoware makes and distributes CD/DVD-Rs with the GMT package and many
useful data sets.  For more details and a full description of the data
sets (up to 60 Gb of data!) visit http://www.geoware-online.com/.
