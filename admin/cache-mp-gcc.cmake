# Cache settings for building the macOS release with GCC and OpenMP
# This cache file is set for the binary paths of macports
#
SET ( CMAKE_C_COMPILER "/opt/local/bin/gcc-mp-9" CACHE STRING "GNU MP C compiler" )
SET ( CMAKE_CXX_COMPILER "/opt/local/bin/g++-mp-9" CACHE STRING "GNU MP C++ compiler" )
SET ( CMAKE_C_FLAGS -flax-vector-conversions CACHE STRING "C FLAGS")
SET ( CMAKE_C_FLAGS_DEBUG -flax-vector-conversions CACHE STRING "C FLAGS DEBUG")
SET ( CMAKE_C_FLAGS_RELEASE -flax-vector-conversions CACHE STRING "C FLAGS RELEASE")
SET ( OpenMP_C_FLAGS -flax-vector-conversions CACHE STRING "C FLAGS OPENMP")
