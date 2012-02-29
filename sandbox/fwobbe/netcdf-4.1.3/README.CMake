$Id$

== HOWTO build netCDF with CMake and MSVC ==

Probably only works with MSVC. All other compilers untested.

-- Prerequisites --

* Install Microsoft Windows SDK for Windows
* Install CMake (http://www.cmake.org/). Make sure CMake is added to PATH.
* Install Subversion (http://alagazam.net/). Make sure svn is in the PATH.
* Obtain the HDF4 and HDF5 binary releases for your platform (x84/x64) from
  http://www.hdfgroup.org/. Install the *shared* versions, e.g.:
    HDF4.2.7-win764-vs10shared.zip (Windows 64-bit CMake VS 2010, Intel Fortran 12)
    HDF5188-win64-shared.zip (Windows 64-bit CMake VS 2008 C, C++, IVF 11.1)

* Get XDR from svn://gmtserver.soest.hawaii.edu/gmt5/sandbox/fwobbe/bsd-xdr/
* Get cURL from http://curl.haxx.se/download.html

-- Building Prerequisites --

The following examples assume that your build files are located in z:\software and
its subdirectories bin, include, lib, src.

1. Copy HDF4 and HDF5 DLLs to z:\software\bin

The HDF Group files are usually located in c:\Program Files\HDF Group. This is OK but
to avoid setting up complicated PATHs to all the DLLs we simply copy all DLLs from
c:\Program Files\HDF Group\HDF<major-version>\<version>\bin to z:\software\bin.


2. Open Windows SDK Command Prompt and set PATH to our binary dir

  set PATH=%PATH%;z:\software\bin


3. cURL

- extract files in z:\software\src

- cURL versions greater than 7.21.7 have a bug in the CMakeLists.txt. You need to patch the file curl-<version>/CMakeLists.txt:

--------8<-------------------------------------------------------

--- curl-7.24.0/CMakeLists.txt.orig     2012-02-27 11:44:29.000000000 +0100
+++ curl-7.24.0/CMakeLists.txt  2012-02-27 11:44:29.000000000 +0100
@@ -148,9 +148,9 @@
 mark_as_advanced(ENABLE_IPV6)
 
 if(WIN32)
-  find_library(WSOCK32_LIBRARY wsock32)
-  find_library(WS2_32_LIBRARY ws2_32)
-  list_spaces_append_once(CMAKE_C_STANDARD_LIBRARIES ${WSOCK32_LIBRARY} ${WS2_32_LIBRARY})  # bufferoverflowu.lib
+  # Windows standard libraries are located in C:/Program Files/Microsoft SDKs/[...]
+  # They are already included in the default MSVC LIBPATH => no find_library is needed!
+  list_spaces_append_once(CMAKE_C_STANDARD_LIBRARIES wsock32.lib ws2_32.lib)  # bufferoverflowu.lib
   if(CURL_DISABLE_LDAP)
     # Remove wldap32.lib from space-separated list
     string(REPLACE " " ";" _LIST ${CMAKE_C_STANDARD_LIBRARIES})

------------------------------------------------------->8--------

- In the Windows SDK Command Prompt:

  z:
  cd z:\software\src\curl-7.24.0\
  mkdir build
  cd build

- Invoke CMake:
  cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=z:\software -DCMAKE_BUILD_TYPE=Release -DCMAKE_FIND_ROOT_PATH="c:\Program Files\HDF Group\HDF5\1.8.8" -DCMAKE_USE_OPENSSL=OFF ..

  CMAKE_INSTALL_PREFIX points to the installation directory. The headers,
the library and the executables will be installed in the subfolders
include, lib and bin respectively.

  CMAKE_FIND_ROOT_PATH should point to your HDF5 root directory. It contains zlib.

- Build and install cURL:

  nmake
  nmake install

- Move libcurl.sll from z:\software\lib to z:\software\bin

- Type 'curl --version' in the Command Prompt. It should greet you with an
  info message.


4. XDR

- Get the sources:
   cd \software\src
   svn co svn://gmtserver.soest.hawaii.edu/gmt5/sandbox/fwobbe/bsd-xdr bsd-xdr

- Read README.CMake if you like

- In the Windows SDK Command Prompt:

   cd <path-to-xdr-source-dir>
   mkdir build
   cd build
   cmake -G "NMake Makefiles" ..

- If everything went fine you can now build and install XDR:
   nmake
   nmake install

- Check if 'xdrsizeof_test' works in the prompt.



-- Building netCDF --

Once we have installed and build all the library dependencies we can
attempt to deploy netCDF.

- Get the CMake bundled sources of netCDF via subversion from
  svn://gmtserver.soest.hawaii.edu/gmt5/sandbox/fwobbe/netcdf-4.1.3

- Edit cmake/ConfigUser.cmake and set correct paths to HDF, xdr etc.

- Build and install

  cd \software\src\netcdf-4.1.3
  mkdir build
  cd build
  cmake -G "NMake Makefiles" ..
  nmake
  nmake install

- Test if nccopy, ncdump and ncgen work in the prompt

Enjoy!

