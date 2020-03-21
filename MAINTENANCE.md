# Maintaining GMT

Below are instructions for developers and advanced users.

## Contents

- [Building documentation](#building-documentation)
- [Running tests](#running-tests)
- [Creating source packages](#creating-source-packages)
- [Packaging](#packaging)
- [Updating the development source codes](#updating-the-development-source-codes)

## Building documentation

To build the GMT documentation you need to install the [Sphinx](http://www.sphinx-doc.org/)
documentation builder. After configuring and building GMT, you can build GMT documentation with:

```
cmake --build . --target docs_depends     # Generate images included in the documentation
cmake --build . --target optimize_images  # Optimize PNG images for documentation [optional]
cmake --build . --target animation        # Generate animations included in the documentation [optional]
cmake --build . --target docs_man         # UNIX manual pages
cmake --build . --target docs_html        # HTML manual, tutorial, cookbook, and API reference
```

Note: [pngquant](https://pngquant.org/) is needed for optimizing images.

## Running tests

GMT ships with more than 800 tests to make sure that any changes won't break
its functionality. To enable testing, you need add following lines
in your `ConfigUserAdvanced.cmake` when configuring GMT:

```
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_ANIMATIONS TRUE)
set (DO_API_TESTS ON)
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)
```

Optionally set *N_TEST_JOBS* to the number of ctest jobs to run simultaneously.

Now you can run all the tests with:

    cmake --build . --target check

You can also run `ctest` commands directly in the build directory.
Below are some common used ctest commands.

1.  Running all tests in 4 parallel jobs:

        ctest -j 4

2.  Re-run all failing tests in previous run:

        ctest -j 4 --rerun-failed

3.  Select individual tests using regexp with ctest:

        ctest --output-on-failure -R ex2[3-6]

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
cpack -G TGZ|TBZ2|Bundle|ZIP|NSIS
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
