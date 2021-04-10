<a href="https://www.generic-mapping-tools.org/">
    <img src="https://www.generic-mapping-tools.org/_static/gmt-logo.png" alt="GMT logo" title="GMT" align="right" height="120" />
</a>

# Generic Mapping Tools

![Tests](https://github.com/GenericMappingTools/gmt/workflows/Tests/badge.svg)
[![CodeCov](https://img.shields.io/codecov/c/github/GenericMappingTools/gmt.svg)](https://codecov.io/gh/GenericMappingTools/gmt/)
[![Coverity](https://scan.coverity.com/projects/7153/badge.svg)](https://scan.coverity.com/projects/gmt)
[![Documentation (development version)](https://img.shields.io/badge/docs-development-green.svg)](http://docs.generic-mapping-tools.org/dev/)
[![GitHub release](https://img.shields.io/github/release/GenericMappingTools/gmt)](https://github.com/GenericMappingTools/gmt/releases)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.0-4baaaa.svg)](CODE_OF_CONDUCT.md) 

## What is GMT?

GMT is an open source collection of about 90 command-line tools for manipulating
geographic and Cartesian data sets (including filtering, trend fitting, gridding,
projecting, etc.) and producing high-quality illustrations ranging from simple x–y
plots via contour maps to artificially illuminated surfaces and 3D perspective
views. The GMT supplements add another 50 more specialized and discipline-specific
tools. GMT supports over 30 map projections and transformations and requires
support data such as [GSHHG](http://www.soest.hawaii.edu/pwessel/gshhg/)
coastlines, rivers, and political boundaries and optionally
[DCW](http://www.soest.hawaii.edu/pwessel/dcw) country polygons.

GMT is developed and maintained by [the GMT Team](AUTHORS.md),
with help from a global set of [contributors](AUTHORS.md)
and support by the [National Science Foundation](http://www.nsf.gov/).

## The GMT World Domination

Considering its flexibility at no charge, people worldwide are using GMT in their
work and at home. Most users of GMT are Earth, ocean or planetary scientists, but
there are apparently no limits to the kind of applications that may benefit from
GMT. We know GMT is used in medical research, engineering, physics, mathematics,
social and biological sciences, and by geographers, fisheries institutes, oil
companies, a wide range of government agencies, and last but not least innumerable
hobbyists.

## Installation

GMT has been installed successfully under UNIX/Linux/macOS/FreeBSD/OpenBSD on workstations. It
also installs under Windows and in UNIX emulators such as Cygwin or on virtual
machines.  We anticipate few problems if you are installing the package on
other platforms.

Note there are three GMT tar archives available (#3 is optional):

1. gmt-6.x.x.tar.gz:          The GMT 6 distribution
2. gshhg-gmt-2.x.x.tar.gz:    All five resolutions of GSHHG coastline data
3. dcw-gmt-2.x.x.tar.gz:      Digital Chart of the World polygon data

For macOS and Windows users there are separate installers available.
You can obtain GMT and support data from the [GMT main site](https://www.generic-mapping-tools.org).

Refer to the [install instructions](INSTALL.md) to install GMT,
and [build instructions](BUILDING.md) to build GMT from the sources.

## Citation

If you think it is appropriate, you may consider paying us back by including
our latest article in the reference list of your future publications that
will benefit from the availability of GMT:

> Wessel, P., Luis, J. F., Uieda, L., Scharroo, R., Wobbe, F., Smith, W. H. F., & Tian, D. (2019).
> The Generic Mapping Tools version 6. Geochemistry, Geophysics, Geosystems, 20, 5556–5564.
> https://doi.org/10.1029/2019GC008515

## Software support

You haven't bought anything so you cannot expect full service.  However, if
you find a bug in any of the programs, please report it to us by
[opening an issue](https://github.com/GenericMappingTools/gmt/issues/)
rather than trying to fix it yourself so that we, and through us,
other users may benefit from your find.  Make sure you
provide us with enough information so that we can recreate the problem.

For general questions, please post on the
[GMT Community Forum](https://forum.generic-mapping-tools.org/).

## Contributing

Contributions are welcome and appreciated. Please refer to the [contributing guidelines](CONTRIBUTING.md) for more details.

## License

Copyright (c) 1991-2021 by [the GMT Team](AUTHORS.md).

GMT is released under the
[GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)
version 3 or any later version. See [LICENSE.TXT](LICENSE.TXT) for full details.

## Acknowledgment

GMT relies on several other Open Source software libraries, programs and data for its
operation.  We gratefully acknowledge the importance to GMT of these products.
GMT may be linked with these libraries (* means optional):

[Network Common Data Form (netCDF)](https://www.unidata.ucar.edu/software/netcdf/),
[Geospatial Data Abstraction Library (GDAL*)](https://gdal.org),
[Perl Compatible Regular Expressions (PCRE*)](https://www.pcre.org),
[Fastest Fourier Transform in the West (FFTW*)](http://www.fftw.org),
[Linear Algebra Package (LAPACK*)](http://www.netlib.org/lapack/),
[Basic Linear Algebra Subprograms (BLAS*)](http://www.netlib.org/blas/),
[GLIB*](https://developer.gnome.org/glib/), and
[ZLIB*](https://www.zlib.net). GMT may call these executables:
GDAL (ogr2ogr, gdal_translate), [Ghostscript](https://www.ghostscript.com),
[FFmpeg](https://www.ffmpeg.org),
[xdg-open](https://www.freedesktop.org/wiki/Software/xdg-utils/), and
[GraphicsMagick](http://www.graphicsmagick.org).

GMT uses (or can access) data derived from these sources:

- [Scientific Color Maps (CPT)](http://www.fabiocrameri.ch/visualisation.php)
- [Earth 15" DEM](http://dx.doi.org/10.1029/2019EA000658)
- [Earth 1" SRTM DEM](https://lpdaac.usgs.gov/products/srtmgl3v003)
- [Earth 1' crustal age](http://dx.doi.org/10.1029/2020GC009214)
- [Earth 30" Blue Marble images](https://visibleearth.nasa.gov/images/57752/blue-marble-land-surface-shallow-water-and-shaded-topography)
- [Earth 30" Black Marble images](https://earthobservatory.nasa.gov/features/NightLights/page3.php)
