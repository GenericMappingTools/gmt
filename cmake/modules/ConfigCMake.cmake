#
#
# Useful CMake variables.
#
# There are five configuration files:
#
#   1) "ConfigDefault.cmake" - is version controlled and used to add new default
#      variables and set defaults for everyone.
#   2) "ConfigUser.cmake" in the source tree - is not version controlled
#      (currently listed in .gitignore) and used to override basic default settings on
#      a per-user basis.
#   3) "ConfigUser.cmake" in the build tree - is used to override
#      "ConfigUser.cmake" in the source tree.
#   4) "ConfigUserAdvanced.cmake" in the source tree - is not version controlled
#      (currently listed in .gitignore) and used to override advanced default settings on
#      a per-user basis.
#   5) "ConfigUserAdvanced.cmake" in the build tree - is used to override
#      "ConfigUserAdvanced.cmake" in the source tree.
#
# NOTE: If you want to change CMake behaviour just for yourself,
#       copy "ConfigUserTemplate.cmake" to "ConfigUser.cmake" and then edit
#       "ConfigUser.cmake" for basic settings. For advanced settings,
#       copy "ConfigUserAdvancedTemplate.cmake" to "ConfigUserAdvanced.cmake" and edit it.
#       DO NOT EDIT "ConfigDefault.cmake" or the CMake template files.
#
include ("${CMAKE_SOURCE_DIR}/cmake/ConfigDefault.cmake")

# A "ConfigUser.cmake" in the source tree overrides the advanced defaults.
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")
	include ("${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")
