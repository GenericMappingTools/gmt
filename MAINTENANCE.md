# Maintaining GMT

Below are instructions for developers and advanced users.

## Contents

- [Building documentation](#building-documentation)
- [Running tests](#running-tests)
- [Creating source packages](#creating-source-packages)
- [Packaging](#packaging)
- [Updating the development source codes](#updating-the-development-source-codes)

## Building documentation

The GMT documentations are available in different formats.
To generate the documentation you need to install the [Sphinx](http://www.sphinx-doc.org/)
documentation builder. After building GMT, you can build GMT documentation with:

```
cmake --build . --target docs_man   # UNIX manual pages
cmake --build . --target docs_html  # HTML manual, tutorial, cookbook, and API reference
```

## Running tests

A complete set of the example scripts used to create all the example plots,
including all necessary data files, are provided by the installation.
To enable testing, you need following lines in your `ConfigUserAdvanced.cmake`:

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
[NSIS](http://nsis.sourceforge.net/). After building GMT and the documentation,
build and place the executables, including the supplements, with

```
cmake --build . --target install
```

and then create the package with either one of these:

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
