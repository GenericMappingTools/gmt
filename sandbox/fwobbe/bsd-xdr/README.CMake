== HOWTO build XDR with CMake and MSVC ==

Probably only works with MSVC. All other compilers untested.

-- Prerequisites --

Install:

* Microsoft Windows SDK for Windows
* CMake (http://www.cmake.org/). Make sure CMake is added to PATH.

-- Building XDR --

1. Edit cmake/ConfigUser.cmake

CMAKE_INSTALL_PREFIX points to the installation directory. The headers,
the library and the executables will be installed in the subfolders
include, lib and bin respectively.

2. Open Windows SDK Command Prompt

3. cd <path-to-xdr-source-dir>
   mkdir build
   cd build
   cmake ..

4. If everything went fine you can now build and install XDR:
   nmake
   nmake install

Enjoy!