endif (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")

# If you've got a 'ConfigUser.cmake' in the build tree then that overrides the
# one in the source tree.
if (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")
	include ("${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")
endif (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")

# A "ConfigUserAdvanced.cmake" in the source tree overrides the advanced defaults.
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUserAdvanced.cmake")
	include ("${CMAKE_SOURCE_DIR}/cmake/ConfigUserAdvanced.cmake")
endif (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUserAdvanced.cmake")

# If you've got a 'ConfigUserAdvanced.cmake' in the build tree then that overrides the
# one in the source tree.
if (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUserAdvanced.cmake")
	include ("${CMAKE_BINARY_DIR}/cmake/ConfigUserAdvanced.cmake")
endif (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUserAdvanced.cmake")

###########################################################
# Do any needed processing of the configuration variables #
###########################################################

# Set default build type to 'Release'
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_BUILD_TYPE)

# Here we change it to add the git commit hash for non-public releases
set (GMT_PACKAGE_VERSION_WITH_GIT_REVISION ${GMT_PACKAGE_VERSION})

# Check if it's a git repository or not
if (EXISTS ${GMT_SOURCE_DIR}/.git)
	set (HAVE_GIT_VERSION TRUE)
endif (EXISTS ${GMT_SOURCE_DIR}/.git)

# Add the last git commit hash and date to the package version if this is a non-public release.
# A non-public release has a FALSE 'GMT_PUBLIC_RELEASE' variable in 'ConfigDefault.cmake'.
if (GIT_FOUND AND HAVE_GIT_VERSION AND NOT GMT_PUBLIC_RELEASE)
	# Get the last git commit hash
	execute_process (
		COMMAND ${GIT_EXECUTABLE} describe --abbrev=7 --always --dirty
		WORKING_DIRECTORY ${GMT_SOURCE_DIR}
		RESULT_VARIABLE GIT_RETURN_CODE
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	if (GIT_RETURN_CODE)
		message (STATUS "Unable to determine git commit hash for non-public release - ignoring.")
	else (GIT_RETURN_CODE)
		if (GIT_COMMIT_HASH)
			# For non-public release, add the last git commit hash and date
			execute_process (
				COMMAND ${GIT_EXECUTABLE} log -1 --date=short --pretty=format:%cd
				WORKING_DIRECTORY ${GMT_SOURCE_DIR}
				RESULT_VARIABLE GIT_DATE_RETURN_CODE
				OUTPUT_VARIABLE GIT_COMMIT_DATE
				OUTPUT_STRIP_TRAILING_WHITESPACE)
			string(REPLACE "-" "." GIT_COMMIT_DATE "${GIT_COMMIT_DATE}")
			set (GMT_PACKAGE_VERSION_WITH_GIT_REVISION "${GMT_PACKAGE_VERSION}_${GIT_COMMIT_HASH}_${GIT_COMMIT_DATE}")
		endif (GIT_COMMIT_HASH)
	endif (GIT_RETURN_CODE)
endif (GIT_FOUND AND HAVE_GIT_VERSION AND NOT GMT_PUBLIC_RELEASE)

# apply license restrictions
if (LICENSE_RESTRICTED) # on
	if (LICENSE_RESTRICTED STREQUAL GPL)
		# restrict to GPL
	elseif (LICENSE_RESTRICTED STREQUAL LGPL)
		# restrict to LGPL
	else (LICENSE_RESTRICTED STREQUAL GPL)
		# unknown license
		message (WARNING "unknown license: ${LICENSE_RESTRICTED}")
	endif (LICENSE_RESTRICTED STREQUAL GPL)
	# restrictions that apply to any of the above licenses
else (LICENSE_RESTRICTED) # off
	# no restrictions at all
endif (LICENSE_RESTRICTED)

# reset list of extra license files
set (GMT_EXTRA_LICENSE_FILES)

# location of GNU license files
set (COPYING_GPL ${GMT_SOURCE_DIR}/COPYINGv3)
set (COPYING_LGPL ${GMT_SOURCE_DIR}/COPYING.LESSERv3)

# GMT paths used in the code
if (NOT GMT_DATADIR)
	# do not reset user setting
	if (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_DATADIR "share")
	else(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_DATADIR "share/gmt${GMT_INSTALL_NAME_SUFFIX}")
	endif(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
endif (NOT GMT_DATADIR)

# Install path GMT_DOCDIR
if (NOT GMT_DOCDIR)
	# do not reset user setting
	if (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_DOCDIR "${GMT_DATADIR}/doc")
	else(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_DOCDIR "share/doc/gmt${GMT_INSTALL_NAME_SUFFIX}")
	endif(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
endif (NOT GMT_DOCDIR)

# Install path GMT_MANDIR
if (NOT GMT_MANDIR)
	# do not reset user setting
	if (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_MANDIR "${GMT_DATADIR}/man")
	else(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_MANDIR "${GMT_DOCDIR}/man")
	endif(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
endif (NOT GMT_MANDIR)

# Install path for GMT binaries, headers and libraries
include (GNUInstallDirs) # defines CMAKE_INSTALL_LIBDIR (lib/lib64)
if (NOT GMT_LIBDIR)
	set (GMT_LIBDIR ${CMAKE_INSTALL_LIBDIR})
endif(NOT GMT_LIBDIR)

if (NOT GMT_BINDIR)
	set (GMT_BINDIR bin)
endif(NOT GMT_BINDIR)

if (NOT GMT_INCLUDEDIR)
	set (GMT_INCLUDEDIR include/gmt${GMT_INSTALL_NAME_SUFFIX})
endif(NOT GMT_INCLUDEDIR)

if (GMT_DATA_URL) # Backwards compatibility with old ConfigUser.cmake files
	set (GMT_DATA_SERVER ${GMT_DATA_URL})
endif (GMT_DATA_URL)

# use, i.e. don't skip the full RPATH for the build tree
set (CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# set the RPATH to be used when installing
if (NOT DEFINED GMT_INSTALL_RELOCATABLE)
	set (GMT_INSTALL_RELOCATABLE FALSE)
endif (NOT DEFINED GMT_INSTALL_RELOCATABLE)
if (GMT_INSTALL_RELOCATABLE)
	# make executables relocatable on supported platforms (relative RPATH)
	if (UNIX AND NOT CYGWIN)
		# find relative libdir from executable dir
		file (RELATIVE_PATH _rpath /${GMT_BINDIR} /${GMT_LIBDIR})
		# remove trailing /
		string (REGEX REPLACE "/$" "" _rpath "${_rpath}")
		if (APPLE)
			# relative RPATH on osx
			# CMP0042: CMake 3.0: MACOSX_RPATH is enabled by default
			set (CMAKE_MACOSX_RPATH ON)
			set (CMAKE_INSTALL_NAME_DIR @rpath)
			set (CMAKE_INSTALL_RPATH "@rpath;@executable_path/${_rpath}")
		else (APPLE)
			# relative RPATH on Linux, Solaris, etc.
			set (CMAKE_INSTALL_RPATH "\$ORIGIN/${_rpath}")
		endif (APPLE)
	endif (UNIX AND NOT CYGWIN)
else (GMT_INSTALL_RELOCATABLE)
	# set absolute RPATH
	if (APPLE)
		# CMP0042: CMake 3.0: MACOSX_RPATH is enabled by default
		set (CMAKE_MACOSX_RPATH OFF)
		set (CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}")
	else (APPLE)
		# the RPATH to be used when installing, but only if it's not a
		# system directory
		list (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
			"${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}" isSystemDir)
		if ("${isSystemDir}" STREQUAL "-1")
			set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}")
		endif ("${isSystemDir}" STREQUAL "-1")
	endif (APPLE)
endif (GMT_INSTALL_RELOCATABLE)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# When running examples/tests with CTest (out-of-source) we need support for
# running GMT programs from within ${GMT_BINARY_DIR}:
if (DO_EXAMPLES OR DO_TESTS AND NOT SUPPORT_EXEC_IN_BINARY_DIR)
	message (WARNING "Enabling SUPPORT_EXEC_IN_BINARY_DIR (required for "
	"testing). Please disable testing on release builds.")
	set (SUPPORT_EXEC_IN_BINARY_DIR ON)
endif (DO_EXAMPLES OR DO_TESTS AND NOT SUPPORT_EXEC_IN_BINARY_DIR)

# Make GNU and Intel C compiler default to C99
if (CMAKE_C_COMPILER_ID MATCHES "(GNU|Intel)" AND NOT CMAKE_C_FLAGS MATCHES "-std=")
	set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
endif ()

# Suppress MSVC deprecation and security warnings
if (MSVC)
    set (CMAKE_C_FLAGS "/D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE ${CMAKE_C_FLAGS}")
    set (CMAKE_C_FLAGS "/D_CRT_NONSTDC_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE ${CMAKE_C_FLAGS}")
endif (MSVC)

# Handle the special developer option GMT_DOCS_DEPEND_ON_GMT
# Normally this is ON.
if (NOT DEFINED GMT_DOCS_DEPEND_ON_GMT)
	set (GMT_DOCS_DEPEND_ON_GMT TRUE)
endif (NOT DEFINED GMT_DOCS_DEPEND_ON_GMT)
if (GMT_DOCS_DEPEND_ON_GMT)
	add_custom_target (gmt_for_img_convert DEPENDS gmt)
else (GMT_DOCS_DEPEND_ON_GMT)
	add_custom_target (gmt_for_img_convert)
endif (GMT_DOCS_DEPEND_ON_GMT)
