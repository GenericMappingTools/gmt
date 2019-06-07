# Building GMT

This document describes how to build GMT from source codes on Linux and macOS.

## Contents

For general users:

- [Build and runtime dependencies](#build-and-runtime-dependencies)
- [Installing dependencies](#installing-dependencies)
  * [Ubuntu/Debian](#ubuntudebian)
  * [RHEL/CentOS](#rhelcentos)
  * [Fedora](#fedora)
  * [Archlinux](#archlinux)
  * [FreeBSD](#freebsd)
  * [macOS with homebrew](#macos-with-homebrew)
- [Getting GMT source codes](#getting-gmt-source-codes)
- [Configuring](#configuring)
- [Building GMT source codes](#building-gmt-source-codes)
- [Installing](#installing)
- [Setting path](#setting-path)

For developers and advanced users:

- [Building documentation](#building-documentation)
- [Running tests](#running-tests)
- [Creating source packages](#creating-source-packages)
- [Packaging](#packaging)
- [Updating the development source codes](#updating-the-development-source-codes)

## Build and runtime dependencies

To build GMT, you must install:

- [CMake](https://cmake.org/) (>=2.8.5)
- [Ghostscript](https://www.ghostscript.com/)
- [netCDF](https://www.unidata.ucar.edu/software/netcdf/) (>=4.0, netCDF-4/HDF5 support mandatory)
- [curl](https://curl.haxx.se/)

Optionally install for more capabilities within GMT:

- [GDAL](https://www.gdal.org/) (Ability to read and write numerous grid and image formats)
- [PCRE](https://www.pcre.org/) or PCRE2 (Regular expression support)
- [FFTW](http://www.fftw.org/) single-precision (Fast FFTs, >=3.3 [not needed under macOS])
- LAPACK (Fast matrix inversion [not needed under macOS])
- BLAS (Fast matrix multiplications [not needed under macOS])

For movie-making capabilities these executables are needed:

- [GraphicsMagick](http://www.graphicsmagick.org/) (Convert images to animated GIFs)
- [FFmpeg](http://www.ffmpeg.org/) (Convert images to videos)

Optionally install for building GMT documentations and running tests:

- [Sphinx](http://www.sphinx-doc.org) (>=1.4.x, for building the manpage, HTML and PDF documentation)
- [TeXLive](https://www.tug.org/texlive/) (for building the PDF documentation)
- [GraphicsMagick](http://www.graphicsmagick.org/) (for running the tests)

You also need download support data:

- [GSHHG](https://www.soest.hawaii.edu/pwessel/gshhg/): A Global Self-consistent, Hierarchical, High-resolution Geography Database (>=2.2.0)
- [DCW-GMT](https://www.soest.hawaii.edu/pwessel/dcw/): The Digital Chart of the World (optional, >=1.0.5)

## Installing dependencies

### Ubuntu/Debian

For Ubuntu and Debian, there are prepackaged development binaries available.
Install the GMT dependencies with:

    # Install required dependencies
    sudo apt-get install build-essential cmake libcurl4-gnutls-dev libnetcdf-dev ghostscript

    # Install optional dependencies
    sudo apt-get install libgdal1-dev libfftw3-dev libpcre3-dev liblapack-dev libblas-dev

    # to enable movie-making
    sudo apt-get install graphicsmagick ffmpeg

    # to enable testing
    sudo apt-get install graphicsmagick

    # to build the documentation
    sudo apt-get install python-sphinx

    # to build the documentation in PDF format
    sudo apt-get install texlive-latex-recommended texlive-fonts-recommended texlive-latex-extra latexmk

### RHEL/CentOS

For RHEL and CentOS, GMT's dependencies are available from Extra Packages for Enterprise Linux.
You can add this repository by telling yum:

    sudo yum install epel-release

You then can install the GMT dependencies with:

    # Install necessary dependencies
    sudo yum install cmake libcurl-devel netcdf-devel ghostscript

    # Install optional dependencies
    sudo yum install gdal-devel pcre-devel fftw3-devel lapack-devel openblas-devel

    # to enable movie-making
    # ffmpeg is provided by [rmpfusion](https://rpmfusion.org/)
    sudo yum localinstall --nogpgcheck https://download1.rpmfusion.org/free/el/rpmfusion-free-release-7.noarch.rpm
    sudo yum install GraphicsMagick ffmpeg

    # to enable testing
    sudo yum install GraphicsMagick

    # to build the documentation
    sudo yum install python-sphinx

    # to build the documentation in PDF format
    sudo yum install python3-sphinx-latex

### Fedora

For Fedora, there are prepackaged development binaries available.
Install the GMT dependencies with:

    # Install necessary dependencies
    sudo dnf install cmake libcurl-devel netcdf-devel ghostscript

    # Install optional dependencies
    sudo dnf install gdal-devel pcre-devel fftw3-devel lapack-devel openblas-devel

    # to enable movie-making
    # ffmpeg is provided by [rmpfusion](https://rpmfusion.org/)
    sudo dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
    sudo dnf install GraphicsMagick ffmpeg

    # to enable testing
    sudo dnf install GraphicsMagick

    # to build the documentation
    sudo dnf install python-sphinx

    # to build the documentation in PDF format
    sudo dnf install python-sphinx-latex

### Archlinux

For Archlinux, there are prepackaged development binaries available.
Install the gmt dependencies with:

    # install necessary dependencies
    sudo pacman -S base-devel cmake libcurl-gnutls netcdf ghostscript

    # install optional dependencies
    sudo pacman -S gdal pcre fftw lapack openblas

    # to enable movie-making
    sudo pacman -S graphicsmagick ffmpeg

    # to enable testing
    sudo pacman -S graphicsmagick

    # to build the documentation
    sudo pacman -S python-sphinx

### FreeBSD

For FreeBSD, there are prepackaged development binaries available.
Install the gmt dependencies with:

    # install necessary dependencies
    sudo pkg install shells/bash devel/cmake ftp/curl science/netcdf print/ghostscript9

    # install optional dependencies
    sudo pkg install graphics/gdal devel/pcre math/fftw3-float math/lapack math/openblas

    # to enable movie-making
    sudo pkg install graphics/GraphicsMagick multimedia/ffmpeg

    # to enable testing
    sudo pkg install graphics/GraphicsMagick

    # to build the documentation
    sudo pkg install py36-sphinx

### macOS with homebrew

For macOS with [homebrew](https://brew.sh/) installed, you can install the dependencies with:

    # Install necessary dependencies
    brew install cmake curl netcdf ghostscript

    # Install optional dependencies
    brew install gdal pcre fftw

    # to enable movie-making
    brew install graphicsmagick ffmpeg

    # to enable testing
    brew install graphicsmagick

    # to build the documentation
    brew install sphinx-doc

    # to build the documentation in PDF format
    brew cask install mactex-no-gui

## Getting GMT source codes

The latest stable release of the GMT source codes (filename: gmt-x.x.x-src.tar.gz)
are available from:

- [GMT Download Page](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Download)
- [GitHub Release Page](https://github.com/GenericMappingTools/gmt/releases)

If you want to build/use the latest developing/unstable GMT, you can get the source codes from GitHub by:

    git clone https://github.com/GenericMappingTools/gmt

You can also get supporting data GSHHG and DCW (filename: gshhg-gmt-x.x.x.tar.gz and dcw-gmt-x.x.x.tar.gz) from:

- [GMT Download Page](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Download)
- [GMT FTP site](ftp://ftp.soest.hawaii.edu/gmt)

Extract the files and put them in a separate directory (need not be where you eventually want to install GMT).

## Configuring

GMT can be built on any platform supported by CMake. CMake is a cross-platform,
open-source system for managing the build process. The building process is
controlled by two configuration files in the `cmake` directory:

-   *ConfigDefault.cmake*: is version controlled and used to add new default
    variables and set defaults for everyone. **You should not edit this file.**
-   *ConfigUser.cmake*: is not version controlled and used to override defaults
    on a per-user basis.
    There is a template file, *ConfigUserTemplate.cmake*, that you should copy
    to *ConfigUser.cmake* and make your changes therein.

In the source tree, copy the template configuration file
`cmake/ConfigUserTemplate.cmake` to `cmake/ConfigUser.cmake`,
and edit the file according to your demands. This is an example:

```
set (CMAKE_INSTALL_PREFIX /opt/gmt)
set (GSHHG_ROOT /path/to/gshhg)
set (DCW_ROOT /path/to/dcw)
```

See the additional comments in `cmake/ConfigUserTemplate.cmake` for more details.

Now that you made your configuration choices, it is time for invoking CMake.
Create a subdirectory where the build files will be generated, e.g., in the
source tree `mkdir build` and then `cd build`.

In the build subdirectory, type

```
cmake ..
```

For advanced users, you can append the option ``-G Ninja`` to use the
build tool [Ninja](https://ninja-build.org/), which is a small build system
with a focus on speed.

## Building GMT source codes

In the build directory, type

```
cmake --build .
```

which will compile all the programs. You can also append ``--parallel [<jobs>]``
to enable parallel build, in which *jobs* is the maximum number of concurrent
processes to use when building. If *jobs* is omitted the native build tool's
default number is used.


## Installing

```
cmake --build . --target install
```

will install gmt executable, library, development headers and built-in data
to the specified GMT install location.
Optionally it will also install the GSHHG shorelines (if found), DCW (if found),
UNIX manpages, and HTML and PDF documentation.

Depending on where GMT is being installed, you might need
write permission for this step so you can copy files to system directories.
Using ``sudo`` will often do the trick.

## Setting path

Make sure you set the PATH to include the directory containing the GMT executables (BINDIR)
if this is not a standard directory like /usr/local/bin. Then, you should now be able to
run GMT programs.

---

**Below are instructions for developers and advanced users.**

## Building documentation

The GMT documentations are available in different formats and can be generated with:

```
cmake --build . --target docs_man           # UNIX manual pages
cmake --build . --target docs_html          # HTML manual, cookbook, and API reference
cmake --build . --target docs_pdf           # PDF manual, cookbook, and API reference
cmake --build . --target docs_pdf_shrink    # Like docs_pdf but with reduced size
```

To generate the documentation you need to install the Sphinx documentation
builder, and for PDFs you also need LaTeX. You can choose to install the
documentation files from an external location instead of generating the
Manpages, PDF, and HTML files from the sources. This is convenient if Sphinx
and/or LaTeX are not available. Set *GMT_INSTALL_EXTERNAL_DOC* in
`cmake/ConfigUser.cmake`.


## Running tests

A complete set of the example scripts used to create all the example plots,
including all necessary data files, are provided by the installation.
To enable testing, you need following lines in your `ConfigUser.cmake`:

```
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_ANIMATIONS TRUE)
set (DO_API_TESTS ON)
```

Then run:

```
cmake --build . --target check
```

Optionally set *N_TEST_JOBS* to the number of ctest jobs to run simultaneously.

You can also select individual tests using regexp with ctest, e.g.:

```
ctest -R ex2[3-6]
```

## Creating source packages

Edit `cmake/ConfigDefault.cmake` and set
*GMT_PACKAGE_VERSION_MAJOR*, *GMT_PACKAGE_VERSION_MINOR*, and
*GMT_PACKAGE_VERSION_PATCH*. Also uncomment and set
*GMT_SOURCE_CODE_CONTROL_VERSION_STRING* to the current git commit hash.
Then create source packages with:

```
cmake --build . --target gmt_release      # export the source tree and install doc
cmake --build . --target gmt_release_tar
```

## Packaging

Currently, packaging with CPack works on macOS (Bundle, TGZ, TBZ2),
Windows (ZIP, NSIS), and UNIX (TGZ, TBZ2). On Windows you need to install
[NSIS](http://nsis.sourceforge.net/). After building GMT and the documentation run
either one of these:

```
cmake --build . --target package
cpack -G <TGZ|TBZ2|Bundle|ZIP|NSIS>
```

## Updating the development source codes

Assuming you did not delete the build directory, this is just as simple as

```
cd path-to-gmt
git pull
cd build
cmake --build .
cmake --build . --target install
```

CMake will detect any changes to the source files and will automatically
reconfigure. If you deleted all files inside the build directory you have to
run cmake again manually.
