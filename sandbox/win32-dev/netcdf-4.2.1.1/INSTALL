Documentation for getting and building netCDF
*********************************************

This document is for getting and building the netCDF C library and
utilities, version 4.3.  Other libraries that depend on the netCDF C
library, such as the Fortran and C++ libraries, are available as
separate distributions that can be built and installed after the C
library is successfully installed.  The netCDF-Java library is also a
separate distribution that is currently independent of the netCDF C
library.

Getting NetCDF
**************

The easiest way to get netCDF is through a package management program,
such as rpm, yum, adept, and others. NetCDF is available from many
different repositories, including the default Red Hat and Ubuntu
repositories.

When getting netCDF from a software repository, you will wish to get
the development version of the package ("netcdf-devel"). This includes
the netcdf.h header file.

Unfortunately, you may not be able to get a recent version of netCDF
from a package management system, in which case you must build from
source code. Get the

  ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf.tar.gz

source distribution for the latest, fully-tested release.

Alternatively, you may wish to try the 

  ftp://ftp.unidata.ucar.edu/pub/netcdf/snapshot/netcdf-4-daily.tar.gz

daily snapshot. It is generated nightly at the Unidata Program
Center. It has passed all tests on our (Linux) test machine, but not
necessarily all platform compatibility tests.

Warning: the daily snapshot release contains bug-fixes and new
features added since the last full release. It may also contain
portability bugs.

Once you have downloaded and unpacked the distribution, see the
following section on building.

Building NetCDF
***************

The netCDF-C library and utilities requires 3rd party libraries for
full functionality.

  *  Building with NetCDF-4 and the Remote Data Client
  *  Building NetCDF with Classic Library Only
  *  Building with HDF4 Support
  *  Building with Parallel I/O Support

Building with NetCDF-4 and the Remote Data Client
*************************************************

The usual way of building netCDF requires the HDF5, zlib, and curl
libraries. (And, optionally, the szlib library). Versions required are
at least HDF5 1.8.8, zlib 1.2.5, and curl 7.18.0 or later.
(Optionally, if building with szlib, get szip 2.0 or later.)

HDF5 1.8.9 and zlib 1.2.7 packages are available from the netCDF-4 ftp
site:

  ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4

If you wish to use the remote data client code, then you will also
need libcurl, which can be obtained from the curl website:

  http://curl.haxx.se/download.html

Make sure you run ``make check'' for the HDF5 and zlib
distributions. They are very well-behaved distributions, but sometimes
the build doesn't work (perhaps because of something subtly
misconfigured on the target machine). If one of these libraries is not
working, netCDF will have serious problems.

Note that for building netCDF, it is not necessary to build the HDF5
Fortran, C++, or Java API's.  Only the HDF5 C library is used.

Optionally, you can also build netCDF-4 with the szip library
(a.k.a. szlib). NetCDF cannot create szipped data files, but can read
HDF5 data files that have used szip.

There are license restrictions on the use of szip, see the section on
licensing terms in the web page on szip compression in HDF products:

  http://www.hdfgroup.org/doc_resource/SZIP/

These license restrictions seem to apply to commercial users who are
writing data. (Data readers are not restricted.) But here at NetCDF
World Headquarters, in Sunny Boulder, Colorado, there are no lawyers,
only programmers, so please read the szip documents for the license
agreement to see how it applies to your situation.

If ``make check'' fails for either zlib or HDF5, the problem must be
resolved before the netCDF-4 installation can continue. For HDF5
problems, see the HDF5 help services:

  http://www.hdfgroup.org/services/support.html

Build zlib like this:

  ./configure --prefix=/home/ed/local
  make check install

Then you build HDF5, specifying the location of the zlib library:

  ./configure --with-zlib=/home/ed/local --prefix=/home/ed/local 
  make check install

Note that for shared libraries, you may need to add the install
directory to the LD_LIBRARY_PATH environment variable. See the netCDF
FAQ:

  http://www.unidata.ucar.edu/netcdf/docs/faq.html#Shared%20Libraries

for more details on using shared libraries.

If you are building HDF5 with szip, then include the --with-szlib=
option, with the directory holding the szip library.

