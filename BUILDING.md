# Building GMT

## Build and runtime dependencies

To build GMT, you must install:

- [CMake](https://cmake.org/) (>=2.8.5)
- [Ghostscript](https://www.ghostscript.com/)
- [netCDF](https://www.unidata.ucar.edu/software/netcdf/) (>=4.0, netCDF-4/HDF5 support mandatory)
- [curl](https://curl.haxx.se/)

Optionally install for more capabilities within GMT:

- [GDAL](https://www.gdal.org/) (Ability to read and write numerous grid and image formats)
- [PCRE](https://www.pcre.org/) or PCRE2 (Regular expression support)
- [FFTW](http://www.fftw.org/) single-precision (Fast FFTs [not needed under macOS])
- LAPACK (Fast matrix inversion [not needed under macOS])
- BLAS (Fast matrix multiplications [not needed underr macOS])

Optinally install for building GMT documentations and running tests:

- [Sphinx](http://www.sphinx-doc.org) (>=1.4.x, for building the manpage, HTML and PDF documentation)
- TeXLive (for building the PDF documentation)
- [GraphicsMagick](http://www.graphicsmagick.org/) (for running the tests)

You also need download support data:

- gshhg: A Global Self-consistent, Hierarchical, High-resolution Geography Database (>=2.2.2)
- dcw-gmt: The Digital Chart of the World (optional, >=1.0.5)

## Install dependencies

### Ubuntu/Debian

For Ubuntu/Debian, there are prepackaged development binaries available. 
Install the GMT dependencies with:

    # Install required dependencies
    sudo apt-get install build-essential cmake libcurl4-gnutls-dev libnetcdf-dev ghostscript

    # Install optional dependencies
    sudo apt-get install libgdal1-dev libfftw3-dev libpcre3-dev liblapack-dev libblas-dev
    
    # to enable testing
    sudo apt-get install graphicsmagick
    
    # to build the documentation
    sudo apt-get install python-sphinx
    
    # to build the documentation in PDF format
    sudo apt-get install texlive texlive-latex-extra

### RHEL/CentOS/Fedora

GMT's dependencies are available from Extra Packages for Enterprise Linux.
For RHEL and CentOS you can add this repository by telling yum:

    sudo yum install epel-release

You then can install the GMT's dependencies with:

    # Install necessary dependencies
    sudo yum install cmake libcurl-devel netcdf-devel ghostscript

    # Install optional dependencies
    sudo yum install gdal-devel pcre-devel fftw3-devel lapack-devel openblas-devel

    # to enable testing
    sudo yum install GraphicsMagick

    # to build the documentation
    sudo yum install python-sphinx

    # to build the documentation in PDF format
    sudo yum install texlive texlive-latex-extra

### macOS with homebrew

For homebrew users, you can install the dependencies with:

    # Install necessary dependencies
    brew install cmake curl netcdf ghostscript

    # Install optional dependencies
    brew install gdal pcre fftw

    # to enable testing
    brew install graphicsmagick

    # to build the documentation
    brew install sphinx-doc

    # to build the documentation in PDF format
    brew cask install mactex-no-gui

## Get GMT source codes

The latest stable release of the GMT source codes are available from:

- [GMT Download Page](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Download) (filename: gmt-x.x.x-src.tar.gz)
- [GitHub Release Page](https://github.com/GenericMappingTools/gmt/releases)

You can also get supporting data GSHHG and DCW from:

- [GMT Download Page](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Download) (filename: gshhg-gmt-x.x.x.tar.gz and dcw-gmt-x.x.x.tar.gz)
- [GMT FTP site](ftp://ftp.soest.hawaii.edu/gmt)

Extract the files and put them in a separate directory (need not be where you eventually want to install GMT).

If you want to build/use the latest developing/unstable GMT, you can get the source codes from GitHub by:

    git clone https://github.com/GenericMappingTools/gmt

## Configuring

GMT can be build on any platform supported by CMake. CMake is a cross-platform, open-source system for managing the build process. The building process is
controlled by two configuration files in the `cmake` directory:

- "ConfigDefault.cmake": is version controlled and used to add new default
    variables and set defaults for everyone. **You should not edit this file.**
- "ConfigUser.cmake": is not version controlled and used to override defaults
    on a per-user basis.
    There is a template file, ConfigUserTemplate.cmake, that you should copy
    to ConfigUser.cmake and make your changes therein.

In the source tree copy `cmake/ConfigUserTemplate.cmake` to `cmake/ConfigUser.cmake`,
and edit the file according to your demands. This is an example:

```
set (CMAKE_INSTALL_PREFIX /opt/gmt)
set (GSHHG_ROOT <path to gshhg>)
set (DCW_ROOT <path to dcw>)
```

See the additional comments in `cmake/ConfigUserTemplate.cmake` for more details.

Now that you made your configuration choices, it is time for invoking CMake.
Create a subdirectory where the build files will be generated, e.g., in the
source tree `mkdir build` and then `cd build`.

In the build subdirectory, type

```
cmake ..
```

## Building GMT source codes

In the build directory, type

```
make -j
```

which will compile all the programs.

You can add an argument *x* to *-j* (e.g. *-j4*) which means make will 
use *x* cores in the build; this depends on the number of cores in your CPU 
and if hyperthreading is available or not. By using *-j* without any argument, 
make will not limit the number of jobs that can run simultaneously. 
cmake will build out-of-source in the the directory build. 

## Building documentation

The GMT documentations are available in different formats and can be generated with:

```
make -j docs_man            # UNIX manual pages
make -j docs_html           # HTML manual, cookbook, and API reference
make -j docs_pdf            # PDF manual, cookbook, and API reference
make -j docs_pdf_shrink     # Like docs_pdf but with reduced size
```

To generate the documentation you need to install the Sphinx documentation
builder, and for PDFs you also need LaTeX. You can choose to install the
documentation files from an external location instead of generating the
Manpages, PDF, and HTML files from the sources. This is convenient if Sphinx
and/or LaTeX are not available. Set *GMT_INSTALL_EXTERNAL_DOC* in
`cmake/ConfigUser.cmake`.

## Install

```
make -j install
```

will install gmt executable, library, development headers and build-in data 
to the specified GMT install location. 
Optionally it will also install the GSHHG shorelines (if found), DCW (if found), 
UNIX manpages, and HTML and PDF documentation.

## Tests

A complete set of the example scripts used to create all the example plots,
including all necessary data files, are provided by the installation.
To enable testing, you need following lines in your `ConfigUser.cmake`:

```
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_ANIMATIONS TRUE)
```

Then run:

```
make check
```

Optionally set *N_TEST_JOBS* to the number of ctest jobs to run simultaneously.

You can also select individual tests using regexp with ctest, e.g.:

```
ctest -R ex2[3-6]
```

## Updating the developement source codes

Assuming you did not delete the build directory, this is just as simple as

```
cd path-to-gmt
git pull
cd build
make -j install
```

CMake will detect any changes to the source files and will automatically
reconfigure. If you deleted all files inside the build directory you have to
run cmake again manually.

## Packaging

Currently, packaging with CPack works on MacOSX (Bundle, TGZ, TBZ2),
Windows (ZIP, NSIS), and UNIX (TGZ, TBZ2). On Windows you need to install
[NSIS](http://nsis.sourceforge.net/). After building GMT and the documentation run
either one of these:

```
make package
cpack -G <TGZ|TBZ2|Bundle|ZIP|NSIS>
```

## Creating a source package

Set *GMT_RELEASE_PREFIX* in `cmake/ConfigUser.cmake` and run cmake. Then do

```
make -j docs_depends  # optional but increases speed (parallel build)
make gmt_release      # export the source tree and install doc
```

You should then edit ${GMT_RELEASE_PREFIX}/cmake/ConfigDefault.cmake and
set *GMT_PACKAGE_VERSION_MAJOR*, *GMT_PACKAGE_VERSION_MINOR*, and
*GMT_PACKAGE_VERSION_PATCH*. Also uncomment and set
*GMT_SOURCE_CODE_CONTROL_VERSION_STRING* to the current git version. Then
create tarballs with:

```
make -j gmt_release_tar
```
