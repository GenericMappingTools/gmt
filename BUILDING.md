# Building GMT

This document describes how to build GMT from source codes
(stable release or development version) on Linux, macOS and Windows.

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
  * [Windows](#windows)
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

### Windows

For some software, e.g. CMake, Ghostscript, GraphicsMagick and FFmpeg,
you can download binary installers to install them.
If there is an option to add it to the system PATH, remember to tick it.

For other dependency libraries, it's recommended to install them
via [vcpkg](https://github.com/microsoft/vcpkg).
To use vcpkg, make sure you have met the prerequisites:

- Windows 10, 8.1, 7
- [Visual Studio 2015 Update 3 or newer](https://visualstudio.microsoft.com/)
  with "Desktop development with C++" installed
- [Git](https://git-scm.com/)
- [CMake](https://cmake.org) >=3.12.4

Open a command prompt, and install vcpkg with:

    cd C:\
    git clone https://github.com/microsoft/vcpkg
    cd C:\vcpkg
    .\bootstrap-vcpkg.bat

After installing vcpkg, you can install the GMT dependency libraries with (it may take more than 30 minutes):

    # Build and insatll libraries
    # If you want to build x64 libraries (recommended)
    vcpkg install netcdf-c gdal pcre fftw3 clapack openblas --triplet x64-windows

    # If you want to build x86 libraries
    # NOTE: clapack and openblas currently aren't available for x86-windows.
    vcpkg install netcdf-c gdal pcre fftw3 --triplet x86-windows

    # hook up user-wide integration (note: requires admin on first use)
    vcpkg integrate install

After installing these dependency libraries, you need to add the bin path
(i.e. `C:\vcpkg\installed\x64-windows\bin`) to the system PATH,
to allow executables find the DLL shared libraries.

## Getting GMT source codes

The latest stable release of the GMT source codes (filename: gmt-x.x.x-src.tar.gz)
are available from the [GMT website](https://www.generic-mapping-tools.org) and
[GitHub Release Page](https://github.com/GenericMappingTools/gmt/releases)

If you want to build/use the latest developing/unstable GMT, you can get the source codes from GitHub by:

    git clone https://github.com/GenericMappingTools/gmt

You can also get supporting data GSHHG and DCW (filename: gshhg-gmt-x.x.x.tar.gz and dcw-gmt-x.x.x.tar.gz)
from any of the [GMT FTP sites](MIRRORS.md).

Extract the files and put them in a separate directory (need not be where you eventually want to install GMT).

## Configuring

GMT can be built on any platform supported by CMake. CMake is a cross-platform,
open-source system for managing the build process. The building process is
controlled by two configuration files in the `cmake` directory:

-   *ConfigDefault.cmake*: is version controlled and used to add new default
    variables and set defaults for everyone. **You should NOT edit this file.**
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
set (COPY_GSHHG true)
set (COPY_DCW true)
```

For Windows users, a good example is:

```
set (CMAKE_INSTALL_PREFIX "C:/programs/gmt6")
set (GSHHG_ROOT <path to gshhg>)
set (DCW_ROOT <path to dcw>)
set (COPY_GSHHG true)
set (COPY_DCW true)
set (GMT_INSTALL_MODULE_LINKS FALSE)
set (CMAKE_C_FLAGS "/D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE ${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "/D_CRT_NONSTDC_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE ${CMAKE_C_FLAGS}")
```

See the additional comments in `cmake/ConfigUserTemplate.cmake` for more details.

Now that you made your configuration choices, it is time for invoking CMake.
To keep separately generated files from binary tree and source files from source tree,
you should create a build directory in the top-level directory,
where the build files will be generated, and change into your build directory:

```
mkdir build
cd build
cmake ..
```

For Windows users, you need open a command prompt and run:

```
mkdir build
cd build
# For x64 build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_GENERATOR_PLATFORM=x64
# For x86 build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_GENERATOR_PLATFORM=x86
```

For advanced users, you can append the option ``-G Ninja`` to use the
build tool [Ninja](https://ninja-build.org/), which is a small build system
with a focus on speed.


## Building GMT source codes

In the build directory, type

```
# Linux/macOS
cmake --build .

# Windows
cmake --build . --config Release
```

which will compile all the programs. You can also append ``--parallel [<jobs>]``
to enable parallel build, in which *jobs* is the maximum number of concurrent
processes to use when building. If *jobs* is omitted the native build tool's
default number is used.

## Installing

```
# Linux/macOS
cmake --build . --target install

# Windows
cmake --build . --target install --config Release
```

will install gmt executable, library, development headers and built-in data
to the specified GMT install location.
Optionally it will also install the GSHHG shorelines (if found), DCW (if found),
UNIX manpages, and HTML documentations.

Depending on where GMT is being installed, you might need
write permission for this step so you can copy files to system directories.
Using ``sudo`` will often do the trick.

## Setting path

Make sure you set the PATH to include the directory containing the GMT executables
if this is not a standard directory like `/usr/local/bin`. Then, you should now be able to
run GMT programs.

---

**Below are instructions for developers and advanced users.**

## Building documentation

The GMT documentations are available in different formats and can be generated with:

```
cmake --build . --target docs_man           # UNIX manual pages
cmake --build . --target docs_html          # HTML manual, tutorial, cookbook, and API reference
cmake --build . --target docs_pdf           # PDF tutorial, cookbook, and API reference
cmake --build . --target docs_pdf_shrink    # Like docs_pdf but with reduced size
```

To generate the documentation you need to install the [Sphinx](http://www.sphinx-doc.org/en/master/)
documentation builder, and for PDFs you also need LaTeX. You can choose to install the
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
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)
```

Then run:

```
cmake --build . --target check
```

Optionally set *N_TEST_JOBS* to the number of ctest jobs to run simultaneously.

You can also select individual tests using regexp with ctest, e.g.:

```
ctest --output-on-failure -R ex2[3-6]
```

## Creating source packages

Edit `cmake/ConfigDefault.cmake` and set
*GMT_PACKAGE_VERSION_MAJOR*, *GMT_PACKAGE_VERSION_MINOR*, and
*GMT_PACKAGE_VERSION_PATCH*. Also set *GMT_PUBLIC_RELEASE* to TRUE.
Then create source packages with:

```
cmake --build . --target gmt_release      # export the source tree and documentation
cmake --build . --target gmt_release_tar  # create tarballs (in tar.gz and tar.xz formats)
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