After HDF5 is done, build netcdf, specifying the location of the HDF5,
zlib, and (if built into HDF5) the szip header files and libraries in
the CPPFLAGS and LDFLAGS environment variables.  For example:

  CPPFLAGS=-I/home/ed/local/include LDFLAGS=-L/home/ed/local/lib ./configure --prefix=/home/ed/local
  make check install

The configure script will try to find necessary tools in your
path. When you run configure you may optionally use the --prefix
argument to change the default installation directory. The above
examples install the zlib, HDF5, and netCDF-4 libraries in
/home/ed/local/lib, the header file in /home/ed/local/include, and the
utilities in /home/ed/local/bin.  If you don't provide a --prefix
option, installation will be in /usr/local/, in subdirectories lib/,
include/, and bin/.

Building NetCDF with Classic Library Only
*****************************************

It is possible to build the netCDF C libraries and utilities so that
only the netCDF classic and 64-bit offset formats are supported, or
the remote data access client is not built.  (See

  http://www.unidata.ucar.edu/netcdf/docs/netcdf_format.html

for more information about the netCDF format variants.  See the
netCDF-DAP site

  http://opendap.org/netCDF-DAP 

for more information about remote client access to data on OPeNDAP
servers.)

To build without support for the netCDF-4 formats or the additional
netCDF-4 functions, but with remote access, use:

  ./configure --prefix=/home/ed/local --disable-netcdf-4
  make check install

(Replace ``/home/ed/local'' with the name of the directory where
netCDF is to be installed.)

Starting with version 4.1.1 the netCDF C libraries and utilities have
supported remote data access, using the OPeNDAP protocols.  To build 
with full support for netCDF-4 APIs and format but without remote
client access, use:

  ./configure --prefix=/home/ed/local --disable-dap
  make check install

To build without netCDF-4 support or remote client access, use:

  ./configure --prefix=/home/ed/local --disable-netcdf-4 --disable-dap
  make check install

If you get the message that netCDF installed correctly, then you are
done!

Building with HDF4 Support
**************************

The netCDF-4 library can (since version 4.1) read HDF4 data files, if
they were created with the SD (Scientific Data) API. To enable this
feature, use the --enable-hdf4 option. The location for the HDF4
header files and library must be set in the CPPFLAGS and LDFLAGS
options.

For HDF4 access to work, the library must be build with netCDF-4
features.

Building with Parallel I/O Support
**********************************

For parallel I/O to work, HDF5 must be installed with
–enable-parallel, and an MPI library (and related libraries) must be
made available to the HDF5 configure. This can be accomplished with
the mpicc wrapper script, in the case of MPICH2.

The following works to build HDF5 with parallel I/O on our netCDF
testing system:

  CC=mpicc ./configure --enable-parallel --prefix=/shecky/local_par --with-zlib=/shecky/local_par
  make check install

If the HDF5 used by netCDF has been built with parallel I/O, then
netCDF will also be built with support for parallel I/O. This allows
parallel I/O access to netCDF-4/HDF5 files.  (See

  http://www.unidata.ucar.edu/netcdf/docs/netcdf_format.html

for more information about the netCDF format variants.)

If parallel I/O access to netCDF classic and 64-bit offset files is
also needed, the parallel-netcdf library should also be installed,
(and the replacement pnetcdf.h at

  ftp://ftp.unidata.ucar.edu/pub/netcdf/contrib/pnetcdf.h

must be used). Then configure netCDF with the --enable-pnetcdf flag.

Linking to NetCDF
*****************

For static build, to use netCDF-4 you must link to all the libraries,
netCDF, HDF5, zlib, szip (if used with HDF5 build), and curl (if the
remote access client has not been disabled). This will mean -L options
to your build for the locations of the libraries, and -l (lower-case
L) for the names of the libraries.

For example, one user reports that she can build other applications
with netCDF-4 by setting the LIBS environment variable:

  LIBS='-L/X/netcdf-4.0/lib -lnetcdf -L/X/hdf5-1.8.6/lib -lhdf5_hl -lhdf5 -lz -lm -L/X/szip-2.1/lib -lsz'

For shared builds, only -lnetcdf is needed. All other libraries will
be found automatically.

The ``nc-config --all'' command can be used to learn what options are
needed for the local netCDF installation.

For example, this works for linking an application named myapp.c with
netCDF-4 libraries:

    cc -o myapp myapp.c `nc-config --cflags --libs`
