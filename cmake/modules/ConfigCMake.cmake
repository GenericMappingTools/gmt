#
# $Id$
#
# Useful CMake variables.
#
# There are three configuration files:
#   1) "ConfigDefault.cmake" - is version controlled and used to add new default
#      variables and set defaults for everyone.
#   2) "ConfigUser.cmake" in the source tree - is not version controlled
#      (currently listed in svn:ignore property) and used to override defaults on
#      a per-user basis.
#   3) "ConfigUser.cmake" in the build tree - is used to override
#      "ConfigUser.cmake" in the source tree.
#
# NOTE: If you want to change CMake behaviour just for yourself then copy
#      "ConfigUserTemplate.cmake" to "ConfigUser.cmake" and then edit
#      "ConfigUser.cmake" (not "ConfigDefault.cmake" or "ConfigUserTemplate.cmake").
#
include ("${CMAKE_SOURCE_DIR}/cmake/ConfigDefault.cmake")

# If "ConfigUser.cmake" doesn't exist then create one for convenience.
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")
	include ("${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")
endif (EXISTS "${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake")

# If you've got a 'ConfigUser.cmake' in the build tree then that overrides the
# one in the source tree.
if (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")
	include ("${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")
endif (EXISTS "${CMAKE_BINARY_DIR}/cmake/ConfigUser.cmake")

###########################################################
# Do any needed processing of the configuration variables #
###########################################################

# Build type
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_BUILD_TYPE)

# Here we change it to add the SVN revision number for non-public releases - see Package.cmake for
# why this has to be done here.
set (GMT_PACKAGE_VERSION_WITH_SVN_REVISION ${GMT_PACKAGE_VERSION})
# Add the Subversion version number to the package filename if this is a non-public release.
# A non-public release has an empty 'GMT_SOURCE_CODE_CONTROL_VERSION_STRING' variable in 'ConfigDefault.cmake'.
set (HAVE_SVN_VERSION)
if (NOT GMT_SOURCE_CODE_CONTROL_VERSION_STRING)
	# Get the location, inside the staging area location, to copy the application bundle to.
	execute_process (
		COMMAND svnversion ${GMT_SOURCE_DIR}
		RESULT_VARIABLE SVN_VERSION_RESULT
		OUTPUT_VARIABLE SVN_VERSION_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	if (SVN_VERSION_RESULT)
		message (STATUS "Unable to determine svn version number for non-public release - ignoring.")
	else (SVN_VERSION_RESULT)
		if (SVN_VERSION_OUTPUT MATCHES "Unversioned")
			message (STATUS "Unversioned source tree, non-public release.")
		else (SVN_VERSION_OUTPUT MATCHES "Unversioned")
			# The 'svnversion' command can output a range of revisions with a colon
			# separator - but this causes problems with filenames so we'll remove the
			# colon and the end revision after it.
			string (REGEX REPLACE ":.*$" "" SVN_VERSION ${SVN_VERSION_OUTPUT})
			if (NOT SVN_VERSION STREQUAL exported)
				# Set the updated package version.
				set (GMT_PACKAGE_VERSION_WITH_SVN_REVISION "${GMT_PACKAGE_VERSION}_r${SVN_VERSION}")
				set (HAVE_SVN_VERSION TRUE)
			endif (NOT SVN_VERSION STREQUAL exported)
		endif (SVN_VERSION_OUTPUT MATCHES "Unversioned")
	endif (SVN_VERSION_RESULT)
endif (NOT GMT_SOURCE_CODE_CONTROL_VERSION_STRING)

# The current GMT version.
set (GMT_VERSION_STRING "${GMT_PACKAGE_NAME} ${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}")

set (GMT_LONG_VERSION_STRING "${GMT_PACKAGE_NAME} - ${GMT_PACKAGE_DESCRIPTION_SUMMARY}, Version ${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}")

# Get date
try_run (_exit_today _compiled_today
	${CMAKE_BINARY_DIR}/CMakeTmp
	${CMAKE_MODULE_PATH}/today.c
	CMAKE_FLAGS
	RUN_OUTPUT_VARIABLE _today)

if (NOT _compiled_today OR _exit_today EQUAL -1)
	message (WARNING "Date not implemented, please file a bug report.")
	set(_today "1313;13;13;Undecember")
endif (NOT _compiled_today OR _exit_today EQUAL -1)

list(GET _today 0 YEAR)
list(GET _today 1 MONTH)
list(GET _today 2 DAY)
list(GET _today 3 MONTHNAME)
list(GET _today 0 1 2 DATE)
string (REPLACE ";" "-" DATE "${DATE}")
set (_today)

# set package date
if (NOT GMT_VERSION_YEAR)
	set (GMT_VERSION_YEAR ${YEAR})
endif (NOT GMT_VERSION_YEAR)

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
		set (GMT_DATADIR
			"share/gmt${GMT_INSTALL_NAME_SUFFIX}")
	endif(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
endif (NOT GMT_DATADIR)

# Install path GMT_DOCDIR
if (NOT GMT_DOCDIR)
	# do not reset user setting
	if (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_DOCDIR "${GMT_DATADIR}/doc")
	else(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_DOCDIR
			"share/doc/gmt${GMT_INSTALL_NAME_SUFFIX}")
	endif(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
endif (NOT GMT_DOCDIR)

# Install path GMT_MANDIR
if (NOT GMT_MANDIR)
	# do not reset user setting
	if (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_MANDIR "${GMT_DATADIR}/man")
	else(GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
		set (GMT_MANDIR
			"${GMT_DOCDIR}/man")
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

# use, i.e. don't skip the full RPATH for the build tree
set (CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# set the RPATH to be used when installing
if (NOT DEFINED GMT_INSTALL_RELOCATABLE)
	set (GMT_INSTALL_RELOCATABLE TRUE)
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
			set (CMAKE_INSTALL_NAME_DIR @executable_path/${_rpath})
		else (APPLE)
			# relative RPATH on Linux, Solaris, etc.
			set (CMAKE_INSTALL_RPATH "\$ORIGIN/${_rpath}")
		endif (APPLE)
	endif (UNIX AND NOT CYGWIN)
else (GMT_INSTALL_RELOCATABLE)
	# set absolute RPATH
	if (APPLE)
		set (CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}")
	else (APPLE)
		set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}")
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

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
