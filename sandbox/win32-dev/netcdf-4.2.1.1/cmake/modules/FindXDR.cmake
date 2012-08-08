#
# - Find xdr
# Find the native XDR includes and library
#
#  XDR_INCLUDE_DIRS  - where to find rpc/xdr.h
#  XDR_LIBRARIES     - List of libraries when using xdr.
#  XDR_FOUND         - True if xdr found.
#

include(SelectLibraryConfigurations)
include(FindPackageHandleStandardArgs)

find_path( XDR_INCLUDE_DIRS rpc/xdr.h
  ENV
  XDR_ROOT
  PATH_SUFFIXES include Include
  )
mark_as_advanced( XDR_INCLUDE_DIRS )

find_library ( XDR_LIBRARIES
  NAMES xdr
  ENV XDR_ROOT
  PATH_SUFFIXES lib Lib
  )
mark_as_advanced ( XDR_LIBRARIES )

find_package_handle_standard_args( XDR DEFAULT_MSG
  XDR_LIBRARIES XDR_INCLUDE_DIRS )

