#
# $Id$
#
# Locate Sphinx documentation generator
#
# This module accepts the following environment variables:
#
#    SPHINX_DIR or SPHINX_ROOT - Specify the location of Sphinx
#
# This module defines the following CMake variables:
#
#    SPHINX_FOUND - True if sphinx-build is found
#    SPHINX_EXECUTABLE - A variable pointing to sphinx-build

if (DEFINED SPHINX_ROOT AND NOT SPHINX_ROOT)
  set (SPHINX_EXECUTABLE "" CACHE INTERNAL "")
  return()
endif()

find_program (SPHINX_EXECUTABLE sphinx-build
  HINTS
  ${SPHINX_DIR}
  ${SPHINX_ROOT}
  $ENV{SPHINX_DIR}
  $ENV{SPHINX_ROOT}
  PATH_SUFFIXES bin
  PATHS
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  DOC "Sphinx documentation generator")

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)

mark_as_advanced (SPHINX_EXECUTABLE)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
