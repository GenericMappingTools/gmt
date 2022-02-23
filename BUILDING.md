# Building GMT

This document describes how to build GMT from source codes
(stable release or development version) on Linux, FreeBSD, macOS and Windows.

## Contents

- [Build and runtime dependencies](#build-and-runtime-dependencies)
- [Getting GMT source codes](#getting-gmt-source-codes)
- [Configuring](#configuring)
- [Building GMT source codes](#building-gmt-source-codes)
- [Installing](#installing)
- [Setting path](#setting-path)
- [Advanced instructions](#advanced-instructions)

## Build and runtime dependencies

GMT is dependent on some software and libraries to run.
Please refer to the [GMT wiki page](https://github.com/GenericMappingTools/gmt/wiki)
for instructions to install these dependencies on various operation systems.

### Required dependencies

To build GMT, you have to install:

- [CMake](https://cmake.org/) (>=2.8.12)
- [netCDF](https://www.unidata.ucar.edu/software/netcdf/) (>=4.0, netCDF-4/HDF5 support mandatory)
- [curl](https://curl.haxx.se/)
- [GDAL](https://www.gdal.org/) (Ability to read and write numerous grid and image formats)

### Optional dependencies

Optionally install these for more capabilities within GMT:

- [Ghostscript](https://www.ghostscript.com/) (Ability to convert PostScript plots to PDF and rasters)
- [GEOS](https://trac.osgeo.org/geos/) (Ability to buffer lines and polygons)
- [PCRE](https://www.pcre.org/) or PCRE2 (Regular expression support)
- [FFTW](http://www.fftw.org/) single-precision (Fast FFTs, >=3.3 [not needed under macOS])
- [GLib](https://wiki.gnome.org/Projects/GLib) GTHREAD support (>=2.32)
- LAPACK (Fast matrix inversion [not needed under macOS])
- BLAS (Fast matrix multiplications [not needed under macOS])

For movie-making capabilities these executables are needed:

- [GraphicsMagick](http://www.graphicsmagick.org/) (Convert images to animated GIFs)
- [FFmpeg](http://www.ffmpeg.org/) (Convert images to videos)

For viewing documentation under Linux via `gmt docs`, your need `xdg-open`:

- xdg-open (Unified open for a variety of files)

### Development dependencies

Install for building GMT documentation and running tests (not required for general use):

- [Sphinx](http://www.sphinx-doc.org) (>=1.8, for building the documentation)
- [GraphicsMagick](http://www.graphicsmagick.org/) (for running the tests)
- [dvc](https://dvc.org/) (for running the tests and building the documentation)
- [Ninja](https://ninja-build.org/) (optional, build system focused on speed)
- [pngquant](https://pngquant.org/) (optional, for optimizing PNG images in the documentation)

### Required support data

You also need to download support data:

- [GSHHG](https://github.com/GenericMappingTools/gshhg-gmt): A Global Self-consistent, Hierarchical, High-resolution
  Geography Database (>=2.2.0)
- [DCW](https://github.com/GenericMappingTools/dcw-gmt): The Digital Chart of the World (optional, >=2.0.0)

## Getting GMT source codes

The latest stable release of the GMT source codes (filename: gmt-x.x.x-src.tar.gz)
are available from [GMT repository on GitHub](https://github.com/GenericMappingTools/gmt/releases).

If you want to build/use the latest developing/unstable GMT, you can get the source codes by cloning the
[GMT repository on GitHub](https://github.com/GenericMappingTools/gmt). *Here we use `--depth 50` option for a shallow
clone which can reduce the repository size to download.*

    git clone --depth 50 https://github.com/GenericMappingTools/gmt

You can also get supporting data GSHHG and DCW (filename: gshhg-gmt-x.x.x.tar.gz and dcw-gmt-x.x.x.tar.gz)
from the [GMT main site](https://www.generic-mapping-tools.org/download/#support-data).

Extract the files and put them in a separate directory (need not be where you eventually want to install GMT).

> Note for developers: Refer to the [git workflow tutorial](http://www.asmeurer.com/git-workflow/) for more detailed
> instructions on cloning and forking the repository. It is recommended that you use a full clone rather than a shallow
> clone.

## Configuring

GMT can be built on any platform supported by CMake. CMake is a cross-platform,
open-source system for managing the build process. The building process is
controlled by three configuration files in the `cmake` directory:

-   `ConfigDefault.cmake` is version controlled and used to add new default
    variables and set defaults for everyone. **You should NOT edit this file.**
-   `ConfigUser.cmake` is not version controlled and is used to override basic
    default settings on a per-user basis.
-   `ConfigUserAdvanced.cmake` is not version controlled and is used to override
    more advanced default settings on a per-user basis.

GMT provides two template files, `ConfigUserTemplate.cmake` and `ConfigUserAdvancedTemplate.cmake` in the `cmake`
directory. In that directory, you may copy `ConfigUserTemplate.cmake` to `ConfigUser.cmake` and edit to change basic
installation parameters. For more advanced parameters, you may copy `ConfigUserAdvancedTemplate.cmake` to
`ConfigUserAdvanced.cmake` and edit.

> Note for developers: It is necessary to create both `ConfigUser.cmake` and `ConfigUserAdvanced.cmake` in the `cmake`
> directory using the templates provided in order to enable testing. Refer to the section
> [setting up your environment](https://docs.generic-mapping-tools.org/dev/devdocs/contributing.html#setting-up-your-environment) in the
> [contributing guide](https://docs.generic-mapping-tools.org/dev/devdocs/contributing.html)
> for instructions on setting up `cmake/ConfigUserAdvanced.cmake`.

Here is an example of settings you may want to change after copying `cmake/ConfigUserTemplate.cmake` to
`cmake/ConfigUser.cmake`.

```
set (CMAKE_INSTALL_PREFIX "/opt/gmt")
set (GSHHG_ROOT "/path/to/gshhg")
set (DCW_ROOT "/path/to/dcw")
```

For Windows users, a good example is:

```
set (CMAKE_INSTALL_PREFIX "C:/programs/gmt6")
set (GSHHG_ROOT "C:/path/to/gshhg")
set (DCW_ROOT "C:/path/to/dcw")
```

See the additional comments in `cmake/ConfigUserTemplate.cmake` for more details.

Now that you made your configuration choices, it is time for invoking CMake.
To keep generated files separate from source files in the source tree,
you should create a build directory in the top-level directory,
where the build files will be generated, and change into your build directory:

```
mkdir build
cd build
cmake ..
```

For Windows users, you need to open a command prompt and run:

**NOTE:** Commands below are valid only if you have GMT's dependency libraries installed
via vcpkg following [these instructions](https://github.com/GenericMappingTools/gmt/wiki/Install-dependencies-on-Windows-via-vcpkg).

```
mkdir build
cd build
# For x64 build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_GENERATOR_PLATFORM=x64
# For x86 build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_GENERATOR_PLATFORM=x86
```

For advanced users, you can append the option `-G Ninja` to use the
build tool [Ninja](https://ninja-build.org/), which is a small build system
with a focus on speed.


## Building GMT source codes

In the build directory, type

```
# Linux/macOS/FreeBSD
cmake --build .

# Windows
cmake --build . --config Release
```

which will compile all the programs. You can also append ``--parallel [jobs]`` to enable parallel build, in which
``jobs`` is the maximum number of concurrent processes to use when building. If ``jobs`` is omitted the native build
tool's default number is used.

> Note: These instructions build the source code for GMT. Optionally, follow the instructions for
> [building the documentation](https://docs.generic-mapping-tools.org/dev/devdocs/contributing.html#building-the-documentation)
> in the [contributing guide](https://docs.generic-mapping-tools.org/dev/devdocs/contributing.html) to
> build the documentation (for example, to develop the documentation or to use `gmt docs` without the GMT server).

> Note for developers: Refer to the file `admin/bashrc_for_gmt` for useful aliases for configuring and building GMT.

## Installing

```
# Linux/macOS/FreeBSD
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
Using `sudo` will often do the trick.

> Note for developers: Refer to the section
> [Updating the development source codes](https://docs.generic-mapping-tools.org/dev/devdocs/contributing.html#updating-the-development-source-codes) in the
> [contributing guide](https://docs.generic-mapping-tools.org/dev/devdocs/contributing.html) for instructions on how to update the development version of GMT. Also refer to
> the file `admin/bashrc_for_gmt` for useful aliases for updating the development source code.

## Setting path

Make sure you set the `PATH` to include the directory containing the GMT executables
if this is not a standard directory like `/usr/local/bin`.

For Linux/macOS users, open your SHELL configuration file (usually `~/.bashrc`)
and add the line below to it.

```
export PATH=${PATH}:/path/to/gmt/bin
```

Then, you should now be able to run GMT programs.

## Advanced instructions

For advanced users who are interested in building documentation, running tests, or
contributing more to GMT, please refer the [contributing guide](https://docs.generic-mapping-tools.org/dev/devdocs/contributing.html).
