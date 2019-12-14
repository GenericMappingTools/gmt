.. index:: ! gmt-config

**********
gmt-config
**********

.. only:: not man

    Get information about the gmt installation

Synopsis
--------

**gmt-config** [ *options* ]

Description
-----------

**gmt-config** is used by developers who need to know technical information
about the current GMT installation, such as what include files to supply,
the path to the library, or if it was built with GDAL support, and more.

Required Arguments
------------------

None.

Optional Arguments
------------------

**--help**
    Display this help message and exit.

**--all**
    Display all options.

**--bits**
    Whether library is build 32-bit or 64-bit.

**--cc**
    The C compiler used.

**--cflags**
    The pre-processor and compiler flags used.

**--datadir**
    GMT's data directories.

**--dataserver**
    The URL of the remote GMT data server.

**--dcw**
    The location of the DCW database.

**--dep-libs**
    The dependent libraries.

**--gshhg**
    The location of the GSHHG database.

**--has-fftw**
    Whether FFTW is used in build.

**--has-gdal**
    Whether GDAL is used in build.

**--has-pcre**
    Whether PCRE is used in build.

**--has-lapack**
    Whether LAPACK is used in build.

**--has-openmp**
    Whether GMT was built with OpenMP support.

**--includedir**
    The include directory.

**--libdir**
    The library directory.

**--libs**
    The library linking information.

**--plugindir**
    GMT's plugin directory.

**--prefix**
    The install prefix.

**--version**
    The library version

Examples
--------

To Determine the compiler flags that is required for an external to to find the GMT function prototypes, try::

    gmt-config --cflags

See Also
--------

:doc:`gmt`
